/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef _WIN32
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#endif

#include "rxvmmemory.h"
#include "rxvalue.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#define RXVM_MEMORY_THREAD_LOCAL __declspec(thread)
#else
#define RXVM_MEMORY_THREAD_LOCAL __thread
#endif

#ifdef _WIN32
#include <malloc.h>
#include <windows.h>
typedef CRITICAL_SECTION rxvm_memory_mutex;
static void rxvm_memory_mutex_init(rxvm_memory_mutex *mutex) {
    InitializeCriticalSection(mutex);
}
static void rxvm_memory_mutex_destroy(rxvm_memory_mutex *mutex) {
    DeleteCriticalSection(mutex);
}
static void rxvm_memory_mutex_lock(rxvm_memory_mutex *mutex) {
    EnterCriticalSection(mutex);
}
static void rxvm_memory_mutex_unlock(rxvm_memory_mutex *mutex) {
    LeaveCriticalSection(mutex);
}
#else
#include <pthread.h>
typedef pthread_mutex_t rxvm_memory_mutex;
static void rxvm_memory_mutex_init(rxvm_memory_mutex *mutex) {
    (void)pthread_mutex_init(mutex, 0);
}
static void rxvm_memory_mutex_destroy(rxvm_memory_mutex *mutex) {
    (void)pthread_mutex_destroy(mutex);
}
static void rxvm_memory_mutex_lock(rxvm_memory_mutex *mutex) {
    (void)pthread_mutex_lock(mutex);
}
static void rxvm_memory_mutex_unlock(rxvm_memory_mutex *mutex) {
    (void)pthread_mutex_unlock(mutex);
}
#endif

#define RXVM_MEMORY_SLAB_MAGIC UINT32_C(0x52584d53)
#define RXVM_MEMORY_EXTENT_MAGIC UINT64_C(0xd34f5258564d4531)
#define RXVM_MEMORY_HEADER_VERSION 1u
#define RXVM_MEMORY_SLAB_HEADER_SIZE 64u
#define RXVM_MEMORY_ALIGNMENT 16u
#define RXVM_MEMORY_DEPOT_PER_CLASS_LIMIT 2u
#define RXVM_MEMORY_DEPOT_GLOBAL_LIMIT 32u
#define RXVM_MEMORY_CLASS_REFERENCE 18u
#define RXVM_MEMORY_SLAB_FLAG_AVAILABLE 0x01u
#define RXVM_MEMORY_SLAB_FLAG_EMPTY 0x02u

typedef struct rxvm_memory_slab rxvm_memory_slab;
typedef struct rxvm_memory_extent rxvm_memory_extent;

typedef struct rxvm_memory_class_state {
    rxvm_memory_slab *available;
    rxvm_memory_slab *empty;
    uint64_t allocation_calls;
    uint64_t live_allocations;
    uint64_t peak_live_allocations;
} rxvm_memory_class_state;

struct rxvm_memory_slab {
    rxvm_memory_context *context;
    rxvm_memory_worker *owner;
    rxvm_memory_slab *owner_next;
    rxvm_memory_slab *class_next;
    void *local_free;
    uint32_t magic;
    uint32_t owner_generation;
    uint32_t slot_size;
    uint16_t slot_count;
    uint16_t bump_count;
    uint16_t live_count;
    uint8_t class_id;
    uint8_t flags;
    uint32_t version_reserved;
};

typedef char rxvm_memory_slab_header_must_be_64_bytes[
        sizeof(rxvm_memory_slab) == RXVM_MEMORY_SLAB_HEADER_SIZE ? 1 : -1];
typedef char rxvm_memory_slab_size_must_be_power_of_two[
        RXVM_MEMORY_SLAB_SIZE != 0u &&
        (RXVM_MEMORY_SLAB_SIZE & (RXVM_MEMORY_SLAB_SIZE - 1u)) == 0u ? 1 : -1];
typedef char rxvm_memory_standard_size_must_be_power_of_two[
        RXVM_MEMORY_MAX_STANDARD_SIZE != 0u &&
        (RXVM_MEMORY_MAX_STANDARD_SIZE &
         (RXVM_MEMORY_MAX_STANDARD_SIZE - 1u)) == 0u ? 1 : -1];
typedef char rxvm_memory_smallest_slot_count_must_fit_header[
        (RXVM_MEMORY_SLAB_SIZE - RXVM_MEMORY_SLAB_HEADER_SIZE) /
        RXVM_MEMORY_ALIGNMENT <= UINT16_MAX ? 1 : -1];

struct rxvm_memory_extent {
    uint64_t magic;
    rxvm_memory_worker *owner;
    rxvm_memory_context *context;
    rxvm_memory_extent *previous;
    rxvm_memory_extent *next;
    void *system_pointer;
    size_t requested_size;
    size_t capacity;
};

struct rxvm_memory_worker {
    rxvm_memory_context *context;
    rxvm_memory_worker *context_next;
    rxvm_memory_slab *owned_slabs;
    rxvm_memory_extent *oversized;
    rxvm_memory_class_state classes[RXVM_MEMORY_CLASS_COUNT];
    rxvm_memory_stats stats;
    uint32_t id;
    uint32_t generation;
    uintptr_t owner_thread_token;
};

