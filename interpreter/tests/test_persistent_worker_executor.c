/*
 * PERF3-13 E5 persistent fixed-affinity worker proof.
 *
 * The private executor moves copied logical inputs and integer completions;
 * it never shares RXVM values, frames or mutable module state between workers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

void rxvm_signal_thread_doorbell_poc_statistics(
        unsigned long *callback_count,
        unsigned long *maximum_depth);

#include "rxvm.h"
#include "rxvmexecutor.h"

static int failures;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (!(condition)) {                                                   \
            fprintf(stderr, "FAIL: %s\n", (message));                       \
            failures++;                                                       \
        }                                                                     \
    } while (0)

static rxvm_executor_request *submit_one(
        rxvm_executor *executor,
        size_t affinity,
        const char *procedure,
        const char *argument) {
    const char *arguments[1];
    rxvm_executor_request *request = 0;
    rxvm_executor_result result;

    arguments[0] = argument;
    result = rxvm_executor_submit(executor, affinity, procedure, 1,
                                  arguments, &request);
    if (result != RXVM_EXECUTOR_OK) {
        fprintf(stderr, "FAIL: submit %s to worker %zu: %s\n", procedure,
                affinity, rxvm_executor_result_name(result));
        failures++;
        return 0;
    }
    CHECK(request != 0, "accepted submission returns a request handle");
    CHECK(rxvm_executor_request_affinity(request) == affinity,
          "request retains its fixed worker affinity");
    return request;
}

static rxvm_executor_request *submit_zero(
        rxvm_executor *executor,
        size_t affinity,
        const char *procedure) {
    rxvm_executor_request *request = 0;
    rxvm_executor_result result;

    result = rxvm_executor_submit(executor, affinity, procedure, 0, 0,
                                  &request);
    if (result != RXVM_EXECUTOR_OK) {
        fprintf(stderr, "FAIL: submit %s to worker %zu: %s\n", procedure,
                affinity, rxvm_executor_result_name(result));
        failures++;
        return 0;
    }
    CHECK(request != 0, "accepted zero-argument submission returns a handle");
    CHECK(rxvm_executor_request_affinity(request) == affinity,
          "zero-argument request retains fixed worker affinity");
    return request;
}

static int wait_completed(rxvm_executor_request *request,
                          int expected,
                          const char *description) {
    rxvm_executor_request_state state;
    int result = 0;

    if (!request) return 0;
    state = rxvm_executor_request_wait(request, &result);
    if (state != RXVM_EXECUTOR_REQUEST_COMPLETED || result != expected) {
        fprintf(stderr,
                "FAIL: %s: state=%s result=%d expected=%d\n",
                description, rxvm_executor_request_state_name(state), result,
                expected);
        failures++;
        return 0;
    }
    return 1;
}

static void destroy_terminal(rxvm_executor_request **request) {
    if (!request || !*request) return;
    CHECK(rxvm_executor_request_destroy(*request) == RXVM_EXECUTOR_OK,
          "destroy a terminal request handle");
    *request = 0;
}

static void run_affinity_and_isolation(const char *rxbin) {
    rxvm_executor *executor;
    rxvm_executor_result create_result;
    rxvm_executor_request *spin0 = 0;
    rxvm_executor_request *spin1 = 0;
    rxvm_executor_request *recursive = 0;
    rxvm_executor_request *add0 = 0;
    rxvm_executor_request *add1 = 0;
    rxvm_executor_request *again0 = 0;
    rxvm_executor_request *again1 = 0;
    rxvm_executor_request *get0 = 0;
    rxvm_executor_request *get1 = 0;
    rxvm_executor_request *missing = 0;
    rxvm_executor_request *recovery = 0;
    rxvm_executor_statistics statistics;
    rxvm_executor_request_state state;
    int result = 0;
    size_t leaks;

    executor = rxvm_executor_create(rxbin, 2u, 16u, &create_result);
    CHECK(executor && create_result == RXVM_EXECUTOR_OK,
          "create two persistent E5 workers over one sealed generation");
    if (!executor) return;

    /* Worker zero cannot reach a call boundary without the VM interrupt. */
    spin0 = submit_zero(executor, 0u, "e5worker.loop_forever");
    spin1 = submit_one(executor, 1u, "e5worker.spin", "3000000");
    if (spin0) {
        state = rxvm_executor_request_wait_started(spin0);
        CHECK(state == RXVM_EXECUTOR_REQUEST_RUNNING,
              "worker zero starts its request on the persistent thread");
    }
    if (spin1) {
        state = rxvm_executor_request_wait_started(spin1);
        CHECK(state == RXVM_EXECUTOR_REQUEST_RUNNING,
              "worker one starts while worker zero is executing");
    }
    if (spin0) {
        CHECK(rxvm_executor_cancel(spin0) == RXVM_EXECUTOR_OK,
              "running cancellation request is accepted");
        state = rxvm_executor_request_wait(spin0, &result);
        CHECK(state == RXVM_EXECUTOR_REQUEST_CANCELLED,
              "running cancellation stops an otherwise infinite VM request");
        CHECK(rxvm_executor_cancel(spin0) == RXVM_EXECUTOR_ALREADY_TERMINAL,
              "a terminal request cannot receive a second cancellation");
    }
    wait_completed(spin1, 3000000,
                   "the other worker completes independently");
    destroy_terminal(&spin0);
    destroy_terminal(&spin1);

    recursive = submit_zero(executor, 0u, "e5worker.recurse_forever");
    if (recursive) {
        state = rxvm_executor_request_wait_started(recursive);
        CHECK(state == RXVM_EXECUTOR_REQUEST_RUNNING,
              "recursive request starts on its persistent worker");
        CHECK(rxvm_executor_cancel(recursive) == RXVM_EXECUTOR_OK,
              "recursive cancellation request is accepted");
        state = rxvm_executor_request_wait(recursive, &result);
        CHECK(state == RXVM_EXECUTOR_REQUEST_CANCELLED,
              "call-boundary safepoints stop otherwise infinite recursion");
    }
    destroy_terminal(&recursive);

    add0 = submit_one(executor, 0u, "e5worker.add", "5");
    add1 = submit_one(executor, 1u, "e5worker.add", "11");
    wait_completed(add0, 5, "worker zero initializes its private module global");
    wait_completed(add1, 11, "worker one initializes its private module global");
    destroy_terminal(&add0);
    destroy_terminal(&add1);

    again0 = submit_one(executor, 0u, "e5worker.add", "2");
    again1 = submit_one(executor, 1u, "e5worker.add", "3");
    wait_completed(again0, 7,
                   "worker zero retains warm state on the same affinity");
    wait_completed(again1, 14,
                   "worker one retains distinct warm state on its affinity");
    destroy_terminal(&again0);
    destroy_terminal(&again1);

    get0 = submit_zero(executor, 0u, "e5worker.get");
    get1 = submit_zero(executor, 1u, "e5worker.get");
    wait_completed(get0, 7, "worker zero global remains isolated");
    wait_completed(get1, 14, "worker one global remains isolated");
    destroy_terminal(&get0);
    destroy_terminal(&get1);

    CHECK(rxvm_executor_submit(executor, 0u, "e5worker.missing", 0, 0,
                               &missing) == RXVM_EXECUTOR_OK,
          "accept a request whose logical procedure is absent");
    if (missing) {
        state = rxvm_executor_request_wait(missing, &result);
        CHECK(state == RXVM_EXECUTOR_REQUEST_PROCEDURE_NOT_FOUND,
              "missing procedure produces a typed terminal failure");
        CHECK(rxvm_executor_request_wait(missing, &result) == state,
              "terminal completion is stable across a repeated wait");
    }
    destroy_terminal(&missing);
    recovery = submit_one(executor, 0u, "e5worker.identity", "37");
    wait_completed(recovery, 37,
                   "worker remains usable after a failed request");
    destroy_terminal(&recovery);

    rxvm_executor_statistics_get(executor, &statistics);
    CHECK(statistics.worker_count == 2u &&
              statistics.queue_capacity_per_worker == 16u,
          "executor reports the selected fixed worker topology");
    CHECK(statistics.maximum_parallel_requests == 2u,
          "two persistent workers execute VM requests simultaneously");
    CHECK(statistics.running_requests == 0u,
          "no request remains running after the isolation panel");
    CHECK(statistics.cancelled_requests == 2u,
          "loop and recursive cancellation each publish one completion");
    CHECK(statistics.failed_requests == 1u,
          "missing procedure contributes exactly one failed completion");

    leaks = rxvm_executor_destroy(executor);
    CHECK(leaks == 0u,
          "two-worker sealed runtime tears down with zero live allocations");
}

