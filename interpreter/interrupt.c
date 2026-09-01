/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//
// Created by Adrian Sutherland on 05/04/2025.
//


/* TODO - We need to redefine CREXX Signals for external os events as follows:

    // --- User/System Initiated Interruption & Termination ---

    // CREXX_EVENT_INTERRUPT:
    //   Meaning: A polite request from the user (e.g., Ctrl+C) to stop current activity
    //            and/or initiate a graceful shutdown. The application should attempt
    //            to clean up and exit, but can choose to ignore or finish current tasks.
    //   OS Mapping: Windows: CTRL_C_EVENT
    //               POSIX:   SIGINT
    CREXX_EVENT_INTERRUPT,

    // CREXX_EVENT_BREAK:
    //   Meaning: A more insistent request from the user (e.g., Ctrl+Break) to halt
    //            execution and/or initiate an immediate graceful shutdown. This is
    //            often used if the application is unresponsive to an INTERRUPT.
    //            The application should prioritize stopping quickly over completing
    //            non-critical tasks.
    //   OS Mapping: Windows: CTRL_BREAK_EVENT
    //               POSIX:   SIGQUIT (Note: POSIX SIGQUIT often implies a core dump,
    //                        which this event does not require or imply on CREXX/Windows.)
    CREXX_EVENT_BREAK,

    // CREXX_EVENT_SHUTDOWN:
    //   Meaning: A critical request from the OS or a control system to perform a
    //            graceful shutdown. This is typically due to the user closing the console
    //            window, logging off, or the entire system shutting down/rebooting.
    //            The application MUST perform all necessary cleanup (saving data,
    //            flushing logs, releasing resources) and exit within a short grace period
    //            to avoid forceful termination by the OS.
    //   OS Mapping: Windows: CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
    //               POSIX:   SIGTERM (typically sent programmatically, not via console UI)
    CREXX_EVENT_SHUTDOWN,

    // --- Application Control & Management ---

    // CREXX_EVENT_RELOAD_CONFIG:
    //   Meaning: A request to reload the application's configuration settings from
    //            its source (e.g., a file) without restarting the application.
    //            The application should attempt to apply new settings safely and
    //            continue operation.
    //   OS Mapping: Windows: Custom IPC (e.g., RESTful API, Named Pipe, UDP listener)
    //               POSIX:   SIGHUP (common convention for daemons to reload config)
    CREXX_EVENT_RELOAD_CONFIG,
*/


#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* pthread_getattr_np() */
#endif
#define _POSIX_C_SOURCE 200809L /* Request POSIX features */
#include <signal.h>
#include <stddef.h>  /* For NULL */
#include <stdint.h>  /* For uintptr_t */
#include <stdlib.h>  /* For abort */
#include <string.h>  /* For memset */

/* Include VM signal definitions */
#include "rxvmintp.h" /* For RXSIGNAL_* codes, and set / clear interrupt functions */

#define RXVM_THREAD_DOORBELL_E5_MAX_WORKERS 64u

#ifdef _WIN32
    #include <windows.h> /* For platform specifics if needed later */
    /* Windows doesn't have some POSIX signals */
    #define SIGHUP  -1 /* Not applicable */
    #define SIGPIPE -1 /* Not applicable */
    #define SIGUSR1 -1 /* Not applicable */
    #define SIGUSR2 -1 /* Not applicable */
    #define SIGCHLD -1 /* Not applicable */
    #define MAX_OS_SIGNALS 32 /* Max OS signal number we might store */
#else
    #include <pthread.h>
    #include <unistd.h>  /* POSIX specifics */
#if defined(__APPLE__) || defined(__linux__)
    #define CREXX_POSIX_THREAD_DOORBELL_E5 1
#endif

#if defined(__APPLE__)
    /* _POSIX_C_SOURCE intentionally hides these Darwin worker-stack queries. */
    size_t pthread_get_stacksize_np(pthread_t);
    void *pthread_get_stackaddr_np(pthread_t);
#endif
    /* Define MAX_OS_SIGNALS for POSIX based on common values like NSIG */
    /* NSIG itself isn't guaranteed, use a reasonable upper bound */
    #define MAX_OS_SIGNALS 128
#endif

/* --- Global Variables --- */

/* Storage for original signal dispositions */
#ifdef _WIN32
    /* Type for the signal handler function */
    typedef void (*OsSignalHandlerFunc)(int);
    static OsSignalHandlerFunc g_original_os_handlers[MAX_OS_SIGNALS];
#else
    static struct sigaction g_original_os_actions[MAX_OS_SIGNALS];
#endif

/* Tracks which VM signals have our handler active (indexed by RXSIGNAL_* code) */
/* 0=inactive, 1=active handler, -1=set to ignore by us */
static volatile sig_atomic_t g_handler_active[RXSIGNAL_MAX];
static rxvm_context *g_process_main_context;
static volatile sig_atomic_t *g_process_main_interrupts;
static size_t g_signal_users;

#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
/*
 * PERF3-13 E5 POSIX native doorbell. SIGURG is not mapped to a public VM
 * signal. Each worker registers its immutable stack range while SIGURG is
 * blocked. During run(), the slot points at the existing execution-local
 * pending word and SIGURG is unblocked. The handler identifies its target by
 * the current stack address and performs only the existing sig_atomic_t OR.
 *
 * TLS is deliberately excluded from this signal-handler path: Apple Clang's
 * Mach-O form called the TLV resolver, and the Linux proof must likewise keep
 * all libc/runtime traversal outside the handler.
 */
