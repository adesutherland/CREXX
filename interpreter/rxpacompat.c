/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxpacompat.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct rxpa_compatibility_coordinator {
    rxpa_compatibility_context *legacy_contexts;
    size_t legacy_context_count;
    size_t active_legacy_executions;
    unsigned char transitioning;
    unsigned char locked_mode;
} rxpa_compatibility_coordinator;

static rxpa_compatibility_coordinator rxpa_coordinator;

#ifdef _WIN32
#include <windows.h>

static INIT_ONCE rxpa_compatibility_once = INIT_ONCE_STATIC_INIT;
static CRITICAL_SECTION rxpa_compatibility_lock;
static SRWLOCK rxpa_coordinator_lock = SRWLOCK_INIT;
static CONDITION_VARIABLE rxpa_coordinator_condition =
        CONDITION_VARIABLE_INIT;

static BOOL CALLBACK rxpa_init_compatibility_lock(PINIT_ONCE once,
                                                   PVOID parameter,
                                                   PVOID *context) {
    (void)once;
    (void)parameter;
    (void)context;
    InitializeCriticalSection(&rxpa_compatibility_lock);
    return TRUE;
}

void rxpa_compatibility_enter(void) {
    InitOnceExecuteOnce(&rxpa_compatibility_once,
                        rxpa_init_compatibility_lock, NULL, NULL);
    EnterCriticalSection(&rxpa_compatibility_lock);
}

void rxpa_compatibility_leave(void) {
    LeaveCriticalSection(&rxpa_compatibility_lock);
}

static void rxpa_coordinator_enter(void) {
    AcquireSRWLockExclusive(&rxpa_coordinator_lock);
}

static void rxpa_coordinator_leave(void) {
    ReleaseSRWLockExclusive(&rxpa_coordinator_lock);
}

static void rxpa_coordinator_wait(void) {
    if (!SleepConditionVariableSRW(&rxpa_coordinator_condition,
                                   &rxpa_coordinator_lock, INFINITE, 0)) {
        abort();
    }
}

static void rxpa_coordinator_broadcast(void) {
    WakeAllConditionVariable(&rxpa_coordinator_condition);
}
#else
#include <pthread.h>

static pthread_once_t rxpa_compatibility_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t rxpa_compatibility_lock;
static pthread_mutex_t rxpa_coordinator_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t rxpa_coordinator_condition = PTHREAD_COND_INITIALIZER;

static void rxpa_init_compatibility_lock(void) {
    pthread_mutexattr_t attributes;
    if (pthread_mutexattr_init(&attributes) != 0 ||
        pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE) != 0 ||
        pthread_mutex_init(&rxpa_compatibility_lock, &attributes) != 0) {
        abort();
    }
    (void)pthread_mutexattr_destroy(&attributes);
}

void rxpa_compatibility_enter(void) {
    if (pthread_once(&rxpa_compatibility_once,
                     rxpa_init_compatibility_lock) != 0 ||
        pthread_mutex_lock(&rxpa_compatibility_lock) != 0) {
        abort();
    }
}

void rxpa_compatibility_leave(void) {
    if (pthread_mutex_unlock(&rxpa_compatibility_lock) != 0) abort();
}

static void rxpa_coordinator_enter(void) {
    if (pthread_mutex_lock(&rxpa_coordinator_lock) != 0) abort();
}

static void rxpa_coordinator_leave(void) {
    if (pthread_mutex_unlock(&rxpa_coordinator_lock) != 0) abort();
}

static void rxpa_coordinator_wait(void) {
    if (pthread_cond_wait(&rxpa_coordinator_condition,
                          &rxpa_coordinator_lock) != 0) {
        abort();
    }
}

static void rxpa_coordinator_broadcast(void) {
    if (pthread_cond_broadcast(&rxpa_coordinator_condition) != 0) abort();
}
#endif

void rxpa_compatibility_test_wait_transition_started(void) {
    rxpa_coordinator_enter();
    while (!rxpa_coordinator.transitioning) rxpa_coordinator_wait();
    rxpa_coordinator_leave();
}