static void run_backpressure_copy_and_drain(const char *rxbin) {
    rxvm_executor *executor;
    rxvm_executor_result create_result;
    rxvm_executor_result submit_result;
    rxvm_executor_request *running = 0;
    rxvm_executor_request *queued = 0;
    rxvm_executor_request *rejected = 0;
    rxvm_executor_request *copied = 0;
    rxvm_executor_request *drained = 0;
    rxvm_executor_request_state state;
    char copied_argument[8] = "41";
    int result = 0;
    size_t leaks;

    executor = rxvm_executor_create(rxbin, 1u, 1u, &create_result);
    CHECK(executor && create_result == RXVM_EXECUTOR_OK,
          "create one persistent worker with a one-entry bounded queue");
    if (!executor) return;

    running = submit_zero(executor, 0u, "e5worker.loop_forever");
    if (running) {
        state = rxvm_executor_request_wait_started(running);
        CHECK(state == RXVM_EXECUTOR_REQUEST_RUNNING,
              "backpressure control occupies the worker");
    }
    queued = submit_one(executor, 0u, "e5worker.identity", "17");
    submit_result = rxvm_executor_submit(executor, 0u, "e5worker.identity",
                                         1, (const char *const[]){"19"},
                                         &rejected);
    CHECK(submit_result == RXVM_EXECUTOR_QUEUE_FULL && rejected == 0,
          "bounded queue rejects excess work with explicit backpressure");
    if (queued) {
        CHECK(rxvm_executor_cancel(queued) == RXVM_EXECUTOR_OK,
              "queued cancellation removes the request immediately");
        state = rxvm_executor_request_wait(queued, &result);
        CHECK(state == RXVM_EXECUTOR_REQUEST_CANCELLED,
              "queued cancellation publishes one terminal completion");
    }

    copied = submit_one(executor, 0u, "e5worker.identity", copied_argument);
    strcpy(copied_argument, "99");
    CHECK(rxvm_executor_cancel(running) == RXVM_EXECUTOR_OK,
          "running backpressure control accepts cooperative cancellation");
    state = rxvm_executor_request_wait(running, &result);
    CHECK(state == RXVM_EXECUTOR_REQUEST_CANCELLED,
          "running control cancels at its request boundary");
    wait_completed(copied, 41,
                   "cancel bit is disarmed and the next copied request runs");
    destroy_terminal(&running);
    destroy_terminal(&queued);
    destroy_terminal(&copied);

    drained = submit_one(executor, 0u, "e5worker.identity", "23");
    leaks = rxvm_executor_destroy(executor);
    CHECK(leaks == 0u,
          "shutdown drains and joins the persistent worker without leaks");
    wait_completed(drained, 23,
                   "shutdown drains an accepted request before returning");
    destroy_terminal(&drained);

}

