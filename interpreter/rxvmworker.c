/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmworker.h"

#include <stdlib.h>

#ifdef _WIN32
#define RXVM_WORKER_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_WORKER_THREAD_LOCAL __thread
#endif

struct rxvm_runtime {
    rxvm_memory_context *memory_context;
    size_t worker_count;
};

static RXVM_WORKER_THREAD_LOCAL unsigned char rxvm_worker_thread_marker;

static uintptr_t rxvm_worker_current_thread_token(void) {
    return (uintptr_t)&rxvm_worker_thread_marker;
}

rxvm_runtime *rxvm_runtime_create(void) {
    rxvm_runtime *runtime = (rxvm_runtime *)calloc(1, sizeof(*runtime));
    if (!runtime) return 0;
    runtime->memory_context = rxvm_memory_context_create();
    if (!runtime->memory_context) {
        free(runtime);
        return 0;
    }
    return runtime;
}

size_t rxvm_runtime_destroy(rxvm_runtime *runtime) {
    size_t leaks;
    if (!runtime) return 0;
    if (runtime->worker_count != 0u) {
        /* Destroying a live worker domain would invalidate worker arenas. */
        abort();
    }
    leaks = rxvm_memory_context_destroy(runtime->memory_context);
    runtime->memory_context = 0;
    free(runtime);
    return leaks;
}

rxvm_memory_context *rxvm_runtime_memory_context(rxvm_runtime *runtime) {
    return runtime ? runtime->memory_context : 0;
}

size_t rxvm_runtime_worker_count(const rxvm_runtime *runtime) {
    return runtime ? runtime->worker_count : 0u;
}

int rxvm_worker_initialize(rxvm_worker *worker, rxvm_runtime *runtime) {
    if (!worker || !runtime || !runtime->memory_context) return 0;
    worker->runtime = runtime;
    worker->memory_worker =
            rxvm_memory_worker_create(runtime->memory_context);
    if (!worker->memory_worker) {
        worker->runtime = 0;
        worker->owner_thread_token = 0;
        worker->execution_depth = 0u;
        worker->state = RXVM_WORKER_UNINITIALIZED;
        return 0;
    }
    worker->owner_thread_token = rxvm_worker_current_thread_token();
    worker->execution_depth = 0u;
    worker->state = RXVM_WORKER_IDLE;
    runtime->worker_count++;
    return 1;
}

size_t rxvm_worker_destroy(rxvm_worker *worker) {
    size_t leaks;
    rxvm_runtime *runtime;
    if (!worker || worker->state == RXVM_WORKER_UNINITIALIZED ||
        worker->state == RXVM_WORKER_STOPPED) {
        return 0;
    }
    if (!rxvm_worker_is_current_thread_owner(worker) ||
        worker->state != RXVM_WORKER_DRAINING) {
        abort();
    }
    runtime = worker->runtime;
    leaks = rxvm_memory_worker_destroy(worker->memory_worker);
    worker->memory_worker = 0;
    worker->owner_thread_token = 0;
    worker->execution_depth = 0u;
    worker->state = RXVM_WORKER_STOPPED;
    if (!runtime || runtime->worker_count == 0u) abort();
    runtime->worker_count--;
    return leaks;
}

int rxvm_worker_is_current_thread_owner(const rxvm_worker *worker) {
    return worker && worker->owner_thread_token != 0u &&
           worker->owner_thread_token == rxvm_worker_current_thread_token();
}

rxvm_worker_state rxvm_worker_get_state(const rxvm_worker *worker) {
    return worker ? worker->state : RXVM_WORKER_UNINITIALIZED;
}

const char *rxvm_worker_state_name(rxvm_worker_state state) {
    switch (state) {
        case RXVM_WORKER_UNINITIALIZED: return "uninitialized";
        case RXVM_WORKER_IDLE: return "idle";
        case RXVM_WORKER_RUNNING: return "running";
        case RXVM_WORKER_DRAINING: return "draining";
        case RXVM_WORKER_STOPPED: return "stopped";
        default: return "invalid";
    }
}

static rxvm_worker_transition_result rxvm_worker_validate_owner(
        const rxvm_worker *worker) {
    if (!rxvm_worker_is_current_thread_owner(worker)) {
        return RXVM_WORKER_TRANSITION_WRONG_THREAD;
    }
    return RXVM_WORKER_TRANSITION_OK;
}

rxvm_worker_transition_result rxvm_worker_begin_execution(rxvm_worker *worker) {
    rxvm_worker_transition_result result = rxvm_worker_validate_owner(worker);
    if (result != RXVM_WORKER_TRANSITION_OK) return result;
    if (worker->state == RXVM_WORKER_IDLE) {
        worker->execution_depth = 1u;
        worker->state = RXVM_WORKER_RUNNING;
        return RXVM_WORKER_TRANSITION_OK;
    }
    if (worker->state == RXVM_WORKER_RUNNING &&
        worker->execution_depth != 0u &&
        worker->execution_depth != (size_t)-1) {
        worker->execution_depth++;
        return RXVM_WORKER_TRANSITION_OK;
    }
    return RXVM_WORKER_TRANSITION_INVALID_STATE;
}

rxvm_worker_transition_result rxvm_worker_end_execution(rxvm_worker *worker) {
    rxvm_worker_transition_result result = rxvm_worker_validate_owner(worker);
    if (result != RXVM_WORKER_TRANSITION_OK) return result;
    if (worker->state != RXVM_WORKER_RUNNING ||
        worker->execution_depth == 0u) {
        return RXVM_WORKER_TRANSITION_INVALID_STATE;
    }
    worker->execution_depth--;
    if (worker->execution_depth == 0u) worker->state = RXVM_WORKER_IDLE;
    return RXVM_WORKER_TRANSITION_OK;
}

rxvm_worker_transition_result rxvm_worker_begin_draining(rxvm_worker *worker) {
    rxvm_worker_transition_result result = rxvm_worker_validate_owner(worker);
    if (result != RXVM_WORKER_TRANSITION_OK) return result;
    if (worker->state != RXVM_WORKER_IDLE ||
        worker->execution_depth != 0u) {
        return RXVM_WORKER_TRANSITION_INVALID_STATE;
    }
    worker->state = RXVM_WORKER_DRAINING;
    return RXVM_WORKER_TRANSITION_OK;
}
