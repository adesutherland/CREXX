/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmintp.h"
#include "rxvmvars.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

void rxvm_addfunc(rxpa_libfunc func, char *name, char *option,
                  char *type, char *args);
void rxvm_addfunc_for_plugin(const char *plugin_id, rxpa_libfunc func,
                             char *name, char *option, char *type, char *args);
void rxvm_register_static_plugin_capability(const char *plugin_id,
                                            uint32_t capabilities);
void rxvm_register_static_plugin_manifest_v2(
        const rxpa_plugin_manifest_v2 *manifest);

typedef struct call_gate {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
#else
    pthread_mutex_t mutex;
    pthread_cond_t condition;
#endif
    int ready;
    int release;
    int attempting;
    int active;
    int maximum_active;
} call_gate;

static call_gate legacy_gate;
static call_gate static_gate;
static call_gate dynamic_gate;
static call_gate binding_gate;
static call_gate session_gate;
static uint32_t call_capabilities;
static const char *dynamic_directory;
static const char *dynamic_plugin_base;
static const char *dynamic_call_kind;
static int dynamic_failures;
static int dynamic_second_destroyed;
static int recursive_inner_called;
static int static_failures;
static int binding_failures;
static int session_failures;
static int session_created;
static int session_destroyed;
static int session_live;
static int session_ids[2];

#if defined(_MSC_VER)
#define TEST_THREAD_LOCAL __declspec(thread)
#else
#define TEST_THREAD_LOCAL __thread
#endif

typedef struct test_plugin_session {
    int id;
    int calls;
} test_plugin_session;

static TEST_THREAD_LOCAL test_plugin_session *current_test_session;
static test_plugin_session session_sentinel;

static void legacy_probe(rxinteger numargs, rxpa_attribute_value *args,
                         rxpa_attribute_value result,
                         rxpa_attribute_value signal);
static int call_plugin_procedure(rxvm_context *context, const char *kind);
static proc_runtime *context_find_procedure(rxvm_context *context,
                                            const char *name);
static uint32_t context_procedure_capabilities(rxvm_context *context,
                                               const char *name);
static rxvm_native_invoker context_procedure_invoker(
        rxvm_context *context, const char *name);
static int expect_float_native_arity_signal(rxvm_context *context,
                                            const char *procedure_name,
                                            int argument_count);

static int value_string_equals(const value *actual, const char *expected) {
    size_t expected_length = strlen(expected);
    return actual && actual->string_value &&
           actual->string_length == expected_length &&
           memcmp(actual->string_value, expected, expected_length) == 0;
}

static int value_binary_equals(value *actual,
                               const unsigned char *expected,
                               size_t expected_length) {
    size_t actual_length = 0u;
    void *actual_payload = get_native_payload(actual, &actual_length,
                                              NULL, NULL);
    return actual_length == expected_length &&
           (expected_length == 0u ||
            (actual_payload &&
             memcmp(actual_payload, expected, expected_length) == 0));
}

static void gate_lock(call_gate *gate) {
#ifdef _WIN32
    EnterCriticalSection(&gate->mutex);
#else
    (void)pthread_mutex_lock(&gate->mutex);
#endif
}

static void gate_unlock(call_gate *gate) {
#ifdef _WIN32
    LeaveCriticalSection(&gate->mutex);
#else
    (void)pthread_mutex_unlock(&gate->mutex);
#endif
}

static void gate_wait(call_gate *gate) {
#ifdef _WIN32
    SleepConditionVariableCS(&gate->condition, &gate->mutex, INFINITE);
#else
    (void)pthread_cond_wait(&gate->condition, &gate->mutex);
#endif
}

static void gate_broadcast(call_gate *gate) {
#ifdef _WIN32
    WakeAllConditionVariable(&gate->condition);
#else
    (void)pthread_cond_broadcast(&gate->condition);
#endif
}

static void gate_init(call_gate *gate) {
    memset(gate, 0, sizeof(*gate));
#ifdef _WIN32
    InitializeCriticalSection(&gate->mutex);
    InitializeConditionVariable(&gate->condition);
#else
    (void)pthread_mutex_init(&gate->mutex, NULL);
    (void)pthread_cond_init(&gate->condition, NULL);
#endif
}

static void gate_destroy(call_gate *gate) {
#ifdef _WIN32
    DeleteCriticalSection(&gate->mutex);
#else
    (void)pthread_cond_destroy(&gate->condition);
    (void)pthread_mutex_destroy(&gate->mutex);
#endif
}

/* The coordinator has no externally visible "transition is blocked" event,
 * so this negative assertion retains a bounded observation window. Positive
 * overlap and legacy serialization use condition-variable handshakes. */
static void observe_legacy_transition_blocked(void) {
#ifdef _WIN32
    Sleep(50);
#else
    (void)usleep(50000);
#endif
}

static void static_probe(rxinteger numargs, rxpa_attribute_value *args,
                         rxpa_attribute_value result,
                         rxpa_attribute_value signal) {
    (void)numargs;
    (void)args;
    (void)result;
    (void)signal;
}

static uint32_t test_session_capabilities(const char *procedure_name) {
    if (strcmp(procedure_name, "e3.session_probe") == 0) {
        return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
    }
    if (strcmp(procedure_name, "e3.session_reentrant") == 0) {
        return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
    }
    if (strcmp(procedure_name, "e3.session_invalid") == 0) {
        return RXPA_PROCEDURE_CAP_KNOWN_V2;
    }
    return 0u;
}

static void *test_session_create(void) {
    test_plugin_session *session = calloc(1u, sizeof(*session));
    if (!session) return NULL;
    gate_lock(&session_gate);
    session->id = ++session_created;
    session_live++;
    gate_unlock(&session_gate);
    return session;
}

static void test_session_destroy(void *opaque_session) {
    gate_lock(&session_gate);
    session_destroyed++;
    session_live--;
    gate_unlock(&session_gate);
    free(opaque_session);
}

static int test_session_enter(void *opaque_session, uint32_t capabilities,
                              void **previous) {
    if (!opaque_session || !previous ||
        capabilities != RXPA_PROCEDURE_CAP_SESSION_AFFINE) return -1;
    *previous = current_test_session;
    current_test_session = (test_plugin_session *)opaque_session;
    return 0;
}

static void test_session_leave(void *previous) {
    current_test_session = (test_plugin_session *)previous;
}

