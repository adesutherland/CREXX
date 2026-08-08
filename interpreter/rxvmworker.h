/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMWORKER_H
#define CREXX_RXVMWORKER_H

#include "rxvmmemory.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rxvm_runtime rxvm_runtime;

typedef enum rxvm_worker_state {
    RXVM_WORKER_UNINITIALIZED = 0,
    RXVM_WORKER_IDLE = 1,
    RXVM_WORKER_RUNNING = 2,
    RXVM_WORKER_DRAINING = 3,
    RXVM_WORKER_STOPPED = 4
} rxvm_worker_state;

typedef enum rxvm_worker_transition_result {
    RXVM_WORKER_TRANSITION_OK = 0,
    RXVM_WORKER_TRANSITION_WRONG_THREAD = 1,
    RXVM_WORKER_TRANSITION_INVALID_STATE = 2
} rxvm_worker_transition_result;

/*
 * The runtime is the execution domain. Gate E1 gives each public VM context
 * its own runtime; later Gate E slices may register multiple workers with one
 * runtime without changing the worker-local allocator contract.
 */
rxvm_runtime *rxvm_runtime_create(void);
size_t rxvm_runtime_destroy(rxvm_runtime *runtime);
rxvm_memory_context *rxvm_runtime_memory_context(rxvm_runtime *runtime);
size_t rxvm_runtime_worker_count(const rxvm_runtime *runtime);

/* One worker is permanently affine to the thread that initializes it. */
typedef struct rxvm_worker {
    rxvm_runtime *runtime;
    rxvm_memory_worker *memory_worker;
    uintptr_t owner_thread_token;
    size_t execution_depth;
    rxvm_worker_state state;
} rxvm_worker;

int rxvm_worker_initialize(rxvm_worker *worker, rxvm_runtime *runtime);
size_t rxvm_worker_destroy(rxvm_worker *worker);

int rxvm_worker_is_current_thread_owner(const rxvm_worker *worker);
rxvm_worker_state rxvm_worker_get_state(const rxvm_worker *worker);
const char *rxvm_worker_state_name(rxvm_worker_state state);

rxvm_worker_transition_result rxvm_worker_begin_execution(rxvm_worker *worker);
rxvm_worker_transition_result rxvm_worker_end_execution(rxvm_worker *worker);
rxvm_worker_transition_result rxvm_worker_begin_draining(rxvm_worker *worker);

#ifdef __cplusplus
}
#endif

#endif