void rxpa_compatibility_context_init(rxpa_compatibility_context *context,
                                     rxvm_memory_worker *memory_worker) {
    if (!context || !memory_worker) abort();
    context->memory_worker = memory_worker;
    context->legacy_invoker_slots = NULL;
    context->legacy_invoker_count = 0u;
    context->legacy_invoker_capacity = 0u;
    context->execution_depth = 0u;
    context->coordinator_next = NULL;
    context->direct_invoker = NULL;
    context->locked_invoker = NULL;
    context->legacy_registered = 0u;
}

static int rxpa_compatibility_grow_slots(
        rxpa_compatibility_context *context) {
    size_t old_size;
    size_t new_size;
    size_t new_capacity;
    rxvm_native_invoker **new_slots;

    if (context->legacy_invoker_count < context->legacy_invoker_capacity) {
        return 1;
    }
    new_capacity = context->legacy_invoker_capacity
                       ? context->legacy_invoker_capacity * 2u
                       : 8u;
    if (new_capacity < context->legacy_invoker_capacity ||
        new_capacity > SIZE_MAX / sizeof(*new_slots)) {
        return 0;
    }
    old_size = context->legacy_invoker_count * sizeof(*new_slots);
    new_size = new_capacity * sizeof(*new_slots);
    new_slots = (rxvm_native_invoker **)rxvm_memory_resize_bytes(
            context->memory_worker, context->legacy_invoker_slots,
            old_size, new_size);
    if (!new_slots) return 0;
    context->legacy_invoker_slots = new_slots;
    context->legacy_invoker_capacity = new_capacity;
    return 1;
}

static void rxpa_compatibility_rebind_all_legacy(void) {
    rxpa_compatibility_context *context = rxpa_coordinator.legacy_contexts;
    while (context) {
        size_t index;
        for (index = 0u; index < context->legacy_invoker_count; index++) {
            *context->legacy_invoker_slots[index] = context->locked_invoker;
        }
        context = context->coordinator_next;
    }
}

int rxpa_compatibility_bind_legacy(
        rxpa_compatibility_context *context,
        rxvm_native_invoker *invoker_slot,
        rxvm_native_invoker direct_invoker,
        rxvm_native_invoker locked_invoker) {
    int first_legacy_procedure;
    int started_transition = 0;
    size_t index;

    if (!context || !context->memory_worker || !invoker_slot ||
        !direct_invoker || !locked_invoker) {
        return 0;
    }

    rxpa_coordinator_enter();
    while (rxpa_coordinator.transitioning &&
           !(context->legacy_registered && context->execution_depth != 0u)) {
        rxpa_coordinator_wait();
    }
    for (index = 0u; index < context->legacy_invoker_count; index++) {
        if (context->legacy_invoker_slots[index] == invoker_slot) {
            *invoker_slot = rxpa_coordinator.locked_mode
                                ? locked_invoker
                                : direct_invoker;
            rxpa_coordinator_leave();
            return 1;
        }
    }
    if (!rxpa_compatibility_grow_slots(context)) {
        rxpa_coordinator_leave();
        return 0;
    }

    first_legacy_procedure = !context->legacy_registered;
    if (!first_legacy_procedure &&
        (context->direct_invoker != direct_invoker ||
         context->locked_invoker != locked_invoker)) {
        rxpa_coordinator_leave();
        abort();
    }

    if (first_legacy_procedure && !rxpa_coordinator.locked_mode &&
        rxpa_coordinator.legacy_context_count != 0u) {
        /*
         * Stop new direct legacy execution and wait for every existing direct
         * legacy executor to leave its VM execution boundary before rebinding.
         * The new context is paused in its loader and is not published yet.
        */
        rxpa_coordinator.transitioning = 1u;
        started_transition = 1;
        rxpa_coordinator_broadcast();
        while (rxpa_coordinator.active_legacy_executions != 0u) {
            rxpa_coordinator_wait();
        }
        rxpa_compatibility_rebind_all_legacy();
        rxpa_coordinator.locked_mode = 1u;
    }

    if (first_legacy_procedure) {
        context->direct_invoker = direct_invoker;
        context->locked_invoker = locked_invoker;
        context->coordinator_next = rxpa_coordinator.legacy_contexts;
        rxpa_coordinator.legacy_contexts = context;
        rxpa_coordinator.legacy_context_count++;
        context->legacy_registered = 1u;
        if (!rxpa_coordinator.locked_mode && context->execution_depth != 0u) {
            rxpa_coordinator.active_legacy_executions++;
        }
    }

    *invoker_slot = rxpa_coordinator.locked_mode
                        ? locked_invoker
                        : direct_invoker;
    context->legacy_invoker_slots[context->legacy_invoker_count++] =
            invoker_slot;

    if (started_transition) {
        rxpa_coordinator.transitioning = 0u;
        rxpa_coordinator_broadcast();
    }
    rxpa_coordinator_leave();
    return 1;
}