#define RXVM_THREAD_DOORBELL_E5_SIGNAL SIGURG
typedef struct rxvm_thread_doorbell_e5_slot {
    volatile uint32_t active;
    pthread_t thread;
    uintptr_t stack_low;
    uintptr_t stack_high;
    volatile sig_atomic_t *pending;
} rxvm_thread_doorbell_e5_slot;

static rxvm_thread_doorbell_e5_slot
        g_thread_doorbell_slots[RXVM_THREAD_DOORBELL_E5_MAX_WORKERS];
static struct sigaction g_thread_doorbell_previous_action;
static size_t g_thread_doorbell_users;

#if defined(__linux__) && defined(__GCC_ATOMIC_INT_LOCK_FREE)
typedef char crexx_thread_doorbell_active_must_be_lock_free[
        (sizeof(uint32_t) == sizeof(unsigned int) &&
         __GCC_ATOMIC_INT_LOCK_FREE == 2) ? 1 : -1];
#else
typedef char crexx_thread_doorbell_active_must_be_lock_free[
        __atomic_always_lock_free(sizeof(uint32_t), 0) ? 1 : -1];
#endif

#if defined(__GNUC__) || defined(__clang__)
/* Keep the stack-range probe on the real pthread stack and keep sanitizer
 * runtime calls out of this async-signal-safe path. */
#define RXVM_THREAD_DOORBELL_E5_HANDLER_ATTRIBUTES \
        __attribute__((no_stack_protector, no_sanitize_address))
#else
#define RXVM_THREAD_DOORBELL_E5_HANDLER_ATTRIBUTES
#endif

static void RXVM_THREAD_DOORBELL_E5_HANDLER_ATTRIBUTES
rxvm_thread_doorbell_e5_handler(int os_signal) {
    volatile unsigned char stack_marker = 0u;
    const uintptr_t stack_address = (uintptr_t)&stack_marker;
    size_t index;

    if (os_signal != RXVM_THREAD_DOORBELL_E5_SIGNAL) return;
    for (index = 0u; index < RXVM_THREAD_DOORBELL_E5_MAX_WORKERS; index++) {
        rxvm_thread_doorbell_e5_slot *slot =
                &g_thread_doorbell_slots[index];
        volatile sig_atomic_t *pending;

        if (!__atomic_load_n(&slot->active, __ATOMIC_ACQUIRE) ||
            stack_address < slot->stack_low ||
            stack_address >= slot->stack_high) {
            continue;
        }
        pending = slot->pending;
        if (pending) {
            *pending |= rxsignal_mask(RXSIGNAL_CANCEL);
        }
        return;
    }
}

#undef RXVM_THREAD_DOORBELL_E5_HANDLER_ATTRIBUTES

static rxvm_thread_doorbell_e5_slot *
rxvm_thread_doorbell_e5_find_current(void) {
    const pthread_t self = pthread_self();
    size_t index;

    for (index = 0u; index < RXVM_THREAD_DOORBELL_E5_MAX_WORKERS; index++) {
        rxvm_thread_doorbell_e5_slot *slot =
                &g_thread_doorbell_slots[index];
        if (__atomic_load_n(&slot->active, __ATOMIC_ACQUIRE) &&
            pthread_equal(slot->thread, self)) {
            return slot;
        }
    }
    return 0;
}

static void rxvm_thread_doorbell_e5_mask(int how) {
    sigset_t doorbell;

    sigemptyset(&doorbell);
    sigaddset(&doorbell, RXVM_THREAD_DOORBELL_E5_SIGNAL);
    if (pthread_sigmask(how, &doorbell, 0) != 0) abort();
}

static int rxvm_thread_doorbell_e5_stack_range(
        pthread_t self,
        uintptr_t *stack_low,
        uintptr_t *stack_high) {
#if defined(__APPLE__)
    const uintptr_t high = (uintptr_t)pthread_get_stackaddr_np(self);
    const size_t size = pthread_get_stacksize_np(self);

    if (!high || !size || size > high) return -1;
    *stack_low = high - size;
    *stack_high = high;
    return 0;
#elif defined(__linux__)
    pthread_attr_t attributes;
    void *base = 0;
    size_t size = 0u;
    uintptr_t low;
    int rc;

    if (pthread_getattr_np(self, &attributes) != 0) return -1;
    rc = pthread_attr_getstack(&attributes, &base, &size);
    if (pthread_attr_destroy(&attributes) != 0) rc = -1;
    low = (uintptr_t)base;
    if (rc != 0 || !low || !size || size > UINTPTR_MAX - low) return -1;
    *stack_low = low;
    *stack_high = low + size;
    return 0;
#else
    (void)self;
    (void)stack_low;
    (void)stack_high;
    return -1;
#endif
}
#endif

