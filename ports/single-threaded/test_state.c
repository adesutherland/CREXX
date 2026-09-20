/* cREXX License (MIT), Copyright (c) 2026 the cREXX contributors. */
#include "rxvmintp.h"
#include <stdio.h>
#include <string.h>

static int checks, failures;
#define CHECK(x) do { ++checks; if (!(x)) { ++failures; \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); } } while (0)
static void direct(void *f, int n, value **a, value *r, value *s) {
    (void)f; (void)n; (void)a; (void)r; (void)s;
}
static void locked(void *f, int n, value **a, value *r, value *s) {
    rxpa_compatibility_enter(); direct(f, n, a, r, s); rxpa_compatibility_leave();
}

int main(void) {
    rxvm_runtime *runtime = rxvm_runtime_create();
    rxvm_context a = {0}, b = {0};
    rxvm_memory_worker *previous;
    rxpa_compatibility_context ca, cb;
    rxvm_native_invoker ia = 0, ib = 0;
    rxvm_memory_stats stats;
    rxpa_loaded_plugin unavailable;
    unsigned char *small, *large, *resized;
    volatile sig_atomic_t outer = 0, inner = 0, *saved_outer, *saved_inner;
    memset(&unavailable, 0xff, sizeof(unavailable));
    CHECK(rxpa_open_plugin(0, "absent", &unavailable) == -1);
    CHECK(unavailable.handle == 0);
    CHECK(rxpa_live_plugin_handle_count() == 0);
    CHECK(runtime != 0);
    if (!runtime) return 1;
    CHECK(rxvm_worker_initialize(&a.worker, runtime));
    CHECK(rxvm_worker_initialize(&b.worker, runtime));
    CHECK(rxvm_runtime_worker_count(runtime) == 2);
    CHECK(rxvm_worker_begin_execution(&a.worker) == RXVM_WORKER_TRANSITION_OK);
    CHECK(rxvm_worker_begin_execution(&a.worker) == RXVM_WORKER_TRANSITION_OK);
    CHECK(rxvm_worker_begin_draining(&a.worker) == RXVM_WORKER_TRANSITION_INVALID_STATE);
    CHECK(rxvm_worker_end_execution(&a.worker) == RXVM_WORKER_TRANSITION_OK);
    CHECK(a.worker.execution_depth == 1);
    CHECK(rxvm_active_context_enter(&a) == 0);
    CHECK(rxvm_active_context_enter(&b) == &a);
    rxvm_active_context_leave(&a);
    CHECK(rxvm_active_context_current() == &a);
    rxvm_active_context_leave(0);
    CHECK(rxvm_active_context_current() == 0);
    previous = rxvm_memory_enter(a.worker.memory_worker);
    CHECK(previous == 0);
    CHECK(rxvm_memory_enter(b.worker.memory_worker) == a.worker.memory_worker);
    rxvm_memory_leave(a.worker.memory_worker);
    CHECK(rxvm_memory_current_worker() == a.worker.memory_worker);
    small = rxvm_memory_alloc_bytes(a.worker.memory_worker, 64);
    large = rxvm_memory_alloc_bytes(a.worker.memory_worker, 70000);
    CHECK(small != 0 && large != 0);
    if (!small || !large) return 1;
    memset(small, 0x5a, 64);
    CHECK(rxvm_memory_owner(small) == a.worker.memory_worker);
    resized = rxvm_memory_resize_bytes(a.worker.memory_worker, small, 64, 80000);
    CHECK(resized != 0);
    if (!resized) return 1;
    CHECK(resized[0] == 0x5a && resized[63] == 0x5a);
    CHECK(rxvm_memory_alloc_bytes(a.worker.memory_worker, SIZE_MAX) == 0);
    rxvm_memory_get_stats(rxvm_runtime_memory_context(runtime), &stats);
    CHECK(stats.live_allocations == 2);
    CHECK(stats.peak_live_allocations >= 2);
    CHECK(stats.allocation_failures >= 1);
    CHECK(rxvm_memory_release(resized) == RXVM_MEMORY_OK);
    CHECK(rxvm_memory_release(large) == RXVM_MEMORY_OK);
    rxvm_memory_get_stats(rxvm_runtime_memory_context(runtime), &stats);
    CHECK(stats.live_allocations == 0);
    small = rxvm_memory_alloc_bytes(a.worker.memory_worker, 64);
    CHECK(small != 0);
    CHECK(rxvm_memory_release(small) == RXVM_MEMORY_OK);
    {
        const size_t sizes[] = {16, 2048, 2049, 4096, 16384, 16385};
        unsigned i;
        for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
            unsigned char *block = rxvm_memory_alloc_bytes(a.worker.memory_worker, sizes[i]);
            CHECK(block != 0);
            if (!block) return 1;
            memset(block, 0x37, sizes[i]);
            CHECK(rxvm_memory_owner(block) == a.worker.memory_worker);
            resized = rxvm_memory_resize_bytes(a.worker.memory_worker, block,
                                               sizes[i], sizes[i] + 17000);
            CHECK(resized != 0);
            if (!resized) return 1;
            CHECK(resized[0] == 0x37 && resized[sizes[i] - 1] == 0x37);
            CHECK(rxvm_memory_release(resized) == RXVM_MEMORY_OK);
        }
        rxvm_memory_get_stats(rxvm_runtime_memory_context(runtime), &stats);
        CHECK(stats.live_allocations == 0);
    }
    {
        const size_t counts[] = {1, 8, 16, 32, 64, 65};
        unsigned i;
        for (i = 0; i < sizeof(counts) / sizeof(counts[0]); ++i) {
            value *values = rxvm_memory_alloc_values(a.worker.memory_worker, counts[i]);
            CHECK(values != 0);
            if (!values) return 1;
            memset(values, 0, counts[i] * sizeof(*values));
            CHECK(rxvm_memory_owner(values) == a.worker.memory_worker);
            CHECK(rxvm_memory_release(values) == RXVM_MEMORY_OK);
        }
        void *reference = rxvm_memory_alloc_reference_cell(a.worker.memory_worker);
        CHECK(reference != 0);
        CHECK(rxvm_memory_release(reference) == RXVM_MEMORY_OK);
        rxvm_memory_get_stats(rxvm_runtime_memory_context(runtime), &stats);
        CHECK(stats.live_allocations == 0);
    }

    rxpa_compatibility_context_init(&ca, a.worker.memory_worker);
    rxpa_compatibility_context_init(&cb, b.worker.memory_worker);
    CHECK(rxpa_compatibility_bind_legacy(&ca, &ia, direct, locked));
    rxpa_compatibility_execution_enter(&ca);
    CHECK(rxpa_compatibility_bind_legacy(&cb, &ib, direct, locked));
    CHECK(ia == locked && ib == locked);
    rxpa_compatibility_execution_enter(&cb);
    rxpa_compatibility_enter();
    rxpa_compatibility_enter();
    rxpa_compatibility_leave();
    rxpa_compatibility_leave();
    rxpa_compatibility_execution_leave(&cb);
    rxpa_compatibility_execution_leave(&ca);
    rxvm_memory_enter(b.worker.memory_worker);
    rxpa_compatibility_context_destroy(&cb);
    rxvm_memory_leave(a.worker.memory_worker);
    rxpa_compatibility_context_destroy(&ca);

    CHECK(initialize_vm_signals() == 0);
    CHECK(rxvm_signal_bind_process_main(&a) == 0);
    CHECK(rxvm_signal_bind_process_main(&b) == -1);
    CHECK(rxvm_signal_enter_execution(&a, &outer, &saved_outer) == 0);
    rxvm_signal_raise_process_main(RXSIGNAL_OTHER);
    CHECK(outer == rxsignal_mask(RXSIGNAL_OTHER));
    CHECK(rxvm_signal_enter_execution(&a, &inner, &saved_inner) == 0);
    CHECK(outer == 0 && inner == rxsignal_mask(RXSIGNAL_OTHER));
    CHECK(rxvm_signal_leave_execution(&a, &outer, 0) == -1);
    CHECK(rxvm_signal_leave_execution(&a, &inner, saved_inner) == 0);
    CHECK(inner == 0 && outer == rxsignal_mask(RXSIGNAL_OTHER));
    rxvm_signal_clear_process_main(RXSIGNAL_OTHER);
    CHECK(outer == 0);
    CHECK(rxvm_signal_leave_execution(&a, &outer, saved_outer) == 0);
    cleanup_vm_signals();
    CHECK(rxvm_signal_bind_process_main(&b) == 0);
    initialize_vm_signals(); cleanup_vm_signals();
    rxvm_memory_leave(previous);
    CHECK(rxvm_worker_end_execution(&a.worker) == RXVM_WORKER_TRANSITION_OK);
    CHECK(rxvm_worker_begin_draining(&a.worker) == RXVM_WORKER_TRANSITION_OK);
    CHECK(rxvm_worker_begin_draining(&b.worker) == RXVM_WORKER_TRANSITION_OK);
    CHECK(rxvm_worker_destroy(&a.worker) == 0);
    CHECK(rxvm_worker_destroy(&b.worker) == 0);
    CHECK(rxvm_runtime_worker_count(runtime) == 0);
    CHECK(rxvm_runtime_destroy(runtime) == 0);
    printf("Single-thread state: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
