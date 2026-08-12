/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmexecutor.h"

#include "rxvm.h"
#include "rxvmintp.h"
#include "rxvmprogram.h"
#include "rxvmworker.h"
#include "rxastree.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Isolated PERF3-13 E5 PoC hooks; deliberately not in the RXVML header. */
int rxvm_signal_thread_doorbell_poc_install(void);
void rxvm_signal_thread_doorbell_poc_uninstall(void);
int rxvm_signal_thread_doorbell_poc_prepare_current(void);
void rxvm_signal_thread_doorbell_poc_discard_current(void);
void rxvm_signal_thread_doorbell_poc_release_current(void);

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
static void executor_condition_broadcast(rxvm_executor_condition *condition) {
    WakeAllConditionVariable(condition);
}
#else
#include <pthread.h>
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
    return pthread_cond_init(condition, 0) == 0;
}
static void executor_condition_destroy(rxvm_executor_condition *condition) {
    if (pthread_cond_destroy(condition) != 0) abort();
}
static void executor_condition_wait(rxvm_executor_condition *condition,
                                    rxvm_executor_mutex *mutex) {
    if (pthread_cond_wait(condition, mutex) != 0) abort();
}
static void executor_condition_broadcast(rxvm_executor_condition *condition) {
    if (pthread_cond_broadcast(condition) != 0) abort();
}
#endif

typedef struct rxvm_executor_worker rxvm_executor_worker;

struct rxvm_executor_request {
    rxvm_executor_worker *worker;
    char *procedure;
    char **argv;
    int argc;
    int procedure_result;
    size_t affinity;
    unsigned char cancel_requested;
    rxvm_executor_request_state state;
    rxvm_executor_mutex mutex;
    rxvm_executor_condition changed;
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
    rxvm_executor_mutex mutex;
    rxvm_executor_condition changed;
    rxvm_executor_result startup_result;
    unsigned char initialized;
    unsigned char thread_started;
    unsigned char startup_complete;
    unsigned char stopping;
    unsigned char stopped;
};

struct rxvm_executor {
    rxvm_runtime *runtime;
    const rxvm_program_generation *generation;
    rxvm_executor_worker *workers;
    size_t worker_count;
    size_t queue_capacity;
    rxvm_executor_statistics statistics;
    rxvm_executor_mutex statistics_mutex;
    unsigned char statistics_mutex_initialized;
    unsigned char native_doorbell;
};

static int request_state_is_terminal(rxvm_executor_request_state state) {
    return state == RXVM_EXECUTOR_REQUEST_COMPLETED ||
           state == RXVM_EXECUTOR_REQUEST_CANCELLED ||
           state == RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND ||
           state == RXVM_EXECUTOR_REQUEST_SETUP_FAILED;
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
    for (i = 0; i < request->argc; i++) free(request->argv[i]);
    free(request->argv);
    free(request->procedure);
    executor_condition_destroy(&request->changed);
    executor_mutex_destroy(&request->mutex);
    free(request);
}

static rxvm_executor_request *executor_request_create(
        rxvm_executor_worker *worker,
        const char *procedure,
        int argc,
        const char *const *argv) {
    rxvm_executor_request *request;
    int i;

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
    if (argc > 0) {
        if ((size_t)argc > SIZE_MAX / sizeof(*request->argv)) {
            executor_request_storage_destroy(request);
            return 0;
        }
        request->argv = (char **)calloc((size_t)argc,
                                        sizeof(*request->argv));
        if (!request->argv) {
            executor_request_storage_destroy(request);
            return 0;
        }
        for (i = 0; i < argc; i++) {
            request->argv[i] = executor_copy_string(argv[i]);
            if (!request->argv[i]) {
                request->argc = i;
                executor_request_storage_destroy(request);
                return 0;
            }
        }
    }
    request->worker = worker;
    request->argc = argc;
    request->affinity = worker->affinity;
    request->state = RXVM_EXECUTOR_REQUEST_QUEUED;
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
    } else {
        executor->statistics.failed_requests++;
    }
    executor_mutex_unlock(&executor->statistics_mutex);
}

static void executor_request_complete(
        rxvm_executor_request *request,
        rxvm_executor_request_state state,
        int procedure_result) {
    rxvm_executor *executor = request->worker->executor;

    executor_mutex_lock(&request->mutex);
    /* Disarm this request while cancellation is excluded by the same mutex.
     * A late post after rxvm_call() returned must never reach the next request
     * assigned to this persistent VM. */
    if (executor->native_doorbell) {
        rxvm_signal_thread_doorbell_poc_discard_current();
    }
    if (request->cancel_requested) state = RXVM_EXECUTOR_REQUEST_CANCELLED;
    executor_statistics_finish(executor, state);
    request->procedure_result = procedure_result;
    request->state = state;
    request->worker = 0;
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
        int *procedure_result) {
    proc_runtime *procedure = 0;

    if (strcmp(request->procedure, "main") != 0 &&
        !src_node(worker->context->exposed_proc_tree, request->procedure,
                  (size_t *)&procedure)) {
        return RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND;
    }
    *procedure_result = rxvm_call(worker->context, request->procedure,
                                  request->argc, request->argv);
    return RXVM_EXECUTOR_REQUEST_COMPLETED;
}

