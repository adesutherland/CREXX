#include "mock_odbc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static SRWLOCK mock_lock = SRWLOCK_INIT;
static CONDITION_VARIABLE mock_condition = CONDITION_VARIABLE_INIT;
#define MOCK_LOCK() AcquireSRWLockExclusive(&mock_lock)
#define MOCK_UNLOCK() ReleaseSRWLockExclusive(&mock_lock)
#define MOCK_WAIT() SleepConditionVariableSRW( \
        &mock_condition, &mock_lock, INFINITE, 0)
#define MOCK_BROADCAST() WakeAllConditionVariable(&mock_condition)
#else
#include <pthread.h>
static pthread_mutex_t mock_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mock_condition = PTHREAD_COND_INITIALIZER;
#define MOCK_LOCK() ((void)pthread_mutex_lock(&mock_lock))
#define MOCK_UNLOCK() ((void)pthread_mutex_unlock(&mock_lock))
#define MOCK_WAIT() ((void)pthread_cond_wait(&mock_condition, &mock_lock))
#define MOCK_BROADCAST() ((void)pthread_cond_broadcast(&mock_condition))
#endif

typedef struct mock_odbc_handle {
    SQLSMALLINT type;
    int connected;
    int row;
    int mode;
    unsigned int bound_parameters;
    SQLPOINTER parameter_values[8];
    SQLLEN *parameter_indicators[8];
    SQLSMALLINT parameter_value_types[8];
    char sql[256];
} mock_odbc_handle;

static int mock_live_handles;
static int mock_active_executes;
static int mock_maximum_execute_overlap;
static int mock_overlap_barrier_enabled;

static mock_odbc_handle *mock_handle(SQLHANDLE handle,
                                     SQLSMALLINT expected_type) {
    mock_odbc_handle *item = (mock_odbc_handle *)handle;
    if (!item || item->type != expected_type) return NULL;
    return item;
}

static void mock_copy_text(SQLPOINTER target, SQLLEN target_size,
                           const char *text, SQLLEN *indicator) {
    size_t length = strlen(text);
    if (indicator) *indicator = (SQLLEN)length;
    if (target && target_size > 0) {
        size_t copy = length < (size_t)target_size - 1u
                ? length : (size_t)target_size - 1u;
        memcpy(target, text, copy);
        ((char *)target)[copy] = 0;
    }
}

SQLRETURN SQLAllocHandle(SQLSMALLINT handle_type, SQLHANDLE input,
                         SQLHANDLE *output) {
    mock_odbc_handle *item;
    if (!output || (handle_type != SQL_HANDLE_ENV && !input)) return SQL_ERROR;
    item = calloc(1u, sizeof(*item));
    if (!item) return SQL_ERROR;
    item->type = handle_type;
    *output = item;
    MOCK_LOCK();
    mock_live_handles++;
    MOCK_UNLOCK();
    return SQL_SUCCESS;
}

SQLRETURN SQLFreeHandle(SQLSMALLINT handle_type, SQLHANDLE handle) {
    mock_odbc_handle *item = mock_handle(handle, handle_type);
    if (!item) return SQL_ERROR;
    free(item);
    MOCK_LOCK();
    mock_live_handles--;
    MOCK_UNLOCK();
    return SQL_SUCCESS;
}

SQLRETURN SQLSetEnvAttr(SQLHENV environment, SQLINTEGER attribute,
                        SQLPOINTER value, SQLINTEGER length) {
    (void)attribute; (void)value; (void)length;
    return mock_handle(environment, SQL_HANDLE_ENV) ? SQL_SUCCESS : SQL_ERROR;
}

SQLRETURN SQLConnect(SQLHDBC connection, SQLCHAR *dsn, SQLSMALLINT dsn_length,
                     SQLCHAR *user, SQLSMALLINT user_length,
                     SQLCHAR *password, SQLSMALLINT password_length) {
    mock_odbc_handle *item = mock_handle(connection, SQL_HANDLE_DBC);
    (void)dsn_length; (void)user; (void)user_length;
    (void)password; (void)password_length;
    if (!item || !dsn || strcmp((char *)dsn, "fail") == 0) return SQL_ERROR;
    item->connected = 1;
    return SQL_SUCCESS;
}