static void session_probe(rxinteger numargs, rxpa_attribute_value *args,
                          rxpa_attribute_value result,
                          rxpa_attribute_value signal) {
    test_plugin_session *session = current_test_session;
    void *previous = NULL;
    (void)numargs;
    (void)args;
    (void)result;
    (void)signal;
    if (!session ||
        test_session_enter(session,
                           RXPA_PROCEDURE_CAP_SESSION_AFFINE,
                           &previous) != 0 ||
        previous != session || current_test_session != session) {
        gate_lock(&session_gate);
        session_failures++;
        gate_unlock(&session_gate);
        return;
    }
    test_session_leave(previous);
    if (current_test_session != session) {
        gate_lock(&session_gate);
        session_failures++;
        gate_unlock(&session_gate);
        return;
    }

    gate_lock(&session_gate);
    session->calls++;
    session_gate.active++;
    if (session_gate.active > session_gate.maximum_active) {
        session_gate.maximum_active = session_gate.active;
    }
    gate_broadcast(&session_gate);
    while (session_gate.maximum_active < 2) gate_wait(&session_gate);
    session_gate.active--;
    gate_broadcast(&session_gate);
    gate_unlock(&session_gate);
}

static void register_session_probe(void) {
    static const rxpa_plugin_manifest_v2 manifest = {
        sizeof(rxpa_plugin_manifest_v2), RXPA_PLUGIN_MANIFEST_ABI_V2,
        "e3-session-probe", test_session_capabilities,
        test_session_create, test_session_destroy,
        test_session_enter, test_session_leave
    };
    rxvm_register_static_plugin_manifest_v2(&manifest);
    rxvm_addfunc_for_plugin("e3-session-probe", session_probe,
                            "e3.session_probe", "b", ".void", "");
    rxvm_addfunc_for_plugin("e3-session-probe", static_probe,
                            "e3.session_reentrant", "b", ".void", "");
    rxvm_addfunc_for_plugin("e3-session-probe", static_probe,
                            "e3.session_invalid", "b", ".void", "");
    rxvm_addfunc_for_plugin("e3-session-probe", static_probe,
                            "e3.session_legacy", "b", ".void", "");
}

static int call_bound_session(rxvm_context *context) {
    proc_runtime *procedure = context_find_procedure(
            context, "e3.session_probe");
    rxvm_memory_worker *previous_worker;
    rxvm_context *previous_context;
    int failed = 0;
    if (!procedure || procedure->native_invoker != rxvm_callfunc_session ||
        procedure->native_capabilities !=
                RXPA_PROCEDURE_CAP_SESSION_AFFINE) return 1;
    previous_worker = rxvm_memory_enter(context->worker.memory_worker);
    if (rxvm_worker_begin_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        rxvm_memory_leave(previous_worker);
        return 1;
    }
    rxpa_compatibility_execution_enter(&context->rxpa_compatibility);
    previous_context = rxvm_active_context_enter(context);
    current_test_session = &session_sentinel;
    rxvm_call_native_procedure(procedure, 0, NULL, NULL, NULL);
    if (current_test_session != &session_sentinel) failed = 1;
    current_test_session = NULL;
    rxvm_active_context_leave(previous_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    if (rxvm_worker_end_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) failed = 1;
    rxvm_memory_leave(previous_worker);
    return failed;
}

static void run_static_session_context(int index) {
    rxvm_context context;
    int failed = 0;
    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    if (rxldmodp(&context) <= 0 || !context.rxpa_sessions ||
        context_procedure_invoker(&context, "e3.session_probe") !=
                rxvm_callfunc_session ||
        context_procedure_invoker(&context, "e3.session_reentrant") !=
                rxvm_callfunc_direct ||
        context_procedure_capabilities(&context, "e3.session_invalid") != 0u ||
        context_procedure_capabilities(&context, "e3.session_legacy") != 0u) {
        failed = 1;
    } else {
        session_ids[index] =
                ((test_plugin_session *)context.rxpa_sessions->session)->id;
    }

    gate_lock(&session_gate);
    session_gate.ready++;
    gate_broadcast(&session_gate);
    while (!session_gate.release) gate_wait(&session_gate);
    gate_unlock(&session_gate);

    if (!failed && call_bound_session(&context) != 0) failed = 1;
    rxfremod(&context);
    if (failed) {
        gate_lock(&session_gate);
        session_failures++;
        gate_unlock(&session_gate);
    }
}

#ifdef _WIN32
static DWORD WINAPI static_session_thread_entry(LPVOID opaque) {
    run_static_session_context((int)(intptr_t)opaque);
    return 0;
}
#else
static void *static_session_thread_entry(void *opaque) {
    run_static_session_context((int)(intptr_t)opaque);
    return NULL;
}
#endif

static int test_static_sessions(void) {
    int index;
#ifdef _WIN32
    HANDLE threads[2];
#else
    pthread_t threads[2];
#endif
    gate_init(&session_gate);
    session_failures = 0;
    session_created = 0;
    session_destroyed = 0;
    session_live = 0;
    session_ids[0] = session_ids[1] = 0;
    register_session_probe();
    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        threads[index] = CreateThread(NULL, 0, static_session_thread_entry,
                                      (LPVOID)(intptr_t)index, 0, NULL);
        if (!threads[index]) return 1;
#else
        if (pthread_create(&threads[index], NULL,
                           static_session_thread_entry,
                           (void *)(intptr_t)index) != 0) return 1;
#endif
    }
    gate_lock(&session_gate);
    while (session_gate.ready != 2) gate_wait(&session_gate);
    session_gate.release = 1;
    gate_broadcast(&session_gate);
    gate_unlock(&session_gate);
    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        WaitForSingleObject(threads[index], INFINITE);
        CloseHandle(threads[index]);
#else
        (void)pthread_join(threads[index], NULL);
#endif
    }
    if (session_created != 2 || session_destroyed != 2 || session_live != 0 ||
        !session_ids[0] || !session_ids[1] ||
        session_ids[0] == session_ids[1] ||
        session_gate.maximum_active != 2) session_failures++;
    gate_destroy(&session_gate);
    if (session_failures) {
        fprintf(stderr,
                "RXPA static session lifecycle failed: failures=%d created=%d destroyed=%d live=%d ids=%d/%d overlap=%d\n",
                session_failures, session_created, session_destroyed,
                session_live, session_ids[0], session_ids[1],
                session_gate.maximum_active);
        return 1;
    }
    return 0;
}

static void *failing_session_create(void) {
    return NULL;
}

static void noop_session_destroy(void *session) {
    (void)session;
}

static int noop_session_enter(void *session, uint32_t capabilities,
                              void **previous) {
    (void)session;
    (void)capabilities;
    if (previous) *previous = NULL;
    return 0;
}

static void noop_session_leave(void *previous) {
    (void)previous;
}

static uint32_t all_session_capabilities(const char *procedure_name) {
    (void)procedure_name;
    return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
}

