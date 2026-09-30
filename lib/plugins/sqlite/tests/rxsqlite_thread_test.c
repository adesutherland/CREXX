/* Focused lifecycle qualification for the SQLite RXPA session/thread contract. */

#define BUILD_DLL
#define PLUGIN_ID rxsqlite
#include "crexxpa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
typedef HANDLE test_thread;
typedef CRITICAL_SECTION test_mutex;
typedef CONDITION_VARIABLE test_condition;
#else
#include <pthread.h>
typedef pthread_t test_thread;
typedef pthread_mutex_t test_mutex;
typedef pthread_cond_t test_condition;
#endif

typedef struct mock_value {
    char text[2048];
    rxinteger integer;
    double real;
    void *payload;
    size_t payload_length;
    const rxpa_native_payload_ops *payload_ops;
    unsigned int payload_flags;
} mock_value;

static char *mock_getstring(rxpa_attribute_value value) {
    return ((mock_value *)value)->text;
}

static void mock_setstring(rxpa_attribute_value value, const char *text) {
    mock_value *target = (mock_value *)value;
    snprintf(target->text, sizeof(target->text), "%s", text ? text : "");
}

static void mock_setint(rxpa_attribute_value value, rxinteger integer) {
    ((mock_value *)value)->integer = integer;
}

static rxinteger mock_getint(rxpa_attribute_value value) {
    return ((mock_value *)value)->integer;
}

static void mock_setfloat(rxpa_attribute_value value, double real) {
    ((mock_value *)value)->real = real;
}

static double mock_getfloat(rxpa_attribute_value value) {
    return ((mock_value *)value)->real;
}

static void *mock_getnative(rxpa_attribute_value value, size_t *length,
                            const rxpa_native_payload_ops **ops,
                            unsigned int *flags) {
    mock_value *source = (mock_value *)value;
    if (length) *length = source->payload_length;
    if (ops) *ops = source->payload_ops;
    if (flags) *flags = source->payload_flags;
    return source->payload;
}

static int mock_setnative(rxpa_attribute_value value, const void *payload,
                          size_t length,
                          const rxpa_native_payload_ops *ops,
                          unsigned int flags) {
    mock_value *target = (mock_value *)value;
    void *copy = NULL;
    if (length) {
        copy = malloc(length);
        if (!copy) return -1;
        memcpy(copy, payload, length);
    }
    if (target->payload && target->payload_ops &&
        target->payload_ops->finalize) {
        target->payload_ops->finalize(target);
    }
    free(target->payload);
    target->payload = copy;
    target->payload_length = length;
    target->payload_ops = ops;
    target->payload_flags = flags;
    return 0;
}

static int mock_isinitialized(rxpa_attribute_value value) {
    return value != NULL;
}

#include "rxsqlite.c"

typedef struct start_gate {
    test_mutex mutex;
    test_condition condition;
    int ready;
    int total;
    int released;
} start_gate;

typedef struct worker_arguments {
    start_gate *gate;
    const char *database_path;
    int worker_id;
    int result;
} worker_arguments;

#if defined(_WIN32)
static void test_mutex_initialize(test_mutex *mutex) {
    InitializeCriticalSection(mutex);
}

static void test_mutex_destroy(test_mutex *mutex) {
    DeleteCriticalSection(mutex);
}

static void test_mutex_lock(test_mutex *mutex) {
    EnterCriticalSection(mutex);
}

static void test_mutex_unlock(test_mutex *mutex) {
    LeaveCriticalSection(mutex);
}

static void test_condition_initialize(test_condition *condition) {
    InitializeConditionVariable(condition);
}

static void test_condition_destroy(test_condition *condition) {
    (void)condition;
}

static void test_condition_wait(test_condition *condition,
                                test_mutex *mutex) {
    (void)SleepConditionVariableCS(condition, mutex, INFINITE);
}

static void test_condition_broadcast(test_condition *condition) {
    WakeAllConditionVariable(condition);
}
#else
static void test_mutex_initialize(test_mutex *mutex) {
    (void)pthread_mutex_init(mutex, NULL);
}

static void test_mutex_destroy(test_mutex *mutex) {
    (void)pthread_mutex_destroy(mutex);
}

