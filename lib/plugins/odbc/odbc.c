#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

#include "crexxpa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>

// Global handles
static SQLHENV henv = NULL;
static SQLHDBC hdbc = NULL;
static SQLHSTMT hstmt = NULL;


PROCEDURE(odbc_connect) {
    char *dsn = GETSTRING(ARG0);
    char *user = GETSTRING(ARG1);
    char *password = GETSTRING(ARG2);

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv) != SQL_SUCCESS) RETURNINTX(-1);
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0);

    if (SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        RETURNINTX(-2);
    }
    
#ifdef _WIN32
       // Check if this is a CSV connection (empty username and password)
    if (username[0] == '\0' && password[0] == '\0') {
        char connStrOut[1024];
        SQLSMALLINT connStrOutLen;

        // Use the exact driver name we found
        snprintf(connStr, sizeof(connStr),
                "Driver={Microsoft Access Text Driver (*.txt, *.csv)};"
                "DefaultDir=%s;"
                "Extensions=CSV;"
                "Format=Delimited(;);"
                "CharacterSet=ANSI;"
                "ReadOnly=True;"
                "IMEX=1;"
                "FirstRowHasNames=0;"
                "MaxScanRows=0;"
                "TextDelimiter=;;"
                "ColumnDelimiter=;;"
                "Separators=;",
                dsn);

        ret = SQLDriverConnect(hdbc,
                             NULL,
                             (SQLCHAR*)connStr,
                             SQL_NTS,
                             (SQLCHAR*)connStrOut,
                             sizeof(connStrOut),
                             &connStrOutLen,
                             SQL_DRIVER_NOPROMPT);

        if (!SQL_SUCCEEDED(ret)) {
            // If first attempt fails, try with Excel driver as backup
            snprintf(connStr, sizeof(connStr),
                    "Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};"
                    "DefaultDir=%s;"
                    "Extensions=CSV;"
                    "FirstRowHasNames=0;"
                    "Format=Delimited;"
                    "ColNameHeader=True;"
                    "Delimited=;;"
                    "CharacterSet=ANSI;",
                    dsn);

            ret = SQLDriverConnect(hdbc,
                                 NULL,
                                 (SQLCHAR*)connStr,
                                 SQL_NTS,
                                 (SQLCHAR*)connStrOut,
                                 sizeof(connStrOut),
                                 &connStrOutLen,
                                 SQL_DRIVER_NOPROMPT);
        }
    } else {
        // Regular database connection
        ret = SQLConnect(hdbc,
                        (SQLCHAR*)dsn, SQL_NTS,
                        (SQLCHAR*)username, SQL_NTS,
                        (SQLCHAR*)password, SQL_NTS);
    }
#endif
    
    SQLRETURN ret = SQLConnect(hdbc, (SQLCHAR *)dsn, SQL_NTS, (SQLCHAR *)user, SQL_NTS, (SQLCHAR *)password, SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
      SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
      SQLFreeHandle(SQL_HANDLE_ENV, henv);
      RETURNINTX(-3);
    }

    RETURNINTX(0);
    ENDPROC
}

// Disconnect
PROCEDURE(odbc_disconnect) {
    if (hstmt) SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc) { SQLDisconnect(hdbc); SQLFreeHandle(SQL_HANDLE_DBC, hdbc); }
    if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);
    hstmt = hdbc = henv = NULL;
    RETURNINTX(0);
    ENDPROC
}

// Execute statement
PROCEDURE(odbc_execute) {
    char *sql = GETSTRING(ARG0);
    if (SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt) != SQL_SUCCESS) RETURNINTX(-1);
    SQLRETURN ret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -2);
    ENDPROC
}