#if defined(_WIN32)
/*
 * Windows 11 special-user-APC counterpart to the POSIX E5 doorbell.  The API
 * is resolved at runtime so this private carrier neither adds a QueueUserAPC2
 * import nor prevents the rest of cREXX from starting on older Windows.
 *
 * APC data points only at this process-static slot array.  A worker publishes
 * its run()-local pending word while executing and clears it before the word
 * leaves scope.  One outstanding APC is coalesced per worker; discard waits
 * for that callback to retire before the slot can be reused by another run.
 */
#define RXVM_QUEUE_USER_APC_FLAGS_SPECIAL_USER_APC 0x1u
#define RXVM_CANCEL_MASK ((LONG)(1u << (RXSIGNAL_CANCEL - 1)))

typedef BOOL (WINAPI *rxvm_queue_user_apc2_fn)(
        PAPCFUNC callback,
        HANDLE thread,
        ULONG_PTR data,
        ULONG flags);

typedef struct rxvm_thread_doorbell_e5_slot {
    volatile LONG active;
    DWORD thread_id;
    PVOID volatile pending;
    volatile LONG deferred;
    volatile LONG queued;
} rxvm_thread_doorbell_e5_slot;

static rxvm_thread_doorbell_e5_slot
        g_thread_doorbell_slots[RXVM_THREAD_DOORBELL_E5_MAX_WORKERS];
static rxvm_queue_user_apc2_fn g_queue_user_apc2;
static size_t g_thread_doorbell_users;
static volatile LONG g_thread_doorbell_callback_count;
static volatile LONG g_thread_doorbell_callback_depth;
static volatile LONG g_thread_doorbell_callback_max_depth;

typedef char crexx_thread_doorbell_pending_must_match_long[
        sizeof(sig_atomic_t) == sizeof(LONG) ? 1 : -1];

static VOID CALLBACK rxvm_thread_doorbell_e5_callback(ULONG_PTR data) {
    rxvm_thread_doorbell_e5_slot *slot =
            (rxvm_thread_doorbell_e5_slot *)data;
    volatile LONG *pending;
    LONG depth;
    LONG observed;

    depth = InterlockedIncrement(&g_thread_doorbell_callback_depth);
    observed = g_thread_doorbell_callback_max_depth;
    while (observed < depth) {
        LONG prior = InterlockedCompareExchange(
                &g_thread_doorbell_callback_max_depth, depth, observed);
        if (prior == observed) break;
        observed = prior;
    }
    pending = (volatile LONG *)InterlockedCompareExchangePointer(
            &slot->pending, NULL, NULL);
    if (InterlockedCompareExchange(&slot->active, 0, 0)) {
        if (pending) {
            InterlockedOr(pending, RXVM_CANCEL_MASK);
        } else {
            /* RUNNING is visible just before rxvm_call() publishes its local
             * word. Publish a deferred bit, then recheck so either this APC
             * or enter_execution() must observe the handoff. */
            InterlockedExchange(&slot->deferred, 1);
            pending = (volatile LONG *)InterlockedCompareExchangePointer(
                    &slot->pending, NULL, NULL);
            if (pending && InterlockedExchange(&slot->deferred, 0)) {
                InterlockedOr(pending, RXVM_CANCEL_MASK);
            }
        }
    }
    InterlockedIncrement(&g_thread_doorbell_callback_count);
    InterlockedExchange(&slot->queued, 0);
    InterlockedDecrement(&g_thread_doorbell_callback_depth);
}

static rxvm_thread_doorbell_e5_slot *
rxvm_thread_doorbell_e5_find_current(void) {
    const DWORD self = GetCurrentThreadId();
    size_t index;

    for (index = 0u; index < RXVM_THREAD_DOORBELL_E5_MAX_WORKERS; index++) {
        rxvm_thread_doorbell_e5_slot *slot =
                &g_thread_doorbell_slots[index];
        if (InterlockedCompareExchange(&slot->active, 0, 0) &&
            slot->thread_id == self) {
            return slot;
        }
    }
    return NULL;
}

static void rxvm_thread_doorbell_e5_wait_idle(
        rxvm_thread_doorbell_e5_slot *slot) {
    while (InterlockedCompareExchange(&slot->queued, 0, 0)) {
        SwitchToThread();
    }
}
#endif

#ifdef _WIN32
static SRWLOCK g_signal_lifecycle_lock = SRWLOCK_INIT;
#define SIGNAL_LIFECYCLE_LOCK() AcquireSRWLockExclusive(&g_signal_lifecycle_lock)
#define SIGNAL_LIFECYCLE_UNLOCK() ReleaseSRWLockExclusive(&g_signal_lifecycle_lock)
#else
static pthread_mutex_t g_signal_lifecycle_lock = PTHREAD_MUTEX_INITIALIZER;
#define SIGNAL_LIFECYCLE_LOCK() ((void)pthread_mutex_lock(&g_signal_lifecycle_lock))
#define SIGNAL_LIFECYCLE_UNLOCK() ((void)pthread_mutex_unlock(&g_signal_lifecycle_lock))
#endif

int rxvm_signal_bind_process_main(rxvm_context *context) {
    int rc = 0;

    if (!context) return -1;
    SIGNAL_LIFECYCLE_LOCK();
    if (g_process_main_context && g_process_main_context != context) {
        rc = -1;
    } else {
        g_process_main_context = context;
    }
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
}