static uint64_t monotonic_nanoseconds(void) {
#if defined(_WIN32)
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;
    uint64_t whole_seconds;
    uint64_t remainder;

    if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0 ||
        !QueryPerformanceCounter(&counter) || counter.QuadPart < 0) {
        abort();
    }
    whole_seconds = (uint64_t)counter.QuadPart /
                    (uint64_t)frequency.QuadPart;
    remainder = (uint64_t)counter.QuadPart %
                (uint64_t)frequency.QuadPart;
    return whole_seconds * 1000000000ULL +
           (remainder * 1000000000ULL) / (uint64_t)frequency.QuadPart;
#else
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) abort();
    return (uint64_t)now.tv_sec * 1000000000ULL + (uint64_t)now.tv_nsec;
#endif
}

static int parse_positive_size(const char *text, size_t *value_out) {
    char *end = 0;
    unsigned long long value;

    if (!text || !*text || !value_out) return 0;
    value = strtoull(text, &end, 10);
    if (!end || *end || !value || value > (unsigned long long)SIZE_MAX) {
        return 0;
    }
    *value_out = (size_t)value;
    return 1;
}

static int run_benchmark_direct(const char *rxbin,
                                size_t jobs,
                                const char *iterations,
                                uint64_t *elapsed_out,
                                long long *checksum_out) {
    struct rxvm_context *context = rxvm_create();
    char *arguments[1];
    uint64_t started;
    size_t job;

    if (!context) return 0;
    if (!rxvm_load_file(context, (char *)rxbin) ||
        rxvm_link(context) != 0 || rxvm_prepare(context) != 0) {
        rxvm_destroy(context);
        return 0;
    }
    arguments[0] = (char *)iterations;
    started = monotonic_nanoseconds();
    for (job = 0u; job < jobs; job++) {
        *checksum_out += rxvm_call(context, "e5worker.spin", 1, arguments);
    }
    *elapsed_out = monotonic_nanoseconds() - started;
    rxvm_destroy(context);
    return 1;
}

