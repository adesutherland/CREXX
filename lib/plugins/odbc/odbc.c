/*
 * cREXX ODBC RXPA plugin
 *
 * The legacy procedure ABI is retained.  A V2-aware host creates one
 * connection/session object per VM and binds every database procedure to it
 * at load time.  An older host reaches the process-default session.
 */
#include "crexxpa.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef CREXX_ODBC_MOCK
#include "mock_odbc.h"
#else
#include <sql.h>
#include <sqlext.h>
#endif

#ifdef _WIN32
#include <windows.h>
static SRWLOCK odbc_generation_lock = SRWLOCK_INIT;
#define ODBC_GENERATION_LOCK() AcquireSRWLockExclusive(&odbc_generation_lock)
#define ODBC_GENERATION_UNLOCK() ReleaseSRWLockExclusive(&odbc_generation_lock)
#else
#include <pthread.h>
static pthread_mutex_t odbc_generation_lock = PTHREAD_MUTEX_INITIALIZER;
#define ODBC_GENERATION_LOCK() \
    ((void)pthread_mutex_lock(&odbc_generation_lock))
#define ODBC_GENERATION_UNLOCK() \
    ((void)pthread_mutex_unlock(&odbc_generation_lock))
#endif

#if defined(_MSC_VER)
#define ODBC_THREAD_LOCAL __declspec(thread)
#else
#define ODBC_THREAD_LOCAL __thread
#endif

typedef enum odbc_parameter_kind {
    ODBC_PARAMETER_STRING,
    ODBC_PARAMETER_INTEGER,
    ODBC_PARAMETER_FLOAT,
    ODBC_PARAMETER_NULL
} odbc_parameter_kind;

typedef struct odbc_parameter {
    SQLUSMALLINT index;
    odbc_parameter_kind kind;
    char *string_value;
    SQLBIGINT integer_value;
    double float_value;
    SQLLEN indicator;
    struct odbc_parameter *next;
} odbc_parameter;

typedef struct odbc_statement {
    rxinteger id;
    SQLHSTMT handle;
    odbc_parameter *parameters;
    struct odbc_statement *next;
} odbc_statement;

typedef struct odbc_session {
    SQLHENV environment;
    SQLHDBC connection;
    SQLHSTMT default_statement;
    odbc_statement *statements;
    uint32_t generation;
    uint32_t next_statement;
    int connected;
    int transaction_active;
    char diagnostic_state[6];
    SQLINTEGER diagnostic_native;
    char diagnostic_message[SQL_MAX_MESSAGE_LENGTH];
} odbc_session;

static ODBC_THREAD_LOCAL odbc_session *odbc_current_session;
static odbc_session odbc_default_session;
static uint32_t odbc_next_generation;

static uint32_t odbc_allocate_generation_locked(void) {
    uint32_t generation = ++odbc_next_generation;
    if (!generation || generation > UINT32_C(0x7fffffff)) {
        odbc_next_generation = 1u;
        generation = 1u;
    }
    return generation;
}

static void odbc_clear_diagnostic(odbc_session *session) {
    if (!session) return;
    session->diagnostic_state[0] = 0;
    session->diagnostic_native = 0;
    session->diagnostic_message[0] = 0;
}

static void odbc_capture_diagnostic(odbc_session *session,
                                    SQLSMALLINT handle_type,
                                    SQLHANDLE handle,
                                    const char *fallback) {
    SQLSMALLINT length = 0;
    SQLRETURN rc;
    if (!session) return;
    odbc_clear_diagnostic(session);
    if (handle) {
        rc = SQLGetDiagRec(handle_type, handle, 1,
                           (SQLCHAR *)session->diagnostic_state,
                           &session->diagnostic_native,
                           (SQLCHAR *)session->diagnostic_message,
                           (SQLSMALLINT)sizeof(session->diagnostic_message),
                           &length);
        if (SQL_SUCCEEDED(rc)) {
            session->diagnostic_state[5] = 0;
            session->diagnostic_message[
                    sizeof(session->diagnostic_message) - 1u] = 0;
            return;
        }
    }
    snprintf(session->diagnostic_message,
             sizeof(session->diagnostic_message), "%s",
             fallback ? fallback : "ODBC operation failed");
}

static uint32_t odbc_allocate_generation(void) {
    uint32_t generation;
    ODBC_GENERATION_LOCK();
    generation = odbc_allocate_generation_locked();
    ODBC_GENERATION_UNLOCK();
    return generation;
}

static void odbc_session_initialize(odbc_session *session) {
    if (!session) return;
    memset(session, 0, sizeof(*session));
    session->generation = odbc_allocate_generation();
    session->next_statement = 1u;
}

static odbc_session *odbc_session_current(void) {
    if (odbc_current_session) return odbc_current_session;
    ODBC_GENERATION_LOCK();
    if (!odbc_default_session.generation) {
        memset(&odbc_default_session, 0, sizeof(odbc_default_session));
        odbc_default_session.generation = odbc_allocate_generation_locked();
        odbc_default_session.next_statement = 1u;
    }
    ODBC_GENERATION_UNLOCK();
    return &odbc_default_session;
}