struct rxvm_memory_context {
    rxvm_memory_mutex mutex;
    rxvm_memory_worker *workers;
    rxvm_memory_slab *reserve[RXVM_MEMORY_CLASS_COUNT];
    uint32_t reserve_count[RXVM_MEMORY_CLASS_COUNT];
    uint32_t reserve_total;
    uint32_t next_worker_id;
    uint32_t next_generation;
    uint64_t standard_slabs_from_system;
    uint64_t standard_slabs_to_system;
    uint64_t depot_refills;
    uint64_t depot_returns;
    uint64_t depot_hits;
    uint64_t trim_calls;
    uint64_t wrong_owner_allocations;
    uint64_t wrong_owner_resizes;
    uint64_t wrong_owner_frees;
};

static RXVM_MEMORY_THREAD_LOCAL rxvm_memory_worker *rxvm_memory_tls_worker;
static RXVM_MEMORY_THREAD_LOCAL unsigned char rxvm_memory_thread_marker;

static uintptr_t rxvm_memory_current_thread_token(void) {
    return (uintptr_t)&rxvm_memory_thread_marker;
}

typedef enum rxvm_memory_wrong_owner_operation {
    RXVM_MEMORY_WRONG_OWNER_ALLOCATION = 0,
    RXVM_MEMORY_WRONG_OWNER_RESIZE = 1,
    RXVM_MEMORY_WRONG_OWNER_FREE = 2
} rxvm_memory_wrong_owner_operation;

static void rxvm_memory_record_wrong_owner(
        rxvm_memory_worker *worker,
        rxvm_memory_wrong_owner_operation operation) {
    rxvm_memory_context *context;

    if (!worker || !worker->context) return;
    context = worker->context;
    rxvm_memory_mutex_lock(&context->mutex);
    if (operation == RXVM_MEMORY_WRONG_OWNER_ALLOCATION) {
        context->wrong_owner_allocations++;
    } else if (operation == RXVM_MEMORY_WRONG_OWNER_RESIZE) {
        context->wrong_owner_resizes++;
    } else {
        context->wrong_owner_frees++;
    }
    rxvm_memory_mutex_unlock(&context->mutex);
}

static const uint32_t rxvm_memory_byte_sizes[RXVM_MEMORY_BYTE_CLASS_COUNT] = {
    16u, 32u, 64u, 128u, 256u, 512u,
    1024u, 2048u, 4096u, 8192u, 16384u
};

static void *rxvm_memory_system_slab_allocate(void) {
#ifdef _WIN32
    return _aligned_malloc(RXVM_MEMORY_SLAB_SIZE, RXVM_MEMORY_SLAB_SIZE);
#else
    void *pointer = 0;
    if (posix_memalign(&pointer, RXVM_MEMORY_SLAB_SIZE,
                       RXVM_MEMORY_SLAB_SIZE) != 0) {
        return 0;
    }
    return pointer;
#endif
}

static void rxvm_memory_system_slab_free(void *pointer) {
#ifdef _WIN32
    _aligned_free(pointer);
#else
    free(pointer);
#endif
}

static int rxvm_memory_size_add_overflows(size_t left,
                                          size_t right,
                                          size_t *result) {
    if (left > SIZE_MAX - right) return 1;
    *result = left + right;
    return 0;
}

static int rxvm_memory_size_multiply_overflows(size_t left,
                                               size_t right,
                                               size_t *result) {
    if (left && right > SIZE_MAX / left) return 1;
    *result = left * right;
    return 0;
}

static uintptr_t rxvm_memory_align_up(uintptr_t value, size_t alignment) {
    return (value + alignment - 1u) & ~((uintptr_t)alignment - 1u);
}

static uint32_t rxvm_memory_class_slot_size(uint8_t class_id) {
    size_t count;
    size_t result;

    if (class_id < RXVM_MEMORY_BYTE_CLASS_COUNT) {
        return rxvm_memory_byte_sizes[class_id];
    }
    if (class_id < RXVM_MEMORY_CLASS_REFERENCE) {
        count = (size_t)1u << (class_id - RXVM_MEMORY_BYTE_CLASS_COUNT);
        result = sizeof(value) * count;
        return result <= UINT32_MAX ? (uint32_t)result : 0u;
    }
    if (class_id == RXVM_MEMORY_CLASS_REFERENCE) {
        return (uint32_t)sizeof(rxvm_reference_cell);
    }
    return 0u;
}

static int rxvm_memory_byte_class(size_t size, uint8_t *class_id) {
    uint8_t index;
    size_t wanted = size ? size : 1u;

    for (index = 0; index < RXVM_MEMORY_BYTE_CLASS_COUNT; index++) {
        if (wanted <= rxvm_memory_byte_sizes[index]) {
            *class_id = index;
            return 1;
        }
    }
    return 0;
}

static int rxvm_memory_value_class(size_t count, uint8_t *class_id) {
    uint8_t index = 0;
    size_t capacity = 1u;
    size_t wanted = count ? count : 1u;

    while (capacity < wanted && index + 1u < RXVM_MEMORY_VALUE_CLASS_COUNT) {
        capacity <<= 1u;
        index++;
    }
    if (capacity < wanted) return 0;
    *class_id = (uint8_t)(RXVM_MEMORY_BYTE_CLASS_COUNT + index);
    return 1;
}

static unsigned char *rxvm_memory_slab_payload(rxvm_memory_slab *slab) {
    return (unsigned char *)slab + RXVM_MEMORY_SLAB_HEADER_SIZE;
}

static void rxvm_memory_worker_add_owned_slab(rxvm_memory_worker *worker,
                                               rxvm_memory_slab *slab) {
    slab->owner_next = worker->owned_slabs;
    worker->owned_slabs = slab;
    worker->stats.standard_slabs_owned++;
}