SQLRETURN SQLDisconnect(SQLHDBC connection) {
    mock_odbc_handle *item = mock_handle(connection, SQL_HANDLE_DBC);
    if (!item) return SQL_ERROR;
    item->connected = 0;
    return SQL_SUCCESS;
}

static SQLRETURN mock_set_sql(SQLHSTMT statement, SQLCHAR *sql,
                              int prepared) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    if (!item || !sql || strstr((char *)sql, "FAIL")) return SQL_ERROR;
    snprintf(item->sql, sizeof(item->sql), "%s", (char *)sql);
    item->mode = prepared ? 3 : 0;
    item->row = 0;
    return SQL_SUCCESS;
}

SQLRETURN SQLExecDirect(SQLHSTMT statement, SQLCHAR *sql,
                        SQLINTEGER length) {
    (void)length;
    return mock_set_sql(statement, sql, 0);
}

SQLRETURN SQLPrepare(SQLHSTMT statement, SQLCHAR *sql, SQLINTEGER length) {
    (void)length;
    return mock_set_sql(statement, sql, 1);
}

SQLRETURN SQLBindParameter(SQLHSTMT statement, SQLUSMALLINT parameter,
                           SQLSMALLINT input_output_type,
                           SQLSMALLINT value_type,
                           SQLSMALLINT parameter_type,
                           SQLULEN column_size, SQLSMALLINT decimal_digits,
                           SQLPOINTER value, SQLLEN buffer_length,
                           SQLLEN *indicator) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    (void)input_output_type; (void)value_type; (void)parameter_type;
    (void)column_size; (void)decimal_digits; (void)buffer_length;
    if (!item || !parameter || parameter > 8u) return SQL_ERROR;
    if (value_type == SQL_C_CHAR && value &&
        strcmp((const char *)value, "FAIL bind") == 0) return SQL_ERROR;
    item->parameter_values[parameter - 1u] = value;
    item->parameter_indicators[parameter - 1u] = indicator;
    item->parameter_value_types[parameter - 1u] = value_type;
    if (parameter > item->bound_parameters) item->bound_parameters = parameter;
    return SQL_SUCCESS;
}

SQLRETURN SQLExecute(SQLHSTMT statement) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    unsigned int parameter;
    if (!item || !item->sql[0]) return SQL_ERROR;
    for (parameter = 0u; parameter < item->bound_parameters; parameter++) {
        if (item->parameter_value_types[parameter] == SQL_C_CHAR &&
            item->parameter_values[parameter] &&
            (!item->parameter_indicators[parameter] ||
             *item->parameter_indicators[parameter] != SQL_NULL_DATA)) {
            volatile size_t bound_length = strlen(
                    (const char *)item->parameter_values[parameter]);
            (void)bound_length;
        }
    }
    item->row = 0;
    MOCK_LOCK();
    mock_active_executes++;
    if (mock_active_executes > mock_maximum_execute_overlap) {
        mock_maximum_execute_overlap = mock_active_executes;
    }
    MOCK_BROADCAST();
    while (mock_overlap_barrier_enabled &&
           mock_maximum_execute_overlap < 2) {
        MOCK_WAIT();
    }
    mock_active_executes--;
    MOCK_BROADCAST();
    MOCK_UNLOCK();
    return SQL_SUCCESS;
}

SQLRETURN SQLFreeStmt(SQLHSTMT statement, SQLUSMALLINT option) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    if (!item) return SQL_ERROR;
    if (option == SQL_CLOSE) item->row = 0;
    if (option == SQL_RESET_PARAMS) {
        item->bound_parameters = 0;
        memset(item->parameter_values, 0, sizeof(item->parameter_values));
        memset(item->parameter_indicators, 0,
               sizeof(item->parameter_indicators));
        memset(item->parameter_value_types, 0,
               sizeof(item->parameter_value_types));
    }
    return SQL_SUCCESS;
}