static int run_benchmark_executor(const char *rxbin,
                                  size_t worker_count,
                                  size_t jobs,
                                  const char *iterations,
                                  uint64_t *elapsed_out,
                                  long long *checksum_out,
                                  size_t *maximum_parallel_out) {
    rxvm_executor *executor;
    rxvm_executor_request **requests;
    rxvm_executor_result create_result;
    rxvm_executor_statistics statistics;
    uint64_t started;
    size_t job;
    int ok = 1;

    executor = rxvm_executor_create(rxbin, worker_count, jobs, &create_result);
    if (!executor || create_result != RXVM_EXECUTOR_OK) return 0;
    requests = (rxvm_executor_request **)calloc(jobs, sizeof(*requests));
    if (!requests) {
        (void)rxvm_executor_destroy(executor);
        return 0;
    }

    started = monotonic_nanoseconds();
    for (job = 0u; job < jobs; job++) {
        const char *arguments[1] = {iterations};
        if (rxvm_executor_submit(executor, job % worker_count,
                                 "e5worker.spin", 1, arguments,
                                 &requests[job]) != RXVM_EXECUTOR_OK) {
            ok = 0;
            break;
        }
    }
    for (job = 0u; job < jobs; job++) {
        int result = 0;
        if (!requests[job]) continue;
        if (rxvm_executor_request_wait(requests[job], &result) !=
                RXVM_EXECUTOR_REQUEST_COMPLETED) {
            ok = 0;
        }
        *checksum_out += result;
    }
    *elapsed_out = monotonic_nanoseconds() - started;
    rxvm_executor_statistics_get(executor, &statistics);
    *maximum_parallel_out = statistics.maximum_parallel_requests;
    for (job = 0u; job < jobs; job++) {
        if (requests[job] &&
            rxvm_executor_request_destroy(requests[job]) != RXVM_EXECUTOR_OK) {
            ok = 0;
        }
    }
    free(requests);
    if (rxvm_executor_destroy(executor) != 0u) ok = 0;
    return ok;
}

static int run_benchmark(int argc, char **argv) {
    const char *mode;
    const char *rxbin;
    size_t jobs;
    size_t iterations;
    size_t worker_count = 0u;
    size_t maximum_parallel = 0u;
    uint64_t elapsed = 0u;
    long long checksum = 0;
    double jobs_per_second;
    int ok;

    if (argc != 6 || !parse_positive_size(argv[4], &jobs) ||
        !parse_positive_size(argv[5], &iterations)) {
        fprintf(stderr,
                "usage: %s --benchmark direct|executor1|executor2 "
                "E5_RXBIN JOBS ITERATIONS\n", argv[0]);
        return 2;
    }
    mode = argv[2];
    rxbin = argv[3];
    if (strcmp(mode, "direct") == 0) {
        ok = run_benchmark_direct(rxbin, jobs, argv[5], &elapsed, &checksum);
    } else {
        if (strcmp(mode, "executor1") == 0) worker_count = 1u;
        else if (strcmp(mode, "executor2") == 0) worker_count = 2u;
        else {
            fprintf(stderr, "ERROR: unknown E5 benchmark mode: %s\n", mode);
            return 2;
        }
        ok = run_benchmark_executor(rxbin, worker_count, jobs, argv[5],
                                    &elapsed, &checksum, &maximum_parallel);
    }
    if (!ok || !elapsed ||
        checksum != (long long)jobs * (long long)iterations ||
        (worker_count && maximum_parallel != worker_count)) {
        fprintf(stderr,
                "E5_BENCHMARK result=FAIL mode=%s jobs=%zu iterations=%zu "
                "elapsed_ns=%llu checksum=%lld max_parallel=%zu\n",
                mode, jobs, iterations, (unsigned long long)elapsed, checksum,
                maximum_parallel);
        return 1;
    }
    jobs_per_second = ((double)jobs * 1000000000.0) / (double)elapsed;
    printf("E5_RATE: %.9f jobs_per_second\n", jobs_per_second);
    printf("E5_BENCHMARK result=PASS mode=%s workers=%zu jobs=%zu "
           "iterations=%zu elapsed_ns=%llu checksum=%lld max_parallel=%zu\n",
           mode, worker_count, jobs, iterations, (unsigned long long)elapsed,
           checksum, maximum_parallel);
    return 0;
}