static void rxvm_memory_worker_remove_owned_slab(rxvm_memory_worker *worker,
                                                  rxvm_memory_slab *slab) {
    rxvm_memory_slab **cursor = &worker->owned_slabs;
    while (*cursor) {
        if (*cursor == slab) {
            *cursor = slab->owner_next;
            slab->owner_next = 0;
            if (worker->stats.standard_slabs_owned) {
                worker->stats.standard_slabs_owned--;
            }
            return;
        }
        cursor = &(*cursor)->owner_next;
    }
}

static void rxvm_memory_class_add_available(rxvm_memory_worker *worker,
                                            rxvm_memory_slab *slab) {
    rxvm_memory_class_state *state = &worker->classes[slab->class_id];
    if (slab->flags & RXVM_MEMORY_SLAB_FLAG_AVAILABLE) return;
    slab->class_next = state->available;
    state->available = slab;
    slab->flags |= RXVM_MEMORY_SLAB_FLAG_AVAILABLE;
}

static void rxvm_memory_class_remove_available(rxvm_memory_worker *worker,
                                               rxvm_memory_slab *slab) {
    rxvm_memory_class_state *state = &worker->classes[slab->class_id];
    rxvm_memory_slab **cursor = &state->available;
    if (!(slab->flags & RXVM_MEMORY_SLAB_FLAG_AVAILABLE)) return;
    while (*cursor) {
        if (*cursor == slab) {
            *cursor = slab->class_next;
            slab->class_next = 0;
            slab->flags &= (uint8_t)~RXVM_MEMORY_SLAB_FLAG_AVAILABLE;
            return;
        }
        cursor = &(*cursor)->class_next;
    }
    slab->flags &= (uint8_t)~RXVM_MEMORY_SLAB_FLAG_AVAILABLE;
    slab->class_next = 0;
}

static void rxvm_memory_slab_prepare(rxvm_memory_slab *slab,
                                     rxvm_memory_context *context,
                                     rxvm_memory_worker *worker,
                                     uint8_t class_id) {
    uint32_t slot_size = rxvm_memory_class_slot_size(class_id);

    memset(slab, 0, sizeof(*slab));
    slab->context = context;
    slab->owner = worker;
    slab->magic = RXVM_MEMORY_SLAB_MAGIC;
    slab->owner_generation = worker->generation;
    slab->slot_size = slot_size;
    slab->slot_count = (uint16_t)((RXVM_MEMORY_SLAB_SIZE -
                                  RXVM_MEMORY_SLAB_HEADER_SIZE) / slot_size);
    slab->class_id = class_id;
    slab->version_reserved = RXVM_MEMORY_HEADER_VERSION;
}

static rxvm_memory_slab *rxvm_memory_depot_acquire(rxvm_memory_worker *worker,
                                                    uint8_t class_id) {
    rxvm_memory_context *context = worker->context;
    rxvm_memory_slab *slab;

    rxvm_memory_mutex_lock(&context->mutex);
    context->depot_refills++;
    slab = context->reserve[class_id];
    if (slab) {
        context->reserve[class_id] = slab->class_next;
        slab->class_next = 0;
        context->reserve_count[class_id]--;
        context->reserve_total--;
        context->depot_hits++;
    }
    rxvm_memory_mutex_unlock(&context->mutex);

    if (!slab) {
        slab = (rxvm_memory_slab *)rxvm_memory_system_slab_allocate();
        if (!slab) return 0;
        rxvm_memory_mutex_lock(&context->mutex);
        context->standard_slabs_from_system++;
        rxvm_memory_mutex_unlock(&context->mutex);
    }

    rxvm_memory_slab_prepare(slab, context, worker, class_id);
    rxvm_memory_worker_add_owned_slab(worker, slab);
    rxvm_memory_class_add_available(worker, slab);
    return slab;
}

static void rxvm_memory_depot_return(rxvm_memory_worker *worker,
                                     rxvm_memory_slab *slab) {
    rxvm_memory_context *context = worker->context;
    uint8_t class_id = slab->class_id;
    int retain;

    rxvm_memory_class_remove_available(worker, slab);
    if (worker->classes[class_id].empty == slab) {
        worker->classes[class_id].empty = 0;
    }
    rxvm_memory_worker_remove_owned_slab(worker, slab);

    slab->owner = 0;
    slab->owner_generation = 0;
    slab->local_free = 0;
    slab->bump_count = 0;
    slab->live_count = 0;
    slab->flags = 0;

    rxvm_memory_mutex_lock(&context->mutex);
    context->depot_returns++;
    retain = context->reserve_count[class_id] <
                     RXVM_MEMORY_DEPOT_PER_CLASS_LIMIT &&
             context->reserve_total < RXVM_MEMORY_DEPOT_GLOBAL_LIMIT;
    if (retain) {
        slab->class_next = context->reserve[class_id];
        context->reserve[class_id] = slab;
        context->reserve_count[class_id]++;
        context->reserve_total++;
    } else {
        context->standard_slabs_to_system++;
    }
    rxvm_memory_mutex_unlock(&context->mutex);

    if (!retain) rxvm_memory_system_slab_free(slab);
}