static void odbc_free_parameters(odbc_parameter *parameter) {
    while (parameter) {
        odbc_parameter *next = parameter->next;
        free(parameter->string_value);
        free(parameter);
        parameter = next;
    }
}

static void odbc_close_statement(odbc_statement *statement) {
    if (!statement) return;
    if (statement->handle) {
        SQLFreeHandle(SQL_HANDLE_STMT, statement->handle);
        statement->handle = SQL_NULL_HSTMT;
    }
    odbc_free_parameters(statement->parameters);
    statement->parameters = NULL;
    free(statement);
}

static void odbc_close_all_statements(odbc_session *session) {
    odbc_statement *statement;
    if (!session) return;
    statement = session->statements;
    session->statements = NULL;
    while (statement) {
        odbc_statement *next = statement->next;
        odbc_close_statement(statement);
        statement = next;
    }
    if (session->default_statement) {
        SQLFreeHandle(SQL_HANDLE_STMT, session->default_statement);
        session->default_statement = SQL_NULL_HSTMT;
    }
}

static void odbc_disconnect_session(odbc_session *session) {
    if (!session) return;
    odbc_close_all_statements(session);
    if (session->connection) {
        if (session->transaction_active) {
            (void)SQLEndTran(SQL_HANDLE_DBC, session->connection,
                             SQL_ROLLBACK);
        }
        if (session->connected) (void)SQLDisconnect(session->connection);
        SQLFreeHandle(SQL_HANDLE_DBC, session->connection);
    }
    if (session->environment) {
        SQLFreeHandle(SQL_HANDLE_ENV, session->environment);
    }
    session->environment = SQL_NULL_HENV;
    session->connection = SQL_NULL_HDBC;
    session->connected = 0;
    session->transaction_active = 0;
}

static void *odbc_session_create(void) {
    odbc_session *session = malloc(sizeof(*session));
    if (!session) return NULL;
    odbc_session_initialize(session);
    return session;
}

static void odbc_session_destroy(void *opaque_session) {
    odbc_session *session = (odbc_session *)opaque_session;
    if (!session) return;
    odbc_disconnect_session(session);
    free(session);
}

static int odbc_session_enter(void *opaque_session, uint32_t capabilities,
                              void **previous) {
    if (!opaque_session || !previous ||
        capabilities != RXPA_PROCEDURE_CAP_SESSION_AFFINE) return -1;
    *previous = odbc_current_session;
    odbc_current_session = (odbc_session *)opaque_session;
    return 0;
}

static void odbc_session_leave(void *previous) {
    odbc_current_session = (odbc_session *)previous;
}

static uint32_t odbc_procedure_capabilities(const char *procedure_name) {
    if (procedure_name && strcmp(procedure_name, "odbc.show_message") == 0) {
        return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
    }
#ifdef CREXX_ODBC_MOCK
    if (procedure_name && strncmp(procedure_name, "odbc._mock_", 11u) == 0) {
        return RXPA_PROCEDURE_CAP_PROCESS_REENTRANT;
    }
#endif
    return RXPA_PROCEDURE_CAP_SESSION_AFFINE;
}

RXPA_PLUGIN_SESSION_AWARE(odbc_session_create, odbc_session_destroy,
                          odbc_session_enter, odbc_session_leave,
                          odbc_procedure_capabilities)

static int odbc_require_connection(odbc_session *session) {
    if (session && session->connected && session->connection) return 1;
    if (session) {
        odbc_capture_diagnostic(session, SQL_HANDLE_DBC,
                                session->connection,
                                "ODBC session is not connected");
    }
    return 0;
}

static int odbc_replace_default_statement(odbc_session *session) {
    SQLRETURN rc;
    if (!odbc_require_connection(session)) return 0;
    if (session->default_statement) {
        SQLFreeHandle(SQL_HANDLE_STMT, session->default_statement);
        session->default_statement = SQL_NULL_HSTMT;
    }
    rc = SQLAllocHandle(SQL_HANDLE_STMT, session->connection,
                        &session->default_statement);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_DBC,
                                session->connection,
                                "Unable to allocate ODBC statement");
        session->default_statement = SQL_NULL_HSTMT;
        return 0;
    }
    return 1;
}

static rxinteger odbc_make_statement_id(odbc_session *session) {
    uint32_t sequence;
    if (!session->next_statement) return 0;
    sequence = session->next_statement++;
    return (rxinteger)(((uint64_t)session->generation << 32) | sequence);
}

static odbc_statement *odbc_find_statement(odbc_session *session,
                                           rxinteger id) {
    odbc_statement *statement;
    if (!session || id <= 0 ||
        (uint32_t)((uint64_t)id >> 32) != session->generation) return NULL;
    statement = session->statements;
    while (statement) {
        if (statement->id == id) return statement;
        statement = statement->next;
    }
    return NULL;
}

static int odbc_narrow_ordinal(rxinteger source, SQLUSMALLINT *target) {
    if (!target || source <= 0 || (uint64_t)source > UINT16_MAX) return 0;
    *target = (SQLUSMALLINT)source;
    return 1;
}