static int test_session_factory_failure(void) {
    static const rxpa_plugin_manifest_v2 manifest = {
        sizeof(rxpa_plugin_manifest_v2), RXPA_PLUGIN_MANIFEST_ABI_V2,
        "e3-session-failure", all_session_capabilities,
        failing_session_create, noop_session_destroy,
        noop_session_enter, noop_session_leave
    };
    rxvm_context context;
    int rc;
    rxvm_register_static_plugin_manifest_v2(&manifest);
    rxvm_addfunc_for_plugin("e3-session-failure", static_probe,
                            "e3.session_failure", "b", ".void", "");
    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    rc = rxldmodp(&context);
    if (rc != -1 || context.num_modules != 0 || context.rxpa_sessions ||
        context.rxpa_session_bindings) {
        fprintf(stderr, "RXPA session factory failure did not roll back\n");
        rxfremod(&context);
        return 1;
    }
    rxfremod(&context);
    return 0;
}

static int malformed_factory_called;

static void *malformed_session_create(void) {
    malformed_factory_called++;
    return &malformed_factory_called;
}

static uint32_t all_reentrant_capabilities(const char *procedure_name) {
    (void)procedure_name;
    return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
}

static int test_malformed_session_manifest(void) {
    static const rxpa_plugin_manifest_v2 manifest = {
        sizeof(rxpa_plugin_manifest_v2), RXPA_PLUGIN_MANIFEST_ABI_V2,
        "e3-session-malformed", all_reentrant_capabilities,
        malformed_session_create, NULL, NULL, NULL
    };
    rxvm_context context;
    int failed = 0;
    malformed_factory_called = 0;
    rxvm_register_static_plugin_manifest_v2(&manifest);
    rxvm_addfunc_for_plugin("e3-session-malformed", static_probe,
                            "e3.session_malformed", "b", ".void", "");
    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    if (rxldmodp(&context) <= 0 || malformed_factory_called != 0 ||
        context_procedure_capabilities(&context,
                                       "e3.session_malformed") != 0u ||
        context.rxpa_sessions) failed = 1;
    rxfremod(&context);
    if (failed) {
        fprintf(stderr, "Malformed RXPA V2 manifest did not fail closed\n");
        return 1;
    }
    return 0;
}

static proc_runtime *context_find_procedure(rxvm_context *context,
                                            const char *name) {
    size_t module_index;

    for (module_index = 0; module_index < context->num_modules; module_index++) {
        module *mod = context->modules[module_index];
        size_t procedure_index;
        for (procedure_index = 0;
             procedure_index < mod->procedure_count;
             procedure_index++) {
            proc_runtime *procedure = &mod->procedures[procedure_index];
            if (procedure->name && strcmp(procedure->name, name) == 0) {
                return procedure;
            }
        }
    }
    return NULL;
}

static int context_has_procedure(rxvm_context *context, const char *name) {
    return context_find_procedure(context, name) != NULL;
}

static uint32_t context_procedure_capabilities(rxvm_context *context,
                                               const char *name) {
    size_t module_index;
    for (module_index = 0; module_index < context->num_modules; module_index++) {
        module *mod = context->modules[module_index];
        size_t procedure_index;
        for (procedure_index = 0;
             procedure_index < mod->procedure_count;
             procedure_index++) {
            const proc_runtime *procedure = &mod->procedures[procedure_index];
            if (procedure->name && strcmp(procedure->name, name) == 0) {
                return procedure->native_capabilities;
            }
        }
    }
    return UINT32_MAX;
}

static rxvm_native_invoker context_procedure_invoker(rxvm_context *context,
                                                     const char *name) {
    proc_runtime *procedure = context_find_procedure(context, name);
    return procedure ? procedure->native_invoker : NULL;
}

static void register_binding_probes(void) {
    rxvm_register_static_plugin_capability(
            "e3b-binding-reentrant", RXPA_PLUGIN_CAP_PROCESS_REENTRANT);
    rxvm_addfunc_for_plugin("e3b-binding-reentrant", legacy_probe,
                            "e3b.binding_reentrant", "b", ".void", "");
    rxvm_addfunc(legacy_probe, "e3b.binding_legacy", "b", ".void", "");
}

static void run_static_context(void) {
    rxvm_context context;
    int result;
    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    result = rxldmodp(&context);
    if (result <= 0 ||
        !context_has_procedure(&context, "e3b.static_probe") ||
        context_procedure_capabilities(&context, "e3b.static_probe") !=
                RXPA_PLUGIN_CAP_PROCESS_REENTRANT ||
        call_plugin_procedure(&context, "cipher") != 0 ||
        call_plugin_procedure(&context, "hash") != 0 ||
        call_plugin_procedure(&context, "stack") != 0 ||
        call_plugin_procedure(&context, "id") != 0) {
        gate_lock(&static_gate);
        static_failures++;
        gate_unlock(&static_gate);
    }

    gate_lock(&static_gate);
    static_gate.ready++;
    gate_broadcast(&static_gate);
    while (!static_gate.release) gate_wait(&static_gate);
    gate_unlock(&static_gate);
    rxfremod(&context);
}

#ifdef _WIN32
static DWORD WINAPI static_thread_entry(LPVOID unused) {
    (void)unused;
    run_static_context();
    return 0;
}
#else
static void *static_thread_entry(void *unused) {
    (void)unused;
    run_static_context();
    return NULL;
}
#endif

static int test_static_replay(void) {
    int index;
#ifdef _WIN32
    HANDLE threads[2];
#else
    pthread_t threads[2];
#endif

    rxvm_register_static_plugin_capability(
            "e3b-probe", RXPA_PLUGIN_CAP_PROCESS_REENTRANT);
    rxvm_addfunc_for_plugin("e3b-probe", static_probe,
                            "e3b.static_probe", "b", ".void", "");
    static_failures = 0;
    gate_init(&static_gate);

    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        threads[index] = CreateThread(NULL, 0, static_thread_entry, NULL, 0, NULL);
        if (!threads[index]) return 1;
#else
        if (pthread_create(&threads[index], NULL, static_thread_entry, NULL) != 0) return 1;
#endif
    }

    gate_lock(&static_gate);
    while (static_gate.ready != 2) gate_wait(&static_gate);
    static_gate.release = 1;
    gate_broadcast(&static_gate);
    gate_unlock(&static_gate);

    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        WaitForSingleObject(threads[index], INFINITE);
        CloseHandle(threads[index]);
#else
        (void)pthread_join(threads[index], NULL);
#endif
    }
    gate_destroy(&static_gate);
    if (static_failures) {
        fprintf(stderr, "Static RXPA replay failed in %d context(s)\n",
                static_failures);
        return 1;
    }
    return 0;
}