static void rxvm_memory_stats_allocate(rxvm_memory_worker *worker,
                                       uint8_t class_id,
                                       size_t requested,
                                       size_t capacity,
                                       int oversized) {
    rxvm_memory_class_state *state = 0;
    rxvm_memory_stats *stats = &worker->stats;

    stats->allocation_calls++;
    stats->cumulative_requested_bytes += requested;
    stats->cumulative_capacity_bytes += capacity;
    stats->live_allocations++;
    stats->live_capacity_bytes += capacity;
    if (stats->live_allocations > stats->peak_live_allocations) {
        stats->peak_live_allocations = stats->live_allocations;
    }
    if (stats->live_capacity_bytes > stats->peak_live_capacity_bytes) {
        stats->peak_live_capacity_bytes = stats->live_capacity_bytes;
    }
    if (oversized) {
        stats->oversized_live_allocations++;
        stats->oversized_live_bytes += capacity;
#ifdef CREXX_VM_MEMORY_CENSUS
        {
            size_t bucket = 0;
            size_t upper = 0;
            stats->oversized_allocation_calls++;
            stats->cumulative_oversized_requested_bytes += requested;
            if (requested > stats->maximum_oversized_request_bytes)
                stats->maximum_oversized_request_bytes = requested;
            if (stats->oversized_live_allocations >
                    stats->peak_oversized_live_allocations)
                stats->peak_oversized_live_allocations =
                        stats->oversized_live_allocations;
            if (stats->oversized_live_bytes >
                    stats->peak_oversized_live_bytes)
                stats->peak_oversized_live_bytes =
                        stats->oversized_live_bytes;
            if (requested) {
                bucket = 1u;
                upper = 1u;
                while (upper < requested &&
                        bucket + 1u < RXVM_MEMORY_SIZE_HISTOGRAM_BUCKETS) {
                    if (upper > SIZE_MAX / 2u) {
                        bucket = RXVM_MEMORY_SIZE_HISTOGRAM_BUCKETS - 1u;
                        break;
                    }
                    upper <<= 1u;
                    bucket++;
                }
            }
            stats->oversized_request_histogram[bucket]++;
        }
#endif
        return;
    }
    state = &worker->classes[class_id];
    state->allocation_calls++;
    state->live_allocations++;
    if (state->live_allocations > state->peak_live_allocations) {
        state->peak_live_allocations = state->live_allocations;
    }
}

static void rxvm_memory_stats_free(rxvm_memory_worker *worker,
                                   uint8_t class_id,
                                   size_t capacity,
                                   int oversized) {
    rxvm_memory_stats *stats = &worker->stats;
    stats->free_calls++;
    if (stats->live_allocations) stats->live_allocations--;
    if (stats->live_capacity_bytes >= capacity) {
        stats->live_capacity_bytes -= capacity;
    } else {
        stats->live_capacity_bytes = 0;
    }
    if (oversized) {
        if (stats->oversized_live_allocations) {
            stats->oversized_live_allocations--;
        }
        if (stats->oversized_live_bytes >= capacity) {
            stats->oversized_live_bytes -= capacity;
        } else {
            stats->oversized_live_bytes = 0;
        }
        return;
    }
    if (worker->classes[class_id].live_allocations) {
        worker->classes[class_id].live_allocations--;
    }
}

static void *rxvm_memory_alloc_standard(rxvm_memory_worker *worker,
                                        uint8_t class_id,
                                        size_t requested) {
    rxvm_memory_class_state *state;
    rxvm_memory_slab *slab;
    void *result;

    if (!worker || class_id >= RXVM_MEMORY_CLASS_COUNT) return 0;
    if (!rxvm_memory_worker_is_current_thread_owner(worker)) {
        rxvm_memory_record_wrong_owner(
                worker, RXVM_MEMORY_WRONG_OWNER_ALLOCATION);
        return 0;
    }
    state = &worker->classes[class_id];
    slab = state->available;
    if (!slab) {
        slab = rxvm_memory_depot_acquire(worker, class_id);
        if (!slab) {
            worker->stats.allocation_failures++;
            return 0;
        }
    }

    if (slab->local_free) {
        result = slab->local_free;
        slab->local_free = *(void **)result;
    } else {
        result = rxvm_memory_slab_payload(slab) +
                 (size_t)slab->bump_count * slab->slot_size;
        slab->bump_count++;
    }

    if (state->empty == slab) state->empty = 0;
    slab->flags &= (uint8_t)~RXVM_MEMORY_SLAB_FLAG_EMPTY;
    slab->live_count++;
    if (slab->live_count == slab->slot_count) {
        rxvm_memory_class_remove_available(worker, slab);
    }
    rxvm_memory_stats_allocate(worker, class_id, requested,
                               slab->slot_size, 0);
    return result;
}

static void *rxvm_memory_alloc_oversized(rxvm_memory_worker *worker,
                                         size_t size) {
    size_t total;
    void *system_pointer;
    uintptr_t aligned;
    rxvm_memory_extent *extent;
    size_t wanted = size ? size : 1u;

    if (worker && !rxvm_memory_worker_is_current_thread_owner(worker)) {
        rxvm_memory_record_wrong_owner(
                worker, RXVM_MEMORY_WRONG_OWNER_ALLOCATION);
        return 0;
    }
    if (rxvm_memory_size_add_overflows(sizeof(*extent),
                                       RXVM_MEMORY_ALIGNMENT - 1u, &total) ||
        rxvm_memory_size_add_overflows(total, wanted, &total)) {
        if (worker) worker->stats.allocation_failures++;
        return 0;
    }
    system_pointer = malloc(total);
    if (!system_pointer) {
        if (worker) worker->stats.allocation_failures++;
        return 0;
    }
    aligned = rxvm_memory_align_up((uintptr_t)system_pointer + sizeof(*extent),
                                   RXVM_MEMORY_ALIGNMENT);
    extent = (rxvm_memory_extent *)(aligned - sizeof(*extent));
    memset(extent, 0, sizeof(*extent));
    extent->magic = RXVM_MEMORY_EXTENT_MAGIC;
    extent->owner = worker;
    extent->context = worker ? worker->context : 0;
    extent->system_pointer = system_pointer;
    extent->requested_size = wanted;
    extent->capacity = wanted;
    if (worker) {
        extent->next = worker->oversized;
        if (worker->oversized) worker->oversized->previous = extent;
        worker->oversized = extent;
        rxvm_memory_stats_allocate(worker, 0, wanted, wanted, 1);
    }
    return (void *)aligned;
}