static int odbc_bind_parameter(odbc_session *session,
                               odbc_statement *statement,
                               SQLUSMALLINT index,
                               odbc_parameter_kind kind,
                               const char *string_value,
                               SQLBIGINT integer_value,
                               double float_value) {
    odbc_parameter **link;
    odbc_parameter *old_parameter;
    odbc_parameter *parameter;
    SQLRETURN rc;
    if (!session || !statement || !index) return 0;
    link = &statement->parameters;
    while (*link && (*link)->index != index) link = &(*link)->next;
    old_parameter = *link;
    parameter = calloc(1u, sizeof(*parameter));
    if (!parameter) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                statement->handle,
                                "Unable to allocate ODBC parameter");
        return 0;
    }
    parameter->index = index;
    parameter->kind = kind;
    parameter->integer_value = integer_value;
    parameter->float_value = float_value;
    if (kind == ODBC_PARAMETER_STRING) {
        size_t length = strlen(string_value ? string_value : "");
        parameter->string_value = malloc(length + 1u);
        if (!parameter->string_value) {
            odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                    statement->handle,
                                    "Unable to copy ODBC string parameter");
            free(parameter);
            return 0;
        }
        memcpy(parameter->string_value, string_value ? string_value : "",
               length + 1u);
        parameter->indicator = SQL_NTS;
        rc = SQLBindParameter(statement->handle, index, SQL_PARAM_INPUT,
                              SQL_C_CHAR, SQL_VARCHAR,
                              (SQLULEN)length, 0,
                              parameter->string_value,
                              (SQLLEN)(length + 1u),
                              &parameter->indicator);
    } else if (kind == ODBC_PARAMETER_INTEGER) {
        parameter->indicator = 0;
        rc = SQLBindParameter(statement->handle, index, SQL_PARAM_INPUT,
                              SQL_C_SBIGINT, SQL_BIGINT, 0, 0,
                              &parameter->integer_value,
                              (SQLLEN)sizeof(parameter->integer_value),
                              &parameter->indicator);
    } else if (kind == ODBC_PARAMETER_FLOAT) {
        parameter->indicator = 0;
        rc = SQLBindParameter(statement->handle, index, SQL_PARAM_INPUT,
                              SQL_C_DOUBLE, SQL_DOUBLE, 0, 0,
                              &parameter->float_value,
                              (SQLLEN)sizeof(parameter->float_value),
                              &parameter->indicator);
    } else {
        parameter->indicator = SQL_NULL_DATA;
        rc = SQLBindParameter(statement->handle, index, SQL_PARAM_INPUT,
                              SQL_C_CHAR, SQL_VARCHAR, 0, 0,
                              NULL, 0, &parameter->indicator);
    }
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                statement->handle,
                                "Unable to bind ODBC parameter");
        free(parameter->string_value);
        free(parameter);
        return 0;
    }
    if (old_parameter) {
        parameter->next = old_parameter->next;
        *link = parameter;
        free(old_parameter->string_value);
        free(old_parameter);
    } else {
        parameter->next = statement->parameters;
        statement->parameters = parameter;
    }
    return 1;
}

static SQLHSTMT odbc_statement_from_optional_id(odbc_session *session,
                                                rxinteger id) {
    odbc_statement *statement;
    if (!id) return session ? session->default_statement : SQL_NULL_HSTMT;
    statement = odbc_find_statement(session, id);
    return statement ? statement->handle : SQL_NULL_HSTMT;
}

static rxinteger odbc_fetch_handle(odbc_session *session, SQLHSTMT handle) {
    SQLRETURN rc;
    if (!handle) return -1;
    rc = SQLFetch(handle);
    if (rc == SQL_NO_DATA) return 1;
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "ODBC fetch failed");
        return -1;
    }
    return 0;
}

static rxinteger odbc_columns_handle(odbc_session *session,
                                     SQLHSTMT handle) {
    SQLSMALLINT columns = 0;
    if (!handle || !SQL_SUCCEEDED(SQLNumResultCols(handle, &columns))) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to read ODBC column count");
        return -1;
    }
    return (rxinteger)columns;
}

static const char *odbc_get_column_handle(odbc_session *session,
                                          SQLHSTMT handle,
                                          SQLUSMALLINT column,
                                          char *buffer,
                                          size_t buffer_size) {
    SQLLEN indicator = 0;
    SQLRETURN rc;
    if (!handle || !column || !buffer || !buffer_size) return "";
    buffer[0] = 0;
    rc = SQLGetData(handle, column, SQL_C_CHAR, buffer,
                    (SQLLEN)buffer_size, &indicator);
    if (indicator == SQL_NULL_DATA) return "";
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to read ODBC column");
        return "";
    }
    buffer[buffer_size - 1u] = 0;
    return buffer;
}