// Fetch row
PROCEDURE(odbc_fetch) {
    SQLRETURN ret = SQLFetch(hstmt);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Get number of columns
PROCEDURE(odbc_columns) {
    SQLSMALLINT columns;
    if (SQLNumResultCols(hstmt, &columns) != SQL_SUCCESS) RETURNINTX(-1);
    RETURNINTX(columns);
    ENDPROC
}

// Get column data
PROCEDURE(odbc_get_column) {
    int column = GETINT(ARG0);
    char buffer[1024];
    SQLLEN indicator;
    SQLRETURN ret = SQLGetData(hstmt, column, SQL_C_CHAR, buffer, sizeof(buffer), &indicator);
    RETURNSTRX(SQL_SUCCEEDED(ret) ? buffer : "");
    ENDPROC
}

// Get column name
PROCEDURE(odbc_column_name) {
    int column = GETINT(ARG0);
    SQLCHAR colName[256];
    SQLSMALLINT colNameLen;
    SQLRETURN ret = SQLColAttribute(hstmt, column, SQL_DESC_NAME, colName, sizeof(colName), &colNameLen, NULL);
    RETURNSTRX(SQL_SUCCEEDED(ret) ? (char*)colName : "");
    ENDPROC
}

// Get column type
PROCEDURE(odbc_column_type) {
    int column = GETINT(ARG0);
    SQLLEN dataType;
    SQLRETURN ret = SQLColAttribute(hstmt, column, SQL_DESC_TYPE, NULL, 0, NULL, &dataType);
    RETURNINTX(SQL_SUCCEEDED(ret) ? (int)dataType : -1);
    ENDPROC
}

// Begin transaction
PROCEDURE(odbc_begin_transaction) {
    SQLRETURN ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Commit transaction
PROCEDURE(odbc_commit) {
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Rollback transaction
PROCEDURE(odbc_rollback) {
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Get error message
PROCEDURE(odbc_error_message) {
    SQLCHAR sqlstate[6];
    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native_error;
    SQLSMALLINT length;
    SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message, sizeof(message), &length);
    RETURNSTRX((char*)message);
    ENDPROC
}

// Get diagnostics
PROCEDURE(odbc_get_diagnostics) {
    SQLCHAR sqlstate[6];
    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native_error;
    SQLSMALLINT length;
    SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message, sizeof(message), &length);
    RETURNSTRX((char*)message);
    ENDPROC
}

// Get table list
PROCEDURE(odbc_tables) {
    SQLRETURN ret = SQLTables(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Get primary keys
PROCEDURE(odbc_primary_keys) {
    char *table = GETSTRING(ARG0);
    SQLRETURN ret = SQLPrimaryKeys(hstmt, NULL, 0, NULL, 0, (SQLCHAR*)table, SQL_NTS);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Execute batch queries
PROCEDURE(odbc_execute_batch) {
    char *sql = GETSTRING(ARG0);
    char *delimiter = GETSTRING(ARG1);
    SQLRETURN ret;
    char *token = strtok(sql, delimiter);
    while (token) {
        ret = SQLExecDirect(hstmt, (SQLCHAR*)token, SQL_NTS);
        if (!SQL_SUCCEEDED(ret)) RETURNINTX(-1);
        token = strtok(NULL, delimiter);
    }
    RETURNINTX(0);
    ENDPROC
}

// Get database info
PROCEDURE(odbc_get_info) {
    char info[256];
    SQLGetInfo(hdbc, SQL_DBMS_NAME, info, sizeof(info), NULL);
    RETURNSTRX(info);
    ENDPROC
}

// Get row count
PROCEDURE(odbc_row_count) {
    SQLLEN rowCount;
    SQLRETURN ret = SQLRowCount(hstmt, &rowCount);
    RETURNINTX(SQL_SUCCEEDED(ret) ? rowCount : -1);
    ENDPROC
}

// Get database name
PROCEDURE(odbc_database) {
    char *newdb = GETSTRING(ARG0);
    SQLRETURN ret = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, newdb, SQL_NTS);
    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

// Display message
PROCEDURE(show_message) {
    char *line1 = GETSTRING(ARG0);
    char *line2 = GETSTRING(ARG1);
    char *line3 = GETSTRING(ARG2);
    char *line4 = GETSTRING(ARG3);
    printf("%s\n%s\n%s\n%s\n", line1, line2, line3, line4);
    RETURNINTX(0);
    ENDPROC
}


PROCEDURE(odbc_column_info) {
    int column = GETINT(ARG0);
    SQLCHAR colName[256];
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLULEN colSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;

    SQLRETURN ret = SQLDescribeCol(hstmt, column, colName, sizeof(colName), &colNameLen, &colType, &colSize, &decimalDigits, &nullable);

    if (!SQL_SUCCEEDED(ret)) {
        RETURNSTRX("");
    }

    char result[512];
    snprintf(result, sizeof(result), "%s,%d,%lu,%d,%d", colName, colType, colSize, decimalDigits, nullable);
    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(odbc_get_connection_attr) {
    int attr = GETINT(ARG0);
    SQLINTEGER value;
    SQLRETURN ret = SQLGetConnectAttr(hdbc, attr, &value, 0, NULL);

    RETURNINTX(SQL_SUCCEEDED(ret) ? value : -1);
    ENDPROC
}

PROCEDURE(odbc_move_to) {
    int row = GETINT(ARG0);
    SQLRETURN ret = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, row);

    RETURNINTX(SQL_SUCCEEDED(ret) ? 0 : -1);
    ENDPROC
}

LOADFUNCS
    // Register REXX procedures
    ADDPROC(odbc_connect, "odbc.odbc_connect", "b",".int","dsn=.string,user=.string,password=.string");
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
//    ADDPROC(odbc_prepare, "odbc.odbc_prepare", "b",".int", "sql=.string");
//    ADDPROC(odbc_bind_param, "odbc.odbc_bind_param", "b",".int", "paramNum=.int,value=.string");
    ADDPROC(odbc_error_message, "odbc.odbc_error_message", "b",".string", "");
    ADDPROC(odbc_get_diagnostics, "odbc.odbc_get_diagnostics", "b",".string", "");
    ADDPROC(odbc_column_info, "odbc.odbc_column_info", "b",".string", "column=.int");
    ADDPROC(odbc_tables, "odbc.odbc_tables", "b",".string", "");
    ADDPROC(odbc_primary_keys, "odbc.odbc_primary_keys", "b",".string", "table=.string");
    ADDPROC(odbc_execute_batch, "odbc.odbc_execute_batch", "b",".int", "sql=.string,delimiter=.string");
    ADDPROC(odbc_get_info, "odbc.odbc_get_info", "b",".string", "");
    ADDPROC(odbc_get_connection_attr, "odbc.odbc_get_connection_attr", "b",".int", "attr=.int");
    ADDPROC(odbc_move_to, "odbc.odbc_move_to", "b",".int", "row=.int");
    ADDPROC(odbc_row_count, "odbc.odbc_row_count", "b",".int", "");
    ADDPROC(odbc_database, "odbc.odbc_database", "b",".string", "newdb=.string");
    ADDPROC(show_message, "odbc.show_message", "b",".string", "line1=.string,line2=.string,line3=.string,line4=.string,");
ENDLOADFUNCS
