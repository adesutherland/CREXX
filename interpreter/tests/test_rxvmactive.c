/* cREXX E2 active-state ownership checks. */

#include "rxvmintp.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef struct active_shared {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
#else
    pthread_mutex_t mutex;
    pthread_cond_t condition;
#endif
    int ready;
    int failures;
} active_shared;

typedef struct active_thread {
    active_shared *shared;
    unsigned char signal;
} active_thread;

static void active_lock(active_shared *shared) {
#ifdef _WIN32
    EnterCriticalSection(&shared->mutex);
#else
    (void)pthread_mutex_lock(&shared->mutex);
#endif
}

static void active_unlock(active_shared *shared) {
#ifdef _WIN32
    LeaveCriticalSection(&shared->mutex);
#else
    (void)pthread_mutex_unlock(&shared->mutex);
#endif
}

static void active_wait(active_shared *shared) {
#ifdef _WIN32
    SleepConditionVariableCS(&shared->condition, &shared->mutex, INFINITE);
#else
    (void)pthread_cond_wait(&shared->condition, &shared->mutex);
#endif
}

static void active_broadcast(active_shared *shared) {
#ifdef _WIN32
    WakeAllConditionVariable(&shared->condition);
#else
    (void)pthread_cond_broadcast(&shared->condition);
#endif
}

static void run_active_thread(active_thread *thread) {
    rxvm_context context;
    rxvm_context *previous;
    rxvm_context *nested_active_previous;
    volatile sig_atomic_t pending_interrupts = 0;
    volatile sig_atomic_t nested_pending_interrupts = 0;
    volatile sig_atomic_t *previous_pending_interrupts = NULL;
    volatile sig_atomic_t *nested_previous_pending_interrupts = NULL;
    sig_atomic_t expected = rxsignal_mask(thread->signal);

    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    if (rxvm_worker_begin_execution(&context.worker) !=
        RXVM_WORKER_TRANSITION_OK) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
        rxfremod(&context);
        return;
    }
    if (rxvm_signal_enter_execution(
            &context, &pending_interrupts,
            &previous_pending_interrupts) != 0 ||
        previous_pending_interrupts != NULL) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
        (void)rxvm_worker_end_execution(&context.worker);
        rxfremod(&context);
        return;
    }

    previous = rxvm_active_context_enter(&context);
    nested_active_previous = rxvm_active_context_enter(&context);
    if (previous != NULL || nested_active_previous != &context ||
        rxvm_active_context_current() != &context) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
    }
    rxvm_active_context_leave(nested_active_previous);
    raise_signal(thread->signal);
    if (rxvm_signal_enter_execution(
            &context, &nested_pending_interrupts,
            &nested_previous_pending_interrupts) != 0 ||
        nested_previous_pending_interrupts != &pending_interrupts ||
        pending_interrupts != 0 ||
        nested_pending_interrupts != expected) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
    }
    clear_signal(thread->signal);
    if (rxvm_signal_leave_execution(
            &context, &nested_pending_interrupts,
            nested_previous_pending_interrupts) != 0 ||
        pending_interrupts != 0) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
    }
    raise_signal(thread->signal);

    active_lock(thread->shared);
    thread->shared->ready++;
    active_broadcast(thread->shared);
    while (thread->shared->ready != 2) active_wait(thread->shared);
    active_unlock(thread->shared);

    if (context.active.pending_interrupts != &pending_interrupts ||
        pending_interrupts != expected) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
    }
    clear_signal(thread->signal);
    if (pending_interrupts != 0) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
    }
    rxvm_active_context_leave(previous);
    if (rxvm_signal_leave_execution(
            &context, &pending_interrupts,
            previous_pending_interrupts) != 0 ||
        context.active.pending_interrupts != NULL ||
        rxvm_active_context_current() != NULL ||
        rxvm_worker_end_execution(&context.worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        active_lock(thread->shared);
        thread->shared->failures++;
        active_unlock(thread->shared);
    }
    rxfremod(&context);
}

#ifdef _WIN32
static DWORD WINAPI active_thread_entry(LPVOID argument) {
    run_active_thread((active_thread *)argument);
    return 0;
}
#else
static void *active_thread_entry(void *argument) {
    run_active_thread((active_thread *)argument);
    return NULL;
}
#endif