static const char *odbc_column_name_handle(odbc_session *session,
                                           SQLHSTMT handle,
                                           SQLUSMALLINT column,
                                           char *buffer,
                                           size_t buffer_size) {
    SQLSMALLINT length = 0;
    SQLRETURN rc;
    if (!handle || !column || !buffer || !buffer_size) return "";
    buffer[0] = 0;
    rc = SQLColAttribute(handle, column, SQL_DESC_NAME, buffer,
                         (SQLSMALLINT)buffer_size, &length, NULL);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to read ODBC column name");
        return "";
    }
    buffer[buffer_size - 1u] = 0;
    return buffer;
}

static rxinteger odbc_column_type_handle(odbc_session *session,
                                         SQLHSTMT handle,
                                         SQLUSMALLINT column) {
    SQLLEN type = 0;
    SQLRETURN rc;
    if (!handle || !column) return -1;
    rc = SQLColAttribute(handle, column, SQL_DESC_TYPE,
                         NULL, 0, NULL, &type);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to read ODBC column type");
        return -1;
    }
    return (rxinteger)type;
}

PROCEDURE(odbc_connect) {
    odbc_session *session = odbc_session_current();
    const char *dsn = GETSTRING(ARG0);
    const char *user = GETSTRING(ARG1);
    const char *password = GETSTRING(ARG2);
    SQLRETURN rc;
    odbc_disconnect_session(session);
    odbc_clear_diagnostic(session);
    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE,
                        &session->environment);
    if (!SQL_SUCCEEDED(rc)) RETURNINTX(-1);
    rc = SQLSetEnvAttr(session->environment, SQL_ATTR_ODBC_VERSION,
                       (SQLPOINTER)(uintptr_t)SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_ENV,
                                session->environment,
                                "Unable to configure ODBC environment");
        odbc_disconnect_session(session);
        RETURNINTX(-2);
    }
    rc = SQLAllocHandle(SQL_HANDLE_DBC, session->environment,
                        &session->connection);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_ENV,
                                session->environment,
                                "Unable to allocate ODBC connection");
        odbc_disconnect_session(session);
        RETURNINTX(-3);
    }
    rc = SQLConnect(session->connection,
                    (SQLCHAR *)dsn, SQL_NTS,
                    (SQLCHAR *)user, SQL_NTS,
                    (SQLCHAR *)password, SQL_NTS);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_DBC,
                                session->connection,
                                "ODBC connection failed");
        odbc_disconnect_session(session);
        RETURNINTX(-4);
    }
    session->connected = 1;
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_disconnect) {
    odbc_disconnect_session(odbc_session_current());
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_execute) {
    odbc_session *session = odbc_session_current();
    const char *sql = GETSTRING(ARG0);
    SQLRETURN rc;
    if (!odbc_replace_default_statement(session)) RETURNINTX(-1);
    rc = SQLExecDirect(session->default_statement,
                       (SQLCHAR *)sql, SQL_NTS);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                session->default_statement,
                                "ODBC direct execution failed");
        RETURNINTX(-2);
    }
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_prepare) {
    odbc_session *session = odbc_session_current();
    const char *sql = GETSTRING(ARG0);
    odbc_statement *statement;
    SQLRETURN rc;
    if (!odbc_require_connection(session)) RETURNINTX(-1);
    statement = calloc(1u, sizeof(*statement));
    if (!statement) RETURNINTX(-2);
    rc = SQLAllocHandle(SQL_HANDLE_STMT, session->connection,
                        &statement->handle);
    if (SQL_SUCCEEDED(rc)) {
        rc = SQLPrepare(statement->handle, (SQLCHAR *)sql, SQL_NTS);
    }
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                statement->handle,
                                "Unable to prepare ODBC statement");
        odbc_close_statement(statement);
        RETURNINTX(-3);
    }
    statement->id = odbc_make_statement_id(session);
    if (!statement->id) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                statement->handle,
                                "ODBC statement ID space exhausted");
        odbc_close_statement(statement);
        RETURNINTX(-4);
    }
    statement->next = session->statements;
    session->statements = statement;
    RETURNINTX(statement->id);
ENDPROC
}

#define ODBC_BIND_PROCEDURE(function_name, parameter_kind, value_expression) \
PROCEDURE(function_name) { \
    odbc_session *session = odbc_session_current(); \
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0)); \
    SQLUSMALLINT index; \
    if (!statement || !odbc_narrow_ordinal(GETINT(ARG1), &index) || \
        !odbc_bind_parameter(session, statement, index, parameter_kind, \
                             value_expression, 0, 0.0)) RETURNINTX(-1); \
    RETURNINTX(0); \
ENDPROC \
}

ODBC_BIND_PROCEDURE(odbc_bind_string, ODBC_PARAMETER_STRING,
                    GETSTRING(ARG2))

