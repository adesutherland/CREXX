/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmexecutor.h"

#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvmprogram.h"
#include "rxvmvars.h"
#include "rxvmworker.h"
#include "rxastree.h"

#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Private PERF3-13 E5 carrier hooks; deliberately not in the RXVML header. */
int rxvm_signal_thread_doorbell_e5_install(void);
void rxvm_signal_thread_doorbell_e5_uninstall(void);
int rxvm_signal_thread_doorbell_e5_prepare_current(void);
void rxvm_signal_thread_doorbell_e5_discard_current(void);
void rxvm_signal_thread_doorbell_e5_release_current(void);
int rxvm_signal_thread_doorbell_e5_ring(void *thread_handle);

#if defined(_WIN32)
#include <process.h>
#include <windows.h>
typedef CRITICAL_SECTION rxvm_executor_mutex;
typedef CONDITION_VARIABLE rxvm_executor_condition;
typedef HANDLE rxvm_executor_thread;

static int executor_mutex_init(rxvm_executor_mutex *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}
static void executor_mutex_destroy(rxvm_executor_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void executor_mutex_lock(rxvm_executor_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void executor_mutex_unlock(rxvm_executor_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
static int executor_condition_init(rxvm_executor_condition *condition) {
    InitializeConditionVariable(condition);
    return 1;
}
static void executor_condition_destroy(rxvm_executor_condition *condition) {
    (void)condition;
}
static void executor_condition_wait(rxvm_executor_condition *condition,
                                    rxvm_executor_mutex *mutex) {
    if (!SleepConditionVariableCS(condition, mutex, INFINITE)) abort();
}
static int executor_condition_wait_for(rxvm_executor_condition *condition,
                                       rxvm_executor_mutex *mutex,
                                       int64_t microseconds) {
    DWORD milliseconds;
    BOOL result;

    if (microseconds < 0) {
        executor_condition_wait(condition, mutex);
        return 1;
    }
    if (microseconds == 0) return 0;
    milliseconds = microseconds > (int64_t)(INFINITE - 1u) * 1000
            ? INFINITE - 1u
            : (DWORD)((microseconds + 999) / 1000);
    result = SleepConditionVariableCS(condition, mutex, milliseconds);
    if (result) return 1;
    if (GetLastError() == ERROR_TIMEOUT) return 0;
    abort();
    return 0;
}
static void executor_condition_broadcast(rxvm_executor_condition *condition) {
    WakeAllConditionVariable(condition);
}
#else
#include <pthread.h>
#include <signal.h>
#include <time.h>
typedef pthread_mutex_t rxvm_executor_mutex;
typedef pthread_cond_t rxvm_executor_condition;
typedef pthread_t rxvm_executor_thread;

static int executor_mutex_init(rxvm_executor_mutex *mutex) {
    return pthread_mutex_init(mutex, 0) == 0;
}
static void executor_mutex_destroy(rxvm_executor_mutex *mutex) {
    if (pthread_mutex_destroy(mutex) != 0) abort();
}
static void executor_mutex_lock(rxvm_executor_mutex *mutex) {
    if (pthread_mutex_lock(mutex) != 0) abort();
}
static void executor_mutex_unlock(rxvm_executor_mutex *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) abort();
}
static int executor_condition_init(rxvm_executor_condition *condition) {
#if defined(__linux__) && defined(CLOCK_MONOTONIC)
    pthread_condattr_t attributes;
    int result;

    if (pthread_condattr_init(&attributes) != 0) return 0;
    if (pthread_condattr_setclock(&attributes, CLOCK_MONOTONIC) != 0) {
        pthread_condattr_destroy(&attributes);
        return 0;
    }
    result = pthread_cond_init(condition, &attributes) == 0;
    pthread_condattr_destroy(&attributes);
    return result;
#else
    return pthread_cond_init(condition, 0) == 0;
#endif
}
static void executor_condition_destroy(rxvm_executor_condition *condition) {
    if (pthread_cond_destroy(condition) != 0) abort();
}
static void executor_condition_wait(rxvm_executor_condition *condition,
                                    rxvm_executor_mutex *mutex) {
    if (pthread_cond_wait(condition, mutex) != 0) abort();
}
static int executor_condition_wait_for(rxvm_executor_condition *condition,
                                       rxvm_executor_mutex *mutex,
                                       int64_t microseconds) {
    int result;

    if (microseconds < 0) {
        executor_condition_wait(condition, mutex);
        return 1;
    }
    if (microseconds == 0) return 0;
#if defined(__APPLE__)
    {
        struct timespec relative;
        relative.tv_sec = (time_t)(microseconds / 1000000);
        relative.tv_nsec = (long)((microseconds % 1000000) * 1000);
        result = pthread_cond_timedwait_relative_np(
                condition, mutex, &relative);
    }
#else
    {
        struct timespec deadline;
#if defined(__linux__) && defined(CLOCK_MONOTONIC)
        if (clock_gettime(CLOCK_MONOTONIC, &deadline) != 0) abort();
#else
        if (clock_gettime(CLOCK_REALTIME, &deadline) != 0) abort();
#endif
        deadline.tv_sec += (time_t)(microseconds / 1000000);
        deadline.tv_nsec += (long)((microseconds % 1000000) * 1000);
        if (deadline.tv_nsec >= 1000000000L) {
            deadline.tv_sec++;
            deadline.tv_nsec -= 1000000000L;
        }
        result = pthread_cond_timedwait(condition, mutex, &deadline);
    }
#endif
    if (result == 0) return 1;
    if (result == ETIMEDOUT) return 0;
    abort();
    return 0;
}
static void executor_condition_broadcast(rxvm_executor_condition *condition) {
    if (pthread_cond_broadcast(condition) != 0) abort();
}
#endif

static uint64_t executor_monotonic_microseconds(void) {
#if defined(_WIN32)
    return (uint64_t)GetTickCount64() * UINT64_C(1000);
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) abort();
    return (uint64_t)now.tv_sec * UINT64_C(1000000) +
           (uint64_t)now.tv_nsec / UINT64_C(1000);
#endif
}

typedef struct rxvm_executor_worker rxvm_executor_worker;

typedef enum rxvm_executor_event {
    RXVM_EXECUTOR_EVENT_NONE = 0,
    RXVM_EXECUTOR_EVENT_CANCEL = 1,
    RXVM_EXECUTOR_EVENT_DEADLINE = 2,
    RXVM_EXECUTOR_EVENT_KILL = 4,
    RXVM_EXECUTOR_EVENT_SHUTDOWN = 8
} rxvm_executor_event;

typedef struct rxvm_executor_mailbox {
    volatile sig_atomic_t events;
    volatile sig_atomic_t published_generation;
    volatile sig_atomic_t active_generation;
} rxvm_executor_mailbox;

struct rxvm_executor_request {
    rxvm_executor_worker *worker;
    char *procedure;
    rxvm_executor_register_image *arguments;
    rxvm_executor_register_image result;
    int argc;
    int procedure_result;
    size_t affinity;
    sig_atomic_t generation;
    uint64_t callable_id;
    uint64_t completion_sequence;
    rxvm_executor_event terminal_event;
    rxvm_executor_request_state state;
    rxvm_executor_mutex mutex;
    rxvm_executor_condition changed;
    rxvm_executor_register_type expected_result;
    unsigned char uses_callable_id;
};

struct rxvm_executor_worker {
    struct rxvm_executor *executor;
    rxvm_context *context;
    rxvm_executor_request **queue;
    size_t queue_capacity;
    size_t queue_head;
    size_t queue_count;
    size_t affinity;
    rxvm_executor_thread thread;
    rxvm_executor_mailbox mailbox;
    sig_atomic_t next_generation;
    rxvm_executor_mutex mutex;
    rxvm_executor_condition changed;
    rxvm_executor_result startup_result;
    unsigned char initialized;
    unsigned char thread_started;
    unsigned char startup_complete;
    unsigned char stopping;
    unsigned char stopped;
    unsigned char quarantined;
};

static sig_atomic_t executor_atomic_load(
        volatile sig_atomic_t *value) {
#if defined(_WIN32)
    return (sig_atomic_t)InterlockedCompareExchange(
            (volatile LONG *)value, 0, 0);
#elif defined(__GNUC__) || defined(__clang__)
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#else
    return *value;
#endif
}

static void executor_atomic_store(
        volatile sig_atomic_t *value,
        sig_atomic_t replacement) {
#if defined(_WIN32)
    InterlockedExchange((volatile LONG *)value, (LONG)replacement);
#elif defined(__GNUC__) || defined(__clang__)
    __atomic_store_n(value, replacement, __ATOMIC_RELEASE);
#else
    *value = replacement;
#endif
}

static sig_atomic_t executor_atomic_exchange(
        volatile sig_atomic_t *value,
        sig_atomic_t replacement) {
#if defined(_WIN32)
    return (sig_atomic_t)InterlockedExchange(
            (volatile LONG *)value, (LONG)replacement);
#elif defined(__GNUC__) || defined(__clang__)
    return __atomic_exchange_n(value, replacement, __ATOMIC_ACQ_REL);
#else
    sig_atomic_t previous = *value;
    *value = replacement;
    return previous;
#endif
}

static void executor_atomic_or(
        volatile sig_atomic_t *value,
        sig_atomic_t mask) {
#if defined(_WIN32)
    InterlockedOr((volatile LONG *)value, (LONG)mask);
#elif defined(__GNUC__) || defined(__clang__)
    (void)__atomic_fetch_or(value, mask, __ATOMIC_RELEASE);
#else
    *value |= mask;
#endif
}

static sig_atomic_t executor_mailbox_claim(void *owner) {
    rxvm_executor_mailbox *mailbox = (rxvm_executor_mailbox *)owner;
    sig_atomic_t active_generation;
    sig_atomic_t published_generation;
    sig_atomic_t events;

    if (!mailbox) return 0;
    active_generation = executor_atomic_load(&mailbox->active_generation);
    published_generation = executor_atomic_load(
            &mailbox->published_generation);
    if (!active_generation || published_generation != active_generation) {
        return 0;
    }
    events = executor_atomic_exchange(&mailbox->events, 0);
    if (events & (RXVM_EXECUTOR_EVENT_SHUTDOWN |
                  RXVM_EXECUTOR_EVENT_KILL)) {
        return rxsignal_mask(RXSIGNAL_KILL);
    }
    if (events & (RXVM_EXECUTOR_EVENT_DEADLINE |
                  RXVM_EXECUTOR_EVENT_CANCEL)) {
        return rxsignal_mask(RXSIGNAL_CANCEL);
    }
    return 0;
}

static sig_atomic_t executor_worker_next_generation(
        rxvm_executor_worker *worker) {
    if (worker->next_generation >= (sig_atomic_t)INT_MAX) {
        worker->next_generation = 1;
    } else {
        worker->next_generation++;
    }
    return worker->next_generation;
}

static void executor_mailbox_arm(
        rxvm_executor_worker *worker,
        rxvm_executor_request *request) {
    request->generation = executor_worker_next_generation(worker);
    executor_atomic_store(&worker->mailbox.events, 0);
    executor_atomic_store(&worker->mailbox.published_generation,
                          request->generation);
    executor_atomic_store(&worker->mailbox.active_generation,
                          request->generation);
}

static void executor_mailbox_disarm(rxvm_executor_worker *worker) {
    executor_atomic_store(&worker->mailbox.active_generation, 0);
    executor_atomic_store(&worker->mailbox.published_generation, 0);
    (void)executor_atomic_exchange(&worker->mailbox.events, 0);
}

static void executor_mailbox_publish(
        rxvm_executor_worker *worker,
        rxvm_executor_request *request,
        rxvm_executor_event event) {
    executor_atomic_store(&worker->mailbox.published_generation,
                          request->generation);
    executor_atomic_or(&worker->mailbox.events, (sig_atomic_t)event);
}

struct rxvm_executor {
    rxvm_runtime *runtime;
    const rxvm_program_generation *generation;
    rxvm_executor_worker *workers;
    size_t worker_count;
    size_t queue_capacity;
    rxvm_executor_statistics statistics;
    rxvm_executor_mutex statistics_mutex;
    rxvm_executor_mutex completion_mutex;
    rxvm_executor_condition completion_changed;
    uint64_t completion_generation;
    unsigned char statistics_mutex_initialized;
    unsigned char completion_sync_initialized;
    unsigned char owns_runtime;
    unsigned char native_doorbell;
    unsigned char compatibility_doorbell;
    volatile sig_atomic_t shutdown_requested;
};

static void executor_publish_terminal(
        rxvm_executor *executor,
        rxvm_executor_request *request) {
    executor_mutex_lock(&executor->completion_mutex);
    if (executor->completion_generation == UINT64_MAX) abort();
    executor->completion_generation++;
    request->completion_sequence = executor->completion_generation;
    executor_condition_broadcast(&executor->completion_changed);
    executor_mutex_unlock(&executor->completion_mutex);
}

static int executor_worker_ring(rxvm_executor_worker *worker) {
    if (worker->executor->compatibility_doorbell) return 1;
    if (!worker->executor->native_doorbell) return 0;
#if defined(__APPLE__) || defined(__linux__)
    return pthread_kill(worker->thread, SIGURG) == 0;
#elif defined(_WIN32)
    return rxvm_signal_thread_doorbell_e5_ring(worker->thread) == 0;
#else
    return 0;
#endif
}

static int request_state_is_terminal(rxvm_executor_request_state state) {
    return state == RXVM_EXECUTOR_REQUEST_COMPLETED ||
           state == RXVM_EXECUTOR_REQUEST_CANCELLED ||
           state == RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND ||
           state == RXVM_EXECUTOR_REQUEST_SETUP_FAILED ||
           state == RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED ||
           state == RXVM_EXECUTOR_REQUEST_KILLED ||
           state == RXVM_EXECUTOR_REQUEST_SHUTDOWN ||
           state == RXVM_EXECUTOR_REQUEST_EXECUTION_FAILED;
}

static void executor_request_promote_event(
        rxvm_executor_request *request,
        rxvm_executor_event event) {
    if (event > request->terminal_event) request->terminal_event = event;
}

static rxvm_executor_request_state executor_request_event_state(
        rxvm_executor_event event,
        rxvm_executor_request_state fallback) {
    if (event == RXVM_EXECUTOR_EVENT_SHUTDOWN) {
        return RXVM_EXECUTOR_REQUEST_SHUTDOWN;
    }
    if (event == RXVM_EXECUTOR_EVENT_KILL) {
        return RXVM_EXECUTOR_REQUEST_KILLED;
    }
    if (event == RXVM_EXECUTOR_EVENT_DEADLINE) {
        return RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED;
    }
    if (event == RXVM_EXECUTOR_EVENT_CANCEL) {
        return RXVM_EXECUTOR_REQUEST_CANCELLED;
    }
    return fallback;
}

static char *executor_copy_string(const char *source) {
    size_t length;
    char *copy;

    if (!source) source = "";
    length = strlen(source);
    if (length == SIZE_MAX) return 0;
    copy = (char *)malloc(length + 1u);
    if (!copy) return 0;
    memcpy(copy, source, length + 1u);
    return copy;
}

static void executor_request_storage_destroy(
        rxvm_executor_request *request) {
    int i;

    if (!request) return;
    for (i = 0; i < request->argc; i++) {
        free((void *)request->arguments[i].bytes);
    }
    free(request->arguments);
    free((void *)request->result.bytes);
    free(request->procedure);
    executor_condition_destroy(&request->changed);
    executor_mutex_destroy(&request->mutex);
    free(request);
}

static rxvm_executor_request *executor_request_create(
        rxvm_executor_worker *worker,
        const char *procedure,
        size_t argument_count,
        const rxvm_executor_register_image *arguments) {
    rxvm_executor_request *request;
    size_t i;

    request = (rxvm_executor_request *)calloc(1u, sizeof(*request));
    if (!request) return 0;
    if (!executor_mutex_init(&request->mutex)) {
        free(request);
        return 0;
    }
    if (!executor_condition_init(&request->changed)) {
        executor_mutex_destroy(&request->mutex);
        free(request);
        return 0;
    }
    request->procedure = executor_copy_string(procedure);
    if (!request->procedure) {
        executor_request_storage_destroy(request);
        return 0;
    }
    if (argument_count > 0u) {
        if (argument_count > SIZE_MAX / sizeof(*request->arguments)) {
            executor_request_storage_destroy(request);
            return 0;
        }
        request->arguments = (rxvm_executor_register_image *)calloc(
                argument_count, sizeof(*request->arguments));
        if (!request->arguments) {
            executor_request_storage_destroy(request);
            return 0;
        }
        for (i = 0u; i < argument_count; i++) {
            const rxvm_executor_register_image *source = &arguments[i];
            rxvm_executor_register_image *destination =
                    &request->arguments[i];
            destination->type = source->type;
            destination->integer = source->integer;
            if (source->type == RXVM_EXECUTOR_REGISTER_STRING ||
                source->type == RXVM_EXECUTOR_REGISTER_BINARY) {
                unsigned char *copy;
                if ((!source->bytes && source->length) ||
                    source->length == SIZE_MAX) {
                    request->argc = (int)i;
                    executor_request_storage_destroy(request);
                    return 0;
                }
                copy = (unsigned char *)malloc(source->length + 1u);
                if (!copy) {
                    request->argc = (int)i;
                    executor_request_storage_destroy(request);
                    return 0;
                }
                if (source->length) {
                    memcpy(copy, source->bytes, source->length);
                }
                copy[source->length] = 0;
                destination->bytes = (const char *)copy;
                destination->length = source->length;
            } else if (source->type != RXVM_EXECUTOR_REGISTER_INTEGER &&
                       source->type != RXVM_EXECUTOR_REGISTER_NONE) {
                request->argc = (int)i;
                executor_request_storage_destroy(request);
                return 0;
            }
            request->argc = (int)(i + 1u);
        }
    }
    request->worker = worker;
    request->argc = (int)argument_count;
    request->affinity = worker->affinity;
    request->state = RXVM_EXECUTOR_REQUEST_QUEUED;
    request->expected_result = RXVM_EXECUTOR_REGISTER_INTEGER;
    return request;
}

static void executor_statistics_start(rxvm_executor *executor) {
    executor_mutex_lock(&executor->statistics_mutex);
    executor->statistics.running_requests++;
    if (executor->statistics.maximum_parallel_requests <
        executor->statistics.running_requests) {
        executor->statistics.maximum_parallel_requests =
                executor->statistics.running_requests;
    }
    executor_mutex_unlock(&executor->statistics_mutex);
}

static void executor_statistics_finish(
        rxvm_executor *executor,
        rxvm_executor_request_state state) {
    executor_mutex_lock(&executor->statistics_mutex);
    if (!executor->statistics.running_requests) abort();
    executor->statistics.running_requests--;
    if (state == RXVM_EXECUTOR_REQUEST_COMPLETED) {
        executor->statistics.completed_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_CANCELLED) {
        executor->statistics.cancelled_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED) {
        executor->statistics.deadline_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_KILLED) {
        executor->statistics.killed_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_SHUTDOWN) {
        executor->statistics.shutdown_requests++;
    } else {
        executor->statistics.failed_requests++;
    }
    executor_mutex_unlock(&executor->statistics_mutex);
}

static void executor_statistics_terminal_queued(
        rxvm_executor *executor,
        rxvm_executor_request_state state) {
    executor_mutex_lock(&executor->statistics_mutex);
    if (state == RXVM_EXECUTOR_REQUEST_CANCELLED) {
        executor->statistics.cancelled_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED) {
        executor->statistics.deadline_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_KILLED) {
        executor->statistics.killed_requests++;
    } else if (state == RXVM_EXECUTOR_REQUEST_SHUTDOWN) {
        executor->statistics.shutdown_requests++;
    } else {
        executor->statistics.failed_requests++;
    }
    executor_mutex_unlock(&executor->statistics_mutex);
}

static void executor_worker_quarantine(rxvm_executor_worker *worker) {
    executor_mutex_lock(&worker->mutex);
    if (!worker->quarantined) {
        worker->quarantined = 1u;
        executor_mutex_lock(&worker->executor->statistics_mutex);
        worker->executor->statistics.quarantined_workers++;
        if (worker->executor->statistics.maximum_quarantined_workers <
            worker->executor->statistics.quarantined_workers) {
            worker->executor->statistics.maximum_quarantined_workers =
                    worker->executor->statistics.quarantined_workers;
        }
        executor_mutex_unlock(&worker->executor->statistics_mutex);
    }
    executor_mutex_unlock(&worker->mutex);
}

static void executor_worker_unquarantine(rxvm_executor_worker *worker) {
    executor_mutex_lock(&worker->mutex);
    if (worker->quarantined) {
        worker->quarantined = 0u;
        executor_mutex_lock(&worker->executor->statistics_mutex);
        if (!worker->executor->statistics.quarantined_workers) abort();
        worker->executor->statistics.quarantined_workers--;
        executor_mutex_unlock(&worker->executor->statistics_mutex);
    }
    executor_mutex_unlock(&worker->mutex);
}

static void executor_request_complete(
        rxvm_executor_request *request,
        rxvm_executor_request_state state,
        rxvm_executor_register_image *result) {
    rxvm_executor *executor = request->worker->executor;

    executor_mutex_lock(&request->mutex);
    /* Disarm this request while cancellation is excluded by the same mutex.
     * A late post after rxvm_call() returned must never reach the next request
     * assigned to this persistent VM. */
    if (executor->native_doorbell) {
        rxvm_signal_thread_doorbell_e5_discard_current();
    }
    executor_mailbox_disarm(request->worker);
    if (executor_atomic_load(&executor->shutdown_requested)) {
        executor_request_promote_event(
                request, RXVM_EXECUTOR_EVENT_SHUTDOWN);
    }
    state = executor_request_event_state(request->terminal_event, state);
    executor_worker_unquarantine(request->worker);
    executor_statistics_finish(executor, state);
    if (state == RXVM_EXECUTOR_REQUEST_COMPLETED && result) {
        request->result = *result;
        memset(result, 0, sizeof(*result));
        if (request->result.type == RXVM_EXECUTOR_REGISTER_INTEGER) {
            request->procedure_result = (int)request->result.integer;
        }
    } else if (result) {
        free((void *)result->bytes);
        memset(result, 0, sizeof(*result));
    }
    request->state = state;
    request->worker = 0;
    executor_publish_terminal(executor, request);
    executor_condition_broadcast(&request->changed);
    executor_mutex_unlock(&request->mutex);
}

static rxvm_executor_request *executor_worker_pop(
        rxvm_executor_worker *worker) {
    rxvm_executor_request *request;

    request = worker->queue[worker->queue_head];
    worker->queue[worker->queue_head] = 0;
    worker->queue_head = (worker->queue_head + 1u) % worker->queue_capacity;
    worker->queue_count--;
    return request;
}

static rxvm_executor_request_state executor_worker_call(
        rxvm_executor_worker *worker,
        rxvm_executor_request *request,
        rxvm_executor_register_image *result_out) {
    proc_runtime *procedure = 0;
    const char *procedure_name = request->procedure;
    rxvm_context *context = worker->context;
    rxvm_memory_worker *previous;
    proc_runtime *saved_proc;
    int saved_argc;
    value **saved_args;
    value *saved_ret;
    value **arguments = 0;
    value *return_value = 0;
    char *dummy_argv[1];
    int run_status;
    int index;

    memset(result_out, 0, sizeof(*result_out));

    if (request->uses_callable_id) {
        size_t binding_index;
        procedure_name = 0;
        for (binding_index = 0u;
             binding_index < worker->context->graph_binding_count;
             binding_index++) {
            rxvm_graph_binding *binding =
                    worker->context->graph_bindings[binding_index];
            RxGraphCallableView view;
            proc_runtime *target;

            if (!binding || request->callable_id >= binding->callable_count) {
                continue;
            }
            target = rxvm_bound_graph_callable(
                    binding, (RxCallableId)request->callable_id);
            if (!target || !rx_graph_callable(
                    binding->graph, (RxCallableId)request->callable_id,
                    &view)) {
                continue;
            }
            if (procedure_name && strcmp(procedure_name, view.symbol) != 0) {
                return RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND;
            }
            procedure_name = view.symbol;
            procedure = target;
        }
        if (!procedure_name) {
            return RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND;
        }
    }

    if (strcmp(procedure_name, "main") != 0 &&
        !src_node(context->exposed_proc_tree, (char *)procedure_name,
                  (size_t *)&procedure)) {
        return RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND;
    }

    if (!procedure) return RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND;
    previous = rxvm_memory_enter(context->worker.memory_worker);
    if (request->argc) {
        arguments = (value **)rxvm_memory_alloc_bytes(
                context->worker.memory_worker,
                sizeof(*arguments) * (size_t)request->argc);
        if (!arguments) goto setup_failed;
        memset(arguments, 0, sizeof(*arguments) * (size_t)request->argc);
    }
    for (index = 0; index < request->argc; index++) {
        const rxvm_executor_register_image *source =
                &request->arguments[index];
        arguments[index] = value_f_in(context->worker.memory_worker);
        if (!arguments[index]) goto setup_failed;
        if (source->type == RXVM_EXECUTOR_REGISTER_INTEGER) {
            char number[64];
            int number_length = snprintf(
                    number, sizeof(number), "%" PRId64, source->integer);
            if (number_length < 0 ||
                (size_t)number_length >= sizeof(number) ||
                set_string_validated(arguments[index], number,
                                     (size_t)number_length) != 0) {
                goto setup_failed;
            }
        } else if (source->type == RXVM_EXECUTOR_REGISTER_STRING) {
            if (set_string_validated(arguments[index], source->bytes,
                                     source->length) != 0) goto setup_failed;
        } else if (source->type == RXVM_EXECUTOR_REGISTER_BINARY) {
            if (set_binary(arguments[index], source->bytes,
                           source->length) != 0) goto setup_failed;
        }
    }
    return_value = value_f_in(context->worker.memory_worker);
    if (!return_value) goto setup_failed;

    saved_proc = context->ext_proc;
    saved_argc = context->ext_argc;
    saved_args = context->ext_args;
    saved_ret = context->ext_ret;
    context->ext_proc = procedure;
    context->ext_argc = request->argc;
    context->ext_args = arguments;
    context->ext_ret = return_value;
    dummy_argv[0] = (char *)"rxvm_executor";
    run_status = run(context, 0, dummy_argv);
    context->ext_proc = saved_proc;
    context->ext_argc = saved_argc;
    context->ext_args = saved_args;
    context->ext_ret = saved_ret;

    /* run() historically returns an integer procedure result as its process
     * status for an external entry.  An unhandled signal also returns a
     * non-zero status, but leaves the fresh external return cell unchanged.
     * Preserve ordinary non-zero task results and classify only a status that
     * was not published through the external integer return cell as an
     * execution failure.  Non-integer external returns normally finish with
     * status zero. */
    if (run_status != 0 &&
        (request->expected_result != RXVM_EXECUTOR_REGISTER_INTEGER ||
         return_value->int_value != (rxinteger)run_status)) {
        goto execution_failed;
    }

    result_out->type = request->expected_result;
    if (request->expected_result == RXVM_EXECUTOR_REGISTER_INTEGER) {
        result_out->integer = (int64_t)return_value->int_value;
    } else if (request->expected_result == RXVM_EXECUTOR_REGISTER_STRING ||
               request->expected_result == RXVM_EXECUTOR_REGISTER_BINARY) {
        const void *source = request->expected_result ==
                RXVM_EXECUTOR_REGISTER_STRING
                ? (const void *)return_value->string_value
                : (const void *)return_value->binary_value;
        size_t length = request->expected_result ==
                RXVM_EXECUTOR_REGISTER_STRING
                ? (size_t)return_value->string_length
                : return_value->binary_length;
        unsigned char *copy = (unsigned char *)malloc(length + 1u);
        if (!copy) goto setup_failed;
        if (length) memcpy(copy, source, length);
        copy[length] = 0;
        result_out->bytes = (const char *)copy;
        result_out->length = length;
    }

    value_free(return_value);
    for (index = 0; index < request->argc; index++) {
        value_free(arguments[index]);
    }
    (void)rxvm_memory_release(arguments);
    rxvm_memory_leave(previous);
    return RXVM_EXECUTOR_REQUEST_COMPLETED;

execution_failed:
    free((void *)result_out->bytes);
    memset(result_out, 0, sizeof(*result_out));
    value_free(return_value);
    for (index = 0; index < request->argc; index++) {
        value_free(arguments[index]);
    }
    if (arguments) (void)rxvm_memory_release(arguments);
    rxvm_memory_leave(previous);
    return RXVM_EXECUTOR_REQUEST_EXECUTION_FAILED;

setup_failed:
    free((void *)result_out->bytes);
    memset(result_out, 0, sizeof(*result_out));
    if (return_value) value_free(return_value);
    for (index = 0; index < request->argc; index++) {
        if (arguments && arguments[index]) value_free(arguments[index]);
    }
    if (arguments) (void)rxvm_memory_release(arguments);
    rxvm_memory_leave(previous);
    return RXVM_EXECUTOR_REQUEST_SETUP_FAILED;
}

static void executor_worker_run(rxvm_executor_worker *worker) {
    rxvm_executor_request *request;
    rxvm_executor_request_state state;
    rxvm_executor_register_image result;

    if (worker->executor->native_doorbell &&
        rxvm_signal_thread_doorbell_e5_prepare_current() != 0) {
        worker->startup_result = RXVM_EXECUTOR_WORKER_START_FAILED;
    }
    worker->context = worker->startup_result == RXVM_EXECUTOR_OK
            ? rxvm_context_create_in_runtime(worker->executor->runtime) : 0;
    if (worker->context) {
        worker->context->active.external_mailbox_owner = &worker->mailbox;
        worker->context->active.external_mailbox_claim =
                executor_mailbox_claim;
        if (worker->executor->compatibility_doorbell) {
            worker->context->active.compatibility_interrupts =
                    &worker->mailbox.events;
        }
    }
    if (!worker->context ||
        rxvm_program_generation_attach(worker->context,
                                       worker->executor->generation) !=
                RXVM_PROGRAM_OK ||
        rxldmodp(worker->context) < 0 ||
        rxvm_link(worker->context) != 0 ||
        rxvm_prepare(worker->context) != 0) {
        worker->startup_result = RXVM_EXECUTOR_WORKER_START_FAILED;
    } else {
        worker->startup_result = RXVM_EXECUTOR_OK;
    }

    executor_mutex_lock(&worker->mutex);
    worker->startup_complete = 1u;
    executor_condition_broadcast(&worker->changed);
    executor_mutex_unlock(&worker->mutex);

    if (worker->startup_result == RXVM_EXECUTOR_OK) {
        for (;;) {
            executor_mutex_lock(&worker->mutex);
            while (!worker->queue_count && !worker->stopping) {
                executor_condition_wait(&worker->changed, &worker->mutex);
            }
            if (!worker->queue_count && worker->stopping) {
                executor_mutex_unlock(&worker->mutex);
                break;
            }
            request = executor_worker_pop(worker);
            executor_mutex_unlock(&worker->mutex);

            executor_mutex_lock(&request->mutex);
            if (executor_atomic_load(
                        &worker->executor->shutdown_requested)) {
                executor_request_promote_event(
                        request, RXVM_EXECUTOR_EVENT_SHUTDOWN);
            }
            if (request->terminal_event != RXVM_EXECUTOR_EVENT_NONE) {
                state = executor_request_event_state(
                        request->terminal_event,
                        RXVM_EXECUTOR_REQUEST_CANCELLED);
                executor_statistics_terminal_queued(
                        worker->executor, state);
                request->state = state;
                request->worker = 0;
                executor_publish_terminal(worker->executor, request);
                executor_condition_broadcast(&request->changed);
                executor_mutex_unlock(&request->mutex);
                continue;
            }
            executor_mailbox_arm(worker, request);
            if (executor_atomic_load(
                        &worker->executor->shutdown_requested)) {
                executor_request_promote_event(
                        request, RXVM_EXECUTOR_EVENT_SHUTDOWN);
                executor_mailbox_publish(
                        worker, request, RXVM_EXECUTOR_EVENT_SHUTDOWN);
            }
            request->state = RXVM_EXECUTOR_REQUEST_RUNNING;
            executor_condition_broadcast(&request->changed);
            executor_mutex_unlock(&request->mutex);

            executor_statistics_start(worker->executor);
            if (executor_atomic_load(
                        &worker->executor->shutdown_requested)) {
                executor_request_complete(
                        request, RXVM_EXECUTOR_REQUEST_SHUTDOWN, 0);
                continue;
            }
            memset(&result, 0, sizeof(result));
            state = executor_worker_call(worker, request, &result);
            executor_request_complete(request, state, &result);
        }
    }

    if (worker->context) {
        rxvm_destroy(worker->context);
        worker->context = 0;
    }
    if (worker->executor->native_doorbell) {
        rxvm_signal_thread_doorbell_e5_release_current();
    }
    executor_mutex_lock(&worker->mutex);
    worker->stopped = 1u;
    executor_condition_broadcast(&worker->changed);
    executor_mutex_unlock(&worker->mutex);
}

#if defined(_WIN32)
static unsigned __stdcall executor_worker_entry(void *argument) {
    executor_worker_run((rxvm_executor_worker *)argument);
    return 0u;
}
static int executor_thread_start(rxvm_executor_worker *worker) {
    uintptr_t thread = _beginthreadex(0, 0u, executor_worker_entry,
                                      worker, 0u, 0);
    if (!thread) return 0;
    worker->thread = (HANDLE)thread;
    return 1;
}
static void executor_thread_join(rxvm_executor_worker *worker) {
    if (WaitForSingleObject(worker->thread, INFINITE) != WAIT_OBJECT_0) abort();
    CloseHandle(worker->thread);
    worker->thread = 0;
}
#else
static void *executor_worker_entry(void *argument) {
    executor_worker_run((rxvm_executor_worker *)argument);
    return 0;
}
static int executor_thread_start(rxvm_executor_worker *worker) {
    return pthread_create(&worker->thread, 0, executor_worker_entry, worker) == 0;
}
static void executor_thread_join(rxvm_executor_worker *worker) {
    if (pthread_join(worker->thread, 0) != 0) abort();
}
#endif

static int executor_worker_initialize(
        rxvm_executor_worker *worker,
        rxvm_executor *executor,
        size_t affinity,
        size_t queue_capacity) {
    memset(worker, 0, sizeof(*worker));
    worker->executor = executor;
    worker->affinity = affinity;
    worker->queue_capacity = queue_capacity;
    worker->queue = (rxvm_executor_request **)calloc(
            queue_capacity, sizeof(*worker->queue));
    if (!worker->queue) return 0;
    if (!executor_mutex_init(&worker->mutex)) {
        free(worker->queue);
        worker->queue = 0;
        return 0;
    }
    if (!executor_condition_init(&worker->changed)) {
        executor_mutex_destroy(&worker->mutex);
        free(worker->queue);
        worker->queue = 0;
        return 0;
    }
    worker->initialized = 1u;
    return 1;
}

static void executor_worker_storage_destroy(rxvm_executor_worker *worker) {
    if (!worker->initialized) return;
    if (worker->queue_count) abort();
    executor_condition_destroy(&worker->changed);
    executor_mutex_destroy(&worker->mutex);
    free(worker->queue);
    memset(worker, 0, sizeof(*worker));
}

static void executor_stop_and_join_workers(rxvm_executor *executor) {
    size_t i;

    for (i = 0u; i < executor->worker_count; i++) {
        rxvm_executor_worker *worker = &executor->workers[i];
        if (!worker->thread_started) continue;
        executor_mutex_lock(&worker->mutex);
        worker->stopping = 1u;
        executor_condition_broadcast(&worker->changed);
        executor_mutex_unlock(&worker->mutex);
    }
    for (i = 0u; i < executor->worker_count; i++) {
        rxvm_executor_worker *worker = &executor->workers[i];
        if (!worker->thread_started) continue;
        executor_thread_join(worker);
        worker->thread_started = 0u;
    }
}

static void executor_storage_destroy(rxvm_executor *executor) {
    size_t i;

    if (!executor) return;
    for (i = 0u; i < executor->worker_count; i++) {
        executor_worker_storage_destroy(&executor->workers[i]);
    }
    free(executor->workers);
    if (executor->statistics_mutex_initialized) {
        executor_mutex_destroy(&executor->statistics_mutex);
    }
    if (executor->completion_sync_initialized) {
        executor_condition_destroy(&executor->completion_changed);
        executor_mutex_destroy(&executor->completion_mutex);
    }
    free(executor);
}

rxvm_executor *rxvm_executor_create(
        const char *rxbin_path,
        size_t worker_count,
        size_t queue_capacity,
        rxvm_executor_result *result_out) {
    rxvm_executor *executor = 0;
    rxvm_context *source = 0;
    rxvm_executor_result result = RXVM_EXECUTOR_INVALID;
    size_t runtime_leaks = 0u;
    size_t i;

    if (result_out) *result_out = RXVM_EXECUTOR_INVALID;
    if (!rxbin_path || !*rxbin_path || !worker_count || !queue_capacity ||
        worker_count > SIZE_MAX / sizeof(*executor->workers) ||
        queue_capacity > SIZE_MAX / sizeof(*executor->workers[0].queue)) {
        return 0;
    }
    executor = (rxvm_executor *)calloc(1u, sizeof(*executor));
    if (!executor) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    if (!executor_mutex_init(&executor->statistics_mutex)) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    executor->statistics_mutex_initialized = 1u;
    if (!executor_mutex_init(&executor->completion_mutex)) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    if (!executor_condition_init(&executor->completion_changed)) {
        executor_mutex_destroy(&executor->completion_mutex);
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    executor->completion_sync_initialized = 1u;
    {
        const char *doorbell = getenv("CREXX_VM_E5_DOORBELL");
        const char *force_sparse = getenv(
                "CREXX_VM_E5_FORCE_SPARSE_OWNER");
        if (force_sparse && strcmp(force_sparse, "1") == 0) {
            executor->compatibility_doorbell = 1u;
        }
#if defined(_WIN32)
        executor->native_doorbell = !executor->compatibility_doorbell &&
                doorbell &&
                strcmp(doorbell, "windows-special-apc") == 0;
#else
        executor->native_doorbell = !executor->compatibility_doorbell &&
                doorbell &&
                strcmp(doorbell, "posix") == 0;
#endif
    }
    if (executor->native_doorbell &&
        rxvm_signal_thread_doorbell_e5_install() != 0) {
#if defined(_WIN32)
        executor->native_doorbell = 0u;
        executor->compatibility_doorbell = 1u;
#else
        result = RXVM_EXECUTOR_DOORBELL_UNAVAILABLE;
        goto fail;
#endif
    }
    executor->worker_count = worker_count;
    executor->queue_capacity = queue_capacity;
    executor->statistics.worker_count = worker_count;
    executor->statistics.queue_capacity_per_worker = queue_capacity;
    executor->workers = (rxvm_executor_worker *)calloc(
            worker_count, sizeof(*executor->workers));
    if (!executor->workers) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    executor->runtime = rxvm_runtime_create();
    if (!executor->runtime) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    executor->owns_runtime = 1u;
    source = rxvm_context_create_in_runtime(executor->runtime);
    if (!source) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    if (!rxvm_load_file(source, (char *)rxbin_path) ||
        rxvm_program_generation_seal(source, &executor->generation) !=
                RXVM_PROGRAM_OK ||
        rxldmodp(source) < 0 ||
        rxvm_link(source) != 0 || rxvm_prepare(source) != 0) {
        result = RXVM_EXECUTOR_PROGRAM_LOAD_FAILED;
        goto fail;
    }

    for (i = 0u; i < worker_count; i++) {
        rxvm_executor_worker *worker = &executor->workers[i];
        if (!executor_worker_initialize(worker, executor, i, queue_capacity)) {
            result = RXVM_EXECUTOR_OUT_OF_MEMORY;
            goto fail;
        }
        if (!executor_thread_start(worker)) {
            result = RXVM_EXECUTOR_WORKER_START_FAILED;
            goto fail;
        }
        worker->thread_started = 1u;
        executor_mutex_lock(&worker->mutex);
        while (!worker->startup_complete) {
            executor_condition_wait(&worker->changed, &worker->mutex);
        }
        result = worker->startup_result;
        executor_mutex_unlock(&worker->mutex);
        if (result != RXVM_EXECUTOR_OK) goto fail;
    }

    rxvm_destroy(source);
    source = 0;
    if (result_out) *result_out = RXVM_EXECUTOR_OK;
    return executor;

fail:
    if (executor) executor_stop_and_join_workers(executor);
    if (source) rxvm_destroy(source);
    if (executor && executor->runtime && executor->owns_runtime) {
        runtime_leaks = rxvm_runtime_destroy(executor->runtime);
        executor->runtime = 0;
        if (runtime_leaks) abort();
    }
    if (executor && executor->native_doorbell) {
        rxvm_signal_thread_doorbell_e5_uninstall();
        executor->native_doorbell = 0u;
    }
    executor_storage_destroy(executor);
    if (result_out) *result_out = result;
    return 0;
}

rxvm_executor *rxvm_executor_create_attached(
        rxvm_runtime *runtime,
        const rxvm_program_generation *generation,
        size_t worker_count,
        size_t queue_capacity,
        rxvm_executor_result *result_out) {
    rxvm_executor *executor = 0;
    rxvm_executor_result result = RXVM_EXECUTOR_INVALID;
    size_t i;

    if (result_out) *result_out = RXVM_EXECUTOR_INVALID;
    if (!runtime || !generation || !worker_count || !queue_capacity ||
        worker_count > SIZE_MAX / sizeof(*executor->workers) ||
        queue_capacity > SIZE_MAX / sizeof(*executor->workers[0].queue)) {
        return 0;
    }
    executor = (rxvm_executor *)calloc(1u, sizeof(*executor));
    if (!executor) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    if (!executor_mutex_init(&executor->statistics_mutex)) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    executor->statistics_mutex_initialized = 1u;
    if (!executor_mutex_init(&executor->completion_mutex)) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    if (!executor_condition_init(&executor->completion_changed)) {
        executor_mutex_destroy(&executor->completion_mutex);
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    executor->completion_sync_initialized = 1u;
    {
        const char *doorbell = getenv("CREXX_VM_E5_DOORBELL");
        const char *force_sparse = getenv(
                "CREXX_VM_E5_FORCE_SPARSE_OWNER");
        if (force_sparse && strcmp(force_sparse, "1") == 0) {
            executor->compatibility_doorbell = 1u;
        }
#if defined(_WIN32)
        executor->native_doorbell = !executor->compatibility_doorbell &&
                doorbell &&
                strcmp(doorbell, "windows-special-apc") == 0;
#else
        executor->native_doorbell = !executor->compatibility_doorbell &&
                doorbell &&
                strcmp(doorbell, "posix") == 0;
#endif
        if (!executor->native_doorbell &&
            !executor->compatibility_doorbell) {
            /* Local channels require cooperative cancellation on every
             * supported host, including those without a native doorbell. */
            executor->compatibility_doorbell = 1u;
        }
    }
    if (executor->native_doorbell &&
        rxvm_signal_thread_doorbell_e5_install() != 0) {
#if defined(_WIN32)
        executor->native_doorbell = 0u;
        executor->compatibility_doorbell = 1u;
#else
        result = RXVM_EXECUTOR_DOORBELL_UNAVAILABLE;
        goto fail;
#endif
    }
    executor->runtime = runtime;
    executor->generation = generation;
    executor->worker_count = worker_count;
    executor->queue_capacity = queue_capacity;
    executor->statistics.worker_count = worker_count;
    executor->statistics.queue_capacity_per_worker = queue_capacity;
    executor->workers = (rxvm_executor_worker *)calloc(
            worker_count, sizeof(*executor->workers));
    if (!executor->workers) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }

    for (i = 0u; i < worker_count; i++) {
        rxvm_executor_worker *worker = &executor->workers[i];
        if (!executor_worker_initialize(worker, executor, i, queue_capacity)) {
            result = RXVM_EXECUTOR_OUT_OF_MEMORY;
            goto fail;
        }
        if (!executor_thread_start(worker)) {
            result = RXVM_EXECUTOR_WORKER_START_FAILED;
            goto fail;
        }
        worker->thread_started = 1u;
        executor_mutex_lock(&worker->mutex);
        while (!worker->startup_complete) {
            executor_condition_wait(&worker->changed, &worker->mutex);
        }
        result = worker->startup_result;
        executor_mutex_unlock(&worker->mutex);
        if (result != RXVM_EXECUTOR_OK) goto fail;
    }

    if (result_out) *result_out = RXVM_EXECUTOR_OK;
    return executor;

fail:
    if (executor) executor_stop_and_join_workers(executor);
    if (executor && executor->native_doorbell) {
        rxvm_signal_thread_doorbell_e5_uninstall();
        executor->native_doorbell = 0u;
    }
    if (executor) executor->runtime = 0;
    executor_storage_destroy(executor);
    if (result_out) *result_out = result;
    return 0;
}

size_t rxvm_executor_destroy(rxvm_executor *executor) {
    size_t leaks = 0u;

    if (!executor) return 0u;
    executor_stop_and_join_workers(executor);
    if (executor->native_doorbell) {
        rxvm_signal_thread_doorbell_e5_uninstall();
        executor->native_doorbell = 0u;
    }
    if (executor->owns_runtime) {
        leaks = rxvm_runtime_destroy(executor->runtime);
    }
    executor->runtime = 0;
    executor_storage_destroy(executor);
    return leaks;
}

static rxvm_executor_result executor_submit_internal(
        rxvm_executor *executor,
        size_t worker_affinity,
        const char *procedure,
        size_t argument_count,
        const rxvm_executor_register_image *arguments,
        unsigned char uses_callable_id,
        uint64_t callable_id,
        rxvm_executor_register_type expected_result,
        rxvm_executor_request **request_out) {
    rxvm_executor_worker *worker;
    rxvm_executor_request *request;
    size_t tail;
    size_t index;

    if (request_out) *request_out = 0;
    if (!executor || !request_out || !procedure || !*procedure ||
        argument_count > (size_t)INT_MAX ||
        (argument_count && !arguments) ||
        worker_affinity >= executor->worker_count ||
        expected_result < RXVM_EXECUTOR_REGISTER_NONE ||
        expected_result > RXVM_EXECUTOR_REGISTER_BINARY) {
        return RXVM_EXECUTOR_INVALID;
    }
    for (index = 0u; index < argument_count; index++) {
        const rxvm_executor_register_image *argument = &arguments[index];
        if (argument->type < RXVM_EXECUTOR_REGISTER_NONE ||
            argument->type > RXVM_EXECUTOR_REGISTER_BINARY ||
            ((argument->type == RXVM_EXECUTOR_REGISTER_STRING ||
              argument->type == RXVM_EXECUTOR_REGISTER_BINARY) &&
             ((!argument->bytes && argument->length) ||
              argument->length == SIZE_MAX)) ||
            (argument->type == RXVM_EXECUTOR_REGISTER_STRING &&
             argument->length &&
             memchr(argument->bytes, 0, argument->length))) {
            return RXVM_EXECUTOR_INVALID;
        }
    }
    worker = &executor->workers[worker_affinity];
    request = executor_request_create(
            worker, procedure, argument_count, arguments);
    if (!request) return RXVM_EXECUTOR_OUT_OF_MEMORY;
    request->uses_callable_id = uses_callable_id;
    request->callable_id = callable_id;
    request->expected_result = expected_result;

    executor_mutex_lock(&worker->mutex);
    if (worker->stopping ||
        executor_atomic_load(&executor->shutdown_requested)) {
        executor_mutex_unlock(&worker->mutex);
        executor_request_storage_destroy(request);
        return RXVM_EXECUTOR_STOPPING;
    }
    if (worker->queue_count == worker->queue_capacity) {
        executor_mutex_unlock(&worker->mutex);
        executor_mutex_lock(&executor->statistics_mutex);
        executor->statistics.rejected_full_requests++;
        executor_mutex_unlock(&executor->statistics_mutex);
        executor_request_storage_destroy(request);
        return RXVM_EXECUTOR_QUEUE_FULL;
    }
    tail = (worker->queue_head + worker->queue_count) % worker->queue_capacity;
    worker->queue[tail] = request;
    worker->queue_count++;
    executor_mutex_lock(&executor->statistics_mutex);
    executor->statistics.accepted_requests++;
    executor_mutex_unlock(&executor->statistics_mutex);
    executor_condition_broadcast(&worker->changed);
    executor_mutex_unlock(&worker->mutex);
    *request_out = request;
    return RXVM_EXECUTOR_OK;
}

rxvm_executor_result rxvm_executor_submit(
        rxvm_executor *executor,
        size_t worker_affinity,
        const char *procedure,
        int argc,
        const char *const *argv,
        rxvm_executor_request **request_out) {
    rxvm_executor_register_image *arguments = 0;
    rxvm_executor_result result;
    int index;
    if (request_out) *request_out = 0;
    if (argc < 0 || (argc && !argv) ||
        (size_t)argc > SIZE_MAX / sizeof(*arguments)) {
        return RXVM_EXECUTOR_INVALID;
    }
    if (argc) {
        arguments = (rxvm_executor_register_image *)calloc(
                (size_t)argc, sizeof(*arguments));
        if (!arguments) return RXVM_EXECUTOR_OUT_OF_MEMORY;
    }
    for (index = 0; index < argc; index++) {
        arguments[index].type = RXVM_EXECUTOR_REGISTER_STRING;
        arguments[index].bytes = argv[index] ? argv[index] : "";
        arguments[index].length = strlen(arguments[index].bytes);
    }
    result = executor_submit_internal(
            executor, worker_affinity, procedure, (size_t)argc, arguments,
            0u, 0u, RXVM_EXECUTOR_REGISTER_INTEGER, request_out);
    free(arguments);
    return result;
}

static rxvm_executor_result executor_submit_registers_internal(
        rxvm_executor *executor,
        size_t worker_affinity,
        const char *procedure,
        unsigned char uses_callable_id,
        uint64_t callable_id,
        size_t argument_count,
        const rxvm_executor_register_image *arguments,
        rxvm_executor_register_type expected_result,
        rxvm_executor_request **request_out) {
    return executor_submit_internal(
            executor, worker_affinity, procedure, argument_count, arguments,
            uses_callable_id, callable_id, expected_result, request_out);
}

rxvm_executor_result rxvm_executor_submit_registers(
        rxvm_executor *executor,
        size_t worker_affinity,
        const char *procedure,
        size_t argument_count,
        const rxvm_executor_register_image *arguments,
        rxvm_executor_request **request_out) {
    return executor_submit_registers_internal(
            executor, worker_affinity, procedure, 0u, 0u,
            argument_count, arguments, RXVM_EXECUTOR_REGISTER_INTEGER,
            request_out);
}

rxvm_executor_result rxvm_executor_submit_callable_registers(
        rxvm_executor *executor,
        size_t worker_affinity,
        uint64_t callable_id,
        size_t argument_count,
        const rxvm_executor_register_image *arguments,
        rxvm_executor_request **request_out) {
    if (callable_id > UINT32_MAX) {
        if (request_out) *request_out = 0;
        return RXVM_EXECUTOR_INVALID;
    }
    return executor_submit_registers_internal(
            executor, worker_affinity, "@semantic-callable", 1u,
            callable_id, argument_count, arguments,
            RXVM_EXECUTOR_REGISTER_INTEGER, request_out);
}

rxvm_executor_result rxvm_executor_submit_callable_registers_result(
        rxvm_executor *executor,
        size_t worker_affinity,
        uint64_t callable_id,
        size_t argument_count,
        const rxvm_executor_register_image *arguments,
        rxvm_executor_register_type expected_result,
        rxvm_executor_request **request_out) {
    if (callable_id > UINT32_MAX) {
        if (request_out) *request_out = 0;
        return RXVM_EXECUTOR_INVALID;
    }
    return executor_submit_registers_internal(
            executor, worker_affinity, "@semantic-callable", 1u,
            callable_id, argument_count, arguments, expected_result,
            request_out);
}

static rxvm_executor_result executor_request_event(
        rxvm_executor_request *request,
        rxvm_executor_event event) {
    rxvm_executor_worker *worker;
    rxvm_executor_event previous_event;
    size_t i;

    if (!request) return RXVM_EXECUTOR_INVALID;
    executor_mutex_lock(&request->mutex);
    if (request_state_is_terminal(request->state)) {
        executor_mutex_unlock(&request->mutex);
        return RXVM_EXECUTOR_ALREADY_TERMINAL;
    }
    worker = request->worker;
    if (!worker) {
        executor_mutex_unlock(&request->mutex);
        return RXVM_EXECUTOR_INVALID;
    }
    previous_event = request->terminal_event;
    executor_request_promote_event(request, event);
    executor_mutex_lock(&worker->mutex);
    for (i = 0u; i < worker->queue_count; i++) {
        size_t index = (worker->queue_head + i) % worker->queue_capacity;
        if (worker->queue[index] == request) {
            size_t move;
            for (move = i; move + 1u < worker->queue_count; move++) {
                size_t from = (worker->queue_head + move + 1u) %
                              worker->queue_capacity;
                size_t to = (worker->queue_head + move) %
                            worker->queue_capacity;
                worker->queue[to] = worker->queue[from];
            }
            worker->queue[(worker->queue_head + worker->queue_count - 1u) %
                          worker->queue_capacity] = 0;
            worker->queue_count--;
            request->state = executor_request_event_state(
                    request->terminal_event,
                    RXVM_EXECUTOR_REQUEST_CANCELLED);
            executor_statistics_terminal_queued(
                    worker->executor, request->state);
            request->worker = 0;
            executor_publish_terminal(worker->executor, request);
            executor_condition_broadcast(&request->changed);
            executor_mutex_unlock(&worker->mutex);
            executor_mutex_unlock(&request->mutex);
            return RXVM_EXECUTOR_OK;
        }
    }
    executor_mutex_unlock(&worker->mutex);
    if (request->state == RXVM_EXECUTOR_REQUEST_RUNNING) {
        /* The request mutex is the arm/disarm authority. While it is held,
         * completion cannot retire this request and reuse the VM. */
        executor_mailbox_publish(worker, request, event);
        if (event == RXVM_EXECUTOR_EVENT_DEADLINE ||
            event == RXVM_EXECUTOR_EVENT_KILL ||
            event == RXVM_EXECUTOR_EVENT_SHUTDOWN) {
            executor_worker_quarantine(worker);
        }
        if (!executor_worker_ring(worker)) {
            request->terminal_event = previous_event;
            (void)executor_atomic_exchange(&worker->mailbox.events, 0);
            if (event == RXVM_EXECUTOR_EVENT_DEADLINE ||
                event == RXVM_EXECUTOR_EVENT_KILL ||
                event == RXVM_EXECUTOR_EVENT_SHUTDOWN) {
                int previous_quarantine =
                        previous_event == RXVM_EXECUTOR_EVENT_DEADLINE ||
                        previous_event == RXVM_EXECUTOR_EVENT_KILL ||
                        previous_event == RXVM_EXECUTOR_EVENT_SHUTDOWN;
                if (!previous_quarantine) {
                    executor_worker_unquarantine(worker);
                }
            }
            if (previous_event != RXVM_EXECUTOR_EVENT_NONE) {
                executor_mailbox_publish(
                        worker, request, previous_event);
            }
            executor_mutex_unlock(&request->mutex);
            return RXVM_EXECUTOR_INVALID;
        }
    }
    executor_mutex_unlock(&request->mutex);
    return RXVM_EXECUTOR_OK;
}

rxvm_executor_result rxvm_executor_cancel(
        rxvm_executor_request *request) {
    return executor_request_event(request, RXVM_EXECUTOR_EVENT_CANCEL);
}

rxvm_executor_result rxvm_executor_kill(
        rxvm_executor_request *request) {
    return executor_request_event(request, RXVM_EXECUTOR_EVENT_KILL);
}

rxvm_executor_result rxvm_executor_expire(
        rxvm_executor_request *request) {
    return executor_request_event(request, RXVM_EXECUTOR_EVENT_DEADLINE);
}

rxvm_executor_result rxvm_executor_shutdown(rxvm_executor *executor) {
    rxvm_executor_result result = RXVM_EXECUTOR_OK;
    size_t i;

    if (!executor) return RXVM_EXECUTOR_INVALID;
    if (executor_atomic_exchange(&executor->shutdown_requested, 1)) {
        return RXVM_EXECUTOR_OK;
    }
    for (i = 0u; i < executor->worker_count; i++) {
        rxvm_executor_worker *worker = &executor->workers[i];
        sig_atomic_t generation;

        if (!worker->thread_started) continue;
        executor_mutex_lock(&worker->mutex);
        worker->stopping = 1u;
        executor_condition_broadcast(&worker->changed);
        executor_mutex_unlock(&worker->mutex);

        generation = executor_atomic_load(
                &worker->mailbox.active_generation);
        if (!generation) continue;
        executor_atomic_store(&worker->mailbox.published_generation,
                              generation);
        executor_atomic_or(&worker->mailbox.events,
                           RXVM_EXECUTOR_EVENT_SHUTDOWN);
        executor_worker_quarantine(worker);
        if (executor_atomic_load(&worker->mailbox.active_generation) !=
                generation) {
            executor_worker_unquarantine(worker);
            continue;
        }
        if (!executor_worker_ring(worker)) result = RXVM_EXECUTOR_INVALID;
    }
    return result;
}

rxvm_executor_request_state rxvm_executor_request_wait(
        rxvm_executor_request *request,
        int *procedure_result_out) {
    rxvm_executor_request_state state;

    if (!request) return RXVM_EXECUTOR_REQUEST_SETUP_FAILED;
    executor_mutex_lock(&request->mutex);
    while (!request_state_is_terminal(request->state)) {
        executor_condition_wait(&request->changed, &request->mutex);
    }
    state = request->state;
    if (procedure_result_out) *procedure_result_out = request->procedure_result;
    executor_mutex_unlock(&request->mutex);
    return state;
}

rxvm_executor_request_state rxvm_executor_request_wait_completion(
        rxvm_executor_request *request,
        rxvm_executor_completion *completion_out) {
    rxvm_executor_request_state state = rxvm_executor_request_wait(
            request, 0);

    if (completion_out) {
        memset(completion_out, 0, sizeof(*completion_out));
        completion_out->state = state;
        if (state == RXVM_EXECUTOR_REQUEST_COMPLETED) {
            completion_out->result = request->result;
        }
    }
    return state;
}

rxvm_executor_request_state rxvm_executor_request_wait_started(
        rxvm_executor_request *request) {
    rxvm_executor_request_state state;

    if (!request) return RXVM_EXECUTOR_REQUEST_SETUP_FAILED;
    executor_mutex_lock(&request->mutex);
    while (request->state == RXVM_EXECUTOR_REQUEST_QUEUED) {
        executor_condition_wait(&request->changed, &request->mutex);
    }
    state = request->state;
    executor_mutex_unlock(&request->mutex);
    return state;
}

rxvm_executor_request_state rxvm_executor_request_state_get(
        rxvm_executor_request *request) {
    rxvm_executor_request_state state;

    if (!request) return RXVM_EXECUTOR_REQUEST_SETUP_FAILED;
    executor_mutex_lock(&request->mutex);
    state = request->state;
    executor_mutex_unlock(&request->mutex);
    return state;
}

int rxvm_executor_request_completion_snapshot(
        rxvm_executor_request *request,
        rxvm_executor_completion *completion_out,
        uint64_t *completion_sequence_out) {
    int terminal;

    if (completion_out) memset(completion_out, 0, sizeof(*completion_out));
    if (completion_sequence_out) *completion_sequence_out = 0u;
    if (!request) return 0;
    executor_mutex_lock(&request->mutex);
    terminal = request_state_is_terminal(request->state);
    if (terminal) {
        if (completion_out) {
            completion_out->state = request->state;
            if (request->state == RXVM_EXECUTOR_REQUEST_COMPLETED) {
                completion_out->result = request->result;
            }
        }
        if (completion_sequence_out) {
            *completion_sequence_out = request->completion_sequence;
        }
    }
    executor_mutex_unlock(&request->mutex);
    return terminal;
}

uint64_t rxvm_executor_completion_generation_get(
        rxvm_executor *executor) {
    uint64_t generation;

    if (!executor) return 0u;
    executor_mutex_lock(&executor->completion_mutex);
    generation = executor->completion_generation;
    executor_mutex_unlock(&executor->completion_mutex);
    return generation;
}

int rxvm_executor_completion_generation_wait(
        rxvm_executor *executor,
        uint64_t observed_generation,
        int64_t wait_microseconds,
        uint64_t *generation_out) {
    uint64_t deadline = 0u;
    int result = 1;

    if (generation_out) *generation_out = observed_generation;
    if (!executor || wait_microseconds < -1) return -1;
    if (wait_microseconds > 0) {
        uint64_t now = executor_monotonic_microseconds();
        uint64_t duration = (uint64_t)wait_microseconds;
        deadline = duration > UINT64_MAX - now ? UINT64_MAX : now + duration;
    }
    executor_mutex_lock(&executor->completion_mutex);
    while (executor->completion_generation == observed_generation) {
        if (wait_microseconds == 0) {
            result = 0;
            break;
        }
        if (wait_microseconds < 0) {
            executor_condition_wait(&executor->completion_changed,
                                    &executor->completion_mutex);
        } else {
            uint64_t now = executor_monotonic_microseconds();
            int64_t remaining;
            if (now >= deadline) {
                result = 0;
                break;
            }
            remaining = deadline - now > (uint64_t)INT64_MAX
                    ? INT64_MAX : (int64_t)(deadline - now);
            if (!executor_condition_wait_for(
                    &executor->completion_changed,
                    &executor->completion_mutex, remaining) &&
                executor->completion_generation == observed_generation) {
                result = 0;
                break;
            }
        }
    }
    if (generation_out) {
        *generation_out = executor->completion_generation;
    }
    executor_mutex_unlock(&executor->completion_mutex);
    return result;
}

size_t rxvm_executor_request_affinity(
        const rxvm_executor_request *request) {
    return request ? request->affinity : SIZE_MAX;
}

rxvm_executor_result rxvm_executor_request_destroy(
        rxvm_executor_request *request) {
    rxvm_executor_request_state state;

    if (!request) return RXVM_EXECUTOR_INVALID;
    executor_mutex_lock(&request->mutex);
    state = request->state;
    executor_mutex_unlock(&request->mutex);
    if (!request_state_is_terminal(state)) return RXVM_EXECUTOR_INVALID;
    executor_request_storage_destroy(request);
    return RXVM_EXECUTOR_OK;
}

void rxvm_executor_statistics_get(
        rxvm_executor *executor,
        rxvm_executor_statistics *statistics_out) {
    if (!executor || !statistics_out) return;
    executor_mutex_lock(&executor->statistics_mutex);
    *statistics_out = executor->statistics;
    executor_mutex_unlock(&executor->statistics_mutex);
}

const char *rxvm_executor_doorbell_backend_name(
        const rxvm_executor *executor) {
    if (!executor) return "none";
    if (executor->native_doorbell) return "native";
    if (executor->compatibility_doorbell) return "sparse-owner";
    return "none";
}

const char *rxvm_executor_result_name(rxvm_executor_result result) {
    switch (result) {
        case RXVM_EXECUTOR_OK: return "ok";
        case RXVM_EXECUTOR_INVALID: return "invalid";
        case RXVM_EXECUTOR_OUT_OF_MEMORY: return "out-of-memory";
        case RXVM_EXECUTOR_PROGRAM_LOAD_FAILED: return "program-load-failed";
        case RXVM_EXECUTOR_WORKER_START_FAILED: return "worker-start-failed";
        case RXVM_EXECUTOR_QUEUE_FULL: return "queue-full";
        case RXVM_EXECUTOR_STOPPING: return "stopping";
        case RXVM_EXECUTOR_ALREADY_TERMINAL: return "already-terminal";
        case RXVM_EXECUTOR_DOORBELL_UNAVAILABLE: return "doorbell-unavailable";
        default: return "unknown";
    }
}

const char *rxvm_executor_request_state_name(
        rxvm_executor_request_state state) {
    switch (state) {
        case RXVM_EXECUTOR_REQUEST_QUEUED: return "queued";
        case RXVM_EXECUTOR_REQUEST_RUNNING: return "running";
        case RXVM_EXECUTOR_REQUEST_COMPLETED: return "completed";
        case RXVM_EXECUTOR_REQUEST_CANCELLED: return "cancelled";
        case RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND:
            return "procedure-not-found";
        case RXVM_EXECUTOR_REQUEST_SETUP_FAILED: return "setup-failed";
        case RXVM_EXECUTOR_REQUEST_DEADLINE_EXCEEDED:
            return "deadline-exceeded";
        case RXVM_EXECUTOR_REQUEST_KILLED: return "killed";
        case RXVM_EXECUTOR_REQUEST_SHUTDOWN: return "shutdown";
        case RXVM_EXECUTOR_REQUEST_EXECUTION_FAILED:
            return "execution-failed";
        default: return "unknown";
    }
}