static int test_float_static_binding(void) {
    rxvm_context first;
    rxvm_context second;
    int failed = 0;
    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));
    rxinimod(&first);
    rxinimod(&second);
    if (rxldmodp(&first) <= 0 ||
        context_procedure_capabilities(&first, "rxfloat.pi") !=
                RXPA_PROCEDURE_CAP_PROCESS_REENTRANT ||
        context_procedure_invoker(&first, "rxfloat.pi") !=
                rxvm_callfunc_direct ||
        context_procedure_capabilities(&first, "rxmath.pi") !=
                RXPA_PROCEDURE_CAP_PROCESS_REENTRANT ||
        context_procedure_invoker(&first, "rxmath.pi") !=
                rxvm_callfunc_direct) {
        failed = 1;
    }
    if (rxldmodp(&second) <= 0 ||
        context_procedure_invoker(&first, "rxfloat.pi") !=
                rxvm_callfunc_direct ||
        context_procedure_invoker(&second, "rxfloat.pi") !=
                rxvm_callfunc_direct ||
        context_procedure_invoker(&first, "rxmath.pi") !=
                rxvm_callfunc_direct ||
        context_procedure_invoker(&second, "rxmath.pi") !=
                rxvm_callfunc_direct) {
        failed = 1;
    }
    if (!failed) {
        failed += expect_float_native_arity_signal(&first, "rxfloat.pi", 1);
        failed += expect_float_native_arity_signal(&first, "rxfloat.sqrt", 0);
        failed += expect_float_native_arity_signal(&first, "rxfloat.sqrt", 2);
        failed += expect_float_native_arity_signal(&first, "rxfloat.hypot", 1);
        failed += expect_float_native_arity_signal(&first, "rxfloat.hypot", 3);
    }
    rxfremod(&second);
    rxfremod(&first);
    if (failed) {
        fprintf(stderr, "Static rxfloat canonical/compatibility binding failed\n");
        return 1;
    }
    return 0;
}

static int expect_float_native_arity_signal(rxvm_context *context,
                                            const char *procedure_name,
                                            int argument_count) {
    proc_runtime *procedure = context_find_procedure(context, procedure_name);
    rxvm_memory_worker *previous_worker;
    rxvm_context *previous_context;
    value arguments[3];
    value *argument_values[3];
    value result;
    value signal;
    int index;
    int failed = 0;

    if (!procedure || argument_count < 0 || argument_count > 3) return 1;
    value_init(&result);
    value_init(&signal);
    for (index = 0; index < 3; index++) {
        value_init(&arguments[index]);
        set_float(&arguments[index], (double)(index + 1));
        argument_values[index] = &arguments[index];
    }

    previous_worker = rxvm_memory_enter(context->worker.memory_worker);
    if (rxvm_worker_begin_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        rxvm_memory_leave(previous_worker);
        failed = 1;
        goto cleanup;
    }
    rxpa_compatibility_execution_enter(&context->rxpa_compatibility);
    previous_context = rxvm_active_context_enter(context);
    rxvm_call_native_procedure(
            procedure, argument_count,
            argument_count ? argument_values : NULL, &result, &signal);
    rxvm_active_context_leave(previous_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    if (rxvm_worker_end_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        failed = 1;
    }
    rxvm_memory_leave(previous_worker);
    if (signal.int_value != SIGNAL_INVALID_ARGUMENTS) {
        fprintf(stderr,
                "RXPA float arity mismatch: procedure=%s arguments=%d signal=%lld\n",
                procedure_name, argument_count,
                (long long)signal.int_value);
        failed = 1;
    }

cleanup:
    clear_value(&signal);
    clear_value(&result);
    for (index = 2; index >= 0; index--) clear_value(&arguments[index]);
    return failed;
}

static void legacy_probe(rxinteger numargs, rxpa_attribute_value *args,
                         rxpa_attribute_value result,
                         rxpa_attribute_value signal) {
    (void)numargs;
    (void)args;
    (void)result;
    (void)signal;

    gate_lock(&legacy_gate);
    legacy_gate.active++;
    if (legacy_gate.active > legacy_gate.maximum_active) {
        legacy_gate.maximum_active = legacy_gate.active;
    }
    gate_broadcast(&legacy_gate);
    if (call_capabilities == RXPA_PLUGIN_CAP_PROCESS_REENTRANT) {
        while (legacy_gate.maximum_active < 2) gate_wait(&legacy_gate);
        legacy_gate.active--;
        gate_broadcast(&legacy_gate);
        gate_unlock(&legacy_gate);
        return;
    }
    while (legacy_gate.attempting < 2) gate_wait(&legacy_gate);
    legacy_gate.active--;
    gate_broadcast(&legacy_gate);
    gate_unlock(&legacy_gate);
}

static void recursive_inner(rxinteger numargs, rxpa_attribute_value *args,
                            rxpa_attribute_value result,
                            rxpa_attribute_value signal) {
    (void)numargs;
    (void)args;
    (void)result;
    (void)signal;
    recursive_inner_called++;
}

static void recursive_outer(rxinteger numargs, rxpa_attribute_value *args,
                            rxpa_attribute_value result,
                            rxpa_attribute_value signal) {
    (void)numargs;
    (void)args;
    (void)result;
    (void)signal;
    rxvm_callfunc((void *)recursive_inner, 0, NULL, NULL, NULL);
}

static int test_recursive_legacy_call(void) {
    recursive_inner_called = 0;
    rxvm_callfunc((void *)recursive_outer, 0, NULL, NULL, NULL);
    if (recursive_inner_called != 1) {
        fprintf(stderr, "Recursive legacy RXPA call did not complete\n");
        return 1;
    }
    return 0;
}

static int test_branch_free_binding(void) {
    rxvm_context first;
    rxvm_context second;
    rxvm_context late;
    proc_runtime *first_legacy;
    int failed = 0;

    register_binding_probes();
    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));
    memset(&late, 0, sizeof(late));

    rxinimod(&first);
    if (rxldmodp(&first) <= 0) failed = 1;
    first_legacy = context_find_procedure(&first, "e3b.binding_legacy");
    if (!first_legacy || first_legacy->native_invoker != rxvm_callfunc_direct ||
        context_procedure_invoker(&first, "e3b.binding_reentrant") !=
                rxvm_callfunc_direct) {
        failed = 1;
    }

    rxinimod(&second);
    if (rxldmodp(&second) <= 0 ||
        !first_legacy || first_legacy->native_invoker != rxvm_callfunc ||
        context_procedure_invoker(&second, "e3b.binding_legacy") !=
                rxvm_callfunc ||
        context_procedure_invoker(&first, "e3b.binding_reentrant") !=
                rxvm_callfunc_direct ||
        context_procedure_invoker(&second, "e3b.binding_reentrant") !=
                rxvm_callfunc_direct) {
        failed = 1;
    }
    rxfremod(&second);
    rxfremod(&first);

    rxinimod(&late);
    if (rxldmodp(&late) <= 0 ||
        context_procedure_invoker(&late, "e3b.binding_legacy") !=
                rxvm_callfunc ||
        context_procedure_invoker(&late, "e3b.binding_reentrant") !=
                rxvm_callfunc_direct) {
        failed = 1;
    }
    rxfremod(&late);

    if (failed) {
        fprintf(stderr, "RXPA branch-free binding/sticky transition failed\n");
        return 1;
    }
    return 0;
}

