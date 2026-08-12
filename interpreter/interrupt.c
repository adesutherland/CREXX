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


#define _POSIX_C_SOURCE 200809L /* Request POSIX features */
#include <signal.h>
#include <stddef.h>  /* For NULL */
#include <stdint.h>  /* For uintptr_t */
#include <stdlib.h>  /* For abort */
#include <string.h>  /* For memset */

/* Include VM signal definitions */
#include "rxvmintp.h" /* For RXSIGNAL_* codes, and set / clear interrupt functions */

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

#if defined(__APPLE__)
/*
 * PERF3-13 E5 macOS native-doorbell PoC. SIGURG is not mapped to a public VM
 * signal. Each worker registers its immutable stack range while SIGURG is
 * blocked. During run(), the slot points at the existing execution-local
 * pending word and SIGURG is unblocked. The handler identifies its target by
 * the current stack address and performs only the existing sig_atomic_t OR.
 *
 * Apple Clang's Mach-O TLS access can call the TLV resolver, so TLS is
 * deliberately excluded from this signal-handler path.
 */
#define RXVM_THREAD_DOORBELL_POC_SIGNAL SIGURG
#define RXVM_THREAD_DOORBELL_POC_MAX_WORKERS 64u

typedef struct rxvm_thread_doorbell_poc_slot {
    volatile uint32_t active;
    pthread_t thread;
    uintptr_t stack_low;
    uintptr_t stack_high;
    volatile sig_atomic_t *pending;
} rxvm_thread_doorbell_poc_slot;

static rxvm_thread_doorbell_poc_slot
        g_thread_doorbell_slots[RXVM_THREAD_DOORBELL_POC_MAX_WORKERS];
static struct sigaction g_thread_doorbell_previous_action;
static size_t g_thread_doorbell_users;

typedef char crexx_thread_doorbell_active_must_be_lock_free[
        __atomic_always_lock_free(sizeof(uint32_t), 0) ? 1 : -1];