int rxvm_signal_enter_execution(
        rxvm_context *context,
        volatile sig_atomic_t *pending_interrupts,
        volatile sig_atomic_t **previous_pending_interrupts) {
    volatile sig_atomic_t *previous;
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    rxvm_thread_doorbell_e5_slot *doorbell_slot;
#elif defined(_WIN32)
    rxvm_thread_doorbell_e5_slot *doorbell_slot;
#endif

    if (!context || !pending_interrupts || !previous_pending_interrupts)
        return -1;
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    doorbell_slot = rxvm_thread_doorbell_e5_find_current();
    if (doorbell_slot) rxvm_thread_doorbell_e5_mask(SIG_BLOCK);
#elif defined(_WIN32)
    doorbell_slot = rxvm_thread_doorbell_e5_find_current();
#endif
    SIGNAL_LIFECYCLE_LOCK();
    previous = context->active.pending_interrupts;
    *pending_interrupts = previous ? *previous : 0;
    if (previous) *previous = 0;
    context->active.pending_interrupts = pending_interrupts;
    if (g_process_main_context == context)
        g_process_main_interrupts = pending_interrupts;
    *previous_pending_interrupts = previous;
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    if (doorbell_slot) doorbell_slot->pending = pending_interrupts;
#elif defined(_WIN32)
    if (doorbell_slot) {
        InterlockedExchangePointer(&doorbell_slot->pending,
                                   (PVOID)pending_interrupts);
        if (InterlockedExchange(&doorbell_slot->deferred, 0)) {
            InterlockedOr((volatile LONG *)pending_interrupts,
                          RXVM_CANCEL_MASK);
        }
    }
#endif
    SIGNAL_LIFECYCLE_UNLOCK();
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    if (doorbell_slot) rxvm_thread_doorbell_e5_mask(SIG_UNBLOCK);
#endif
    return 0;
}

int rxvm_signal_leave_execution(
        rxvm_context *context,
        volatile sig_atomic_t *pending_interrupts,
        volatile sig_atomic_t *previous_pending_interrupts) {
    int rc = 0;
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    rxvm_thread_doorbell_e5_slot *doorbell_slot =
            rxvm_thread_doorbell_e5_find_current();
#elif defined(_WIN32)
    rxvm_thread_doorbell_e5_slot *doorbell_slot =
            rxvm_thread_doorbell_e5_find_current();
#endif

    if (!context || !pending_interrupts) return -1;
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    if (doorbell_slot) rxvm_thread_doorbell_e5_mask(SIG_BLOCK);
#elif defined(_WIN32)
    if (doorbell_slot) {
        InterlockedExchangePointer(&doorbell_slot->pending, NULL);
    }
#endif
    SIGNAL_LIFECYCLE_LOCK();
    if (context->active.pending_interrupts != pending_interrupts ||
        (g_process_main_context == context &&
         g_process_main_interrupts != pending_interrupts)) {
        rc = -1;
    } else {
        context->active.pending_interrupts = previous_pending_interrupts;
        if (previous_pending_interrupts)
            rxvm_signal_pending_or(previous_pending_interrupts,
                                   *pending_interrupts);
        if (g_process_main_context == context)
            g_process_main_interrupts = previous_pending_interrupts;
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
        if (doorbell_slot) doorbell_slot->pending = previous_pending_interrupts;
#endif
        *pending_interrupts = 0;
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    if (!rc && doorbell_slot && previous_pending_interrupts)
        rxvm_thread_doorbell_e5_mask(SIG_UNBLOCK);
#endif
    return rc;
}

int rxvm_signal_thread_doorbell_e5_install(void) {
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    struct sigaction action;
    int rc = 0;

    memset(&action, 0, sizeof(action));
    action.sa_handler = rxvm_thread_doorbell_e5_handler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, RXVM_THREAD_DOORBELL_E5_SIGNAL);

    SIGNAL_LIFECYCLE_LOCK();
    if (!g_thread_doorbell_users &&
        sigaction(RXVM_THREAD_DOORBELL_E5_SIGNAL, &action,
                  &g_thread_doorbell_previous_action) != 0) {
        rc = -1;
    } else if (g_thread_doorbell_users == SIZE_MAX) {
        rc = -1;
    } else {
        g_thread_doorbell_users++;
    }
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
#elif defined(_WIN32)
    HMODULE kernel32;
    int rc = 0;

    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users == SIZE_MAX) {
        rc = -1;
    } else if (!g_thread_doorbell_users) {
        const char *forced_unavailable = getenv(
                "CREXX_VM_E5_WINDOWS_APC_FORCE_UNAVAILABLE");
        kernel32 = GetModuleHandleW(L"kernel32.dll");
        if ((forced_unavailable && strcmp(forced_unavailable, "1") == 0) ||
            !kernel32) {
            rc = -1;
        } else {
            g_queue_user_apc2 = (rxvm_queue_user_apc2_fn)(uintptr_t)
                    GetProcAddress(kernel32, "QueueUserAPC2");
            if (!g_queue_user_apc2) rc = -1;
        }
    }
    if (!rc) g_thread_doorbell_users++;
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
#else
    return -1;
#endif
}