PROCEDURE(odbc_bind_integer) {
    odbc_session *session = odbc_session_current();
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0));
    SQLUSMALLINT index;
    if (!statement || !odbc_narrow_ordinal(GETINT(ARG1), &index) ||
        !odbc_bind_parameter(session, statement, index,
                             ODBC_PARAMETER_INTEGER, NULL,
                             (SQLBIGINT)GETINT(ARG2), 0.0)) RETURNINTX(-1);
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_bind_float) {
    odbc_session *session = odbc_session_current();
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0));
    SQLUSMALLINT index;
    if (!statement || !odbc_narrow_ordinal(GETINT(ARG1), &index) ||
        !odbc_bind_parameter(session, statement, index,
                             ODBC_PARAMETER_FLOAT, NULL, 0,
                             GETFLOAT(ARG2))) RETURNINTX(-1);
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_bind_null) {
    odbc_session *session = odbc_session_current();
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0));
    SQLUSMALLINT index;
    if (!statement || !odbc_narrow_ordinal(GETINT(ARG1), &index) ||
        !odbc_bind_parameter(session, statement, index,
                             ODBC_PARAMETER_NULL, NULL, 0, 0.0)) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_execute_prepared) {
    odbc_session *session = odbc_session_current();
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0));
    SQLRETURN rc;
    if (!statement) RETURNINTX(-1);
    rc = SQLExecute(statement->handle);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                statement->handle,
                                "ODBC prepared execution failed");
        RETURNINTX(-2);
    }
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_close_prepared) {
    odbc_session *session = odbc_session_current();
    rxinteger id = GETINT(ARG0);
    odbc_statement **link = &session->statements;
    while (*link && (*link)->id != id) link = &(*link)->next;
    if (!*link) RETURNINTX(-1);
    {
        odbc_statement *statement = *link;
        *link = statement->next;
        odbc_close_statement(statement);
    }
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_reset_prepared) {
    odbc_session *session = odbc_session_current();
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0));
    if (!statement) RETURNINTX(-1);
    if (!SQL_SUCCEEDED(SQLFreeStmt(statement->handle, SQL_CLOSE)) ||
        !SQL_SUCCEEDED(SQLFreeStmt(statement->handle, SQL_RESET_PARAMS))) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                statement->handle,
                                "Unable to reset ODBC prepared statement");
        RETURNINTX(-2);
    }
    odbc_free_parameters(statement->parameters);
    statement->parameters = NULL;
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_fetch) {
    RETURNINTX(odbc_fetch_handle(odbc_session_current(),
                                 odbc_session_current()->default_statement));
ENDPROC
}

PROCEDURE(odbc_fetch_statement) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_fetch_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0))));
ENDPROC
}

PROCEDURE(odbc_columns) {
    RETURNINTX(odbc_columns_handle(
            odbc_session_current(),
            odbc_session_current()->default_statement));
ENDPROC
}

PROCEDURE(odbc_columns_statement) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_columns_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0))));
ENDPROC
}

PROCEDURE(odbc_get_column) {
    char buffer[4096];
    odbc_session *session = odbc_session_current();
    RETURNSTRX(odbc_get_column_handle(
            session, session->default_statement,
            (SQLUSMALLINT)GETINT(ARG0), buffer, sizeof(buffer)));
ENDPROC
}

PROCEDURE(odbc_get_column_statement) {
    char buffer[4096];
    odbc_session *session = odbc_session_current();
    RETURNSTRX(odbc_get_column_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0)),
            (SQLUSMALLINT)GETINT(ARG1), buffer, sizeof(buffer)));
ENDPROC
}

PROCEDURE(odbc_column_name) {
    char buffer[256];
    odbc_session *session = odbc_session_current();
    RETURNSTRX(odbc_column_name_handle(
            session, session->default_statement,
            (SQLUSMALLINT)GETINT(ARG0), buffer, sizeof(buffer)));
ENDPROC
}

PROCEDURE(odbc_column_name_statement) {
    char buffer[256];
    odbc_session *session = odbc_session_current();
    RETURNSTRX(odbc_column_name_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0)),
            (SQLUSMALLINT)GETINT(ARG1), buffer, sizeof(buffer)));
ENDPROC
}

PROCEDURE(odbc_column_type) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_column_type_handle(
            session, session->default_statement,
            (SQLUSMALLINT)GETINT(ARG0)));
ENDPROC
}

PROCEDURE(odbc_column_type_statement) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_column_type_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0)),
            (SQLUSMALLINT)GETINT(ARG1)));
ENDPROC
}

static const char *odbc_column_info_handle(odbc_session *session,
                                           SQLHSTMT handle,
                                           SQLUSMALLINT column,
                                           char *buffer,
                                           size_t buffer_size) {
    SQLCHAR name[256];
    SQLSMALLINT name_length = 0;
    SQLSMALLINT type = 0;
    SQLULEN size = 0;
    SQLSMALLINT digits = 0;
    SQLSMALLINT nullable = 0;
    SQLRETURN rc;
    if (!handle || !column) return "";
    rc = SQLDescribeCol(handle, column, name, sizeof(name), &name_length,
                        &type, &size, &digits, &nullable);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to describe ODBC column");
        return "";
    }
    snprintf(buffer, buffer_size, "%s,%d,%llu,%d,%d", (char *)name,
             (int)type, (unsigned long long)size,
             (int)digits, (int)nullable);
    return buffer;
}

PROCEDURE(odbc_column_info) {
    char buffer[512];
    odbc_session *session = odbc_session_current();
    RETURNSTRX(odbc_column_info_handle(
            session, session->default_statement,
            (SQLUSMALLINT)GETINT(ARG0), buffer, sizeof(buffer)));
ENDPROC
}