static int call_bound_legacy(rxvm_context *context) {
    proc_runtime *procedure = context_find_procedure(
            context, "e3b.binding_legacy");
    rxvm_memory_worker *previous_worker;
    rxvm_context *previous_context;
    int failed = 0;

    if (!procedure || procedure->native_invoker != rxvm_callfunc) return 1;
    gate_lock(&legacy_gate);
    legacy_gate.attempting++;
    gate_broadcast(&legacy_gate);
    gate_unlock(&legacy_gate);
    previous_worker = rxvm_memory_enter(context->worker.memory_worker);
    if (rxvm_worker_begin_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        rxvm_memory_leave(previous_worker);
        return 1;
    }
    rxpa_compatibility_execution_enter(&context->rxpa_compatibility);
    previous_context = rxvm_active_context_enter(context);
    rxvm_call_native_procedure(procedure, 0, NULL, NULL, NULL);
    rxvm_active_context_leave(previous_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    if (rxvm_worker_end_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        failed = 1;
    }
    rxvm_memory_leave(previous_worker);
    return failed;
}

static void run_bound_context(void) {
    rxvm_context context;
    int failed = 0;

    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    if (rxldmodp(&context) <= 0 ||
        context_procedure_invoker(&context, "e3b.binding_reentrant") !=
                rxvm_callfunc_direct) {
        failed = 1;
    }

    gate_lock(&binding_gate);
    binding_gate.ready++;
    gate_broadcast(&binding_gate);
    while (!binding_gate.release) gate_wait(&binding_gate);
    gate_unlock(&binding_gate);

    if (!failed && call_bound_legacy(&context) != 0) failed = 1;
    rxfremod(&context);
    if (failed) {
        gate_lock(&binding_gate);
        binding_failures++;
        gate_unlock(&binding_gate);
    }
}

#ifdef _WIN32
static DWORD WINAPI bound_thread_entry(LPVOID unused) {
    (void)unused;
    run_bound_context();
    return 0;
}
#else
static void *bound_thread_entry(void *unused) {
    (void)unused;
    run_bound_context();
    return NULL;
}
#endif

static int test_bound_legacy_serialization(void) {
    int index;
#ifdef _WIN32
    HANDLE threads[2];
#else
    pthread_t threads[2];
#endif

    register_binding_probes();
    binding_failures = 0;
    gate_init(&binding_gate);
    gate_init(&legacy_gate);
    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        threads[index] = CreateThread(NULL, 0, bound_thread_entry,
                                      NULL, 0, NULL);
        if (!threads[index]) return 1;
#else
        if (pthread_create(&threads[index], NULL,
                           bound_thread_entry, NULL) != 0) return 1;
#endif
    }

    gate_lock(&binding_gate);
    while (binding_gate.ready != 2) gate_wait(&binding_gate);
    binding_gate.release = 1;
    gate_broadcast(&binding_gate);
    gate_unlock(&binding_gate);

    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        WaitForSingleObject(threads[index], INFINITE);
        CloseHandle(threads[index]);
#else
        (void)pthread_join(threads[index], NULL);
#endif
    }
    if (legacy_gate.maximum_active != 1) binding_failures++;
    gate_destroy(&legacy_gate);
    gate_destroy(&binding_gate);
    if (binding_failures) {
        fprintf(stderr,
                "Bound legacy RXPA serialization failed: failures=%d maximum=%d\n",
                binding_failures, legacy_gate.maximum_active);
        return 1;
    }
    return 0;
}

static void run_transition_context(void) {
    rxvm_context context;
    int failed = 0;

    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    gate_lock(&binding_gate);
    binding_gate.ready = 1;
    gate_broadcast(&binding_gate);
    while (!binding_gate.release) gate_wait(&binding_gate);
    binding_gate.active = 1;
    gate_broadcast(&binding_gate);
    gate_unlock(&binding_gate);

    if (rxldmodp(&context) <= 0 ||
        context_procedure_invoker(&context, "e3b.binding_legacy") !=
                rxvm_callfunc) {
        failed = 1;
    }
    gate_lock(&binding_gate);
    binding_gate.maximum_active = 1;
    if (failed) binding_failures++;
    gate_broadcast(&binding_gate);
    gate_unlock(&binding_gate);
    rxfremod(&context);
}

#ifdef _WIN32
static DWORD WINAPI transition_thread_entry(LPVOID unused) {
    (void)unused;
    run_transition_context();
    return 0;
}
#else
static void *transition_thread_entry(void *unused) {
    (void)unused;
    run_transition_context();
    return NULL;
}
#endif

static int test_legacy_transition_quiescence(void) {
    rxvm_context first;
    proc_runtime *first_legacy;
    rxvm_memory_worker *previous_worker;
    int execution_started = 0;
#ifdef _WIN32
    HANDLE thread;
#else
    pthread_t thread;
#endif

    register_binding_probes();
    binding_failures = 0;
    gate_init(&binding_gate);
    memset(&first, 0, sizeof(first));
    rxinimod(&first);
    if (rxldmodp(&first) <= 0) binding_failures++;
    first_legacy = context_find_procedure(&first, "e3b.binding_legacy");
    if (!first_legacy || first_legacy->native_invoker != rxvm_callfunc_direct) {
        binding_failures++;
    }

    previous_worker = rxvm_memory_enter(first.worker.memory_worker);
    if (rxvm_worker_begin_execution(&first.worker) !=
            RXVM_WORKER_TRANSITION_OK) {
        binding_failures++;
    } else {
        rxpa_compatibility_execution_enter(&first.rxpa_compatibility);
        execution_started = 1;
    }
#ifdef _WIN32
    thread = CreateThread(NULL, 0, transition_thread_entry, NULL, 0, NULL);
    if (!thread) return 1;
#else
    if (pthread_create(&thread, NULL, transition_thread_entry, NULL) != 0) {
        return 1;
    }
#endif

    gate_lock(&binding_gate);
    while (!binding_gate.ready) gate_wait(&binding_gate);
    binding_gate.release = 1;
    gate_broadcast(&binding_gate);
    while (!binding_gate.active) gate_wait(&binding_gate);
    gate_unlock(&binding_gate);
    observe_legacy_transition_blocked();
    gate_lock(&binding_gate);
    if (binding_gate.maximum_active) binding_failures++;
    gate_unlock(&binding_gate);

    if (execution_started) {
        rxpa_compatibility_execution_leave(&first.rxpa_compatibility);
        if (rxvm_worker_end_execution(&first.worker) !=
                RXVM_WORKER_TRANSITION_OK) {
            binding_failures++;
        }
    }
    rxvm_memory_leave(previous_worker);
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    (void)pthread_join(thread, NULL);
#endif

    if (!first_legacy || first_legacy->native_invoker != rxvm_callfunc ||
        context_procedure_invoker(&first, "e3b.binding_reentrant") !=
                rxvm_callfunc_direct) {
        binding_failures++;
    }
    rxfremod(&first);
    gate_destroy(&binding_gate);
    if (binding_failures) {
        fprintf(stderr,
                "Legacy RXPA transition quiescence failed: failures=%d\n",
                binding_failures);
        return 1;
    }
    return 0;
}