static rxvm_memory_slab *rxvm_memory_find_slab(const void *pointer) {
    uintptr_t address;
    rxvm_memory_slab *slab;
    uintptr_t payload;
    uintptr_t offset;

    if (!pointer) return 0;
    address = (uintptr_t)pointer;
    slab = (rxvm_memory_slab *)(address &
            ~((uintptr_t)RXVM_MEMORY_SLAB_SIZE - 1u));
    if (slab->magic != RXVM_MEMORY_SLAB_MAGIC ||
        slab->version_reserved != RXVM_MEMORY_HEADER_VERSION ||
        !slab->slot_size || slab->class_id >= RXVM_MEMORY_CLASS_COUNT) {
        return 0;
    }
    payload = (uintptr_t)rxvm_memory_slab_payload(slab);
    if (address < payload || address >= (uintptr_t)slab + RXVM_MEMORY_SLAB_SIZE) {
        return 0;
    }
    offset = address - payload;
    if (offset % slab->slot_size != 0 ||
        offset / slab->slot_size >= slab->slot_count) {
        return 0;
    }
    return slab;
}

static rxvm_memory_extent *rxvm_memory_find_extent(const void *pointer) {
    rxvm_memory_extent *extent;
    if (!pointer) return 0;
    extent = (rxvm_memory_extent *)((const unsigned char *)pointer -
                                    sizeof(*extent));
    if (extent->magic != RXVM_MEMORY_EXTENT_MAGIC) {
        return 0;
    }
    return extent;
}

static rxvm_memory_result rxvm_memory_release_standard(rxvm_memory_slab *slab,
                                                       void *pointer) {
    rxvm_memory_worker *worker = slab->owner;
    rxvm_memory_class_state *state;

    if (!worker || slab->owner_generation != worker->generation) {
        return RXVM_MEMORY_INVALID_POINTER;
    }
    if (!rxvm_memory_worker_is_current_thread_owner(worker) ||
        (rxvm_memory_tls_worker && rxvm_memory_tls_worker != worker)) {
        rxvm_memory_record_wrong_owner(
                worker, RXVM_MEMORY_WRONG_OWNER_FREE);
        return RXVM_MEMORY_WRONG_OWNER;
    }
    if (!slab->live_count ||
        ((uintptr_t)pointer - (uintptr_t)rxvm_memory_slab_payload(slab)) /
                slab->slot_size >= slab->bump_count) {
        worker->stats.invalid_frees++;
        return RXVM_MEMORY_INVALID_POINTER;
    }
#ifndef NDEBUG
    {
        void *free_slot = slab->local_free;
        while (free_slot) {
            if (free_slot == pointer) {
                worker->stats.invalid_frees++;
                return RXVM_MEMORY_INVALID_POINTER;
            }
            free_slot = *(void **)free_slot;
        }
    }
#endif

    state = &worker->classes[slab->class_id];
    if (!(slab->flags & RXVM_MEMORY_SLAB_FLAG_AVAILABLE)) {
        rxvm_memory_class_add_available(worker, slab);
    }
    *(void **)pointer = slab->local_free;
    slab->local_free = pointer;
    slab->live_count--;
    rxvm_memory_stats_free(worker, slab->class_id, slab->slot_size, 0);

    if (slab->live_count == 0) {
        slab->flags |= RXVM_MEMORY_SLAB_FLAG_EMPTY;
        if (!state->empty) {
            state->empty = slab;
        } else if (state->empty != slab) {
            rxvm_memory_depot_return(worker, slab);
        }
    }
    return RXVM_MEMORY_OK;
}

static rxvm_memory_result rxvm_memory_release_extent(rxvm_memory_extent *extent) {
    rxvm_memory_worker *worker = extent->owner;

    if (!extent->system_pointer) {
        return RXVM_MEMORY_INVALID_POINTER;
    }
    if (worker &&
        (!rxvm_memory_worker_is_current_thread_owner(worker) ||
         (rxvm_memory_tls_worker && rxvm_memory_tls_worker != worker))) {
        rxvm_memory_record_wrong_owner(
                worker, RXVM_MEMORY_WRONG_OWNER_FREE);
        return RXVM_MEMORY_WRONG_OWNER;
    }
    if (worker) {
        if (extent->previous) {
            extent->previous->next = extent->next;
        } else {
            worker->oversized = extent->next;
        }
        if (extent->next) extent->next->previous = extent->previous;
        rxvm_memory_stats_free(worker, 0, extent->capacity, 1);
    }
    extent->magic = 0;
    free(extent->system_pointer);
    return RXVM_MEMORY_OK;
}

rxvm_memory_context *rxvm_memory_context_create(void) {
    rxvm_memory_context *context =
            (rxvm_memory_context *)calloc(1, sizeof(*context));
    if (!context) return 0;
    rxvm_memory_mutex_init(&context->mutex);
    context->next_worker_id = 1u;
    context->next_generation = 1u;
    return context;
}

