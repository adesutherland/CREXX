/* Focused industrial ODBC/session qualification using the test-only driver. */
#include "rxvmintp.h"
#include "rxvmvars.h"
#include "rxpa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

typedef enum test_argument_kind {
    TEST_ARGUMENT_INTEGER,
    TEST_ARGUMENT_FLOAT,
    TEST_ARGUMENT_STRING
} test_argument_kind;

typedef struct test_argument {
    test_argument_kind kind;
    rxinteger integer;
    double floating;
    const char *string;
} test_argument;

typedef struct test_result {
    rxinteger integer;
    double floating;
    rxinteger signal;
    char string[4096];
} test_result;

typedef struct concurrency_gate {
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE condition;
#else
    pthread_mutex_t mutex;
    pthread_cond_t condition;
#endif
    int ready;
    int release;
    int failures;
} concurrency_gate;

static concurrency_gate odbc_gate;
static const char *odbc_concurrency_directory;

static void concurrency_lock(concurrency_gate *gate) {
#ifdef _WIN32
    EnterCriticalSection(&gate->mutex);
#else
    (void)pthread_mutex_lock(&gate->mutex);
#endif
}
static void concurrency_unlock(concurrency_gate *gate) {
#ifdef _WIN32
    LeaveCriticalSection(&gate->mutex);
#else
    (void)pthread_mutex_unlock(&gate->mutex);
#endif
}
static void concurrency_wait(concurrency_gate *gate) {
#ifdef _WIN32
    SleepConditionVariableCS(&gate->condition, &gate->mutex, INFINITE);
#else
    (void)pthread_cond_wait(&gate->condition, &gate->mutex);
#endif
}
static void concurrency_broadcast(concurrency_gate *gate) {
#ifdef _WIN32
    WakeAllConditionVariable(&gate->condition);
#else
    (void)pthread_cond_broadcast(&gate->condition);
#endif
}
static void concurrency_init(concurrency_gate *gate) {
    memset(gate, 0, sizeof(*gate));
#ifdef _WIN32
    InitializeCriticalSection(&gate->mutex);
    InitializeConditionVariable(&gate->condition);
#else
    (void)pthread_mutex_init(&gate->mutex, NULL);
    (void)pthread_cond_init(&gate->condition, NULL);
#endif
}
static void concurrency_destroy(concurrency_gate *gate) {
#ifdef _WIN32
    DeleteCriticalSection(&gate->mutex);
#else
    (void)pthread_cond_destroy(&gate->condition);
    (void)pthread_mutex_destroy(&gate->mutex);
#endif
}