PROCEDURE(odbc_column_info_statement) {
    char buffer[512];
    odbc_session *session = odbc_session_current();
    RETURNSTRX(odbc_column_info_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0)),
            (SQLUSMALLINT)GETINT(ARG1), buffer, sizeof(buffer)));
ENDPROC
}

static rxinteger odbc_row_count_handle(odbc_session *session,
                                       SQLHSTMT handle) {
    SQLLEN count = 0;
    if (!handle || !SQL_SUCCEEDED(SQLRowCount(handle, &count))) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to read ODBC row count");
        return -1;
    }
    return (rxinteger)count;
}

PROCEDURE(odbc_row_count) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_row_count_handle(session, session->default_statement));
ENDPROC
}

PROCEDURE(odbc_row_count_statement) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_row_count_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0))));
ENDPROC
}

static rxinteger odbc_move_handle(odbc_session *session, SQLHSTMT handle,
                                  SQLLEN row) {
    SQLRETURN rc;
    if (!handle) return -1;
    rc = SQLFetchScroll(handle, SQL_FETCH_ABSOLUTE, row);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, handle,
                                "Unable to move ODBC cursor");
        return -1;
    }
    return 0;
}

PROCEDURE(odbc_move_to) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_move_handle(session, session->default_statement,
                                (SQLLEN)GETINT(ARG0)));
ENDPROC
}

PROCEDURE(odbc_move_to_statement) {
    odbc_session *session = odbc_session_current();
    RETURNINTX(odbc_move_handle(
            session, odbc_statement_from_optional_id(session, GETINT(ARG0)),
            (SQLLEN)GETINT(ARG1)));
ENDPROC
}

PROCEDURE(odbc_begin_transaction) {
    odbc_session *session = odbc_session_current();
    SQLRETURN rc;
    if (!odbc_require_connection(session)) RETURNINTX(-1);
    rc = SQLSetConnectAttr(session->connection, SQL_ATTR_AUTOCOMMIT,
                           (SQLPOINTER)(uintptr_t)SQL_AUTOCOMMIT_OFF, 0);
    if (!SQL_SUCCEEDED(rc)) RETURNINTX(-1);
    session->transaction_active = 1;
    RETURNINTX(0);
ENDPROC
}

static rxinteger odbc_end_transaction(odbc_session *session,
                                      SQLSMALLINT completion) {
    SQLRETURN rc;
    if (!odbc_require_connection(session)) return -1;
    rc = SQLEndTran(SQL_HANDLE_DBC, session->connection, completion);
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_DBC,
                                session->connection,
                                "Unable to complete ODBC transaction");
        return -1;
    }
    session->transaction_active = 0;
    (void)SQLSetConnectAttr(session->connection, SQL_ATTR_AUTOCOMMIT,
                            (SQLPOINTER)(uintptr_t)SQL_AUTOCOMMIT_ON, 0);
    return 0;
}

PROCEDURE(odbc_commit) {
    RETURNINTX(odbc_end_transaction(odbc_session_current(), SQL_COMMIT));
ENDPROC
}

PROCEDURE(odbc_rollback) {
    RETURNINTX(odbc_end_transaction(odbc_session_current(), SQL_ROLLBACK));
ENDPROC
}

PROCEDURE(odbc_error_message) {
    RETURNSTRX(odbc_session_current()->diagnostic_message);
ENDPROC
}

PROCEDURE(odbc_error_message_statement) {
    odbc_session *session = odbc_session_current();
    odbc_statement *statement = odbc_find_statement(session, GETINT(ARG0));
    if (!statement) RETURNSTRX("Invalid ODBC statement ID");
    odbc_capture_diagnostic(session, SQL_HANDLE_STMT, statement->handle,
                            "No ODBC statement diagnostic");
    RETURNSTRX(session->diagnostic_message);
ENDPROC
}

PROCEDURE(odbc_get_diagnostics) {
    char buffer[SQL_MAX_MESSAGE_LENGTH + 32];
    odbc_session *session = odbc_session_current();
    snprintf(buffer, sizeof(buffer), "%s:%ld:%s",
             session->diagnostic_state,
             (long)session->diagnostic_native,
             session->diagnostic_message);
    RETURNSTRX(buffer);
ENDPROC
}