static void test_mutex_lock(test_mutex *mutex) {
    (void)pthread_mutex_lock(mutex);
}

static void test_mutex_unlock(test_mutex *mutex) {
    (void)pthread_mutex_unlock(mutex);
}

static void test_condition_initialize(test_condition *condition) {
    (void)pthread_cond_init(condition, NULL);
}

static void test_condition_destroy(test_condition *condition) {
    (void)pthread_cond_destroy(condition);
}

static void test_condition_wait(test_condition *condition,
                                test_mutex *mutex) {
    (void)pthread_cond_wait(condition, mutex);
}

static void test_condition_broadcast(test_condition *condition) {
    (void)pthread_cond_broadcast(condition);
}
#endif

static void initialize_mock_context(void) {
    memset(&_rxpa_initctx, 0, sizeof(_rxpa_initctx));
    _rxpa_initctx.getstring = mock_getstring;
    _rxpa_initctx.setstring = mock_setstring;
    _rxpa_initctx.setint = mock_setint;
    _rxpa_initctx.getint = mock_getint;
    _rxpa_initctx.setfloat = mock_setfloat;
    _rxpa_initctx.getfloat = mock_getfloat;
    _rxpa_initctx.setnativepayload = mock_setnative;
    _rxpa_initctx.getnativepayload = mock_getnative;
    _rxpa_initctx.isinitialized = mock_isinitialized;
}

static int call_open(const char *path, mock_value *database) {
    mock_value path_value = {0};
    mock_value mode_value = {0};
    mock_value returned = {0};
    mock_value signal = {0};
    rxpa_attribute_value arguments[3];
    mock_setstring(&path_value, path);
    mock_setstring(&mode_value, "create");
    arguments[0] = &path_value;
    arguments[1] = &mode_value;
    arguments[2] = database;
    rxsqlite_open_mode(3, arguments, &returned, &signal);
    return signal.integer ? -100 : (int)returned.integer;
}

static int call_exec(mock_value *database, const char *sql) {
    mock_value sql_value = {0};
    mock_value returned = {0};
    mock_value signal = {0};
    rxpa_attribute_value arguments[2];
    mock_setstring(&sql_value, sql);
    arguments[0] = database;
    arguments[1] = &sql_value;
    rxsqlite_exec(2, arguments, &returned, &signal);
    return signal.integer ? -100 : (int)returned.integer;
}

static int call_busy_timeout(mock_value *database, int milliseconds) {
    mock_value timeout = {0};
    mock_value returned = {0};
    mock_value signal = {0};
    rxpa_attribute_value arguments[2];
    timeout.integer = milliseconds;
    arguments[0] = database;
    arguments[1] = &timeout;
    rxsqlite_busy_timeout(2, arguments, &returned, &signal);
    return signal.integer ? -100 : (int)returned.integer;
}

static int call_close(mock_value *database) {
    mock_value returned = {0};
    mock_value signal = {0};
    rxpa_attribute_value arguments[1];
    arguments[0] = database;
    rxsqlite_close(1, arguments, &returned, &signal);
    return signal.integer ? -100 : (int)returned.integer;
}

static int diagnostic_contains(const char *expected) {
    mock_value code = {0};
    mock_value sqlite_code = {0};
    mock_value operation = {0};
    mock_value message = {0};
    mock_value returned = {0};
    mock_value signal = {0};
    rxpa_attribute_value arguments[4];
    arguments[0] = &code;
    arguments[1] = &sqlite_code;
    arguments[2] = &operation;
    arguments[3] = &message;
    rxsqlite_error(4, arguments, &returned, &signal);
    return !signal.integer && returned.integer == RXSQLITE_SQLITE_ERROR &&
           code.integer == RXSQLITE_SQLITE_ERROR &&
           sqlite_code.integer > 0 && strcmp(operation.text, "exec") == 0 &&
           strstr(message.text, expected) != NULL;
}

static void clear_mock_value(mock_value *value) {
    (void)mock_setnative(value, NULL, 0u, NULL, 0u);
}

