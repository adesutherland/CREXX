#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

#include "crexxpa.h"
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

// Global handles
static SQLHENV henv = NULL;
static SQLHDBC hdbc = NULL;
static SQLHSTMT hstmt = NULL;

void print_available_drivers() {
    SQLCHAR driver[256];
    SQLCHAR attr[256];
    SQLSMALLINT driver_ret;
    SQLSMALLINT attr_ret;
    SQLUSMALLINT direction = SQL_FETCH_FIRST;

    while(SQL_SUCCEEDED(SQLDrivers(henv, direction,
                                 driver, sizeof(driver), &driver_ret,
                                 attr, sizeof(attr), &attr_ret))) {
        direction = SQL_FETCH_NEXT;
        printf("Available Driver: %s\n", driver);
    }
}

SQLRETURN SQL_API connect_to_db(SQLHDBC hdbc, const char* driver, const char* dsn, const char* username, const char* password) {
    SQLRETURN ret;
    char connStr[1024];

    if (driver == NULL || strlen(driver) == 0) {
#ifdef _WIN32
        driver = "ODBC Driver 17 for SQL Server";
#elif defined(__APPLE__) || defined(__linux__)
        driver = "DuckDB";
#endif
    }

#ifdef _WIN32
    snprintf(connStr, sizeof(connStr),
            "Driver={%s};Database=%s;", driver, dsn);
#elif defined(__APPLE__) || defined(__linux__)
    snprintf(connStr, sizeof(connStr),
            "Driver=%s;Database=%s;", driver, dsn);
#endif

    ret = SQLDriverConnect(hdbc, NULL, (SQLCHAR*)connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    
    if (!SQL_SUCCEEDED(ret)) {
        SQLCHAR sqlstate[6], message[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER native_error;
        SQLSMALLINT length;
        SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, 1, sqlstate, &native_error, message, sizeof(message), &length);
        printf("ODBC Error: %s - %s\n", sqlstate, message);
        return SQL_ERROR;
    }
    return ret;
}

PROCEDURE(odbc_connect) {
    char *driver = GETSTRING(ARG0);
    char *dsn = GETSTRING(ARG1);
    char *user = GETSTRING(ARG2);
    char *password = GETSTRING(ARG3);

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv) != SQL_SUCCESS) {
        RETURNINTX(-1);
    }

    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        RETURNINTX(-2);
    }

    print_available_drivers();
    SQLRETURN ret = connect_to_db(hdbc, driver, dsn, user, password);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        RETURNINTX(-3);
    }

    RETURNINTX(0);
    ENDPROC
}

// Placeholder function definitions to ensure they exist
PROCEDURE(odbc_execute) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_fetch) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_disconnect) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_columns) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_get_column) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_column_name) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_column_type) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_begin_transaction) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_commit) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_rollback) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_error_message) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_get_diagnostics) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_column_info) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_tables) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_primary_keys) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_execute_batch) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_get_info) { RETURNSTRX(""); ENDPROC }
PROCEDURE(odbc_get_connection_attr) { RETURNINTX(0); ENDPROC }
PROCEDURE(odbc_move_to) { RETURNINTX(0); ENDPROC }

#ifdef _WIN32
PROCEDURE(show_message) {
    char *message = GETSTRING(ARG0);
    MessageBox(NULL, message, "Information", MB_OK | MB_ICONINFORMATION);
    RETURNINTX(0);
    ENDPROC
}
#else
PROCEDURE(show_message) {
    char *message = GETSTRING(ARG0);
    printf("Message: %s\n", message);
    RETURNINTX(0);
    ENDPROC
}
#endif

LOADFUNCS
    ADDPROC(odbc_connect, "odbc.odbc_connect", "b",".int","driver=.string,dsn=.string,user=.string,password=.string");
    ADDPROC(odbc_execute, "odbc.odbc_execute", "b",".int", "sql=.string");
    ADDPROC(odbc_fetch, "odbc.odbc_fetch", "b",".int", "row=.int");
    ADDPROC(odbc_disconnect, "odbc.odbc_disconnect", "b",".int", "");
    ADDPROC(odbc_columns, "odbc.odbc_columns", "b",".int", "");
    ADDPROC(odbc_get_column, "odbc.odbc_getcolumn", "b",".string", "column=.int");
    ADDPROC(odbc_column_name, "odbc.odbc_colname", "b",".string", "column=.int");
    ADDPROC(odbc_column_type, "odbc.odbc_coltype", "b",".int", "column=.int");
    ADDPROC(odbc_begin_transaction, "odbc.odbc_begin_transaction", "b",".int", "");
    ADDPROC(odbc_commit, "odbc.odbc_commit", "b",".int", "");
    ADDPROC(odbc_rollback, "odbc.odbc_rollback", "b",".int", "");
    ADDPROC(odbc_error_message, "odbc.odbc_error_message", "b",".string", "");
    ADDPROC(odbc_get_diagnostics, "odbc.odbc_get_diagnostics", "b",".string", "");
    ADDPROC(odbc_column_info, "odbc.odbc_column_info", "b",".string", "column=.int");
    ADDPROC(odbc_tables, "odbc.odbc_tables", "b",".string", "");
    ADDPROC(odbc_primary_keys, "odbc.odbc_primary_keys", "b",".string", "table=.string");
ENDLOADFUNCS