static void run_legacy_call(void) {
    gate_lock(&legacy_gate);
    legacy_gate.ready++;
    gate_broadcast(&legacy_gate);
    while (!legacy_gate.release) gate_wait(&legacy_gate);
    legacy_gate.attempting++;
    gate_broadcast(&legacy_gate);
    gate_unlock(&legacy_gate);

    rxvm_callfunc_capabilities((void *)legacy_probe, call_capabilities,
                               0, NULL, NULL, NULL);
}

#ifdef _WIN32
static DWORD WINAPI legacy_thread_entry(LPVOID unused) {
    (void)unused;
    run_legacy_call();
    return 0;
}
#else
static void *legacy_thread_entry(void *unused) {
    (void)unused;
    run_legacy_call();
    return NULL;
}
#endif

static int test_call_policy(uint32_t capabilities, int expected_maximum) {
    int index;
#ifdef _WIN32
    HANDLE threads[2];
#else
    pthread_t threads[2];
#endif

    call_capabilities = capabilities;
    gate_init(&legacy_gate);
    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        threads[index] = CreateThread(NULL, 0, legacy_thread_entry, NULL, 0, NULL);
        if (!threads[index]) return 1;
#else
        if (pthread_create(&threads[index], NULL, legacy_thread_entry, NULL) != 0) return 1;
#endif
    }

    gate_lock(&legacy_gate);
    while (legacy_gate.ready != 2) gate_wait(&legacy_gate);
    legacy_gate.release = 1;
    gate_broadcast(&legacy_gate);
    gate_unlock(&legacy_gate);

    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        WaitForSingleObject(threads[index], INFINITE);
        CloseHandle(threads[index]);
#else
        (void)pthread_join(threads[index], NULL);
#endif
    }

    if (legacy_gate.maximum_active != expected_maximum) {
        fprintf(stderr,
                "RXPA call policy mismatch: capabilities=%u maximum=%d expected=%d\n",
                capabilities, legacy_gate.maximum_active, expected_maximum);
        gate_destroy(&legacy_gate);
        return 1;
    }
    gate_destroy(&legacy_gate);
    return 0;
}

static int test_manifest(const char *directory, const char *file_name,
                         uint32_t expected_capabilities) {
    rxpa_loaded_plugin plugin;
    size_t before = rxpa_live_plugin_handle_count();
    int rc = rxpa_open_plugin((char *)directory, (char *)file_name, &plugin);
    if (rc != 0 ||
        plugin.capabilities != expected_capabilities ||
        rxpa_live_plugin_handle_count() != before + 1u) {
        fprintf(stderr,
                "RXPA manifest query failed: rc=%d capabilities=%u handles=%zu/%zu\n",
                rc, rc == 0 ? plugin.capabilities : 0u,
                rxpa_live_plugin_handle_count(), before);
        if (rc == 0) rxpa_close_plugin(&plugin);
        return 1;
    }
    rxpa_close_plugin(&plugin);
    if (rxpa_live_plugin_handle_count() != before) {
        fprintf(stderr, "RXPA manifest handle was not released\n");
        return 1;
    }
    return 0;
}

static int test_procedure_manifest(const char *directory,
                                   const char *file_name) {
    rxpa_loaded_plugin plugin;
    size_t before = rxpa_live_plugin_handle_count();
    int rc = rxpa_open_plugin((char *)directory, (char *)file_name, &plugin);
    int failed = rc != 0;
    if (!failed &&
        (!plugin.has_manifest_v2 || plugin.capabilities != 0u ||
         rxpa_loaded_plugin_procedure_capabilities(
                 &plugin, "rxfloat.pi") !=
                 RXPA_PROCEDURE_CAP_PROCESS_REENTRANT ||
         rxpa_loaded_plugin_procedure_capabilities(
                 &plugin, "rxmath.pi") !=
                 RXPA_PROCEDURE_CAP_PROCESS_REENTRANT)) {
        failed = 1;
    }
    if (rc == 0) rxpa_close_plugin(&plugin);
    if (rxpa_live_plugin_handle_count() != before) failed = 1;
    if (failed) {
        fprintf(stderr, "RXPA V2 procedure manifest qualification failed\n");
        return 1;
    }
    return 0;
}

static char *copy_plugin_base(const char *file_name) {
    static const char suffix[] = ".rxplugin";
    size_t length = strlen(file_name);
    size_t suffix_length = sizeof(suffix) - 1u;
    char *base;
    if (length >= suffix_length &&
        strcmp(file_name + length - suffix_length, suffix) == 0) {
        length -= suffix_length;
    }
    base = malloc(length + 1u);
    if (!base) return NULL;
    memcpy(base, file_name, length);
    base[length] = 0;
    return base;
}

