/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXVMMEMORY_H
#define CREXX_RXVMMEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RXVM_MEMORY_SLAB_SIZE ((size_t)65536u)
#define RXVM_MEMORY_MAX_STANDARD_SIZE ((size_t)16384u)
#define RXVM_MEMORY_BYTE_CLASS_COUNT 11u
#define RXVM_MEMORY_VALUE_CLASS_COUNT 7u
#define RXVM_MEMORY_CLASS_COUNT 19u
#define RXVM_MEMORY_SIZE_HISTOGRAM_BUCKETS 66u

typedef struct rxvm_memory_context rxvm_memory_context;
typedef struct rxvm_memory_worker rxvm_memory_worker;

typedef enum rxvm_memory_result {
    RXVM_MEMORY_OK = 0,
    RXVM_MEMORY_INVALID_POINTER = 1,
    RXVM_MEMORY_WRONG_OWNER = 2
} rxvm_memory_result;

typedef struct rxvm_memory_stats {
    uint64_t allocation_calls;
    uint64_t free_calls;
    uint64_t reallocation_calls;
    uint64_t allocation_failures;
    uint64_t invalid_frees;
    uint64_t wrong_owner_allocations;
    uint64_t wrong_owner_resizes;
    uint64_t wrong_owner_frees;
    uint64_t cumulative_requested_bytes;
    uint64_t cumulative_capacity_bytes;
    uint64_t live_allocations;
    uint64_t peak_live_allocations;
    uint64_t live_capacity_bytes;
    uint64_t peak_live_capacity_bytes;
    uint64_t oversized_live_allocations;
    uint64_t oversized_live_bytes;
#ifdef CREXX_VM_MEMORY_CENSUS
    uint64_t oversized_allocation_calls;
    uint64_t cumulative_oversized_requested_bytes;
    uint64_t peak_oversized_live_allocations;
    uint64_t peak_oversized_live_bytes;
    uint64_t maximum_oversized_request_bytes;
    uint64_t resize_standard_to_oversized;
    uint64_t resize_oversized_to_standard;
    uint64_t resize_oversized_to_oversized;
    uint64_t oversized_request_histogram[RXVM_MEMORY_SIZE_HISTOGRAM_BUCKETS];
#endif
    uint64_t standard_slabs_owned;
    uint64_t standard_slabs_reserved;
    uint64_t standard_slabs_from_system;
    uint64_t standard_slabs_to_system;
    uint64_t depot_refills;
    uint64_t depot_returns;
    uint64_t depot_hits;
    uint64_t trim_calls;
    uint64_t class_allocation_calls[RXVM_MEMORY_CLASS_COUNT];
    uint64_t class_live_allocations[RXVM_MEMORY_CLASS_COUNT];
    uint64_t class_peak_live_allocations[RXVM_MEMORY_CLASS_COUNT];
} rxvm_memory_stats;

/* One context owns the synchronized whole-slab depot. */
rxvm_memory_context *rxvm_memory_context_create(void);

/* Returns the number of live allocations forcibly released at teardown. */
size_t rxvm_memory_context_destroy(rxvm_memory_context *context);

/* A worker owns its local slabs and ordinary allocation/free paths. */
rxvm_memory_worker *rxvm_memory_worker_create(rxvm_memory_context *context);

/* Returns the number of live allocations detected before releasing storage. */
size_t rxvm_memory_worker_destroy(rxvm_memory_worker *worker);

/* Scoped thread-local binding for legacy/value-service entry points. */
rxvm_memory_worker *rxvm_memory_enter(rxvm_memory_worker *worker);
void rxvm_memory_leave(rxvm_memory_worker *previous_worker);
rxvm_memory_worker *rxvm_memory_current_worker(void);
int rxvm_memory_worker_is_current_thread_owner(
        const rxvm_memory_worker *worker);

/* Power-of-two byte storage, typed value arrays and typed reference cells. */
void *rxvm_memory_alloc_bytes(rxvm_memory_worker *worker, size_t size);
/* Explicit detached extent for process-global or ABI-compatible lifetimes. */
void *rxvm_memory_alloc_unowned_bytes(size_t size);
void *rxvm_memory_calloc_bytes(rxvm_memory_worker *worker,
                               size_t count,
                               size_t size);
void *rxvm_memory_alloc_values(rxvm_memory_worker *worker, size_t count);
void *rxvm_memory_alloc_reference_cell(rxvm_memory_worker *worker);

/*
 * Resize byte storage. copy_size is the initialized prefix to preserve.
 * Capacity is retained in place when the existing class still fits.
 */
void *rxvm_memory_resize_bytes(rxvm_memory_worker *worker,
                               void *pointer,
                               size_t copy_size,
                               size_t new_size);

/* Release recovers ownership from the slab/extent; wrong-owner frees fail. */
rxvm_memory_result rxvm_memory_release(void *pointer);

/* Safe-point reserve release. Local live/partly-used slabs are not moved. */
void rxvm_memory_trim(rxvm_memory_context *context);

size_t rxvm_memory_capacity(const void *pointer);
rxvm_memory_worker *rxvm_memory_owner(const void *pointer);
int rxvm_memory_is_owned(const void *pointer);
void rxvm_memory_get_stats(const rxvm_memory_context *context,
                           rxvm_memory_stats *stats);

#ifdef __cplusplus
}
#endif

#endif