SQLRETURN SQLFetch(SQLHSTMT statement) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    if (!item) return SQL_ERROR;
    if (item->row++) return SQL_NO_DATA;
    return SQL_SUCCESS;
}

SQLRETURN SQLFetchScroll(SQLHSTMT statement, SQLSMALLINT orientation,
                         SQLLEN offset) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    (void)orientation;
    if (!item || offset < 1) return SQL_ERROR;
    item->row = (int)offset;
    return SQL_SUCCESS;
}

SQLRETURN SQLNumResultCols(SQLHSTMT statement, SQLSMALLINT *columns) {
    if (!mock_handle(statement, SQL_HANDLE_STMT) || !columns) return SQL_ERROR;
    *columns = 2;
    return SQL_SUCCESS;
}

SQLRETURN SQLGetData(SQLHSTMT statement, SQLUSMALLINT column,
                     SQLSMALLINT target_type, SQLPOINTER buffer,
                     SQLLEN buffer_length, SQLLEN *indicator) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    const char *text;
    (void)target_type;
    if (!item || !column) return SQL_ERROR;
    if (item->mode == 1) text = "mock_table";
    else if (item->mode == 2) text = "mock_id";
    else text = column == 1 ? "row-value" : "42";
    mock_copy_text(buffer, buffer_length, text, indicator);
    return SQL_SUCCESS;
}

SQLRETURN SQLColAttribute(SQLHSTMT statement, SQLUSMALLINT column,
                          SQLUSMALLINT field, SQLPOINTER character,
                          SQLSMALLINT buffer_length,
                          SQLSMALLINT *string_length, SQLLEN *numeric) {
    const char *name = column == 1 ? "value" : "number";
    if (!mock_handle(statement, SQL_HANDLE_STMT)) return SQL_ERROR;
    if (field == SQL_DESC_NAME) {
        SQLLEN length = 0;
        mock_copy_text(character, buffer_length, name, &length);
        if (string_length) *string_length = (SQLSMALLINT)length;
    } else if (field == SQL_DESC_TYPE && numeric) {
        *numeric = column == 1 ? SQL_VARCHAR : SQL_BIGINT;
    }
    return SQL_SUCCESS;
}

SQLRETURN SQLDescribeCol(SQLHSTMT statement, SQLUSMALLINT column,
                         SQLCHAR *name, SQLSMALLINT buffer_length,
                         SQLSMALLINT *name_length, SQLSMALLINT *type,
                         SQLULEN *size, SQLSMALLINT *digits,
                         SQLSMALLINT *nullable) {
    SQLLEN length = 0;
    if (!mock_handle(statement, SQL_HANDLE_STMT)) return SQL_ERROR;
    mock_copy_text(name, buffer_length, column == 1 ? "value" : "number",
                   &length);
    if (name_length) *name_length = (SQLSMALLINT)length;
    if (type) *type = column == 1 ? SQL_VARCHAR : SQL_BIGINT;
    if (size) *size = 255;
    if (digits) *digits = 0;
    if (nullable) *nullable = 1;
    return SQL_SUCCESS;
}

SQLRETURN SQLSetConnectAttr(SQLHDBC connection, SQLINTEGER attribute,
                            SQLPOINTER value, SQLINTEGER length) {
    (void)attribute; (void)value; (void)length;
    return mock_handle(connection, SQL_HANDLE_DBC) ? SQL_SUCCESS : SQL_ERROR;
}

SQLRETURN SQLGetConnectAttr(SQLHDBC connection, SQLINTEGER attribute,
                            SQLPOINTER value, SQLINTEGER buffer_length,
                            SQLINTEGER *string_length) {
    (void)attribute; (void)buffer_length; (void)string_length;
    if (!mock_handle(connection, SQL_HANDLE_DBC) || !value) return SQL_ERROR;
    *(SQLINTEGER *)value = 1;
    return SQL_SUCCESS;
}

