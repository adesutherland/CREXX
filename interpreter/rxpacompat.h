/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#ifndef CREXX_RXPACOMPAT_H
#define CREXX_RXPACOMPAT_H

#include "rxvalue.h"
#include "rxvmmemory.h"

#include <stddef.h>
#include <stdint.h>

typedef void (*rxvm_native_invoker)(void *function, int args, value **argv,
                                    value *ret, value *signal);
typedef void (*rxpa_session_destroy_function)(void *session);
typedef int (*rxpa_session_enter_function)(void *session,
                                           uint32_t capabilities,
                                           void **previous);
typedef void (*rxpa_session_leave_function)(void *previous);

typedef struct rxpa_session_instance {
    const char *plugin_id;
    void *session;
    rxpa_session_destroy_function destroy;
    rxpa_session_enter_function enter;
    rxpa_session_leave_function leave;
    struct rxpa_session_instance *next;
} rxpa_session_instance;

typedef struct rxpa_session_call_binding {
    void *function;
    rxpa_session_instance *instance;
    uint32_t procedure_capabilities;
    struct rxpa_session_call_binding *next;
} rxpa_session_call_binding;

/*
 * Cold process-wide coordination for legacy RXPA procedures.  The invoker
 * slots belong to one VM context; the coordinator only keeps their addresses
 * while that context is registered and quiesces all direct legacy execution
 * before changing them.
 */
typedef struct rxpa_compatibility_context {
    rxvm_memory_worker *memory_worker;
    rxvm_native_invoker **legacy_invoker_slots;
    size_t legacy_invoker_count;
    size_t legacy_invoker_capacity;
    size_t execution_depth;
    struct rxpa_compatibility_context *coordinator_next;
    rxvm_native_invoker direct_invoker;
    rxvm_native_invoker locked_invoker;
    unsigned char legacy_registered;
} rxpa_compatibility_context;

void rxpa_compatibility_enter(void);
void rxpa_compatibility_leave(void);
void rxpa_compatibility_context_init(rxpa_compatibility_context *context,
                                     rxvm_memory_worker *memory_worker);
void rxpa_compatibility_context_destroy(rxpa_compatibility_context *context);
int rxpa_compatibility_bind_legacy(
        rxpa_compatibility_context *context,
        rxvm_native_invoker *invoker_slot,
        rxvm_native_invoker direct_invoker,
        rxvm_native_invoker locked_invoker);
void rxpa_compatibility_execution_enter(
        rxpa_compatibility_context *context);
void rxpa_compatibility_execution_leave(
        rxpa_compatibility_context *context);
void rxvm_native_payload_copy_call(const rxvm_native_payload_ops *ops,
                                   value *dest, value *source);
void rxvm_native_payload_finalize_call(const rxvm_native_payload_ops *ops,
                                       value *payload);

#endif