static void wait_for_start(start_gate *gate) {
    test_mutex_lock(&gate->mutex);
    gate->ready++;
    test_condition_broadcast(&gate->condition);
    while (!gate->released) {
        test_condition_wait(&gate->condition, &gate->mutex);
    }
    test_mutex_unlock(&gate->mutex);
}

#if defined(_WIN32)
static DWORD WINAPI run_worker(LPVOID opaque_arguments) {
#else
static void *run_worker(void *opaque_arguments) {
#endif
    worker_arguments *arguments = (worker_arguments *)opaque_arguments;
    rxsqlite_session *session = rxsqlite_session_create();
    mock_value database = {0};
    void *previous = NULL;
    char sql[256];
    char missing_name[64];
    int ordinal;
    int result = 0;

    if (!session || rxsqlite_session_enter(
            session, RXPA_PROCEDURE_CAP_SESSION_AFFINE, &previous) != 0) {
        arguments->result = 1;
        rxsqlite_session_destroy(session);
#if defined(_WIN32)
        return 0;
#else
        return NULL;
#endif
    }
    wait_for_start(arguments->gate);
    if (call_open(arguments->database_path, &database) != 0 ||
        call_busy_timeout(&database, 30000) != 0) {
        result = 2;
        goto done;
    }
    for (ordinal = 1; ordinal <= 100; ordinal++) {
        snprintf(sql, sizeof(sql),
                 "INSERT INTO session_rows(worker_id, ordinal) VALUES(%d,%d)",
                 arguments->worker_id, ordinal);
        if (call_exec(&database, sql) != 0) {
            result = 3;
            goto done;
        }
    }
    snprintf(missing_name, sizeof(missing_name), "missing_worker_%d",
             arguments->worker_id);
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", missing_name);
    if (call_exec(&database, sql) != RXSQLITE_SQLITE_ERROR ||
        !diagnostic_contains(missing_name)) {
        result = 4;
        goto done;
    }

done:
    if (database.payload && call_close(&database) != 0 && result == 0) {
        result = 5;
    }
    clear_mock_value(&database);
    rxsqlite_session_leave(previous);
    rxsqlite_session_destroy(session);
    arguments->result = result;
#if defined(_WIN32)
    return 0;
#else
    return NULL;
#endif
}

static int create_test_thread(test_thread *thread,
                              worker_arguments *arguments) {
#if defined(_WIN32)
    *thread = CreateThread(NULL, 0, run_worker, arguments, 0, NULL);
    return *thread ? 0 : -1;
#else
    return pthread_create(thread, NULL, run_worker, arguments);
#endif
}

static void join_test_thread(test_thread thread) {
#if defined(_WIN32)
    (void)WaitForSingleObject(thread, INFINITE);
    (void)CloseHandle(thread);
#else
    (void)pthread_join(thread, NULL);
#endif
}

static int setup_database(const char *path) {
    rxsqlite_session *session = rxsqlite_session_create();
    mock_value database = {0};
    void *previous = NULL;
    int result = 0;
    if (!session || rxsqlite_session_enter(
            session, RXPA_PROCEDURE_CAP_SESSION_AFFINE, &previous) != 0) {
        rxsqlite_session_destroy(session);
        return 1;
    }
    if (call_open(path, &database) != 0 ||
        call_exec(&database, "PRAGMA journal_mode=WAL") != 0 ||
        call_exec(&database,
                  "CREATE TABLE session_rows(worker_id INTEGER NOT NULL, ordinal INTEGER NOT NULL, PRIMARY KEY(worker_id, ordinal))") != 0 ||
        call_close(&database) != 0) {
        result = 2;
    }
    clear_mock_value(&database);
    rxsqlite_session_leave(previous);
    rxsqlite_session_destroy(session);
    return result;
}

static int verify_handle_session_isolation(const char *path) {
    rxsqlite_session *owner = rxsqlite_session_create();
    rxsqlite_session *other = rxsqlite_session_create();
    mock_value database = {0};
    void *previous = NULL;
    int result = 0;
    if (!owner || !other || rxsqlite_session_enter(
            owner, RXPA_PROCEDURE_CAP_SESSION_AFFINE, &previous) != 0) {
        result = 1;
        goto done;
    }
    if (call_open(path, &database) != 0) {
        result = 2;
        goto leave_owner;
    }
    rxsqlite_session_leave(previous);
    previous = NULL;
    if (rxsqlite_session_enter(
            other, RXPA_PROCEDURE_CAP_SESSION_AFFINE, &previous) != 0) {
        result = 3;
        goto restore_owner;
    }
    if (call_exec(&database, "SELECT 1") != RXSQLITE_INVALID_HANDLE) {
        result = 4;
    }
    rxsqlite_session_leave(previous);
    previous = NULL;

restore_owner:
    if (rxsqlite_session_enter(
            owner, RXPA_PROCEDURE_CAP_SESSION_AFFINE, &previous) != 0) {
        result = result ? result : 5;
        goto done;
    }
    if (call_close(&database) != 0 && result == 0) result = 6;

leave_owner:
    clear_mock_value(&database);
    rxsqlite_session_leave(previous);

done:
    rxsqlite_session_destroy(other);
    rxsqlite_session_destroy(owner);
    return result;
}

static int verify_database(const char *path) {
    sqlite3 *database = NULL;
    sqlite3_stmt *statement = NULL;
    int rows = 0;
    int workers = 0;
    int result = sqlite3_open_v2(path, &database,
                                 SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX,
                                 NULL);
    if (result == SQLITE_OK) {
        result = sqlite3_prepare_v2(
            database,
            "SELECT count(*), count(DISTINCT worker_id) FROM session_rows",
            -1, &statement, NULL);
    }
    if (result == SQLITE_OK && sqlite3_step(statement) == SQLITE_ROW) {
        rows = sqlite3_column_int(statement, 0);
        workers = sqlite3_column_int(statement, 1);
    } else {
        result = SQLITE_ERROR;
    }
    if (statement) sqlite3_finalize(statement);
    if (database) sqlite3_close_v2(database);
    return result == SQLITE_OK && rows == 400 && workers == 4;
}

int main(int argc, char **argv) {
    const rxpa_plugin_manifest_v2 *manifest;
    start_gate gate;
    test_thread threads[4];
    worker_arguments arguments[4];
    int index;
    int result = 0;
    if (argc != 2) return 1;
    initialize_mock_context();
    manifest = _rxpa_query_v2();
    if (!manifest || manifest->abi_version != RXPA_PLUGIN_MANIFEST_ABI_V2 ||
        !manifest->session_create || !manifest->session_destroy ||
        !manifest->session_enter || !manifest->session_leave ||
        manifest->procedure_capabilities("rxsqlite.sqliteexec") !=
            RXPA_PROCEDURE_CAP_SESSION_AFFINE) {
        return 2;
    }
    (void)remove(argv[1]);
    if (setup_database(argv[1]) != 0) return 3;
    if (verify_handle_session_isolation(argv[1]) != 0) return 4;

    memset(&gate, 0, sizeof(gate));
    gate.total = 4;
    test_mutex_initialize(&gate.mutex);
    test_condition_initialize(&gate.condition);
    for (index = 0; index < 4; index++) {
        arguments[index].gate = &gate;
        arguments[index].database_path = argv[1];
        arguments[index].worker_id = index + 1;
        arguments[index].result = -1;
        if (create_test_thread(&threads[index], &arguments[index]) != 0) {
            return 5;
        }
    }
    test_mutex_lock(&gate.mutex);
    while (gate.ready < gate.total) {
        test_condition_wait(&gate.condition, &gate.mutex);
    }
    gate.released = 1;
    test_condition_broadcast(&gate.condition);
    test_mutex_unlock(&gate.mutex);
    for (index = 0; index < 4; index++) {
        join_test_thread(threads[index]);
        if (arguments[index].result != 0) result = 10 + arguments[index].result;
    }
    test_condition_destroy(&gate.condition);
    test_mutex_destroy(&gate.mutex);
    if (result != 0 || !verify_database(argv[1])) return result ? result : 20;
    puts("PASS: rxsqlite thread sessions=4 rows=400 diagnostics_isolated=1 handle_isolation=1 fullmutex=1");
    return 0;
}