static const char *odbc_collect_catalogue(odbc_session *session,
                                          int primary_keys,
                                          const char *table,
                                          char *result,
                                          size_t result_size) {
    SQLHSTMT statement = SQL_NULL_HSTMT;
    SQLRETURN rc;
    SQLUSMALLINT column = primary_keys ? 4u : 3u;
    size_t used = 1u;
    result[0] = '0';
    result[1] = 0;
    if (!odbc_require_connection(session) ||
        !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT,
                                      session->connection, &statement))) {
        return "8Unable to allocate catalogue statement";
    }
    if (primary_keys) {
        rc = SQLPrimaryKeys(statement, NULL, 0, NULL, 0,
                            (SQLCHAR *)table, SQL_NTS);
    } else {
        rc = SQLTables(statement, NULL, 0, NULL, 0,
                       NULL, 0, (SQLCHAR *)"TABLE", SQL_NTS);
    }
    if (!SQL_SUCCEEDED(rc)) {
        odbc_capture_diagnostic(session, SQL_HANDLE_STMT, statement,
                                "ODBC catalogue query failed");
        SQLFreeHandle(SQL_HANDLE_STMT, statement);
        return "8ODBC catalogue query failed";
    }
    while (SQL_SUCCEEDED(SQLFetch(statement))) {
        char item[256];
        SQLLEN indicator = 0;
        item[0] = 0;
        rc = SQLGetData(statement, column, SQL_C_CHAR, item,
                        sizeof(item), &indicator);
        if (SQL_SUCCEEDED(rc) && indicator != SQL_NULL_DATA && item[0]) {
            size_t item_length = strlen(item);
            if (used + item_length + 2u >= result_size) break;
            result[used++] = ' ';
            memcpy(result + used, item, item_length + 1u);
            used += item_length;
        }
    }
    SQLFreeHandle(SQL_HANDLE_STMT, statement);
    return result;
}

PROCEDURE(odbc_tables) {
    char result[4096];
    RETURNSTRX(odbc_collect_catalogue(odbc_session_current(), 0, NULL,
                                      result, sizeof(result)));
ENDPROC
}

PROCEDURE(odbc_primary_keys) {
    char result[4096];
    RETURNSTRX(odbc_collect_catalogue(odbc_session_current(), 1,
                                      GETSTRING(ARG0), result,
                                      sizeof(result)));
ENDPROC
}