int main(void) {
    active_shared shared;
    active_thread threads[2];
    rxvm_context main_context;
    volatile sig_atomic_t main_pending_interrupts = 0;
    volatile sig_atomic_t *main_previous_pending_interrupts = NULL;

    /* Stack and embedded RXVML contexts are not required to be zero-filled by
     * their callers. Poison the storage so every active-state field added to
     * rxinimod_common() must be initialized explicitly. */
    memset(&main_context, 0xa5, sizeof(main_context));
    rxinimod(&main_context);
    if (main_context.active.compatibility_interrupts != NULL) {
        fprintf(stderr, "compatibility owner was selected by uninitialized state\n");
        main_context.active.compatibility_interrupts = NULL;
        rxfremod(&main_context);
        return 1;
    }
    if (rxvm_signal_bind_process_main(&main_context) != 0) {
        fprintf(stderr, "could not bind process-main interrupt target\n");
        rxfremod(&main_context);
        return 1;
    }
    if (rxvm_signal_enter_execution(
            &main_context, &main_pending_interrupts,
            &main_previous_pending_interrupts) != 0 ||
        main_previous_pending_interrupts != NULL) {
        fprintf(stderr, "could not publish process-main interrupt word\n");
        rxfremod(&main_context);
        return 1;
    }
    raise_signal(RXSIGNAL_POSIX_INT);
    if (main_pending_interrupts !=
        rxsignal_mask(RXSIGNAL_POSIX_INT)) {
        fprintf(stderr, "context-free interrupt did not target process main\n");
        (void)rxvm_signal_leave_execution(
                &main_context, &main_pending_interrupts,
                main_previous_pending_interrupts);
        rxfremod(&main_context);
        return 1;
    }
    clear_signal(RXSIGNAL_POSIX_INT);
    if (main_pending_interrupts != 0) {
        fprintf(stderr, "context-free interrupt clear missed process main\n");
        (void)rxvm_signal_leave_execution(
                &main_context, &main_pending_interrupts,
                main_previous_pending_interrupts);
        rxfremod(&main_context);
        return 1;
    }

    memset(&shared, 0, sizeof(shared));
    threads[0].shared = &shared;
    threads[0].signal = RXSIGNAL_ERROR;
    threads[1].shared = &shared;
    threads[1].signal = RXSIGNAL_FAILURE;
#ifdef _WIN32
    InitializeCriticalSection(&shared.mutex);
    InitializeConditionVariable(&shared.condition);
    {
        HANDLE handles[2];
        handles[0] = CreateThread(NULL, 0, active_thread_entry,
                                  &threads[0], 0, NULL);
        handles[1] = CreateThread(NULL, 0, active_thread_entry,
                                  &threads[1], 0, NULL);
        if (!handles[0] || !handles[1]) return 1;
        WaitForMultipleObjects(2, handles, TRUE, INFINITE);
        CloseHandle(handles[0]);
        CloseHandle(handles[1]);
    }
    DeleteCriticalSection(&shared.mutex);
#else
    (void)pthread_mutex_init(&shared.mutex, NULL);
    (void)pthread_cond_init(&shared.condition, NULL);
    {
        pthread_t handles[2];
        if (pthread_create(&handles[0], NULL, active_thread_entry,
                           &threads[0]) != 0 ||
            pthread_create(&handles[1], NULL, active_thread_entry,
                           &threads[1]) != 0) return 1;
        (void)pthread_join(handles[0], NULL);
        (void)pthread_join(handles[1], NULL);
    }
    (void)pthread_cond_destroy(&shared.condition);
    (void)pthread_mutex_destroy(&shared.mutex);
#endif

    if (main_pending_interrupts != 0) {
        fprintf(stderr, "worker interrupt escaped into process main\n");
        shared.failures++;
    }
    if (shared.failures) {
        fprintf(stderr, "%d active-state isolation check(s) failed\n",
                shared.failures);
        (void)rxvm_signal_leave_execution(
                &main_context, &main_pending_interrupts,
                main_previous_pending_interrupts);
        rxfremod(&main_context);
        return 1;
    }
    if (rxvm_signal_leave_execution(
            &main_context, &main_pending_interrupts,
            main_previous_pending_interrupts) != 0 ||
        main_context.active.pending_interrupts != NULL) {
        fprintf(stderr, "could not retire process-main interrupt word\n");
        rxfremod(&main_context);
        return 1;
    }
    rxfremod(&main_context);
    puts("PASS: one interrupt word per VM; process main is the OS target");
    return 0;
}