void rxvm_signal_thread_doorbell_e5_uninstall(void) {
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users) {
        g_thread_doorbell_users--;
        if (!g_thread_doorbell_users &&
            sigaction(RXVM_THREAD_DOORBELL_E5_SIGNAL,
                      &g_thread_doorbell_previous_action, 0) != 0) {
            abort();
        }
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#elif defined(_WIN32)
    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users) {
        g_thread_doorbell_users--;
        if (!g_thread_doorbell_users) g_queue_user_apc2 = NULL;
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#endif
}

int rxvm_signal_thread_doorbell_e5_prepare_current(void) {
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    const pthread_t self = pthread_self();
    rxvm_thread_doorbell_e5_slot *slot = 0;
    uintptr_t stack_low = 0u;
    uintptr_t stack_high = 0u;
    size_t index;

    rxvm_thread_doorbell_e5_mask(SIG_BLOCK);
    if (rxvm_thread_doorbell_e5_stack_range(
                self, &stack_low, &stack_high) != 0) return -1;

    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users) {
        for (index = 0u; index < RXVM_THREAD_DOORBELL_E5_MAX_WORKERS;
             index++) {
            if (!__atomic_load_n(&g_thread_doorbell_slots[index].active,
                                 __ATOMIC_ACQUIRE)) {
                slot = &g_thread_doorbell_slots[index];
                slot->thread = self;
                slot->stack_low = stack_low;
                slot->stack_high = stack_high;
                slot->pending = 0;
                __atomic_store_n(&slot->active, 1u, __ATOMIC_RELEASE);
                break;
            }
        }
    }
    SIGNAL_LIFECYCLE_UNLOCK();
    return slot ? 0 : -1;
#elif defined(_WIN32)
    const DWORD self = GetCurrentThreadId();
    rxvm_thread_doorbell_e5_slot *slot = NULL;
    size_t index;

    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users && g_queue_user_apc2) {
        for (index = 0u; index < RXVM_THREAD_DOORBELL_E5_MAX_WORKERS;
             index++) {
            if (!InterlockedCompareExchange(
                        &g_thread_doorbell_slots[index].active, 0, 0)) {
                slot = &g_thread_doorbell_slots[index];
                slot->thread_id = self;
                InterlockedExchangePointer(&slot->pending, NULL);
                InterlockedExchange(&slot->deferred, 0);
                InterlockedExchange(&slot->queued, 0);
                InterlockedExchange(&slot->active, 1);
                break;
            }
        }
    }
    SIGNAL_LIFECYCLE_UNLOCK();
    return slot ? 0 : -1;
#else
    return -1;
#endif
}

void rxvm_signal_thread_doorbell_e5_discard_current(void) {
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    sigset_t pending;
    sigset_t doorbell;
    int received;

    sigemptyset(&doorbell);
    sigaddset(&doorbell, RXVM_THREAD_DOORBELL_E5_SIGNAL);
    for (;;) {
        if (sigpending(&pending) != 0 ||
            !sigismember(&pending, RXVM_THREAD_DOORBELL_E5_SIGNAL)) {
            break;
        }
        if (sigwait(&doorbell, &received) != 0) abort();
    }
#elif defined(_WIN32)
    rxvm_thread_doorbell_e5_slot *slot =
            rxvm_thread_doorbell_e5_find_current();
    if (slot) {
        InterlockedExchangePointer(&slot->pending, NULL);
        rxvm_thread_doorbell_e5_wait_idle(slot);
        InterlockedExchange(&slot->deferred, 0);
    }
#endif
}

