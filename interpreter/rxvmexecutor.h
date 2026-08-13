/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMEXECUTOR_H
#define CREXX_RXVMEXECUTOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PERF3-13 E5 private proof surface.  This is deliberately not installed and
 * is not part of the public RXVML ABI.
 */
typedef struct rxvm_executor rxvm_executor;
typedef struct rxvm_executor_request rxvm_executor_request;

typedef enum rxvm_executor_result {
    RXVM_EXECUTOR_OK = 0,
    RXVM_EXECUTOR_INVALID = 1,
    RXVM_EXECUTOR_OUT_OF_MEMORY = 2,
    RXVM_EXECUTOR_PROGRAM_LOAD_FAILED = 3,
    RXVM_EXECUTOR_WORKER_START_FAILED = 4,
    RXVM_EXECUTOR_QUEUE_FULL = 5,
    RXVM_EXECUTOR_STOPPING = 6,
    RXVM_EXECUTOR_ALREADY_TERMINAL = 7,
    RXVM_EXECUTOR_DOORBELL_UNAVAILABLE = 8
} rxvm_executor_result;

typedef enum rxvm_executor_request_state {
    RXVM_EXECUTOR_REQUEST_QUEUED = 0,
    RXVM_EXECUTOR_REQUEST_RUNNING = 1,
    RXVM_EXECUTOR_REQUEST_COMPLETED = 2,
    RXVM_EXECUTOR_REQUEST_CANCELLED = 3,
    RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND = 4,
    RXVM_EXECUTOR_REQUEST_SETUP_FAILED = 5
} rxvm_executor_request_state;

typedef struct rxvm_executor_statistics {
    size_t worker_count;
    size_t queue_capacity_per_worker;
    size_t accepted_requests;
    size_t completed_requests;
    size_t cancelled_requests;
    size_t failed_requests;
    size_t rejected_full_requests;
    size_t running_requests;
    size_t maximum_parallel_requests;
} rxvm_executor_statistics;

/*
 * Load and seal RXBIN_PATH, then start WORKER_COUNT persistent, fixed-affinity
 * workers.  Each bounded queue has QUEUE_CAPACITY entries.
 */
rxvm_executor *rxvm_executor_create(
        const char *rxbin_path,
        size_t worker_count,
        size_t queue_capacity,
        rxvm_executor_result *result_out);

/*
 * Close submission, drain every accepted non-cancelled request, join workers
 * and destroy the E4 runtime. Request handles may outlive the executor only
 * after they have reached a terminal state.
 */
size_t rxvm_executor_destroy(rxvm_executor *executor);

/* Copy the procedure name and NUL-terminated string arguments into a request. */
rxvm_executor_result rxvm_executor_submit(
        rxvm_executor *executor,
        size_t worker_affinity,
        const char *procedure,
        int argc,
        const char *const *argv,
        rxvm_executor_request **request_out);

/*
 * Queued cancellation removes the request immediately. Running cancellation
 * becomes terminal when the current VM call returns to its request boundary.
 */
rxvm_executor_result rxvm_executor_cancel(
        rxvm_executor_request *request);

/* Wait for a terminal state and return its procedure result when completed. */
rxvm_executor_request_state rxvm_executor_request_wait(
        rxvm_executor_request *request,
        int *procedure_result_out);

/* Wait only until execution starts or the request reaches a terminal state. */
rxvm_executor_request_state rxvm_executor_request_wait_started(
        rxvm_executor_request *request);

rxvm_executor_request_state rxvm_executor_request_state_get(
        rxvm_executor_request *request);
size_t rxvm_executor_request_affinity(
        const rxvm_executor_request *request);

/* A request may be destroyed only after wait() reports a terminal state. */
rxvm_executor_result rxvm_executor_request_destroy(
        rxvm_executor_request *request);

void rxvm_executor_statistics_get(
        rxvm_executor *executor,
        rxvm_executor_statistics *statistics_out);

/* Private PoC observation surface: "native", "sparse-owner" or "none". */
const char *rxvm_executor_doorbell_backend_name(
        const rxvm_executor *executor);

const char *rxvm_executor_result_name(rxvm_executor_result result);
const char *rxvm_executor_request_state_name(
        rxvm_executor_request_state state);

#ifdef __cplusplus
}
#endif

#endif