void rxpa_compatibility_execution_enter(
        rxpa_compatibility_context *context) {
    if (!context || !context->memory_worker) abort();
    rxpa_coordinator_enter();
    while (rxpa_coordinator.transitioning && context->legacy_registered &&
           context->execution_depth == 0u) {
        rxpa_coordinator_wait();
    }
    if (context->execution_depth == SIZE_MAX) {
        rxpa_coordinator_leave();
        abort();
    }
    if (context->execution_depth++ == 0u && context->legacy_registered &&
        !rxpa_coordinator.locked_mode) {
        rxpa_coordinator.active_legacy_executions++;
    }
    rxpa_coordinator_leave();
}

void rxpa_compatibility_execution_leave(
        rxpa_compatibility_context *context) {
    if (!context || !context->memory_worker) abort();
    rxpa_coordinator_enter();
    if (context->execution_depth == 0u) {
        rxpa_coordinator_leave();
        abort();
    }
    context->execution_depth--;
    if (context->execution_depth == 0u && context->legacy_registered &&
        !rxpa_coordinator.locked_mode) {
        if (rxpa_coordinator.active_legacy_executions == 0u) {
            rxpa_coordinator_leave();
            abort();
        }
        rxpa_coordinator.active_legacy_executions--;
        rxpa_coordinator_broadcast();
    }
    rxpa_coordinator_leave();
}

void rxpa_compatibility_context_destroy(rxpa_compatibility_context *context) {
    if (!context || !context->memory_worker) return;
    rxpa_coordinator_enter();
    while (rxpa_coordinator.transitioning) rxpa_coordinator_wait();
    if (context->execution_depth != 0u) {
        rxpa_coordinator_leave();
        abort();
    }
    if (context->legacy_registered) {
        rxpa_compatibility_context **link =
                &rxpa_coordinator.legacy_contexts;
        while (*link && *link != context) link = &(*link)->coordinator_next;
        if (*link != context || rxpa_coordinator.legacy_context_count == 0u) {
            rxpa_coordinator_leave();
            abort();
        }
        *link = context->coordinator_next;
        rxpa_coordinator.legacy_context_count--;
    }
    rxpa_coordinator_leave();

    if (context->legacy_invoker_slots) {
        (void)rxvm_memory_release(context->legacy_invoker_slots);
    }
    context->memory_worker = NULL;
    context->legacy_invoker_slots = NULL;
    context->legacy_invoker_count = 0u;
    context->legacy_invoker_capacity = 0u;
    context->execution_depth = 0u;
    context->coordinator_next = NULL;
    context->direct_invoker = NULL;
    context->locked_invoker = NULL;
    context->legacy_registered = 0u;
}

void rxvm_native_payload_copy_call(const rxvm_native_payload_ops *ops,
                                   value *dest, value *source) {
    if (!ops || !ops->copy) return;
    rxpa_compatibility_enter();
    ops->copy(dest, source);
    rxpa_compatibility_leave();
}

void rxvm_native_payload_finalize_call(const rxvm_native_payload_ops *ops,
                                       value *payload) {
    if (!ops || !ops->finalize) return;
    rxpa_compatibility_enter();
    ops->finalize(payload);
    rxpa_compatibility_leave();
}
