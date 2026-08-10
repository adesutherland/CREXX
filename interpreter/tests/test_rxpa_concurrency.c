/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 */

#include "rxvmintp.h"
#include "rxvmvars.h"

#include <stdio.h>
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
    int active;
    int maximum_active;
} call_gate;

static call_gate legacy_gate;
static call_gate static_gate;
static call_gate dynamic_gate;
static call_gate binding_gate;
static uint32_t call_capabilities;
static const char *dynamic_directory;
static const char *dynamic_plugin_base;
static int dynamic_failures;
static int dynamic_second_destroyed;
static int recursive_inner_called;
static int static_failures;
static int binding_failures;

static void legacy_probe(rxinteger numargs, rxpa_attribute_value *args,
                         rxpa_attribute_value result,
                         rxpa_attribute_value signal);

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

static void pause_for_overlap(void) {
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
                RXPA_PLUGIN_CAP_PROCESS_REENTRANT) {
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
    gate_unlock(&legacy_gate);

    pause_for_overlap();

    gate_lock(&legacy_gate);
    legacy_gate.active--;
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
    pause_for_overlap();
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

static int call_dynamic_probe(rxvm_context *context) {
    proc_runtime *procedure = context_find_procedure(
            context, "rxpa_dynlink.proc1");
    rxvm_memory_worker *previous_worker;
    rxvm_context *previous_context;
    rxvm_worker_transition_result begin_result;
    rxvm_worker_transition_result end_result;
    value result;
    value signal;
    int failed = 0;

    if (!procedure || procedure->native_capabilities !=
            RXPA_PLUGIN_CAP_PROCESS_REENTRANT) return 1;
    previous_worker = rxvm_memory_enter(context->worker.memory_worker);
    begin_result = rxvm_worker_begin_execution(&context->worker);
    if (begin_result != RXVM_WORKER_TRANSITION_OK) {
        rxvm_memory_leave(previous_worker);
        return 1;
    }
    rxpa_compatibility_execution_enter(&context->rxpa_compatibility);
    previous_context = rxvm_active_context_enter(context);
    value_init(&result);
    value_init(&signal);
    rxvm_call_native_procedure(procedure, 0, NULL, &result, &signal);
    if (signal.int_value != SIGNAL_NONE || !result.string_value ||
        strcmp(result.string_value, "dynamic proc1 output") != 0) {
        failed = 1;
    }
    clear_value(&signal);
    clear_value(&result);
    rxvm_active_context_leave(previous_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    end_result = rxvm_worker_end_execution(&context->worker);
    rxvm_memory_leave(previous_worker);
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
    if (!loaded || call_dynamic_probe(&context) != 0) {
        gate_lock(&dynamic_gate);
        dynamic_failures++;
        gate_unlock(&dynamic_gate);
    }

    gate_lock(&dynamic_gate);
    dynamic_gate.ready++;
    gate_broadcast(&dynamic_gate);
    while (!dynamic_gate.release) gate_wait(&dynamic_gate);
    gate_unlock(&dynamic_gate);

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
        if (loaded && call_dynamic_probe(&context) != 0) {
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
                                          const char *file_name) {
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

int main(int argc, char **argv) {
    if (argc != 2) {
        if (argc == 4) {
            if (strcmp(argv[1], "manifest") == 0) {
                return test_manifest(argv[2], argv[3],
                                     RXPA_PLUGIN_CAP_PROCESS_REENTRANT);
            }
            if (strcmp(argv[1], "invalid-manifest") == 0) {
                return test_manifest(argv[2], argv[3], 0u);
            }
            if (strcmp(argv[1], "dynamic") == 0) {
                return test_dynamic_context_ownership(argv[2], argv[3]);
            }
        }
        fprintf(stderr, "Expected static, legacy, reentrant, manifest, or dynamic test mode\n");
        return 1;
    }
    if (strcmp(argv[1], "static") == 0) return test_static_replay();
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