rxvm_memory_worker *rxvm_memory_worker_create(rxvm_memory_context *context) {
    rxvm_memory_worker *worker;
    if (!context) return 0;
    worker = (rxvm_memory_worker *)calloc(1, sizeof(*worker));
    if (!worker) return 0;
    worker->context = context;
    worker->owner_thread_token = rxvm_memory_current_thread_token();

    rxvm_memory_mutex_lock(&context->mutex);
    worker->id = context->next_worker_id++;
    if (!context->next_worker_id) context->next_worker_id = 1u;
    worker->generation = context->next_generation++;
    if (!context->next_generation) context->next_generation = 1u;
    worker->context_next = context->workers;
    context->workers = worker;
    rxvm_memory_mutex_unlock(&context->mutex);
    return worker;
}

rxvm_memory_worker *rxvm_memory_enter(rxvm_memory_worker *worker) {
    rxvm_memory_worker *previous = rxvm_memory_tls_worker;
    if (worker && !rxvm_memory_worker_is_current_thread_owner(worker)) {
        fprintf(stderr,
                "RXVM memory worker entry rejected: wrong owner thread\n");
        abort();
    }
    rxvm_memory_tls_worker = worker;
    return previous;
}

void rxvm_memory_leave(rxvm_memory_worker *previous_worker) {
    rxvm_memory_tls_worker = previous_worker;
}

rxvm_memory_worker *rxvm_memory_current_worker(void) {
    return rxvm_memory_tls_worker;
}

int rxvm_memory_worker_is_current_thread_owner(
        const rxvm_memory_worker *worker) {
    return worker && worker->owner_thread_token != 0u &&
           worker->owner_thread_token == rxvm_memory_current_thread_token();
}

void *rxvm_memory_alloc_bytes(rxvm_memory_worker *worker, size_t size) {
    uint8_t class_id;
    if (!worker) worker = rxvm_memory_tls_worker;
    if (!worker) return rxvm_memory_alloc_oversized(0, size);
    if (rxvm_memory_byte_class(size, &class_id)) {
        return rxvm_memory_alloc_standard(worker, class_id, size ? size : 1u);
    }
    return rxvm_memory_alloc_oversized(worker, size);
}

void *rxvm_memory_alloc_unowned_bytes(size_t size) {
    return rxvm_memory_alloc_oversized(0, size);
}

void *rxvm_memory_calloc_bytes(rxvm_memory_worker *worker,
                               size_t count,
                               size_t size) {
    size_t total;
    void *result;
    if (rxvm_memory_size_multiply_overflows(count, size, &total)) {
        if (!worker) worker = rxvm_memory_tls_worker;
        if (worker && !rxvm_memory_worker_is_current_thread_owner(worker)) {
            rxvm_memory_record_wrong_owner(
                    worker, RXVM_MEMORY_WRONG_OWNER_ALLOCATION);
        } else if (worker) {
            worker->stats.allocation_failures++;
        }
        return 0;
    }
    result = rxvm_memory_alloc_bytes(worker, total);
    if (result) memset(result, 0, total);
    return result;
}

void *rxvm_memory_alloc_values(rxvm_memory_worker *worker, size_t count) {
    uint8_t class_id;
    size_t requested;
    if (!worker) worker = rxvm_memory_tls_worker;
    if (!worker) {
        size_t fallback_size;
        if (rxvm_memory_size_multiply_overflows(count ? count : 1u,
                                                sizeof(value),
                                                &fallback_size)) {
            return 0;
        }
        return rxvm_memory_alloc_oversized(0, fallback_size);
    }
    if (rxvm_memory_size_multiply_overflows(count ? count : 1u,
                                            sizeof(value), &requested)) {
        if (!rxvm_memory_worker_is_current_thread_owner(worker)) {
            rxvm_memory_record_wrong_owner(
                    worker, RXVM_MEMORY_WRONG_OWNER_ALLOCATION);
        } else {
            worker->stats.allocation_failures++;
        }
        return 0;
    }
    if (rxvm_memory_value_class(count, &class_id)) {
        return rxvm_memory_alloc_standard(worker, class_id, requested);
    }
    return rxvm_memory_alloc_oversized(worker, requested);
}

void *rxvm_memory_alloc_reference_cell(rxvm_memory_worker *worker) {
    if (!worker) worker = rxvm_memory_tls_worker;
    if (!worker) return rxvm_memory_alloc_oversized(0,
                                                   sizeof(rxvm_reference_cell));
    return rxvm_memory_alloc_standard(worker, RXVM_MEMORY_CLASS_REFERENCE,
                                      sizeof(rxvm_reference_cell));
}

