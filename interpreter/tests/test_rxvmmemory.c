/* Allocator-only correctness tests. These are deliberately not timings. */

#include "rxvmmemory.h"
#include "rxvalue.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#define CHECK(condition, message) do {                                      \
    if (!(condition)) {                                                     \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, (message));        \
        failures++;                                                         \
    }                                                                       \
} while (0)

static int failures;

typedef struct foreign_allocator_probe {
    rxvm_memory_worker *worker;
    void *owner_standard;
    void *owner_extent;
    void *owner_resize;
    void *foreign_standard;
    void *foreign_extent;
    void *foreign_values_overflow;
    void *foreign_resize;
    rxvm_memory_result standard_release;
    rxvm_memory_result extent_release;
    int tls_was_empty;
} foreign_allocator_probe;

#ifdef _WIN32
static DWORD WINAPI run_foreign_allocator_thread(LPVOID argument) {
#else
static void *run_foreign_allocator_thread(void *argument) {
#endif
    foreign_allocator_probe *probe = (foreign_allocator_probe *)argument;

    probe->tls_was_empty = rxvm_memory_current_worker() == 0;
    probe->foreign_standard = rxvm_memory_alloc_bytes(probe->worker, 32u);
    probe->foreign_extent = rxvm_memory_alloc_bytes(
            probe->worker, RXVM_MEMORY_MAX_STANDARD_SIZE + 1u);
    probe->foreign_values_overflow = rxvm_memory_alloc_values(
            probe->worker, (size_t)-1);
    probe->foreign_resize = rxvm_memory_resize_bytes(
            probe->worker, probe->owner_resize, 16u, 64u);
    probe->standard_release = rxvm_memory_release(probe->owner_standard);
    probe->extent_release = rxvm_memory_release(probe->owner_extent);
#ifdef _WIN32
    return 0;
#else
    return 0;
#endif
}

static void run_foreign_allocator_probe(foreign_allocator_probe *probe) {
#ifdef _WIN32
    HANDLE thread = CreateThread(
            0, 0, run_foreign_allocator_thread, probe, 0, 0);
    CHECK(thread != 0, "foreign allocator probe thread starts");
    if (thread) {
        CHECK(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0,
              "foreign allocator probe thread joins");
        CloseHandle(thread);
    }
#else
    pthread_t thread;
    int create_result = pthread_create(
            &thread, 0, run_foreign_allocator_thread, probe);
    CHECK(create_result == 0, "foreign allocator probe thread starts");
    if (create_result == 0) {
        CHECK(pthread_join(thread, 0) == 0,
              "foreign allocator probe thread joins");
    }
#endif
}

static void test_classes_and_alignment(rxvm_memory_worker *worker) {
    static const size_t requests[] = {
        1u, 16u, 17u, 33u, 255u, 4097u
    };
    static const size_t capacities[] = {
        16u, 16u, 32u, 64u, 256u, 8192u
    };
    void *pointers[sizeof(requests) / sizeof(requests[0])];
    size_t i;

    for (i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
        pointers[i] = rxvm_memory_alloc_bytes(worker, requests[i]);
        CHECK(pointers[i] != 0, "byte allocation succeeds");
        CHECK(((uintptr_t)pointers[i] & 15u) == 0u,
              "byte allocation has 16-byte alignment");
        CHECK(rxvm_memory_capacity(pointers[i]) == capacities[i],
              "byte allocation uses expected power-of-two class");
        CHECK(rxvm_memory_owner(pointers[i]) == worker,
              "byte allocation records worker ownership");
    }
    for (i = 0; i < sizeof(requests) / sizeof(requests[0]); i++) {
        CHECK(rxvm_memory_release(pointers[i]) == RXVM_MEMORY_OK,
              "byte allocation releases through its owner");
    }
}

static void test_geometry_contract(rxvm_memory_context *context,
                                   rxvm_memory_worker *worker) {
    rxvm_memory_stats before;
    rxvm_memory_stats at_limit;
    rxvm_memory_stats above_limit;
    void *standard;
    void *oversized;
    size_t maximum_slots =
            (RXVM_MEMORY_SLAB_SIZE - 64u) /
            RXVM_MEMORY_MAX_STANDARD_SIZE;

    CHECK((RXVM_MEMORY_SLAB_SIZE & (RXVM_MEMORY_SLAB_SIZE - 1u)) == 0u,
          "slab size is a power of two for aligned owner lookup");
    CHECK((RXVM_MEMORY_MAX_STANDARD_SIZE &
           (RXVM_MEMORY_MAX_STANDARD_SIZE - 1u)) == 0u,
          "maximum standard byte class is a power of two");
    if (RXVM_MEMORY_SLAB_SIZE == 65536u &&
        RXVM_MEMORY_MAX_STANDARD_SIZE == 16384u) {
        CHECK(maximum_slots == 3u,
              "S0 preserves its accepted three-slot maximum class");
    } else {
        CHECK(maximum_slots >= 4u,
              "non-control geometry has at least four maximum-class slots");
    }

    rxvm_memory_get_stats(context, &before);
    standard = rxvm_memory_alloc_bytes(worker,
                                       RXVM_MEMORY_MAX_STANDARD_SIZE);
    CHECK(standard != 0, "maximum standard byte allocation succeeds");
    CHECK(rxvm_memory_capacity(standard) == RXVM_MEMORY_MAX_STANDARD_SIZE,
          "maximum standard byte allocation uses its exact class");
    rxvm_memory_get_stats(context, &at_limit);
    CHECK(at_limit.oversized_live_allocations ==
                  before.oversized_live_allocations,
          "maximum standard byte allocation stays in a slab");

    oversized = rxvm_memory_alloc_bytes(worker,
                                        RXVM_MEMORY_MAX_STANDARD_SIZE + 1u);
    CHECK(oversized != 0, "above-limit byte allocation succeeds");
    CHECK(rxvm_memory_capacity(oversized) ==
                  RXVM_MEMORY_MAX_STANDARD_SIZE + 1u,
          "above-limit byte allocation uses an exact extent");
    rxvm_memory_get_stats(context, &above_limit);
    CHECK(above_limit.oversized_live_allocations ==
                  at_limit.oversized_live_allocations + 1u,
          "above-limit byte allocation is classified as oversized");

    CHECK(rxvm_memory_release(oversized) == RXVM_MEMORY_OK,
          "above-limit byte allocation releases");
    CHECK(rxvm_memory_release(standard) == RXVM_MEMORY_OK,
          "maximum standard byte allocation releases");
}

static void test_typed_silos(rxvm_memory_worker *worker) {
    static const size_t counts[] = {1u, 2u, 3u, 8u, 64u, 65u};
    static const size_t rounded[] = {1u, 2u, 4u, 8u, 64u, 65u};
    size_t i;

    for (i = 0; i < sizeof(counts) / sizeof(counts[0]); i++) {
        void *pointer = rxvm_memory_alloc_values(worker, counts[i]);
        CHECK(pointer != 0, "typed value allocation succeeds");
        CHECK(rxvm_memory_capacity(pointer) == sizeof(value) * rounded[i],
              "typed value capacity avoids generic object rounding");
        CHECK(rxvm_memory_release(pointer) == RXVM_MEMORY_OK,
              "typed value allocation releases");
    }
    {
        void *cell = rxvm_memory_alloc_reference_cell(worker);
        CHECK(cell != 0, "reference-cell allocation succeeds");
        CHECK(rxvm_memory_capacity(cell) == sizeof(rxvm_reference_cell),
              "reference cell uses its exact typed class");
        CHECK(rxvm_memory_release(cell) == RXVM_MEMORY_OK,
              "reference-cell allocation releases");
    }
}

static void test_refill_return_and_trim(rxvm_memory_context *context,
                                        rxvm_memory_worker *worker) {
    void *pointers[RXVM_MEMORY_SLAB_SIZE / 16u];
    rxvm_memory_stats stats;
    size_t i;
    size_t count = sizeof(pointers) / sizeof(pointers[0]);

    for (i = 0; i < count; i++) {
        pointers[i] = rxvm_memory_alloc_bytes(worker, 16u);
        CHECK(pointers[i] != 0, "multi-slab allocation succeeds");
    }
    for (i = 0; i < count; i++) {
        CHECK(rxvm_memory_release(pointers[i]) == RXVM_MEMORY_OK,
              "multi-slab allocation releases");
    }
    rxvm_memory_get_stats(context, &stats);
    CHECK(stats.standard_slabs_from_system >= 2u,
          "refill obtains multiple aligned slabs");
    CHECK(stats.depot_returns >= 1u,
          "excess empty slabs return to synchronized depot");
    CHECK(stats.standard_slabs_reserved >= 1u,
          "depot retains a bounded reserve");
    CHECK(stats.standard_slabs_reserved * RXVM_MEMORY_SLAB_SIZE <= 2097152u,
          "depot global reserve remains within the byte-normalized ceiling");
    rxvm_memory_trim(context);
    rxvm_memory_get_stats(context, &stats);
    CHECK(stats.standard_slabs_reserved == 0u,
          "safe-point trim releases the depot reserve");
}

static void test_oversized_and_resize(rxvm_memory_worker *worker) {
    const size_t initial_size = RXVM_MEMORY_MAX_STANDARD_SIZE + 1u;
    const size_t grown_size = RXVM_MEMORY_MAX_STANDARD_SIZE * 2u + 2u;
    unsigned char *pointer =
            (unsigned char *)rxvm_memory_alloc_bytes(worker, initial_size);
    unsigned char *grown;
    unsigned char *shrunk;
    size_t i;

    CHECK(pointer != 0, "oversized allocation succeeds");
    CHECK(rxvm_memory_capacity(pointer) == initial_size,
          "oversized capacity is tracked exactly");
    for (i = 0; i < 128u; i++) pointer[i] = (unsigned char)i;
    grown = (unsigned char *)rxvm_memory_resize_bytes(worker, pointer,
                                                       128u, grown_size);
    CHECK(grown != 0, "oversized allocation grows");
    for (i = 0; i < 128u; i++) {
        CHECK(grown[i] == (unsigned char)i,
              "oversized growth preserves initialized prefix");
    }
    shrunk = (unsigned char *)rxvm_memory_resize_bytes(worker, grown,
                                                        128u, 1024u);
    CHECK(shrunk != 0, "oversized allocation trims into standard class");
    CHECK(rxvm_memory_capacity(shrunk) == 1024u,
          "oversized shrink releases exceptional capacity");
    for (i = 0; i < 128u; i++) {
        CHECK(shrunk[i] == (unsigned char)i,
              "oversized shrink preserves initialized prefix");
    }
    CHECK(rxvm_memory_release(shrunk) == RXVM_MEMORY_OK,
          "resized allocation releases");
}

static void test_tls_and_wrong_owner(rxvm_memory_context *context,
                                     rxvm_memory_worker *first) {
    rxvm_memory_worker *second = rxvm_memory_worker_create(context);
    rxvm_memory_worker *previous;
    void *pointer;

    CHECK(second != 0, "second logical worker can register with depot");
    previous = rxvm_memory_enter(first);
    CHECK(previous == 0, "first scoped worker has empty predecessor");
    pointer = rxvm_memory_alloc_bytes(0, 32u);
    CHECK(pointer != 0, "current worker supplies bounded TLS fallback");
    CHECK(rxvm_memory_enter(second) == first,
          "nested worker entry returns previous binding");
    CHECK(rxvm_memory_resize_bytes(0, pointer, 16u, 64u) == 0,
          "non-owner resize fails before allocating replacement storage");
    CHECK(rxvm_memory_release(pointer) == RXVM_MEMORY_WRONG_OWNER,
          "non-owner free is detected without corrupting local list");
    rxvm_memory_leave(first);
    CHECK(rxvm_memory_release(pointer) == RXVM_MEMORY_OK,
          "restored owner can release allocation");
    rxvm_memory_leave(previous);
    CHECK(rxvm_memory_current_worker() == 0,
          "scoped binding restores the entry state");
    CHECK(rxvm_memory_worker_destroy(second) == 0u,
          "empty worker tears down without leak report");
}

static void test_foreign_thread_without_tls(rxvm_memory_worker *worker) {
    foreign_allocator_probe probe;

    memset(&probe, 0, sizeof(probe));
    probe.worker = worker;
    probe.owner_standard = rxvm_memory_alloc_bytes(worker, 32u);
    probe.owner_extent = rxvm_memory_alloc_bytes(
            worker, RXVM_MEMORY_MAX_STANDARD_SIZE + 1u);
    probe.owner_resize = rxvm_memory_alloc_bytes(worker, 32u);
    CHECK(probe.owner_standard != 0 && probe.owner_extent != 0 &&
                  probe.owner_resize != 0,
          "owner allocations for foreign-thread probe succeed");
    if (!probe.owner_standard || !probe.owner_extent || !probe.owner_resize) {
        if (probe.owner_standard) (void)rxvm_memory_release(probe.owner_standard);
        if (probe.owner_extent) (void)rxvm_memory_release(probe.owner_extent);
        if (probe.owner_resize) (void)rxvm_memory_release(probe.owner_resize);
        return;
    }

    run_foreign_allocator_probe(&probe);

    CHECK(probe.tls_was_empty,
          "foreign allocator thread begins without an entered worker");
    CHECK(probe.foreign_standard == 0,
          "foreign explicit standard allocation is rejected");
    CHECK(probe.foreign_extent == 0,
          "foreign explicit extent allocation is rejected");
    CHECK(probe.foreign_values_overflow == 0,
          "foreign explicit value-array overflow is rejected");
    CHECK(probe.foreign_resize == 0,
          "foreign explicit resize is rejected");
    CHECK(probe.standard_release == RXVM_MEMORY_WRONG_OWNER,
          "foreign standard release without TLS is rejected");
    CHECK(probe.extent_release == RXVM_MEMORY_WRONG_OWNER,
          "foreign extent release without TLS is rejected");

    if (probe.foreign_standard) {
        CHECK(rxvm_memory_release(probe.foreign_standard) == RXVM_MEMORY_OK,
              "owner cleans a foreign-thread standard allocation reproducer");
    }
    if (probe.foreign_extent) {
        CHECK(rxvm_memory_release(probe.foreign_extent) == RXVM_MEMORY_OK,
              "owner cleans a foreign-thread extent allocation reproducer");
    }
    if (probe.foreign_resize) {
        CHECK(rxvm_memory_release(probe.foreign_resize) == RXVM_MEMORY_OK,
              "owner cleans a foreign-thread resize reproducer");
    } else {
        CHECK(rxvm_memory_release(probe.owner_resize) == RXVM_MEMORY_OK,
              "owner retains and releases the rejected resize source");
    }
    if (probe.standard_release != RXVM_MEMORY_OK) {
        CHECK(rxvm_memory_release(probe.owner_standard) == RXVM_MEMORY_OK,
              "owner releases the rejected foreign standard pointer");
    }
    if (probe.extent_release != RXVM_MEMORY_OK) {
        CHECK(rxvm_memory_release(probe.owner_extent) == RXVM_MEMORY_OK,
              "owner releases the rejected foreign extent pointer");
    }
}

static void test_calloc_overflow(rxvm_memory_worker *worker) {
    void *pointer = rxvm_memory_calloc_bytes(worker, (size_t)-1, 2u);
    CHECK(pointer == 0, "calloc multiplication overflow fails safely");
}

static void test_unentered_compatibility_extent(rxvm_memory_worker *worker) {
    unsigned char *pointer = rxvm_memory_alloc_bytes(0, 37u);
    unsigned char *grown;
    rxvm_memory_worker *previous;
    size_t i;

    CHECK(pointer != 0, "unentered compatibility allocation succeeds");
    CHECK(rxvm_memory_capacity(pointer) == 37u,
          "unentered compatibility allocation is an exact tracked extent");
    for (i = 0; i < 37u; i++) pointer[i] = (unsigned char)(i + 1u);
    previous = rxvm_memory_enter(worker);
    grown = rxvm_memory_resize_bytes(0, pointer, 37u, 80u);
    rxvm_memory_leave(previous);
    CHECK(grown != 0, "unentered compatibility extent grows");
    CHECK(rxvm_memory_owner(grown) == 0,
          "detached resize does not adopt the active worker");
    for (i = 0; i < 37u; i++) {
        CHECK(grown[i] == (unsigned char)(i + 1u),
              "unentered growth preserves initialized bytes");
    }
    CHECK(rxvm_memory_release(grown) == RXVM_MEMORY_OK,
          "unentered compatibility extent releases");
}

static void test_teardown_leak_detection(void) {
    rxvm_memory_context *context = rxvm_memory_context_create();
    rxvm_memory_worker *worker = rxvm_memory_worker_create(context);

    CHECK(context != 0 && worker != 0,
          "leak-detection context and worker create");
    if (!context || !worker) return;
    CHECK(rxvm_memory_alloc_bytes(worker, 64u) != 0,
          "intentional teardown leak allocation succeeds");
    CHECK(rxvm_memory_context_destroy(context) == 1u,
          "teardown reports and deterministically releases a live allocation");
}

int main(void) {
    rxvm_memory_context *context = rxvm_memory_context_create();
    rxvm_memory_worker *worker;
    rxvm_memory_stats stats;

    CHECK(context != 0, "memory context creation succeeds");
    if (!context) return 1;
    worker = rxvm_memory_worker_create(context);
    CHECK(worker != 0, "memory worker creation succeeds");
    if (!worker) {
        (void)rxvm_memory_context_destroy(context);
        return 1;
    }

    test_classes_and_alignment(worker);
    test_geometry_contract(context, worker);
    test_typed_silos(worker);
    test_refill_return_and_trim(context, worker);
    test_oversized_and_resize(worker);
    test_tls_and_wrong_owner(context, worker);
    test_foreign_thread_without_tls(worker);
    test_calloc_overflow(worker);
    test_unentered_compatibility_extent(worker);

    rxvm_memory_get_stats(context, &stats);
    CHECK(stats.live_allocations == 0u,
          "all allocator test allocations are released");
    CHECK(stats.wrong_owner_allocations == 3u,
          "foreign standard, extent and value-array allocations appear in telemetry");
    CHECK(stats.wrong_owner_resizes == 2u,
          "TLS-bound and no-TLS foreign resizes appear in telemetry");
    CHECK(stats.wrong_owner_frees == 3u,
          "TLS-bound and no-TLS foreign frees appear in telemetry");
    CHECK(stats.allocation_failures == 1u,
          "overflow failure appears in telemetry");
#ifdef CREXX_VM_MEMORY_CENSUS
    CHECK(stats.oversized_allocation_calls >= 3u,
          "diagnostic census counts typed and byte oversized allocations");
    CHECK(stats.peak_oversized_live_allocations >= 1u &&
                  stats.peak_oversized_live_bytes >=
                  RXVM_MEMORY_MAX_STANDARD_SIZE * 2u + 2u,
          "diagnostic census records oversized live high water");
    CHECK(stats.maximum_oversized_request_bytes >=
                  RXVM_MEMORY_MAX_STANDARD_SIZE * 2u + 2u,
          "diagnostic census records largest oversized request");
    CHECK(stats.resize_oversized_to_oversized >= 1u &&
                  stats.resize_oversized_to_standard >= 1u,
          "diagnostic census classifies oversized resize transitions");
#endif
    CHECK(rxvm_memory_worker_destroy(worker) == 0u,
          "primary worker teardown is leak-free");
    CHECK(rxvm_memory_context_destroy(context) == 0u,
          "depot teardown is deterministic and leak-free");
    test_teardown_leak_detection();

    if (failures) {
        fprintf(stderr, "%d allocator test failure(s)\n", failures);
        return 1;
    }
    printf("rxvmmemory tests passed (slab=%zu max_standard=%zu)\n",
           RXVM_MEMORY_SLAB_SIZE, RXVM_MEMORY_MAX_STANDARD_SIZE);
    return 0;
}
