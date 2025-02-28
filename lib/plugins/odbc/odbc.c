#include <windows.h>
#include "crexxpa.h"
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

// Global handles
static SQLHENV henv = NULL;  // Environment handle
static SQLHDBC hdbc = NULL;  // Connection handle
static SQLHSTMT hstmt = NULL; // Statement handle

int words(const char *str) {
    int count = 0;
    char *token = strtok(str, ";");
    while (token) {
        count++;
        token = strtok(NULL, ";");
    }
    return count;
}

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

SQLRETURN SQL_API connect_to_db(SQLHDBC hdbc, const char* dsn, const char* username, const char* password) {
    SQLRETURN ret;
    char connStr[1024];
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

    if (!SQL_SUCCEEDED(ret)) {
        // Handle error - let's add more detailed error reporting
        SQLCHAR sqlstate[6];
        SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER native_error;
        SQLSMALLINT length;

        SQLGetDiagRec(SQL_HANDLE_DBC,
                     hdbc,
                     1,
                     sqlstate,
                     &native_error,
                     message,
                     sizeof(message),
                     &length);

        printf("ODBC Error: %s - %s\n", sqlstate, message);
        printf("Connection String: %s\n", connStr);  // Add this to see the actual connection string
        return SQL_ERROR;
    }

    return ret;
}

PROCEDURE(odbc_connect) {
    char *dsn = GETSTRING(ARG0);
    char *user = GETSTRING(ARG1);
    char *password = GETSTRING(ARG2);

    // Allocate environment handle
    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv) != SQL_SUCCESS) {
        RETURNINTX(-1);
    }

    // Set ODBC version
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    // Allocate connection handle
    if (SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        RETURNINTX(-2);
    }

    // Print available drivers
    print_available_drivers();

    // Connect to data source
    SQLRETURN ret = connect_to_db(hdbc, dsn, user, password);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        RETURNINTX(-3);
    }

    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(odbc_execute)
{
    char *sql = GETSTRING(ARG0);
    SQLCHAR sqlstate[6];
    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native_error;
    SQLSMALLINT length;

    // Allocate statement handle
    if (SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt) != SQL_SUCCESS) {
        RETURNINTX(-1);
    }
// Execute the SQL statement
    SQLRETURN ret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        // Get error details
        SQLGetDiagRec(SQL_HANDLE_STMT,
                     hstmt,
                     1,
                     sqlstate,
                     &native_error,
                     message,
                     sizeof(message),
                     &length);

        printf("SQL Error: %s - %s\n", sqlstate, message);
        printf("SQL Query: %s\n", sql);

        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        RETURNINTX(-2);
    }

    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(odbc_fetch)
{
    SQLRETURN ret;

    ret = SQLFetch(hstmt);
    if (ret == SQL_NO_DATA) {
        RETURNSTRX("-1");
    }
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNSTRX("-2");
    }

    RETURNSTRX("0");  // Just return success, don't try to get column data here
    ENDPROC
}

PROCEDURE(odbc_disconnect)
{
    if (hstmt) SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    if (hdbc) {
        SQLDisconnect(hdbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    if (henv) SQLFreeHandle(SQL_HANDLE_ENV, henv);

    hstmt = NULL;
    hdbc = NULL;
    henv = NULL;

    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(odbc_columns)
{
    SQLSMALLINT columns;
    SQLCHAR colName[256];
    SQLSMALLINT colNameLen;
    SQLSMALLINT dataType;
    SQLULEN colSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;

    if (SQLNumResultCols(hstmt, &columns) != SQL_SUCCESS) {
        RETURNINTX(-1);
    }

    // Print column information for debugging
    for (SQLSMALLINT i = 1; i <= columns; i++) {
        SQLDescribeCol(hstmt, i, colName, sizeof(colName), &colNameLen,
                      &dataType, &colSize, &decimalDigits, &nullable);
        if(i==1) {
           printf("Table has %d Columns\n", columns);
           printf("--------------------\n");
        }
        printf("Column %d: Name=%s, Type=%d, Size=%lu foundCols=%d\n",
               i, colName, dataType, (unsigned long)colSize,columns);
    }
    printf("--------------------\n");
    RETURNINTX(columns);
    ENDPROC
}

PROCEDURE(odbc_get_column)
{
    int column = GETINT(ARG0);
    char buffer[1024];
    SQLLEN indicator;
    SQLRETURN ret = SQLGetData(hstmt, column, SQL_C_CHAR,
                               buffer, sizeof(buffer), &indicator);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNSTRX("-1");
    }
    RETURNSTRX(buffer);
ENDPROC
}
LOADFUNCS
    // Register REXX procedures
    ADDPROC(odbc_connect, "odbc.odbc_connect", "b",".int","dsn=.string,user=.string,password=.string");
    ADDPROC(odbc_execute, "odbc.odbc_execute", "b",".int", "sql=.string");
    ADDPROC(odbc_fetch, "odbc.odbc_fetch", "b",".string", "");
    ADDPROC(odbc_disconnect, "odbc.odbc_disconnect", "b",".int", "");
    ADDPROC(odbc_columns, "odbc.odbc_columns", "b",".int", "");
    ADDPROC(odbc_get_column, "odbc.odbc_getcolumn", "b",".string", "column=.int");
ENDLOADFUNCS