static void rxvm_thread_doorbell_poc_handler(int os_signal) {
    volatile unsigned char stack_marker = 0u;
    const uintptr_t stack_address = (uintptr_t)&stack_marker;
    size_t index;

    if (os_signal != RXVM_THREAD_DOORBELL_POC_SIGNAL) return;
    for (index = 0u; index < RXVM_THREAD_DOORBELL_POC_MAX_WORKERS; index++) {
        rxvm_thread_doorbell_poc_slot *slot =
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

static rxvm_thread_doorbell_poc_slot *
rxvm_thread_doorbell_poc_find_current(void) {
    const pthread_t self = pthread_self();
    size_t index;

    for (index = 0u; index < RXVM_THREAD_DOORBELL_POC_MAX_WORKERS; index++) {
        rxvm_thread_doorbell_poc_slot *slot =
                &g_thread_doorbell_slots[index];
        if (__atomic_load_n(&slot->active, __ATOMIC_ACQUIRE) &&
            pthread_equal(slot->thread, self)) {
            return slot;
        }
    }
    return 0;
}

static void rxvm_thread_doorbell_poc_mask(int how) {
    sigset_t doorbell;

    sigemptyset(&doorbell);
    sigaddset(&doorbell, RXVM_THREAD_DOORBELL_POC_SIGNAL);
    if (pthread_sigmask(how, &doorbell, 0) != 0) abort();
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
#if defined(__APPLE__)
    rxvm_thread_doorbell_poc_slot *doorbell_slot;
#endif

    if (!context || !pending_interrupts || !previous_pending_interrupts)
        return -1;
#if defined(__APPLE__)
    doorbell_slot = rxvm_thread_doorbell_poc_find_current();
    if (doorbell_slot) rxvm_thread_doorbell_poc_mask(SIG_BLOCK);
#endif
    SIGNAL_LIFECYCLE_LOCK();
    previous = context->active.pending_interrupts;
    *pending_interrupts = previous ? *previous : 0;
    if (previous) *previous = 0;
    context->active.pending_interrupts = pending_interrupts;
    if (g_process_main_context == context)
        g_process_main_interrupts = pending_interrupts;
    *previous_pending_interrupts = previous;
#if defined(__APPLE__)
    if (doorbell_slot) doorbell_slot->pending = pending_interrupts;
#endif
    SIGNAL_LIFECYCLE_UNLOCK();
#if defined(__APPLE__)
    if (doorbell_slot) rxvm_thread_doorbell_poc_mask(SIG_UNBLOCK);
#endif
    return 0;
}

int rxvm_signal_leave_execution(
        rxvm_context *context,
        volatile sig_atomic_t *pending_interrupts,
        volatile sig_atomic_t *previous_pending_interrupts) {
    int rc = 0;
#if defined(__APPLE__)
    rxvm_thread_doorbell_poc_slot *doorbell_slot =
            rxvm_thread_doorbell_poc_find_current();
#endif

    if (!context || !pending_interrupts) return -1;
#if defined(__APPLE__)
    if (doorbell_slot) rxvm_thread_doorbell_poc_mask(SIG_BLOCK);
#endif
    SIGNAL_LIFECYCLE_LOCK();
    if (context->active.pending_interrupts != pending_interrupts ||
        (g_process_main_context == context &&
         g_process_main_interrupts != pending_interrupts)) {
        rc = -1;
    } else {
        context->active.pending_interrupts = previous_pending_interrupts;
        if (previous_pending_interrupts)
            *previous_pending_interrupts |= *pending_interrupts;
        if (g_process_main_context == context)
            g_process_main_interrupts = previous_pending_interrupts;
#if defined(__APPLE__)
        if (doorbell_slot) doorbell_slot->pending = previous_pending_interrupts;
#endif
        *pending_interrupts = 0;
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#if defined(__APPLE__)
    if (!rc && doorbell_slot && previous_pending_interrupts)
        rxvm_thread_doorbell_poc_mask(SIG_UNBLOCK);
#endif
    return rc;
}

int rxvm_signal_thread_doorbell_poc_install(void) {
#if defined(__APPLE__)
    struct sigaction action;
    int rc = 0;

    memset(&action, 0, sizeof(action));
    action.sa_handler = rxvm_thread_doorbell_poc_handler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, RXVM_THREAD_DOORBELL_POC_SIGNAL);

    SIGNAL_LIFECYCLE_LOCK();
    if (!g_thread_doorbell_users &&
        sigaction(RXVM_THREAD_DOORBELL_POC_SIGNAL, &action,
                  &g_thread_doorbell_previous_action) != 0) {
        rc = -1;
    } else if (g_thread_doorbell_users == SIZE_MAX) {
        rc = -1;
    } else {
        g_thread_doorbell_users++;
    }
    SIGNAL_LIFECYCLE_UNLOCK();
    return rc;
#else
    return -1;
#endif
}

void rxvm_signal_thread_doorbell_poc_uninstall(void) {
#if defined(__APPLE__)
    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users) {
        g_thread_doorbell_users--;
        if (!g_thread_doorbell_users &&
            sigaction(RXVM_THREAD_DOORBELL_POC_SIGNAL,
                      &g_thread_doorbell_previous_action, 0) != 0) {
            abort();
        }
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#endif
}

int rxvm_signal_thread_doorbell_poc_prepare_current(void) {
#if defined(__APPLE__)
    const pthread_t self = pthread_self();
    const uintptr_t stack_high =
            (uintptr_t)pthread_get_stackaddr_np(self);
    const size_t stack_size = pthread_get_stacksize_np(self);
    rxvm_thread_doorbell_poc_slot *slot = 0;
    size_t index;

    rxvm_thread_doorbell_poc_mask(SIG_BLOCK);
    if (!stack_high || !stack_size || stack_size > stack_high) return -1;

    SIGNAL_LIFECYCLE_LOCK();
    if (g_thread_doorbell_users) {
        for (index = 0u; index < RXVM_THREAD_DOORBELL_POC_MAX_WORKERS;
             index++) {
            if (!__atomic_load_n(&g_thread_doorbell_slots[index].active,
                                 __ATOMIC_ACQUIRE)) {
                slot = &g_thread_doorbell_slots[index];
                slot->thread = self;
                slot->stack_low = stack_high - stack_size;
                slot->stack_high = stack_high;
                slot->pending = 0;
                __atomic_store_n(&slot->active, 1u, __ATOMIC_RELEASE);
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

void rxvm_signal_thread_doorbell_poc_discard_current(void) {
#if defined(__APPLE__)
    sigset_t pending;
    sigset_t doorbell;
    int received;

    sigemptyset(&doorbell);
    sigaddset(&doorbell, RXVM_THREAD_DOORBELL_POC_SIGNAL);
    for (;;) {
        if (sigpending(&pending) != 0 ||
            !sigismember(&pending, RXVM_THREAD_DOORBELL_POC_SIGNAL)) {
            break;
        }
        if (sigwait(&doorbell, &received) != 0) abort();
    }
#endif
}

void rxvm_signal_thread_doorbell_poc_release_current(void) {
#if defined(__APPLE__)
    rxvm_thread_doorbell_poc_slot *slot;

    rxvm_signal_thread_doorbell_poc_discard_current();
    SIGNAL_LIFECYCLE_LOCK();
    slot = rxvm_thread_doorbell_poc_find_current();
    if (slot) {
        slot->pending = 0;
        slot->stack_low = 0u;
        slot->stack_high = 0u;
        memset(&slot->thread, 0, sizeof(slot->thread));
        __atomic_store_n(&slot->active, 0u, __ATOMIC_RELEASE);
    }
    SIGNAL_LIFECYCLE_UNLOCK();
#endif
}

void rxvm_signal_raise_process_main(unsigned char signal) {
    volatile sig_atomic_t *target = g_process_main_interrupts;
    if (target) *target |= rxsignal_mask(signal);
}

void rxvm_signal_clear_process_main(unsigned char signal) {
    volatile sig_atomic_t *target = g_process_main_interrupts;
    if (target) *target &= ~rxsignal_mask(signal);
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
    sa_new.sa_handler = SIG_IGN; /* Set to ignore */
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