static void executor_worker_run(rxvm_executor_worker *worker) {
    rxvm_executor_request *request;
    rxvm_executor_request_state state;
    int procedure_result;

    if (worker->executor->native_doorbell &&
        rxvm_signal_thread_doorbell_poc_prepare_current() != 0) {
        worker->startup_result = RXVM_EXECUTOR_WORKER_START_FAILED;
    }
    worker->context = worker->startup_result == RXVM_EXECUTOR_OK
            ? rxvm_context_create_in_runtime(worker->executor->runtime) : 0;
    if (!worker->context ||
        rxvm_program_generation_attach(worker->context,
                                       worker->executor->generation) !=
                RXVM_PROGRAM_OK ||
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
            if (request->cancel_requested) {
                executor_mutex_lock(&worker->executor->statistics_mutex);
                worker->executor->statistics.cancelled_requests++;
                executor_mutex_unlock(&worker->executor->statistics_mutex);
                request->state = RXVM_EXECUTOR_REQUEST_CANCELLED;
                request->worker = 0;
                executor_condition_broadcast(&request->changed);
                executor_mutex_unlock(&request->mutex);
                continue;
            }
            request->state = RXVM_EXECUTOR_REQUEST_RUNNING;
            executor_condition_broadcast(&request->changed);
            executor_mutex_unlock(&request->mutex);

            executor_statistics_start(worker->executor);
            procedure_result = 0;
            state = executor_worker_call(worker, request, &procedure_result);
            executor_request_complete(request, state, procedure_result);
        }
    }

    if (worker->context) {
        rxvm_destroy(worker->context);
        worker->context = 0;
    }
    if (worker->executor->native_doorbell) {
        rxvm_signal_thread_doorbell_poc_release_current();
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
    {
        const char *doorbell = getenv("CREXX_VM_POC_DOORBELL");
        executor->native_doorbell = doorbell &&
                strcmp(doorbell, "posix") == 0;
    }
    if (executor->native_doorbell &&
        rxvm_signal_thread_doorbell_poc_install() != 0) {
        result = RXVM_EXECUTOR_WORKER_START_FAILED;
        goto fail;
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
    source = rxvm_context_create_in_runtime(executor->runtime);
    if (!source) {
        result = RXVM_EXECUTOR_OUT_OF_MEMORY;
        goto fail;
    }
    if (!rxvm_load_file(source, (char *)rxbin_path) ||
        rxvm_link(source) != 0 || rxvm_prepare(source) != 0 ||
        rxvm_program_generation_seal(source, &executor->generation) !=
                RXVM_PROGRAM_OK) {
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
    if (executor && executor->runtime) {
        runtime_leaks = rxvm_runtime_destroy(executor->runtime);
        executor->runtime = 0;
        if (runtime_leaks) abort();
    }
    if (executor && executor->native_doorbell) {
        rxvm_signal_thread_doorbell_poc_uninstall();
        executor->native_doorbell = 0u;
    }
    executor_storage_destroy(executor);
    if (result_out) *result_out = result;
    return 0;
}

size_t rxvm_executor_destroy(rxvm_executor *executor) {
    size_t leaks;

    if (!executor) return 0u;
    executor_stop_and_join_workers(executor);
    if (executor->native_doorbell) {
        rxvm_signal_thread_doorbell_poc_uninstall();
        executor->native_doorbell = 0u;
    }
    leaks = rxvm_runtime_destroy(executor->runtime);
    executor->runtime = 0;
    executor_storage_destroy(executor);
    return leaks;
}

rxvm_executor_result rxvm_executor_submit(
        rxvm_executor *executor,
        size_t worker_affinity,
        const char *procedure,
        int argc,
        const char *const *argv,
        rxvm_executor_request **request_out) {
    rxvm_executor_worker *worker;
    rxvm_executor_request *request;
    size_t tail;

    if (request_out) *request_out = 0;
    if (!executor || !request_out || !procedure || !*procedure || argc < 0 ||
        (argc > 0 && !argv) || worker_affinity >= executor->worker_count) {
        return RXVM_EXECUTOR_INVALID;
    }
    worker = &executor->workers[worker_affinity];
    request = executor_request_create(worker, procedure, argc, argv);
    if (!request) return RXVM_EXECUTOR_OUT_OF_MEMORY;

    executor_mutex_lock(&worker->mutex);
    if (worker->stopping) {
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

rxvm_executor_result rxvm_executor_cancel(
        rxvm_executor_request *request) {
    rxvm_executor_worker *worker;
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
    request->cancel_requested = 1u;
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
            executor_mutex_lock(&worker->executor->statistics_mutex);
            worker->executor->statistics.cancelled_requests++;
            executor_mutex_unlock(&worker->executor->statistics_mutex);
            request->state = RXVM_EXECUTOR_REQUEST_CANCELLED;
            request->worker = 0;
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
        if (!worker->executor->native_doorbell) {
            request->cancel_requested = 0u;
            executor_mutex_unlock(&request->mutex);
            return RXVM_EXECUTOR_INVALID;
        }
#if defined(__APPLE__)
        if (pthread_kill(worker->thread, SIGURG) != 0) {
            request->cancel_requested = 0u;
            executor_mutex_unlock(&request->mutex);
            return RXVM_EXECUTOR_INVALID;
        }
#else
        request->cancel_requested = 0u;
        executor_mutex_unlock(&request->mutex);
        return RXVM_EXECUTOR_INVALID;
#endif
    }
    executor_mutex_unlock(&request->mutex);
    return RXVM_EXECUTOR_OK;
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
        default: return "unknown";
    }
}