void *rxvm_memory_resize_bytes(rxvm_memory_worker *worker,
                               void *pointer,
                               size_t copy_size,
                               size_t new_size) {
    size_t old_capacity;
    void *replacement;
    rxvm_memory_worker *owner;

    if (!pointer) return rxvm_memory_alloc_bytes(worker, new_size);
    if (!new_size) {
        (void)rxvm_memory_release(pointer);
        return 0;
    }
    owner = rxvm_memory_owner(pointer);
    if (!owner) {
        rxvm_memory_extent *extent = rxvm_memory_find_extent(pointer);
        if (!extent) return 0;
        if (new_size <= extent->capacity) return pointer;
        /* Detached/process-global extents must not adopt the active worker. */
        replacement = rxvm_memory_alloc_unowned_bytes(new_size);
        if (!replacement) return 0;
        old_capacity = extent->capacity;
        if (copy_size > old_capacity) copy_size = old_capacity;
        if (copy_size > new_size) copy_size = new_size;
        if (copy_size) memcpy(replacement, pointer, copy_size);
        if (rxvm_memory_release(pointer) != RXVM_MEMORY_OK) {
            (void)rxvm_memory_release(replacement);
            return 0;
        }
        return replacement;
    }
    if (!worker) worker = owner;
    if (!rxvm_memory_worker_is_current_thread_owner(owner) ||
        (rxvm_memory_tls_worker && rxvm_memory_tls_worker != owner) ||
        worker != owner) {
        rxvm_memory_record_wrong_owner(
                owner, RXVM_MEMORY_WRONG_OWNER_RESIZE);
        return 0;
    }
    worker->stats.reallocation_calls++;
    old_capacity = rxvm_memory_capacity(pointer);
    if (new_size <= old_capacity && old_capacity <= RXVM_MEMORY_MAX_STANDARD_SIZE) {
        return pointer;
    }
    replacement = rxvm_memory_alloc_bytes(worker, new_size);
    if (!replacement) return 0;
#ifdef CREXX_VM_MEMORY_CENSUS
    if (old_capacity > RXVM_MEMORY_MAX_STANDARD_SIZE) {
        if (new_size > RXVM_MEMORY_MAX_STANDARD_SIZE)
            worker->stats.resize_oversized_to_oversized++;
        else
            worker->stats.resize_oversized_to_standard++;
    } else if (new_size > RXVM_MEMORY_MAX_STANDARD_SIZE) {
        worker->stats.resize_standard_to_oversized++;
    }
#endif
    if (copy_size > old_capacity) copy_size = old_capacity;
    if (copy_size > new_size) copy_size = new_size;
    if (copy_size) memcpy(replacement, pointer, copy_size);
    if (rxvm_memory_release(pointer) != RXVM_MEMORY_OK) {
        (void)rxvm_memory_release(replacement);
        return 0;
    }
    return replacement;
}

rxvm_memory_result rxvm_memory_release(void *pointer) {
    rxvm_memory_slab *slab;
    rxvm_memory_extent *extent;
    if (!pointer) return RXVM_MEMORY_OK;
    extent = rxvm_memory_find_extent(pointer);
    if (extent) return rxvm_memory_release_extent(extent);
    slab = rxvm_memory_find_slab(pointer);
    if (slab) return rxvm_memory_release_standard(slab, pointer);
    if (rxvm_memory_tls_worker) rxvm_memory_tls_worker->stats.invalid_frees++;
    return RXVM_MEMORY_INVALID_POINTER;
}

size_t rxvm_memory_capacity(const void *pointer) {
    rxvm_memory_extent *extent = rxvm_memory_find_extent(pointer);
    rxvm_memory_slab *slab;
    if (extent) return extent->capacity;
    slab = rxvm_memory_find_slab(pointer);
    return slab ? slab->slot_size : 0u;
}

rxvm_memory_worker *rxvm_memory_owner(const void *pointer) {
    rxvm_memory_extent *extent = rxvm_memory_find_extent(pointer);
    rxvm_memory_slab *slab;
    if (extent) return extent->owner;
    slab = rxvm_memory_find_slab(pointer);
    return slab ? slab->owner : 0;
}

int rxvm_memory_is_owned(const void *pointer) {
    return rxvm_memory_find_extent(pointer) != 0 ||
           rxvm_memory_find_slab(pointer) != 0;
}

void rxvm_memory_trim(rxvm_memory_context *context) {
    rxvm_memory_slab *release = 0;
    uint8_t class_id;
    if (!context) return;

    rxvm_memory_mutex_lock(&context->mutex);
    context->trim_calls++;
    for (class_id = 0; class_id < RXVM_MEMORY_CLASS_COUNT; class_id++) {
        rxvm_memory_slab *slab = context->reserve[class_id];
        while (slab) {
            rxvm_memory_slab *next = slab->class_next;
            slab->class_next = release;
            release = slab;
            slab = next;
            context->standard_slabs_to_system++;
        }
        context->reserve[class_id] = 0;
        context->reserve_count[class_id] = 0;
    }
    context->reserve_total = 0;
    rxvm_memory_mutex_unlock(&context->mutex);

    while (release) {
        rxvm_memory_slab *next = release->class_next;
        rxvm_memory_system_slab_free(release);
        release = next;
    }
}