static int compare_u64(const void *left, const void *right) {
    const uint64_t a = *(const uint64_t *)left;
    const uint64_t b = *(const uint64_t *)right;
    return a < b ? -1 : a > b;
}

static int run_doorbell_latency(int argc, char **argv) {
    rxvm_executor_result create_result;
    rxvm_executor *executor;
    uint64_t *samples;
    size_t sample_count;
    size_t sample;
    size_t p95_index;
    int ok = 1;

    if (argc != 4 || !parse_positive_size(argv[3], &sample_count)) {
        fprintf(stderr, "usage: %s --doorbell-latency E5_RXBIN SAMPLES\n",
                argv[0]);
        return 2;
    }
    samples = (uint64_t *)calloc(sample_count, sizeof(*samples));
    if (!samples) return 1;
    executor = rxvm_executor_create(argv[2], 1u, 1u, &create_result);
    if (!executor || create_result != RXVM_EXECUTOR_OK) {
        free(samples);
        return 1;
    }

    for (sample = 0u; sample < sample_count; sample++) {
        rxvm_executor_request *request = submit_zero(
                executor, 0u, "e5worker.loop_forever");
        rxvm_executor_request_state state;
        uint64_t started;
        int result = 0;

        if (!request || rxvm_executor_request_wait_started(request) !=
                            RXVM_EXECUTOR_REQUEST_RUNNING) {
            ok = 0;
            break;
        }
        started = monotonic_nanoseconds();
        if (rxvm_executor_cancel(request) != RXVM_EXECUTOR_OK) {
            ok = 0;
            break;
        }
        state = rxvm_executor_request_wait(request, &result);
        samples[sample] = monotonic_nanoseconds() - started;
        if (state != RXVM_EXECUTOR_REQUEST_CANCELLED ||
            rxvm_executor_request_destroy(request) != RXVM_EXECUTOR_OK) {
            ok = 0;
            break;
        }
        printf("E5_DOORBELL_SAMPLE sample=%zu latency_ns=%llu\n", sample,
               (unsigned long long)samples[sample]);
    }

    if (rxvm_executor_destroy(executor) != 0u) ok = 0;
    if (!ok || sample != sample_count) {
        free(samples);
        fprintf(stderr, "E5_DOORBELL_LATENCY result=FAIL completed=%zu\n",
                sample);
        return 1;
    }
    qsort(samples, sample_count, sizeof(*samples), compare_u64);
    p95_index = (sample_count * 95u + 99u) / 100u;
    if (p95_index) p95_index--;
    printf("E5_DOORBELL_LATENCY result=PASS samples=%zu min_ns=%llu "
           "median_ns=%llu p95_ns=%llu max_ns=%llu\n",
           sample_count,
           (unsigned long long)samples[0],
           (unsigned long long)samples[sample_count / 2u],
           (unsigned long long)samples[p95_index],
           (unsigned long long)samples[sample_count - 1u]);
    free(samples);
    return 0;
}

