/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmworker.h"

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

static int failures;

#define CHECK(condition, message)                                               \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "FAIL: %s\n", (message));                         \
            failures++;                                                        \
        }                                                                       \
    } while (0)

typedef struct foreign_thread_probe {
    rxvm_worker *worker;
    int is_owner;
    int memory_is_owner;
    rxvm_worker_transition_result transition;
} foreign_thread_probe;

#ifdef _WIN32
static DWORD WINAPI probe_foreign_thread(LPVOID argument) {
    foreign_thread_probe *probe = (foreign_thread_probe *)argument;
    probe->is_owner = rxvm_worker_is_current_thread_owner(probe->worker);
    probe->memory_is_owner = rxvm_memory_worker_is_current_thread_owner(
            probe->worker->memory_worker);
    probe->transition = rxvm_worker_begin_execution(probe->worker);
    return 0;
}
#else
static void *probe_foreign_thread(void *argument) {
    foreign_thread_probe *probe = (foreign_thread_probe *)argument;
    probe->is_owner = rxvm_worker_is_current_thread_owner(probe->worker);
    probe->memory_is_owner = rxvm_memory_worker_is_current_thread_owner(
            probe->worker->memory_worker);
    probe->transition = rxvm_worker_begin_execution(probe->worker);
    return 0;
}
#endif

static void run_foreign_thread_probe(foreign_thread_probe *probe) {
#ifdef _WIN32
    HANDLE thread = CreateThread(0, 0, probe_foreign_thread, probe, 0, 0);
    CHECK(thread != 0, "foreign worker probe thread starts");
    if (thread) {
        CHECK(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0,
              "foreign worker probe thread joins");
        CloseHandle(thread);
    }
#else
    pthread_t thread;
    int create_result = pthread_create(&thread, 0, probe_foreign_thread, probe);
    CHECK(create_result == 0, "foreign worker probe thread starts");
    if (create_result == 0) {
        CHECK(pthread_join(thread, 0) == 0,
              "foreign worker probe thread joins");
    }
#endif
}

int main(void) {
    rxvm_runtime *runtime = rxvm_runtime_create();
    rxvm_worker worker = {0};
    foreign_thread_probe probe = {0};

    CHECK(runtime != 0, "runtime creates its memory domain");
    if (!runtime) return 1;
    CHECK(rxvm_runtime_worker_count(runtime) == 0u,
          "new runtime has no workers");
    CHECK(rxvm_worker_initialize(&worker, runtime),
          "worker registers with runtime");
    CHECK(rxvm_runtime_worker_count(runtime) == 1u,
          "runtime counts the registered worker");
    CHECK(rxvm_worker_is_current_thread_owner(&worker),
          "initializing thread owns the worker");
    CHECK(rxvm_worker_get_state(&worker) == RXVM_WORKER_IDLE,
          "initialized worker is idle");

    CHECK(rxvm_worker_begin_execution(&worker) == RXVM_WORKER_TRANSITION_OK,
          "owner starts execution");
    CHECK(rxvm_worker_get_state(&worker) == RXVM_WORKER_RUNNING,
          "worker enters running state");
    CHECK(rxvm_worker_begin_execution(&worker) == RXVM_WORKER_TRANSITION_OK,
          "owner may enter a nested same-thread VM call");
    CHECK(rxvm_worker_end_execution(&worker) == RXVM_WORKER_TRANSITION_OK,
          "owner ends nested execution");
    CHECK(rxvm_worker_get_state(&worker) == RXVM_WORKER_RUNNING,
          "outer execution remains running after nested return");
    CHECK(rxvm_worker_end_execution(&worker) == RXVM_WORKER_TRANSITION_OK,
          "owner ends outer execution");

    probe.worker = &worker;
    probe.is_owner = 1;
    probe.memory_is_owner = 1;
    probe.transition = RXVM_WORKER_TRANSITION_OK;
    run_foreign_thread_probe(&probe);
    CHECK(!probe.is_owner, "foreign thread does not own worker");
    CHECK(!probe.memory_is_owner,
          "foreign thread does not own worker allocator arena");
    CHECK(probe.transition == RXVM_WORKER_TRANSITION_WRONG_THREAD,
          "foreign thread cannot enter worker execution");
    CHECK(rxvm_worker_get_state(&worker) == RXVM_WORKER_IDLE,
          "rejected foreign entry does not change worker state");

    CHECK(rxvm_worker_begin_draining(&worker) == RXVM_WORKER_TRANSITION_OK,
          "idle owner begins deterministic drain");
    CHECK(rxvm_worker_get_state(&worker) == RXVM_WORKER_DRAINING,
          "worker enters draining state");
    CHECK(rxvm_worker_destroy(&worker) == 0u,
          "empty worker destroys without live allocations");
    CHECK(rxvm_worker_get_state(&worker) == RXVM_WORKER_STOPPED,
          "destroyed worker reaches one terminal state");
    CHECK(rxvm_runtime_worker_count(runtime) == 0u,
          "destroyed worker unregisters from runtime");
    CHECK(rxvm_runtime_destroy(runtime) == 0u,
          "empty runtime destroys without live allocations");

    if (failures) {
        fprintf(stderr, "%d worker lifecycle check(s) failed\n", failures);
        return 1;
    }
    puts("PASS: RXVM worker lifecycle and thread ownership");
    return 0;
}