void rxvm_memory_get_stats(const rxvm_memory_context *context_const,
                           rxvm_memory_stats *stats) {
    rxvm_memory_context *context = (rxvm_memory_context *)context_const;
    rxvm_memory_worker *worker;
    uint8_t class_id;
    if (!stats) return;
    memset(stats, 0, sizeof(*stats));
    if (!context) return;

    rxvm_memory_mutex_lock(&context->mutex);
    stats->standard_slabs_reserved = context->reserve_total;
    stats->standard_slabs_from_system = context->standard_slabs_from_system;
    stats->standard_slabs_to_system = context->standard_slabs_to_system;
    stats->depot_refills = context->depot_refills;
    stats->depot_returns = context->depot_returns;
    stats->depot_hits = context->depot_hits;
    stats->trim_calls = context->trim_calls;
    stats->wrong_owner_allocations = context->wrong_owner_allocations;
    stats->wrong_owner_resizes = context->wrong_owner_resizes;
    stats->wrong_owner_frees = context->wrong_owner_frees;
    worker = context->workers;
    while (worker) {
#define RXVM_MEMORY_ADD_STAT(field) stats->field += worker->stats.field
        RXVM_MEMORY_ADD_STAT(allocation_calls);
        RXVM_MEMORY_ADD_STAT(free_calls);
        RXVM_MEMORY_ADD_STAT(reallocation_calls);
        RXVM_MEMORY_ADD_STAT(allocation_failures);
        RXVM_MEMORY_ADD_STAT(invalid_frees);
        RXVM_MEMORY_ADD_STAT(cumulative_requested_bytes);
        RXVM_MEMORY_ADD_STAT(cumulative_capacity_bytes);
        RXVM_MEMORY_ADD_STAT(live_allocations);
        RXVM_MEMORY_ADD_STAT(live_capacity_bytes);
        RXVM_MEMORY_ADD_STAT(oversized_live_allocations);
        RXVM_MEMORY_ADD_STAT(oversized_live_bytes);
#ifdef CREXX_VM_MEMORY_CENSUS
        RXVM_MEMORY_ADD_STAT(oversized_allocation_calls);
        RXVM_MEMORY_ADD_STAT(cumulative_oversized_requested_bytes);
        RXVM_MEMORY_ADD_STAT(resize_standard_to_oversized);
        RXVM_MEMORY_ADD_STAT(resize_oversized_to_standard);
        RXVM_MEMORY_ADD_STAT(resize_oversized_to_oversized);
#endif
        RXVM_MEMORY_ADD_STAT(standard_slabs_owned);
#undef RXVM_MEMORY_ADD_STAT
        if (worker->stats.peak_live_allocations > stats->peak_live_allocations) {
            stats->peak_live_allocations = worker->stats.peak_live_allocations;
        }
        if (worker->stats.peak_live_capacity_bytes >
            stats->peak_live_capacity_bytes) {
            stats->peak_live_capacity_bytes =
                    worker->stats.peak_live_capacity_bytes;
        }
#ifdef CREXX_VM_MEMORY_CENSUS
        if (worker->stats.peak_oversized_live_allocations >
                stats->peak_oversized_live_allocations)
            stats->peak_oversized_live_allocations =
                    worker->stats.peak_oversized_live_allocations;
        if (worker->stats.peak_oversized_live_bytes >
                stats->peak_oversized_live_bytes)
            stats->peak_oversized_live_bytes =
                    worker->stats.peak_oversized_live_bytes;
        if (worker->stats.maximum_oversized_request_bytes >
                stats->maximum_oversized_request_bytes)
            stats->maximum_oversized_request_bytes =
                    worker->stats.maximum_oversized_request_bytes;
        {
            size_t bucket;
            for (bucket = 0;
                 bucket < RXVM_MEMORY_SIZE_HISTOGRAM_BUCKETS; bucket++)
                stats->oversized_request_histogram[bucket] +=
                        worker->stats.oversized_request_histogram[bucket];
        }
#endif
        for (class_id = 0; class_id < RXVM_MEMORY_CLASS_COUNT; class_id++) {
            stats->class_allocation_calls[class_id] +=
                    worker->classes[class_id].allocation_calls;
            stats->class_live_allocations[class_id] +=
                    worker->classes[class_id].live_allocations;
            if (worker->classes[class_id].peak_live_allocations >
                stats->class_peak_live_allocations[class_id]) {
                stats->class_peak_live_allocations[class_id] =
                        worker->classes[class_id].peak_live_allocations;
            }
        }
        worker = worker->context_next;
    }
    rxvm_memory_mutex_unlock(&context->mutex);
}

size_t rxvm_memory_worker_destroy(rxvm_memory_worker *worker) {
    rxvm_memory_context *context;
    rxvm_memory_worker **cursor;
    rxvm_memory_slab *slab;
    rxvm_memory_extent *extent;
    size_t leaks;
    if (!worker) return 0;
    if (!rxvm_memory_worker_is_current_thread_owner(worker)) {
        fprintf(stderr,
                "RXVM memory worker teardown rejected: wrong owner thread\n");
        abort();
    }
    context = worker->context;
    leaks = (size_t)worker->stats.live_allocations;
    if (rxvm_memory_tls_worker == worker) rxvm_memory_tls_worker = 0;

    rxvm_memory_mutex_lock(&context->mutex);
    cursor = &context->workers;
    while (*cursor) {
        if (*cursor == worker) {
            *cursor = worker->context_next;
            break;
        }
        cursor = &(*cursor)->context_next;
    }
    rxvm_memory_mutex_unlock(&context->mutex);

    extent = worker->oversized;
    while (extent) {
        rxvm_memory_extent *next = extent->next;
        extent->magic = 0;
        free(extent->system_pointer);
        extent = next;
    }
    worker->oversized = 0;

    slab = worker->owned_slabs;
    while (slab) {
        rxvm_memory_slab *next = slab->owner_next;
        slab->owner_next = 0;
        slab->live_count = 0;
        rxvm_memory_depot_return(worker, slab);
        slab = next;
    }
    free(worker);
    return leaks;
}

size_t rxvm_memory_context_destroy(rxvm_memory_context *context) {
    size_t leaks = 0;
    if (!context) return 0;
    while (context->workers) {
        leaks += rxvm_memory_worker_destroy(context->workers);
    }
    rxvm_memory_trim(context);
    rxvm_memory_mutex_destroy(&context->mutex);
    free(context);
    return leaks;
}