PROCEDURE(odbc_execute_batch) {
    odbc_session *session = odbc_session_current();
    const char *source = GETSTRING(ARG0);
    const char *delimiter = GETSTRING(ARG1);
    size_t source_length = strlen(source);
    size_t delimiter_length = strlen(delimiter);
    char *copy;
    char *cursor;
    if (!delimiter_length || !odbc_replace_default_statement(session)) {
        RETURNINTX(-1);
    }
    copy = malloc(source_length + 1u);
    if (!copy) RETURNINTX(-2);
    memcpy(copy, source, source_length + 1u);
    cursor = copy;
    while (*cursor) {
        char *next = strstr(cursor, delimiter);
        SQLRETURN rc;
        if (next) *next = 0;
        if (*cursor) {
            rc = SQLExecDirect(session->default_statement,
                               (SQLCHAR *)cursor, SQL_NTS);
            if (!SQL_SUCCEEDED(rc)) {
                odbc_capture_diagnostic(session, SQL_HANDLE_STMT,
                                        session->default_statement,
                                        "ODBC batch execution failed");
                free(copy);
                RETURNINTX(-3);
            }
            (void)SQLFreeStmt(session->default_statement, SQL_CLOSE);
        }
        if (!next) break;
        cursor = next + delimiter_length;
    }
    free(copy);
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(odbc_get_info) {
    odbc_session *session = odbc_session_current();
    char info[256];
    SQLRETURN rc;
    if (!odbc_require_connection(session)) RETURNSTRX("");
    info[0] = 0;
    rc = SQLGetInfo(session->connection, SQL_DBMS_NAME,
                    info, sizeof(info), NULL);
    RETURNSTRX(SQL_SUCCEEDED(rc) ? info : "");
ENDPROC
}

PROCEDURE(odbc_get_connection_attr) {
    odbc_session *session = odbc_session_current();
    SQLINTEGER value = 0;
    SQLRETURN rc;
    if (!odbc_require_connection(session)) RETURNINTX(-1);
    rc = SQLGetConnectAttr(session->connection, (SQLINTEGER)GETINT(ARG0),
                           &value, 0, NULL);
    RETURNINTX(SQL_SUCCEEDED(rc) ? value : -1);
ENDPROC
}

PROCEDURE(odbc_database) {
    odbc_session *session = odbc_session_current();
    const char *catalogue = GETSTRING(ARG0);
    SQLRETURN rc;
    if (!odbc_require_connection(session)) RETURNINTX(-1);
    rc = SQLSetConnectAttr(session->connection, SQL_ATTR_CURRENT_CATALOG,
                           (SQLPOINTER)catalogue, SQL_NTS);
    RETURNINTX(SQL_SUCCEEDED(rc) ? 0 : -1);
ENDPROC
}

PROCEDURE(show_message) {
    printf("%s\n%s\n%s\n%s\n", GETSTRING(ARG0), GETSTRING(ARG1),
           GETSTRING(ARG2), GETSTRING(ARG3));
    RETURNINTX(0);
ENDPROC
}

#ifdef CREXX_ODBC_MOCK
PROCEDURE(odbc_mock_maximum_overlap) {
    RETURNINTX(crexx_mock_odbc_maximum_execute_overlap());
ENDPROC
}

PROCEDURE(odbc_mock_reset_overlap) {
    crexx_mock_odbc_reset_execute_overlap();
    RETURNINTX(0);
ENDPROC
}
#endif

LOADFUNCS
    ADDPROC(odbc_connect, "odbc.odbc_connect", "b", ".int", "dsn=.string,user=.string,password=.string");
    ADDPROC(odbc_disconnect, "odbc.odbc_disconnect", "b", ".int", "");
    ADDPROC(odbc_execute, "odbc.odbc_execute", "b", ".int", "sql=.string");
    ADDPROC(odbc_prepare, "odbc.odbc_prepare", "b", ".int", "sql=.string");
    ADDPROC(odbc_bind_string, "odbc.odbc_bind_string", "b", ".int", "statement=.int,parameter=.int,value=.string");
    ADDPROC(odbc_bind_integer, "odbc.odbc_bind_integer", "b", ".int", "statement=.int,parameter=.int,value=.int");
    ADDPROC(odbc_bind_float, "odbc.odbc_bind_float", "b", ".int", "statement=.int,parameter=.int,value=.float");
    ADDPROC(odbc_bind_null, "odbc.odbc_bind_null", "b", ".int", "statement=.int,parameter=.int");
    ADDPROC(odbc_execute_prepared, "odbc.odbc_execute_prepared", "b", ".int", "statement=.int");
    ADDPROC(odbc_reset_prepared, "odbc.odbc_reset_prepared", "b", ".int", "statement=.int");
    ADDPROC(odbc_close_prepared, "odbc.odbc_close_prepared", "b", ".int", "statement=.int");
    ADDPROC(odbc_fetch, "odbc.odbc_fetch", "b", ".int", "");
    ADDPROC(odbc_fetch_statement, "odbc.odbc_fetch_statement", "b", ".int", "statement=.int");
    ADDPROC(odbc_columns, "odbc.odbc_columns", "b", ".int", "");
    ADDPROC(odbc_columns_statement, "odbc.odbc_columns_statement", "b", ".int", "statement=.int");
    ADDPROC(odbc_get_column, "odbc.odbc_getcolumn", "b", ".string", "column=.int");
    ADDPROC(odbc_get_column_statement, "odbc.odbc_getcolumn_statement", "b", ".string", "statement=.int,column=.int");
    ADDPROC(odbc_column_name, "odbc.odbc_colname", "b", ".string", "column=.int");
    ADDPROC(odbc_column_name_statement, "odbc.odbc_colname_statement", "b", ".string", "statement=.int,column=.int");
    ADDPROC(odbc_column_type, "odbc.odbc_coltype", "b", ".int", "column=.int");
    ADDPROC(odbc_column_type_statement, "odbc.odbc_coltype_statement", "b", ".int", "statement=.int,column=.int");
    ADDPROC(odbc_column_info, "odbc.odbc_column_info", "b", ".string", "column=.int");
    ADDPROC(odbc_column_info_statement, "odbc.odbc_column_info_statement", "b", ".string", "statement=.int,column=.int");
    ADDPROC(odbc_row_count, "odbc.odbc_row_count", "b", ".int", "");
    ADDPROC(odbc_row_count_statement, "odbc.odbc_row_count_statement", "b", ".int", "statement=.int");
    ADDPROC(odbc_move_to, "odbc.odbc_move_to", "b", ".int", "row=.int");
    ADDPROC(odbc_move_to_statement, "odbc.odbc_move_to_statement", "b", ".int", "statement=.int,row=.int");
    ADDPROC(odbc_begin_transaction, "odbc.odbc_begin_transaction", "b", ".int", "");
    ADDPROC(odbc_commit, "odbc.odbc_commit", "b", ".int", "");
    ADDPROC(odbc_rollback, "odbc.odbc_rollback", "b", ".int", "");
    ADDPROC(odbc_error_message, "odbc.odbc_error_message", "b", ".string", "");
    ADDPROC(odbc_error_message_statement, "odbc.odbc_error_message_statement", "b", ".string", "statement=.int");
    ADDPROC(odbc_get_diagnostics, "odbc.odbc_get_diagnostics", "b", ".string", "");
    ADDPROC(odbc_tables, "odbc.odbc_tables", "b", ".string", "");
    ADDPROC(odbc_primary_keys, "odbc.odbc_primary_keys", "b", ".string", "table=.string");
    ADDPROC(odbc_execute_batch, "odbc.odbc_execute_batch", "b", ".int", "sql=.string,delimiter=.string");
    ADDPROC(odbc_get_info, "odbc.odbc_get_info", "b", ".string", "");
    ADDPROC(odbc_get_connection_attr, "odbc.odbc_get_connection_attr", "b", ".int", "attr=.int");
    ADDPROC(odbc_database, "odbc.odbc_database", "b", ".int", "newdb=.string");
    ADDPROC(show_message, "odbc.show_message", "b", ".int", "line1=.string,line2=.string,line3=.string,line4=.string");
#ifdef CREXX_ODBC_MOCK
    ADDPROC(odbc_mock_maximum_overlap, "odbc._mock_maximum_overlap", "b", ".int", "");
    ADDPROC(odbc_mock_reset_overlap, "odbc._mock_reset_overlap", "b", ".int", "");
#endif
ENDLOADFUNCS