static proc_runtime *find_procedure(rxvm_context *context,
                                    const char *name) {
    size_t module_index;
    for (module_index = 0u; module_index < context->num_modules;
         module_index++) {
        module *mod = context->modules[module_index];
        size_t procedure_index;
        for (procedure_index = 0u;
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

static char *copy_text(const char *text) {
    size_t length = strlen(text);
    char *copy = malloc(length + 1u);
    if (copy) memcpy(copy, text, length + 1u);
    return copy;
}

static int invoke(rxvm_context *context, const char *name,
                  const test_argument *arguments, int argument_count,
                  test_result *output) {
    proc_runtime *procedure = find_procedure(context, name);
    value values[4];
    value *value_pointers[4];
    value result;
    value signal;
    rxvm_memory_worker *previous_worker;
    rxvm_context *previous_context;
    int index;
    int failed = 0;
    if (!procedure || argument_count < 0 || argument_count > 4) return 1;
    memset(output, 0, sizeof(*output));
    value_init(&result);
    value_init(&signal);
    for (index = 0; index < argument_count; index++) {
        value_init(&values[index]);
        value_pointers[index] = &values[index];
        if (arguments[index].kind == TEST_ARGUMENT_STRING) {
            set_null_string(&values[index], arguments[index].string);
        } else if (arguments[index].kind == TEST_ARGUMENT_FLOAT) {
            set_float(&values[index], arguments[index].floating);
        } else {
            set_int(&values[index], arguments[index].integer);
        }
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
    rxvm_call_native_procedure(procedure, argument_count,
                               argument_count ? value_pointers : NULL,
                               &result, &signal);
    rxvm_active_context_leave(previous_context);
    rxpa_compatibility_execution_leave(&context->rxpa_compatibility);
    if (rxvm_worker_end_execution(&context->worker) !=
            RXVM_WORKER_TRANSITION_OK) failed = 1;
    rxvm_memory_leave(previous_worker);
    output->integer = result.int_value;
    output->floating = result.float_value;
    output->signal = signal.int_value;
    if (result.string_value) {
        snprintf(output->string, sizeof(output->string), "%.*s",
                 (int)result.string_length, result.string_value);
    }

cleanup:
    clear_value(&signal);
    clear_value(&result);
    for (index = argument_count - 1; index >= 0; index--) {
        clear_value(&values[index]);
    }
    return failed || output->signal != SIGNAL_NONE;
}

static test_argument integer_argument(rxinteger value) {
    test_argument argument;
    memset(&argument, 0, sizeof(argument));
    argument.kind = TEST_ARGUMENT_INTEGER;
    argument.integer = value;
    return argument;
}

static test_argument float_argument(double value) {
    test_argument argument;
    memset(&argument, 0, sizeof(argument));
    argument.kind = TEST_ARGUMENT_FLOAT;
    argument.floating = value;
    return argument;
}

static test_argument string_argument(const char *value) {
    test_argument argument;
    memset(&argument, 0, sizeof(argument));
    argument.kind = TEST_ARGUMENT_STRING;
    argument.string = value;
    return argument;
}

static int expect_integer(rxvm_context *context, const char *name,
                          const test_argument *arguments, int count,
                          rxinteger expected) {
    test_result result;
    if (invoke(context, name, arguments, count, &result) != 0 ||
        result.integer != expected) {
        fprintf(stderr, "%s returned %lld signal %lld, expected %lld\n",
                name, (long long)result.integer, (long long)result.signal,
                (long long)expected);
        return 1;
    }
    return 0;
}

static int load_mock_context(rxvm_context *context, const char *directory) {
    memset(context, 0, sizeof(*context));
    rxinimod(context);
    context->location = copy_text(directory);
    return context->location && rxldmod(context, "rx_odbc") > 0;
}

static void run_odbc_concurrency_context(int index) {
    rxvm_context context;
    test_argument arguments[3];
    test_result result;
    rxinteger statement = -1;
    int failed = 0;
    int initialized = 0;
    (void)index;
    if (!load_mock_context(&context, odbc_concurrency_directory)) {
        initialized = 1;
        failed = 1;
    } else {
        initialized = 1;
        arguments[0] = string_argument("mock-concurrent");
        arguments[1] = string_argument("user");
        arguments[2] = string_argument("password");
        failed |= expect_integer(&context, "odbc.odbc_connect",
                                 arguments, 3, 0);
        arguments[0] = string_argument("select ?");
        if (invoke(&context, "odbc.odbc_prepare", arguments, 1,
                   &result) != 0 || result.integer <= 0) {
            failed = 1;
        } else {
            statement = result.integer;
            arguments[0] = integer_argument(statement);
            arguments[1] = integer_argument(1);
            arguments[2] = integer_argument(index + 1);
            failed |= expect_integer(&context, "odbc.odbc_bind_integer",
                                     arguments, 3, 0);
        }
    }
    concurrency_lock(&odbc_gate);
    odbc_gate.ready++;
    concurrency_broadcast(&odbc_gate);
    while (!odbc_gate.release) concurrency_wait(&odbc_gate);
    concurrency_unlock(&odbc_gate);
    if (!failed) {
        arguments[0] = integer_argument(statement);
        failed |= expect_integer(&context, "odbc.odbc_execute_prepared",
                                 arguments, 1, 0);
    }
    if (initialized) rxfremod(&context);
    if (failed) {
        concurrency_lock(&odbc_gate);
        odbc_gate.failures++;
        concurrency_unlock(&odbc_gate);
    }
}

#ifdef _WIN32
static DWORD WINAPI odbc_concurrency_thread(LPVOID opaque) {
    run_odbc_concurrency_context((int)(intptr_t)opaque);
    return 0;
}
#else
static void *odbc_concurrency_thread(void *opaque) {
    run_odbc_concurrency_context((int)(intptr_t)opaque);
    return NULL;
}
#endif

static int test_odbc_concurrency(rxvm_context *control,
                                 const char *directory) {
    test_result result;
    int index;
#ifdef _WIN32
    HANDLE threads[2];
#else
    pthread_t threads[2];
#endif
    if (invoke(control, "odbc._mock_reset_overlap", NULL, 0, &result) != 0 ||
        result.integer != 0) return 1;
    concurrency_init(&odbc_gate);
    odbc_concurrency_directory = directory;
    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        threads[index] = CreateThread(NULL, 0, odbc_concurrency_thread,
                                      (LPVOID)(intptr_t)index, 0, NULL);
        if (!threads[index]) return 1;
#else
        if (pthread_create(&threads[index], NULL, odbc_concurrency_thread,
                           (void *)(intptr_t)index) != 0) return 1;
#endif
    }
    concurrency_lock(&odbc_gate);
    while (odbc_gate.ready != 2) concurrency_wait(&odbc_gate);
    odbc_gate.release = 1;
    concurrency_broadcast(&odbc_gate);
    concurrency_unlock(&odbc_gate);
    for (index = 0; index < 2; index++) {
#ifdef _WIN32
        WaitForSingleObject(threads[index], INFINITE);
        CloseHandle(threads[index]);
#else
        (void)pthread_join(threads[index], NULL);
#endif
    }
    if (invoke(control, "odbc._mock_maximum_overlap", NULL, 0, &result) != 0 ||
        result.integer != 2) odbc_gate.failures++;
    index = odbc_gate.failures;
    concurrency_destroy(&odbc_gate);
    if (index) {
        fprintf(stderr, "ODBC concurrent session execution failed: %d\n",
                index);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    rxvm_context first;
    rxvm_context second;
    proc_runtime *prepare;
    proc_runtime *show_message;
    test_argument arguments[4];
    test_result result;
    rxinteger first_statement;
    rxinteger second_statement;
    size_t handle_baseline;
    int failed = 0;
    int first_loaded = 0;
    int second_loaded = 0;

    if (argc != 2) {
        fprintf(stderr, "Expected mock ODBC plugin directory\n");
        return 1;
    }
    handle_baseline = rxpa_live_plugin_handle_count();
    first_loaded = load_mock_context(&first, argv[1]);
    second_loaded = load_mock_context(&second, argv[1]);
    if (!first_loaded || !second_loaded) {
        fprintf(stderr, "Unable to load mock ODBC plugin in two contexts\n");
        failed = 1;
        goto cleanup;
    }
    prepare = find_procedure(&first, "odbc.odbc_prepare");
    show_message = find_procedure(&first, "odbc.show_message");
    if (!prepare ||
        prepare->native_capabilities !=
                RXPA_PROCEDURE_CAP_SESSION_AFFINE ||
        prepare->native_invoker != rxvm_callfunc_session ||
        !show_message ||
        show_message->native_capabilities !=
                RXPA_PROCEDURE_CAP_PROCESS_REENTRANT ||
        show_message->native_invoker != rxvm_callfunc_direct ||
        !first.rxpa_sessions || !second.rxpa_sessions ||
        first.rxpa_sessions->session == second.rxpa_sessions->session) {
        fprintf(stderr, "ODBC mixed/session binding policy is incorrect\n");
        failed = 1;
        goto cleanup;
    }

    arguments[0] = string_argument("mock-first");
    arguments[1] = string_argument("user");
    arguments[2] = string_argument("password");
    failed |= expect_integer(&first, "odbc.odbc_connect", arguments, 3, 0);
    arguments[0] = string_argument("mock-second");
    failed |= expect_integer(&second, "odbc.odbc_connect", arguments, 3, 0);

    /* A failed reconnect must tear down partial handles, retain diagnostics,
     * and permit a clean retry in the same VM session. */
    arguments[0] = string_argument("fail");
    failed |= expect_integer(&first, "odbc.odbc_connect", arguments, 3, -4);
    if (invoke(&first, "odbc.odbc_get_diagnostics", NULL, 0, &result) != 0 ||
        !strstr(result.string, "mock diagnostic")) {
        fprintf(stderr, "ODBC failed-connect diagnostics were not retained\n");
        failed = 1;
    }
    arguments[0] = string_argument("mock-first-recovered");
    failed |= expect_integer(&first, "odbc.odbc_connect", arguments, 3, 0);

    arguments[0] = string_argument("FAIL prepare");
    failed |= expect_integer(&first, "odbc.odbc_prepare", arguments, 1, -3);

    arguments[0] = string_argument("select ?, ?, ?, ?");
    if (invoke(&first, "odbc.odbc_prepare", arguments, 1, &result) != 0 ||
        result.integer <= 0) {
        fprintf(stderr, "First prepared statement was not created\n");
        failed = 1;
        goto cleanup;
    }
    first_statement = result.integer;
    arguments[0] = integer_argument(first_statement);
    arguments[1] = integer_argument(1);
    arguments[2] = string_argument("alpha");
    failed |= expect_integer(&first, "odbc.odbc_bind_string", arguments, 3, 0);
    arguments[1] = integer_argument(-1);
    failed |= expect_integer(&first, "odbc.odbc_bind_string", arguments, 3, -1);
    arguments[1] = integer_argument(1);
    arguments[2] = string_argument("FAIL bind");
    failed |= expect_integer(&first, "odbc.odbc_bind_string", arguments, 3, -1);
    arguments[0] = integer_argument(first_statement);
    failed |= expect_integer(&first, "odbc.odbc_execute_prepared", arguments, 1, 0);
    arguments[0] = integer_argument(first_statement);
    arguments[1] = integer_argument(2);
    arguments[2] = integer_argument(42);
    failed |= expect_integer(&first, "odbc.odbc_bind_integer", arguments, 3, 0);
    arguments[1] = integer_argument(3);
    arguments[2] = float_argument(3.5);
    failed |= expect_integer(&first, "odbc.odbc_bind_float", arguments, 3, 0);
    arguments[1] = integer_argument(4);
    failed |= expect_integer(&first, "odbc.odbc_bind_null", arguments, 2, 0);
    arguments[0] = integer_argument(first_statement);
    failed |= expect_integer(&first, "odbc.odbc_execute_prepared", arguments, 1, 0);

    arguments[0] = string_argument("select 2");
    if (invoke(&first, "odbc.odbc_prepare", arguments, 1, &result) != 0 ||
        result.integer <= 0 || result.integer == first_statement) {
        fprintf(stderr, "Second active prepared statement was not created\n");
        failed = 1;
        goto cleanup;
    }
    second_statement = result.integer;
    arguments[0] = integer_argument(second_statement);
    failed |= expect_integer(&first, "odbc.odbc_execute_prepared", arguments, 1, 0);
    arguments[0] = integer_argument(first_statement);
    failed |= expect_integer(&first, "odbc.odbc_fetch_statement", arguments, 1, 0);
    failed |= expect_integer(&first, "odbc.odbc_columns_statement", arguments, 1, 2);
    arguments[1] = integer_argument(1);
    if (invoke(&first, "odbc.odbc_getcolumn_statement",
               arguments, 2, &result) != 0 ||
        strcmp(result.string, "row-value") != 0) {
        fprintf(stderr, "Prepared statement result retrieval failed: %s\n",
                result.string);
        failed = 1;
    }
    if (invoke(&first, "odbc.odbc_colname_statement",
               arguments, 2, &result) != 0 ||
        strcmp(result.string, "value") != 0) {
        fprintf(stderr, "Prepared statement column-name retrieval failed\n");
        failed = 1;
    }
    if (invoke(&first, "odbc.odbc_coltype_statement",
               arguments, 2, &result) != 0 || result.integer != 12) {
        fprintf(stderr, "Prepared statement column-type retrieval failed\n");
        failed = 1;
    }

    /* IDs include the owning session generation and cannot cross VMs. */
    arguments[0] = integer_argument(first_statement);
    failed |= expect_integer(&second, "odbc.odbc_execute_prepared",
                             arguments, 1, -1);

    failed |= expect_integer(&first, "odbc.odbc_begin_transaction", NULL, 0, 0);
    failed |= expect_integer(&first, "odbc.odbc_commit", NULL, 0, 0);
    failed |= expect_integer(&second, "odbc.odbc_begin_transaction", NULL, 0, 0);
    failed |= expect_integer(&second, "odbc.odbc_rollback", NULL, 0, 0);
    arguments[0] = integer_argument(first_statement);
    failed |= expect_integer(&first, "odbc.odbc_reset_prepared", arguments, 1, 0);
    arguments[1] = integer_argument(1);
    arguments[2] = string_argument("rebound");
    failed |= expect_integer(&first, "odbc.odbc_bind_string", arguments, 3, 0);
    arguments[0] = integer_argument(first_statement);
    failed |= expect_integer(&first, "odbc.odbc_execute_prepared", arguments, 1, 0);
    failed |= expect_integer(&first, "odbc.odbc_close_prepared", arguments, 1, 0);

    if (invoke(&first, "odbc.odbc_get_info", NULL, 0, &result) != 0 ||
        strcmp(result.string, "CREXX mock ODBC") != 0) {
        fprintf(stderr, "ODBC DBMS metadata retrieval failed\n");
        failed = 1;
    }
    if (invoke(&first, "odbc.odbc_tables", NULL, 0, &result) != 0 ||
        !strstr(result.string, "mock_table")) {
        fprintf(stderr, "ODBC table metadata retrieval failed\n");
        failed = 1;
    }
    arguments[0] = string_argument("mock_table");
    if (invoke(&first, "odbc.odbc_primary_keys", arguments, 1, &result) != 0 ||
        !strstr(result.string, "mock_id")) {
        fprintf(stderr, "ODBC primary-key metadata retrieval failed\n");
        failed = 1;
    }

    failed |= test_odbc_concurrency(&first, argv[1]);

cleanup:
    /* Leave the second connection and both remaining statements open.  VM
     * teardown must close them before the mock DSO unload assertion runs. */
    rxfremod(&second);
    rxfremod(&first);
    if (rxpa_live_plugin_handle_count() != handle_baseline) {
        fprintf(stderr, "Mock ODBC plugin handle leak: %zu baseline %zu\n",
                rxpa_live_plugin_handle_count(), handle_baseline);
        failed = 1;
    }
    if (failed) return 1;
    puts("ODBC_MOCK_PREPARED_OK");
    return 0;
}