static char *copy_text(const char *text) {
    size_t length = strlen(text);
    char *copy = malloc(length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static int call_plugin_procedure(rxvm_context *context, const char *kind) {
    static const unsigned char sha256_abc[32] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    const char *procedure_name;
    proc_runtime *procedure;
    rxvm_memory_worker *previous_worker;
    rxvm_context *previous_context;
    rxvm_worker_transition_result begin_result;
    rxvm_worker_transition_result end_result;
    value arguments[2];
    value *argument_values[2];
    value result;
    value signal;
    int argument_count = 0;
    int failed = 0;

    value_init(&arguments[0]);
    value_init(&arguments[1]);
    value_init(&result);
    value_init(&signal);
    argument_values[0] = &arguments[0];
    argument_values[1] = &arguments[1];

    if (strcmp(kind, "probe") == 0) {
        procedure_name = "rxpa_dynlink.proc1";
    } else if (strcmp(kind, "cipher") == 0) {
        procedure_name = "cipher.md5";
        argument_count = 1;
        set_null_string(&arguments[0], "abc");
    } else if (strcmp(kind, "hash") == 0) {
        procedure_name = "rxhash.sha256";
        argument_count = 1;
        if (set_native_payload(&arguments[0], "abc", 3u, NULL, 0u) != 0) {
            failed = 1;
        }
    } else if (strcmp(kind, "stack") == 0) {
        procedure_name = "stack.push";
        argument_count = 2;
        set_num_attributes(&arguments[0], 0);
        set_null_string(&arguments[1], "alpha");
    } else if (strcmp(kind, "strings") == 0) {
        procedure_name = "strings.xwords";
        argument_count = 2;
        set_null_string(&arguments[0], "alpha beta gamma");
        set_null_string(&arguments[1], " ");
    } else if (strcmp(kind, "getpi") == 0) {
        procedure_name = "getpi.getpi";
        argument_count = 1;
        set_null_string(&arguments[0], "Monte Carlo");
    } else if (strcmp(kind, "id") == 0) {
        procedure_name = "rxid.uuid7";
    } else if (strcmp(kind, "float") == 0) {
        procedure_name = "rxfloat.pi";
    } else if (strcmp(kind, "stats") == 0) {
        const double packed_values[3] = {1.0, 2.0, 3.0};

        procedure_name = "rxstats.mean";
        argument_count = 1;
        if (set_native_payload(&arguments[0], packed_values,
                               sizeof(packed_values), NULL, 0u) != 0) {
            failed = 1;
        }
    } else if (strcmp(kind, "vector") == 0) {
        const double left_values[2] = {1.0, 0.0};
        const double right_values[2] = {0.6, 0.8};

        procedure_name = "rxvector.cosine";
        argument_count = 2;
        if (set_native_payload(&arguments[0], left_values,
                               sizeof(left_values), NULL, 0u) != 0 ||
            set_native_payload(&arguments[1], right_values,
                               sizeof(right_values), NULL, 0u) != 0) {
            failed = 1;
        }
    } else if (strcmp(kind, "fs") == 0) {
        procedure_name = "rxfs.cwd";
    } else if (strcmp(kind, "platform") == 0) {
        procedure_name = "rxplatform.osname";
    } else {
        failed = 1;
        procedure_name = "";
    }

    procedure = context_find_procedure(context, procedure_name);
    if (failed || !procedure || procedure->native_capabilities !=
            RXPA_PLUGIN_CAP_PROCESS_REENTRANT ||
        procedure->native_invoker != rxvm_callfunc_direct) {
        fprintf(stderr,
                "Bundled RXPA binding failed: kind=%s procedure=%s found=%d capabilities=%u direct=%d\n",
                kind, procedure_name, procedure != NULL,
                procedure ? procedure->native_capabilities : UINT32_MAX,
                procedure && procedure->native_invoker == rxvm_callfunc_direct);
        clear_value(&signal);
        clear_value(&result);
        clear_value(&arguments[1]);
        clear_value(&arguments[0]);
        return 1;
    }
    previous_worker = rxvm_memory_enter(context->worker.memory_worker);
    begin_result = rxvm_worker_begin_execution(&context->worker);
    if (begin_result != RXVM_WORKER_TRANSITION_OK) {
        rxvm_memory_leave(previous_worker);
        clear_value(&signal);
        clear_value(&result);
        clear_value(&arguments[1]);
        clear_value(&arguments[0]);
        return 1;
    }
    rxpa_compatibility_execution_enter(&context->rxpa_compatibility);
    previous_context = rxvm_active_context_enter(context);
    rxvm_call_native_procedure(
            procedure, argument_count,
            argument_count ? argument_values : NULL, &result, &signal);
    if (signal.int_value != SIGNAL_NONE) {
        failed = 1;
    } else if (strcmp(kind, "probe") == 0) {
        failed = !value_string_equals(&result, "dynamic proc1 output");
    } else if (strcmp(kind, "cipher") == 0) {
        failed = !value_string_equals(
                &result, "900150983cd24fb0d6963f7d28e17f72");
    } else if (strcmp(kind, "hash") == 0) {
        failed = !value_binary_equals(&result, sha256_abc,
                                      sizeof(sha256_abc));
    } else if (strcmp(kind, "stack") == 0) {
        failed = result.int_value != 1 || arguments[0].num_attributes != 1u ||
                 !value_string_equals(arguments[0].attributes[0], "alpha");
    } else if (strcmp(kind, "strings") == 0) {
        failed = result.int_value != 3;
    } else if (strcmp(kind, "getpi") == 0) {
        failed = result.float_value < 2.8 || result.float_value > 3.5;
    } else if (strcmp(kind, "id") == 0) {
        failed = !result.string_value || result.string_length != 36u ||
                 result.string_value[8] != '-' || result.string_value[13] != '-' ||
                 result.string_value[18] != '-' || result.string_value[23] != '-';
    } else if (strcmp(kind, "float") == 0) {
        failed = result.float_value < 3.1415 || result.float_value > 3.1417;
    } else if (strcmp(kind, "stats") == 0) {
        failed = result.float_value != 2.0;
    } else if (strcmp(kind, "vector") == 0) {
        failed = result.float_value < 0.599999999999999 ||
                 result.float_value > 0.600000000000001;
    } else if (strcmp(kind, "fs") == 0 || strcmp(kind, "platform") == 0) {
        failed = !result.string_value || result.string_length == 0u;
    }
    if (failed) {
        fprintf(stderr,
                "Bundled RXPA call failed: kind=%s signal=%lld result=%.*s int=%lld float=%.17g attrs=%zu\n",
                kind, (long long)signal.int_value,
                result.string_value ? (int)result.string_length : 6,
                result.string_value ? result.string_value : "<null>",
                (long long)result.int_value, result.float_value,
                arguments[0].num_attributes);
    }
    rxvm_active_context_leave(previous_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    end_result = rxvm_worker_end_execution(&context->worker);
    rxvm_memory_leave(previous_worker);
    clear_value(&signal);
    clear_value(&result);
    clear_value(&arguments[1]);
    clear_value(&arguments[0]);
    return failed || end_result != RXVM_WORKER_TRANSITION_OK;
}

static void run_dynamic_context(int index) {
    rxvm_context context;
    int loaded;

    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    context.location = copy_text(dynamic_directory);
    loaded = context.location &&
             rxldmod(&context, (char *)dynamic_plugin_base) > 0;
    if (!loaded) {
        gate_lock(&dynamic_gate);
        dynamic_failures++;
        gate_unlock(&dynamic_gate);
    }

    gate_lock(&dynamic_gate);
    dynamic_gate.ready++;
    gate_broadcast(&dynamic_gate);
    while (!dynamic_gate.release) gate_wait(&dynamic_gate);
    gate_unlock(&dynamic_gate);

    if (loaded && call_plugin_procedure(&context, dynamic_call_kind) != 0) {
        gate_lock(&dynamic_gate);
        dynamic_failures++;
        gate_unlock(&dynamic_gate);
    }

    if (index == 1) {
        rxfremod(&context);
        gate_lock(&dynamic_gate);
        dynamic_second_destroyed = 1;
        gate_broadcast(&dynamic_gate);
        gate_unlock(&dynamic_gate);
    } else {
        gate_lock(&dynamic_gate);
        while (!dynamic_second_destroyed) gate_wait(&dynamic_gate);
        gate_unlock(&dynamic_gate);
        if (loaded &&
            call_plugin_procedure(&context, dynamic_call_kind) != 0) {
            gate_lock(&dynamic_gate);
            dynamic_failures++;
            gate_unlock(&dynamic_gate);
        }
        rxfremod(&context);
    }
}

#ifdef _WIN32
static DWORD WINAPI dynamic_thread_entry(LPVOID opaque) {
    run_dynamic_context((int)(intptr_t)opaque);
    return 0;
}
#else
static void *dynamic_thread_entry(void *opaque) {
    run_dynamic_context((int)(intptr_t)opaque);
    return NULL;
}
#endif

static int test_dynamic_context_ownership(const char *directory,
                                          const char *file_name,
                                          const char *call_kind) {
    char *plugin_base = copy_plugin_base(file_name);
    size_t before = rxpa_live_plugin_handle_count();
    int index;
#ifdef _WIN32
    HANDLE threads[2];
#else
    pthread_t threads[2];
#endif

    if (!plugin_base) return 1;
    dynamic_directory = directory;
    dynamic_plugin_base = plugin_base;
    dynamic_call_kind = call_kind;
    dynamic_failures = 0;
    dynamic_second_destroyed = 0;
    gate_init(&dynamic_gate);

    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        threads[index] = CreateThread(NULL, 0, dynamic_thread_entry,
                                      (LPVOID)(intptr_t)index, 0, NULL);
        if (!threads[index]) return 1;
#else
        if (pthread_create(&threads[index], NULL, dynamic_thread_entry,
                           (void *)(intptr_t)index) != 0) return 1;
#endif
    }

    gate_lock(&dynamic_gate);
    while (dynamic_gate.ready != 2) gate_wait(&dynamic_gate);
    if (!dynamic_failures &&
        rxpa_live_plugin_handle_count() != before + 2u) {
        dynamic_failures++;
    }
    dynamic_gate.release = 1;
    gate_broadcast(&dynamic_gate);
    gate_unlock(&dynamic_gate);

    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        WaitForSingleObject(threads[index], INFINITE);
        CloseHandle(threads[index]);
#else
        (void)pthread_join(threads[index], NULL);
#endif
    }

    if (rxpa_live_plugin_handle_count() != before) dynamic_failures++;
    gate_destroy(&dynamic_gate);
    free(plugin_base);
    if (dynamic_failures) {
        fprintf(stderr,
                "RXPA dynamic context ownership failed: failures=%d handles=%zu baseline=%zu\n",
                dynamic_failures, rxpa_live_plugin_handle_count(), before);
        return 1;
    }
    return 0;
}

static int test_dynamic_session_factory_failure(const char *directory,
                                                const char *file_name) {
    rxvm_context context;
    char *plugin_base = copy_plugin_base(file_name);
    size_t before = rxpa_live_plugin_handle_count();
    int rc;
    int failed;
    if (!plugin_base) return 1;
    memset(&context, 0, sizeof(context));
    rxinimod(&context);
    context.location = copy_text(directory);
    rc = context.location ? rxldmod(&context, plugin_base) : 0;
    failed = rc != -1 || context.num_modules != 0 ||
             context.rxpa_sessions || context.rxpa_session_bindings ||
             rxpa_live_plugin_handle_count() != before;
    rxfremod(&context);
    free(plugin_base);
    if (failed || rxpa_live_plugin_handle_count() != before) {
        fprintf(stderr,
                "Dynamic RXPA session factory rollback failed: rc=%d handles=%zu/%zu\n",
                rc, rxpa_live_plugin_handle_count(), before);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        if (argc == 5 && strcmp(argv[1], "bundled") == 0) {
            if (test_manifest(argv[2], argv[3],
                              RXPA_PLUGIN_CAP_PROCESS_REENTRANT) != 0) {
                return 1;
            }
            return test_dynamic_context_ownership(
                    argv[2], argv[3], argv[4]);
        }
        if (argc == 5 && strcmp(argv[1], "mixed") == 0) {
            if (test_procedure_manifest(argv[2], argv[3]) != 0) return 1;
            return test_dynamic_context_ownership(
                    argv[2], argv[3], argv[4]);
        }
        if (argc == 4) {
            if (strcmp(argv[1], "manifest") == 0) {
                return test_manifest(argv[2], argv[3],
                                     RXPA_PLUGIN_CAP_PROCESS_REENTRANT);
            }
            if (strcmp(argv[1], "invalid-manifest") == 0) {
                return test_manifest(argv[2], argv[3], 0u);
            }
            if (strcmp(argv[1], "dynamic") == 0) {
                return test_dynamic_context_ownership(
                        argv[2], argv[3], "probe");
            }
            if (strcmp(argv[1], "session-load-failure") == 0) {
                return test_dynamic_session_factory_failure(
                        argv[2], argv[3]);
            }
        }
        fprintf(stderr,
                "Expected static, legacy, reentrant, manifest, dynamic, or bundled test mode\n");
        return 1;
    }
    if (strcmp(argv[1], "static") == 0) return test_static_replay();
    if (strcmp(argv[1], "session") == 0) return test_static_sessions();
    if (strcmp(argv[1], "session-failure") == 0) {
        return test_session_factory_failure();
    }
    if (strcmp(argv[1], "session-malformed") == 0) {
        return test_malformed_session_manifest();
    }
    if (strcmp(argv[1], "float-static") == 0) {
        return test_float_static_binding();
    }
    if (strcmp(argv[1], "binding") == 0) return test_branch_free_binding();
    if (strcmp(argv[1], "bound-legacy") == 0) {
        return test_bound_legacy_serialization();
    }
    if (strcmp(argv[1], "transition") == 0) {
        return test_legacy_transition_quiescence();
    }
    if (strcmp(argv[1], "legacy") == 0) return test_call_policy(0u, 1);
    if (strcmp(argv[1], "recursive") == 0) return test_recursive_legacy_call();
    if (strcmp(argv[1], "reentrant") == 0) {
        return test_call_policy(RXPA_PLUGIN_CAP_PROCESS_REENTRANT, 2);
    }
    fprintf(stderr, "Unknown test mode: %s\n", argv[1]);
    return 1;
}