void rxvm_signal_thread_doorbell_e5_release_current(void) {
#if defined(CREXX_POSIX_THREAD_DOORBELL_E5)
    rxvm_thread_doorbell_e5_slot *slot;

    rxvm_signal_thread_doorbell_e5_discard_current();
    SIGNAL_LIFECYCLE_LOCK();
    slot = rxvm_thread_doorbell_e5_find_current();
    if (slot) {
        slot->pending = 0;
        slot->stack_low = 0u;
        slot->stack_high = 0u;
        memset(&slot->thread, 0, sizeof(slot->thread));
        __atomic_store_n(&slot->active, 0u, __ATOMIC_RELEASE);
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#elif defined(_WIN32)
    rxvm_thread_doorbell_e5_slot *slot;

    rxvm_signal_thread_doorbell_e5_discard_current();
    SIGNAL_LIFECYCLE_LOCK();
    slot = rxvm_thread_doorbell_e5_find_current();
    if (slot) {
        slot->thread_id = 0u;
        InterlockedExchange(&slot->active, 0);
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#endif
}

int rxvm_signal_thread_doorbell_e5_ring(void *thread_handle) {
#if defined(_WIN32)
    const DWORD thread_id = thread_handle
            ? GetThreadId((HANDLE)thread_handle) : 0u;
    rxvm_thread_doorbell_e5_slot *slot = NULL;
    size_t index;

    if (!thread_id || !g_queue_user_apc2) return -1;
    for (index = 0u; index < RXVM_THREAD_DOORBELL_E5_MAX_WORKERS; index++) {
        rxvm_thread_doorbell_e5_slot *candidate =
                &g_thread_doorbell_slots[index];
        if (InterlockedCompareExchange(&candidate->active, 0, 0) &&
            candidate->thread_id == thread_id) {
            slot = candidate;
            break;
        }
    }
    if (!slot) return -1;
    if (InterlockedCompareExchange(&slot->queued, 1, 0) != 0) return 0;
    if (!g_queue_user_apc2(rxvm_thread_doorbell_e5_callback,
                           (HANDLE)thread_handle,
                           (ULONG_PTR)slot,
                           RXVM_QUEUE_USER_APC_FLAGS_SPECIAL_USER_APC)) {
        InterlockedExchange(&slot->queued, 0);
        return -1;
    }
    return 0;
#else
    (void)thread_handle;
    return -1;
#endif
}

void rxvm_signal_thread_doorbell_e5_statistics(
        unsigned long *callback_count,
        unsigned long *maximum_depth) {
#if defined(_WIN32)
    if (callback_count) {
        *callback_count = (unsigned long)InterlockedCompareExchange(
                &g_thread_doorbell_callback_count, 0, 0);
    }
    if (maximum_depth) {
        *maximum_depth = (unsigned long)InterlockedCompareExchange(
                &g_thread_doorbell_callback_max_depth, 0, 0);
    }
#else
    if (callback_count) *callback_count = 0u;
    if (maximum_depth) *maximum_depth = 0u;
#endif
}

void rxvm_signal_pending_or(
        volatile sig_atomic_t *pending,
        sig_atomic_t mask) {
#if defined(_WIN32)
    InterlockedOr((volatile LONG *)pending, (LONG)mask);
#else
    *pending |= mask;
#endif
}

void rxvm_signal_pending_and(
        volatile sig_atomic_t *pending,
        sig_atomic_t mask) {
#if defined(_WIN32)
    InterlockedAnd((volatile LONG *)pending, (LONG)mask);
#else
    *pending &= mask;
#endif
}

void rxvm_signal_raise_process_main(unsigned char signal) {
    volatile sig_atomic_t *target = g_process_main_interrupts;
    if (target) rxvm_signal_pending_or(target, rxsignal_mask(signal));
}

void rxvm_signal_clear_process_main(unsigned char signal) {
    volatile sig_atomic_t *target = g_process_main_interrupts;
    if (target) rxvm_signal_pending_and(target, ~rxsignal_mask(signal));
}

/* --- Mapping between VM Signals and OS Signals --- */
typedef struct {
    int vm_signal;
    int os_signal; /* OS signal number, or -1 if not directly mappable */
} VmOsSignalMap;

/* Define the mapping - adjust as needed! */
static const VmOsSignalMap g_signal_map[] = {
    /* Instruction/Syntax Errors */
    { RXSIGNAL_ERROR,               SIGILL }, /* Treat syntax error as illegal operation */

    /* Termination / User Interaction */
    { RXSIGNAL_TERM,               SIGTERM },
#ifndef _WIN32
    /* POSIX-specific Signals Mappings */
    { RXSIGNAL_POSIX_INT,          SIGINT  },
    { RXSIGNAL_QUIT,               SIGQUIT },
    { RXSIGNAL_POSIX_HUP,          SIGHUP  },
    { RXSIGNAL_POSIX_USR1,         SIGUSR1 },
    { RXSIGNAL_POSIX_USR2,         SIGUSR2 },
    { RXSIGNAL_POSIX_CHLD,         SIGCHLD },
    { RXSIGNAL_NOTREADY,           SIGPIPE }, /* Often related to broken pipes */
#else
    /* Windows-specific Signal Mappings */
    { RXSIGNAL_TERM,          CTRL_CLOSE_EVENT  },
    { RXSIGNAL_QUIT,          CTRL_BREAK_EVENT },
{ RXSIGNAL_POSIX_INT,         CTRL_C_EVENT  },
#endif
    /* Note: RXSIGNAL_KILL (SIGKILL) cannot be caught */
    /* Note: RXSIGNAL_FAILURE, etc. do not map directly */
};
static const size_t g_signal_map_size = sizeof(g_signal_map) / sizeof(g_signal_map[0]);

/* Helper to get OS signal from VM signal */
static int get_os_signal(int vm_signal) {
    size_t i;
    for (i = 0; i < g_signal_map_size; ++i) {
        if (g_signal_map[i].vm_signal == vm_signal) {
            return g_signal_map[i].os_signal;
        }
    }
    return -1; /* Not found or not mapped */
}

/* Helper to get VM signal from OS signal */
static int get_vm_signal(int os_signal) {
    size_t i;
    /* This mapping back might be ambiguous - first match wins here */
    for (i = 0; i < g_signal_map_size; ++i) {
        if (g_signal_map[i].os_signal == os_signal) {
            return g_signal_map[i].vm_signal;
        }
    }
    return RXSIGNAL_MAX; /* Unmapped is an error - RXSIGNAL_MAX is ignored */
}

/* --- Master Signal Handler --- */

/*
 * This handler is called for any OS signal we intercept.
 * It must be simple and only use async-signal-safe functions.
 * It translates the OS signal to a VM signal and sets a bit in the designated
 * product-main VM's own pending-interrupt word.
 */
#ifdef _WIN32
static BOOL WINAPI vm_master_signal_handler(DWORD signum) {
#else
static void vm_master_signal_handler(int signum /* OS Signal Number */) {
#endif
    int vm_signal;

    /* Translate OS signal back to VM signal */
    vm_signal = get_vm_signal((int)(signum));

    /* Check if the VM signal code is valid for the bitmask */
    if (vm_signal > RXSIGNAL_NONE && vm_signal < RXSIGNAL_MAX) {
        rxvm_signal_raise_process_main((unsigned char)vm_signal);
#ifdef _WIN32
        return g_process_main_interrupts != NULL;
#endif
    }

    /* Re-arm handler if needed? */
    /* With sigaction (POSIX) - Not needed if SA_RESETHAND isn't set (default) */
#ifdef _WIN32
    return FALSE; /* Indicate we did not handle it */
#endif
}

/* --- Public API Functions --- */

/**
 * @brief Enables handling for a specific VM interrupt code.
 * Translates the VM code to an OS signal and registers the master handler.
 * Does nothing if the VM code doesn't map to a catchable OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to enable.
 * @return 0 on success or if no action needed, -1 on failure to register handler.
 */
static int enable_interrupt_unlocked(int vm_signal) {
    int os_signal;
#ifndef _WIN32
    struct sigaction sa_new; /* POSIX */
#else
    int handler_installed; /* Windows */
#endif

    /* Validate VM signal code */
    if (vm_signal <= RXSIGNAL_NONE || vm_signal >= RXSIGNAL_MAX) {
        return 0; /* Invalid code, treat as no-op success */
    }

    /* Check if already active */
    if (g_handler_active[vm_signal] == 1) {
        return 0;
    }

    /* Get the corresponding OS signal */
    os_signal = get_os_signal(vm_signal);

    /* Check if mappable and valid OS signal */
    if (os_signal < 0 || os_signal >= MAX_OS_SIGNALS) {
        return 0; /* Not mappable, treat as no-op success */
    }

    /* Register the handler */
#ifndef _WIN32
    /* --- POSIX: Use sigaction --- */
    struct sigaction sa_old;

    if (g_handler_active[vm_signal] == -1) {
        /* Already set to ignore by us, so don't save original */

        /* Query and store the original action *before* setting the new one */
        if (sigaction(os_signal, NULL, &sa_old) == 0) {
            memcpy(&g_original_os_actions[os_signal], &sa_old, sizeof(struct sigaction));
        } else {
            /* perror("Warning: sigaction query failed"); */
            /* Initialize original to SIG_DFL equivalent */
            memset(&g_original_os_actions[os_signal], 0, sizeof(struct sigaction));
            g_original_os_actions[os_signal].sa_handler = SIG_DFL;
            sigemptyset(&g_original_os_actions[os_signal].sa_mask);
        }
    }

    /* Set up our new action */
    memset(&sa_new, 0, sizeof(sa_new));
    sa_new.sa_handler = vm_master_signal_handler;
    sigemptyset(&sa_new.sa_mask); /* Block no other signals during handler */
    sa_new.sa_flags = SA_RESTART;  /* Restart interrupted syscalls */

    /* Install our handler */
    if (sigaction(os_signal, &sa_new, NULL) == -1) {
        perror("Error: sigaction enable failed");
        return -1; /* Failure */
    }
#endif

    /* Mark as active */
    g_handler_active[vm_signal] = 1;
    return 0; /* Success */
}

/**
 * @brief Sets the specific VM interrupt code to be ignored, meaning the linked OS signal is ignored.
 * Does nothing if the VM code doesn't map to a catchable OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to ignore.
 * return 0 on success or if no action is needed, -1 on failure to register handler.
 */
static int ignore_interrupt_unlocked(int vm_signal) {
   int os_signal;
#ifndef _WIN32
    struct sigaction sa_new; /* POSIX */
#endif

    /* Validate VM signal code */
    if (vm_signal <= RXSIGNAL_NONE || vm_signal >= RXSIGNAL_MAX) {
        return 0; /* Invalid code, treat as no-op success */
    }

    /* Check if already ignored by us */
    if (g_handler_active[vm_signal] == -1) {
        return 0;
    }

    /* Get the corresponding OS signal */
    os_signal = get_os_signal(vm_signal);

    /* Check if mappable and valid OS signal */
    if (os_signal < 0 || os_signal >= MAX_OS_SIGNALS) {
        return 0; /* Not mappable, treat as no-op success */
    }

    /* Register the handler */
#ifndef _WIN32
    /* --- POSIX: Use sigaction --- */
    struct sigaction sa_old;

    if (g_handler_active[vm_signal] == 1) {
        /* Already set to active by us, so don't save original */

        /* Query and store the original action *before* setting the new one */
        if (sigaction(os_signal, NULL, &sa_old) == 0) {
            memcpy(&g_original_os_actions[os_signal], &sa_old, sizeof(struct sigaction));
        } else {
            /* perror("Warning: sigaction query failed"); */
            /* Initialize original to SIG_DFL equivalent */
            memset(&g_original_os_actions[os_signal], 0, sizeof(struct sigaction));
            g_original_os_actions[os_signal].sa_handler = SIG_DFL;
            sigemptyset(&g_original_os_actions[os_signal].sa_mask);
        }
    }

    /* Set up our new action to ignore */
    memset(&sa_new, 0, sizeof(sa_new));
    /* SIG_IGN has special lifecycle semantics for SIGCHLD on POSIX: the
     * kernel may reap children before their owning provider can waitpid().
     * Rexx "ignore" means no Rexx notification, not abandonment of child
     * ownership, so retain the default waitable-child disposition. */
    sa_new.sa_handler = os_signal == SIGCHLD ? SIG_DFL : SIG_IGN;
    sigemptyset(&sa_new.sa_mask); /* Block no other signals during handler */
    sa_new.sa_flags = SA_RESTART;  /* Restart interrupted syscalls */

    if (sigaction(os_signal, &sa_new, NULL) == -1) {
        /* perror("Error: sigaction enable failed"); */
        return -1; /* Failure */
    }
#endif

    /* Mark as ignored by us */
    g_handler_active[vm_signal] = -1;
    return 0; /* Success */
}

/**
 * @brief Restores handling for a specific VM interrupt code.
 * Restores the original signal handler for the corresponding OS signal.
 *
 * @param vm_signal The RXSIGNAL_* code to disable.
 * @return 0 on success or if no action needed, -1 on failure to restore handler.
 */
static int restore_interrupt_unlocked(int vm_signal) {
    int os_signal;

    /* Validate VM signal code */
    if (vm_signal <= RXSIGNAL_NONE || vm_signal >= RXSIGNAL_MAX) {
        return 0;
    }

    /* Check if inactive */
    if (!g_handler_active[vm_signal]) {
        return 0;
    }

    /* Get the corresponding OS signal */
    os_signal = get_os_signal(vm_signal);

    /* Check if mappable and valid */
    if (os_signal < 0 || os_signal >= MAX_OS_SIGNALS) {
        /* This case should ideally not happen if it was active, but check anyway */
        g_handler_active[vm_signal] = 0; /* Mark inactive */
        return 0;
    }

    /* Restore original handler */
#ifndef _WIN32
    /* --- POSIX: Restore original action --- */
    if (sigaction(os_signal, &g_original_os_actions[os_signal], NULL) == -1) {
        /* perror("Error: sigaction disable failed"); */
        /* Let's mark inactive as we tried. */
        g_handler_active[vm_signal] = 0;
        return -1; /* Failure */
    }
#endif

    /* Mark as inactive */
    g_handler_active[vm_signal] = 0;
    return 0; /* Success */
}

int enable_interrupt(int vm_signal) {
    int rc;
    SIGNAL_LIFECYCLE_LOCK();
    rc = enable_interrupt_unlocked(vm_signal);
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
}

int ignore_interrupt(int vm_signal) {
    int rc;
    SIGNAL_LIFECYCLE_LOCK();
    rc = ignore_interrupt_unlocked(vm_signal);
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
}

int restore_interrupt(int vm_signal) {
    int rc;
    SIGNAL_LIFECYCLE_LOCK();
    rc = restore_interrupt_unlocked(vm_signal);
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
}

/**
 * @brief Initializes the VM signal handling system.
 * Clears flags, initializes storage, and prepares for enabling interrupts.
 * Should be called once at VM startup.
 * @return 0 on success.
 */
int initialize_vm_signals(void) {
    int i;

    SIGNAL_LIFECYCLE_LOCK();
    if (g_signal_users++ != 0u) {
        SIGNAL_LIFECYCLE_UNLOCK();
        return 0;
    }

    /* Initialize process-scoped handler tracking and storage. */
    for (i = 0; i < RXSIGNAL_MAX; ++i) {
        g_handler_active[i] = 0;
    }
    for (i = 0; i < MAX_OS_SIGNALS; ++i) {
#ifdef _WIN32
        g_original_os_handlers[i] = NULL;
#else
        memset(&g_original_os_actions[i], 0, sizeof(struct sigaction));
#endif
    }

#ifdef _WIN32
    /* Install our handler */
    if (!SetConsoleCtrlHandler(vm_master_signal_handler, TRUE))
    {
        fprintf(stderr, "ERROR: Could not set console control handler. GetLastError: %lu\n", GetLastError());
        g_signal_users = 0u;
        SIGNAL_LIFECYCLE_UNLOCK();
        return -1; // Indicate failure to set up handler
    }
#endif
    SIGNAL_LIFECYCLE_UNLOCK();
    return 0;
}

/**
 * @brief Cleans up signal handlers, restoring originals for active ones.
 * Intended to be called via atexit or manually before VM shutdown.
 */
void cleanup_vm_signals(void) {
    int i;
    SIGNAL_LIFECYCLE_LOCK();
    if (g_signal_users == 0u) {
        SIGNAL_LIFECYCLE_UNLOCK();
        return;
    }
    if (--g_signal_users != 0u) {
        SIGNAL_LIFECYCLE_UNLOCK();
        return;
    }
#ifdef _WIN32
    /* Install our handler */
    if (!SetConsoleCtrlHandler(vm_master_signal_handler, FALSE))
    {
        fprintf(stderr, "ERROR: Could not unload console control handler. GetLastError: %lu\n", GetLastError());
    }
#endif
    for (i = 0; i < RXSIGNAL_MAX; ++i) {
        if (g_handler_active[i]) {
            restore_interrupt_unlocked(i); /* Attempt to restore */
        }
    }
    SIGNAL_LIFECYCLE_UNLOCK();
}