static int run_doorbell_fallback(int argc, char **argv) {
    rxvm_executor_result create_result = RXVM_EXECUTOR_OK;
    rxvm_executor *executor;
    rxvm_executor_request *cancelled = NULL;
    rxvm_executor_request *reuse = NULL;
    int result = 0;
    int ok = 1;

    if (argc != 3) {
        fprintf(stderr, "usage: %s --doorbell-fallback E5_RXBIN\n",
                argv[0]);
        return 2;
    }
    executor = rxvm_executor_create(argv[2], 1u, 1u, &create_result);
    if (!executor || create_result != RXVM_EXECUTOR_OK ||
        strcmp(rxvm_executor_doorbell_backend_name(executor),
               "sparse-owner") != 0) {
        if (executor && rxvm_executor_destroy(executor) != 0u) abort();
        fprintf(stderr, "E5_DOORBELL_FALLBACK result=FAIL create_result=%s\n",
                rxvm_executor_result_name(create_result));
        return 1;
    }

    cancelled = submit_zero(executor, 0u, "e5worker.loop_forever");
    if (!cancelled ||
        rxvm_executor_request_wait_started(cancelled) !=
                RXVM_EXECUTOR_REQUEST_RUNNING ||
        rxvm_executor_cancel(cancelled) != RXVM_EXECUTOR_OK ||
        rxvm_executor_request_wait(cancelled, &result) !=
                RXVM_EXECUTOR_REQUEST_CANCELLED ||
        rxvm_executor_request_destroy(cancelled) != RXVM_EXECUTOR_OK) {
        ok = 0;
    }
    reuse = submit_one(executor, 0u, "e5worker.identity", "31");
    if (!reuse ||
        rxvm_executor_request_wait(reuse, &result) !=
                RXVM_EXECUTOR_REQUEST_COMPLETED ||
        result != 31 ||
        rxvm_executor_request_destroy(reuse) != RXVM_EXECUTOR_OK) {
        ok = 0;
    }
    if (rxvm_executor_destroy(executor) != 0u) ok = 0;
    printf("E5_DOORBELL_FALLBACK backend=sparse-owner cancellation=%s "
           "no_spill=%s result=%s\n",
           ok ? "PASS" : "FAIL", ok ? "PASS" : "FAIL",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

static int run_sparse_progress(int argc, char **argv) {
    static const char *const procedures[] = {
        "e5sparse.conditional_forever",
        "e5sparse.counted_forever",
        "e5sparse.indirect_forever"
    };
    rxvm_executor_result create_result = RXVM_EXECUTOR_OK;
    rxvm_executor *executor;
    size_t procedure_index;

    if (argc != 3) {
        fprintf(stderr, "usage: %s --sparse-progress E5_SPARSE_RXBIN\n",
                argv[0]);
        return 2;
    }
    executor = rxvm_executor_create(argv[2], 1u, 1u, &create_result);
    if (!executor || create_result != RXVM_EXECUTOR_OK ||
        strcmp(rxvm_executor_doorbell_backend_name(executor),
               "sparse-owner") != 0) {
        if (executor && rxvm_executor_destroy(executor) != 0u) abort();
        fprintf(stderr, "E5_SPARSE_PROGRESS result=FAIL create_result=%s\n",
                rxvm_executor_result_name(create_result));
        return 1;
    }

    for (procedure_index = 0u;
         procedure_index < sizeof(procedures) / sizeof(procedures[0]);
         procedure_index++) {
        rxvm_executor_request *request = submit_zero(
                executor, 0u, procedures[procedure_index]);
        rxvm_executor_request_state started;
        rxvm_executor_request_state state;
        rxvm_executor_result cancel_result = RXVM_EXECUTOR_INVALID;
        int result = 0;

        started = request
                ? rxvm_executor_request_wait_started(request)
                : RXVM_EXECUTOR_REQUEST_SETUP_FAILED;
        if (request && started == RXVM_EXECUTOR_REQUEST_RUNNING) {
            cancel_result = rxvm_executor_cancel(request);
        }
        if (!request || started != RXVM_EXECUTOR_REQUEST_RUNNING ||
            cancel_result != RXVM_EXECUTOR_OK) {
            fprintf(stderr,
                    "FAIL: sparse request at %s: started=%s cancel=%s\n",
                    procedures[procedure_index],
                    rxvm_executor_request_state_name(started),
                    rxvm_executor_result_name(cancel_result));
            if (request) {
                state = rxvm_executor_request_wait(request, &result);
                if (rxvm_executor_request_destroy(request) != RXVM_EXECUTOR_OK) {
                    fprintf(stderr,
                            "FAIL: sparse request cleanup at %s: state=%s\n",
                            procedures[procedure_index],
                            rxvm_executor_request_state_name(state));
                }
            }
            failures++;
            break;
        }
        state = rxvm_executor_request_wait(request, &result);
        if (state != RXVM_EXECUTOR_REQUEST_CANCELLED ||
            rxvm_executor_request_destroy(request) != RXVM_EXECUTOR_OK) {
            fprintf(stderr, "FAIL: sparse cancellation at %s: state=%s\n",
                    procedures[procedure_index],
                    rxvm_executor_request_state_name(state));
            failures++;
            break;
        }
    }

    if (!failures) {
        rxvm_executor_request *returns = submit_zero(
                executor, 0u, "e5sparse.return_chain");
        wait_completed(returns, 37,
                       "all sparse-owner bytecode return forms complete");
        destroy_terminal(&returns);
    }
    if (rxvm_executor_destroy(executor) != 0u) failures++;
    printf("E5_SPARSE_PROGRESS conditional=%s counted=%s indirect=%s "
           "returns=%s result=%s\n",
           failures ? "FAIL" : "PASS", failures ? "FAIL" : "PASS",
           failures ? "FAIL" : "PASS", failures ? "FAIL" : "PASS",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}

static int run_doorbell_stress(int argc, char **argv) {
    rxvm_executor_result create_result;
    rxvm_executor *executor;
    size_t rounds;
    size_t round;
    unsigned long callbacks_before = 0u;
    unsigned long callbacks_after = 0u;
    unsigned long maximum_depth = 0u;
    const char *backend;
    int ok = 1;

    if (argc != 4 || !parse_positive_size(argv[3], &rounds)) {
        fprintf(stderr, "usage: %s --doorbell-stress E5_RXBIN ROUNDS\n",
                argv[0]);
        return 2;
    }
    rxvm_signal_thread_doorbell_poc_statistics(&callbacks_before, NULL);
    executor = rxvm_executor_create(argv[2], 2u, 2u, &create_result);
    if (!executor || create_result != RXVM_EXECUTOR_OK) return 1;
    backend = rxvm_executor_doorbell_backend_name(executor);

    for (round = 0u; round < rounds; round++) {
        const size_t affinity = round % 2u;
        rxvm_executor_request *request = submit_zero(
                executor, affinity, "e5worker.loop_forever");
        rxvm_executor_request *reuse = NULL;
        int result = 0;

        if (!request || rxvm_executor_request_wait_started(request) !=
                            RXVM_EXECUTOR_REQUEST_RUNNING ||
            rxvm_executor_cancel(request) != RXVM_EXECUTOR_OK ||
            rxvm_executor_request_wait(request, &result) !=
                            RXVM_EXECUTOR_REQUEST_CANCELLED ||
            rxvm_executor_request_destroy(request) != RXVM_EXECUTOR_OK) {
            ok = 0;
            break;
        }
        reuse = submit_one(executor, affinity, "e5worker.identity", "29");
        if (!reuse || rxvm_executor_request_wait(reuse, &result) !=
                            RXVM_EXECUTOR_REQUEST_COMPLETED ||
            result != 29 ||
            rxvm_executor_request_destroy(reuse) != RXVM_EXECUTOR_OK) {
            ok = 0;
            break;
        }
    }
    if (rxvm_executor_destroy(executor) != 0u) ok = 0;
    rxvm_signal_thread_doorbell_poc_statistics(
            &callbacks_after, &maximum_depth);
#if defined(_WIN32)
    if (strcmp(backend, "native") == 0 &&
        (callbacks_after - callbacks_before < (unsigned long)rounds ||
         maximum_depth < 1u)) {
        ok = 0;
    }
#endif
    printf("E5_DOORBELL_STRESS result=%s backend=%s rounds=%zu callbacks=%lu "
           "maximum_depth=%lu\n",
           ok ? "PASS" : "FAIL", backend, round,
           callbacks_after - callbacks_before,
           maximum_depth);
    return ok ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--benchmark") == 0) {
        return run_benchmark(argc, argv);
    }
    if (argc > 1 && strcmp(argv[1], "--doorbell-latency") == 0) {
        return run_doorbell_latency(argc, argv);
    }
    if (argc > 1 && strcmp(argv[1], "--doorbell-fallback") == 0) {
        return run_doorbell_fallback(argc, argv);
    }
    if (argc > 1 && strcmp(argv[1], "--doorbell-stress") == 0) {
        return run_doorbell_stress(argc, argv);
    }
    if (argc > 1 && strcmp(argv[1], "--sparse-progress") == 0) {
        return run_sparse_progress(argc, argv);
    }
    if (argc != 2) {
        fprintf(stderr, "usage: %s E5_RXBIN\n", argv[0]);
        return 2;
    }

    run_affinity_and_isolation(argv[1]);
    run_backpressure_copy_and_drain(argv[1]);
    if (failures) {
        fprintf(stderr, "E5_PERSISTENT_WORKERS result=FAIL failures=%d\n",
                failures);
        return 1;
    }
    printf("E5_PERSISTENT_WORKERS concurrency=PASS affinity=PASS "
           "copy=PASS backpressure=PASS cancellation=PASS "
           "failure_isolation=PASS drain_join=PASS teardown=PASS result=PASS\n");
    return 0;
}