SQLRETURN SQLEndTran(SQLSMALLINT handle_type, SQLHANDLE handle,
                     SQLSMALLINT completion) {
    (void)completion;
    return mock_handle(handle, handle_type) ? SQL_SUCCESS : SQL_ERROR;
}

SQLRETURN SQLGetDiagRec(SQLSMALLINT handle_type, SQLHANDLE handle,
                        SQLSMALLINT record, SQLCHAR *state,
                        SQLINTEGER *native_error, SQLCHAR *message,
                        SQLSMALLINT buffer_length, SQLSMALLINT *length) {
    SQLLEN message_length = 0;
    (void)record;
    if (!mock_handle(handle, handle_type)) return SQL_ERROR;
    if (state) memcpy(state, "00000", 6u);
    if (native_error) *native_error = 0;
    mock_copy_text(message, buffer_length, "mock diagnostic",
                   &message_length);
    if (length) *length = (SQLSMALLINT)message_length;
    return SQL_SUCCESS;
}

SQLRETURN SQLTables(SQLHSTMT statement, SQLCHAR *catalogue,
                    SQLSMALLINT catalogue_length, SQLCHAR *schema,
                    SQLSMALLINT schema_length, SQLCHAR *table,
                    SQLSMALLINT table_length, SQLCHAR *type,
                    SQLSMALLINT type_length) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    (void)catalogue; (void)catalogue_length; (void)schema;
    (void)schema_length; (void)table; (void)table_length;
    (void)type; (void)type_length;
    if (!item) return SQL_ERROR;
    item->mode = 1;
    item->row = 0;
    return SQL_SUCCESS;
}

SQLRETURN SQLPrimaryKeys(SQLHSTMT statement, SQLCHAR *catalogue,
                         SQLSMALLINT catalogue_length, SQLCHAR *schema,
                         SQLSMALLINT schema_length, SQLCHAR *table,
                         SQLSMALLINT table_length) {
    mock_odbc_handle *item = mock_handle(statement, SQL_HANDLE_STMT);
    (void)catalogue; (void)catalogue_length; (void)schema;
    (void)schema_length; (void)table; (void)table_length;
    if (!item) return SQL_ERROR;
    item->mode = 2;
    item->row = 0;
    return SQL_SUCCESS;
}

SQLRETURN SQLGetInfo(SQLHDBC connection, SQLUSMALLINT info_type,
                     SQLPOINTER value, SQLSMALLINT buffer_length,
                     SQLSMALLINT *string_length) {
    SQLLEN length = 0;
    (void)info_type;
    if (!mock_handle(connection, SQL_HANDLE_DBC)) return SQL_ERROR;
    mock_copy_text(value, buffer_length, "CREXX mock ODBC", &length);
    if (string_length) *string_length = (SQLSMALLINT)length;
    return SQL_SUCCESS;
}

SQLRETURN SQLRowCount(SQLHSTMT statement, SQLLEN *count) {
    if (!mock_handle(statement, SQL_HANDLE_STMT) || !count) return SQL_ERROR;
    *count = 1;
    return SQL_SUCCESS;
}

int crexx_mock_odbc_maximum_execute_overlap(void) {
    int maximum;
    MOCK_LOCK();
    maximum = mock_maximum_execute_overlap;
    mock_overlap_barrier_enabled = 0;
    MOCK_BROADCAST();
    MOCK_UNLOCK();
    return maximum;
}

void crexx_mock_odbc_reset_execute_overlap(void) {
    MOCK_LOCK();
    mock_active_executes = 0;
    mock_maximum_execute_overlap = 0;
    mock_overlap_barrier_enabled = 1;
    MOCK_UNLOCK();
}

#if !defined(_WIN32)
static void mock_odbc_validate_unload(void) __attribute__((destructor));
static void mock_odbc_validate_unload(void) {
    int live;
    MOCK_LOCK();
    live = mock_live_handles;
    MOCK_UNLOCK();
    if (live != 0) {
        fprintf(stderr, "mock ODBC unloaded with %d live handle(s)\n",
                live);
        abort();
    }
}
#endif
