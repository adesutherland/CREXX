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

    // Allocate statement handle with scrollable cursor
    if (SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt) != SQL_SUCCESS) {
        RETURNINTX(-1);
    }

    // Set the statement to use a scrollable cursor
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0);

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

// Fetch the next row or a specific row if a number is provided
PROCEDURE(odbc_fetch)
{
    SQLRETURN ret;
    SQLLEN row = GETINT(ARG0);  // Get the row number from the argument (0 for next)

    if (row == 0) {
        // Fetch the next row
        ret = SQLFetch(hstmt);
    } else {
        // Fetch a specific row
        ret = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, row);
    }

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(0);  // Success
    } else if (ret == SQL_NO_DATA) {
        RETURNINTX(-1);  // No more rows
    } else {
        // Fetch error, provide more details
        SQLCHAR sqlstate[6];
        SQLINTEGER native_error;
        SQLCHAR message[256];
        SQLSMALLINT length;

        SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &native_error, message, sizeof(message), &length);
        printf("SQL Error: %s - %s\n", sqlstate, message);
        RETURNINTX(-2);  // Error during fetch
    }
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

// Get column name by index
PROCEDURE(odbc_column_name)
{
    int column = GETINT(ARG0);
    SQLCHAR colName[256];
    SQLSMALLINT colNameLen;
    
    SQLRETURN ret = SQLColAttribute(hstmt, column, SQL_DESC_NAME,
                                  colName, sizeof(colName), &colNameLen, NULL);
    
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNSTRX("");
    }
    RETURNSTRX((char*)colName);
    ENDPROC
}

// Get column type by index
PROCEDURE(odbc_column_type)
{
    int column = GETINT(ARG0);
    SQLLEN dataType;
    
    SQLRETURN ret = SQLColAttribute(hstmt, column, SQL_DESC_TYPE,
                                  NULL, 0, NULL, &dataType);
    
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(-1);
    }
    RETURNINTX((int)dataType);
    ENDPROC
}

// Begin transaction
PROCEDURE(odbc_begin_transaction)
{
    SQLRETURN ret = SQLSetConnectAttr(hdbc, SQL_ATTR_AUTOCOMMIT, 
                                     (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
    ENDPROC
}

// Commit transaction
PROCEDURE(odbc_commit)
{
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
    ENDPROC
}

// Rollback transaction
PROCEDURE(odbc_rollback)
{
    SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_ROLLBACK);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(odbc_prepare)
{
    char *sql = GETSTRING(ARG0);
    
    if (SQLPrepare(hstmt, (SQLCHAR*)sql, SQL_NTS) != SQL_SUCCESS) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(odbc_bind_param)
{
    int paramNum = GETINT(ARG0);
    char *value = GETSTRING(ARG1);
    
    SQLRETURN ret = SQLBindParameter(hstmt, paramNum, SQL_PARAM_INPUT,
                                   SQL_C_CHAR, SQL_VARCHAR,
                                   strlen(value), 0, value, 0, NULL);
    
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
    ENDPROC
}

PROCEDURE(odbc_error_message)
{
    SQLCHAR sqlstate[6];
    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native_error;
    SQLSMALLINT length;
    
    SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                  &native_error, message, sizeof(message), &length);
    
    RETURNSTRX((char*)message);
    ENDPROC
}

PROCEDURE(odbc_get_diagnostics)
{
    SQLCHAR sqlstate[6];
    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER native_error;
    SQLSMALLINT length;
    char result[SQL_MAX_MESSAGE_LENGTH + 100];
    
    SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                  &native_error, message, sizeof(message), &length);
    
    snprintf(result, sizeof(result), 
             "SQLSTATE: %s\nNative Error: %d\nMessage: %s",
             sqlstate, (int)native_error, message);
    
    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(odbc_column_info)
{
    int column = GETINT(ARG0);
    SQLCHAR colName[256];
    SQLSMALLINT colNameLen;
    SQLSMALLINT dataType;
    SQLULEN colSize;
    SQLSMALLINT decimalDigits;
    SQLSMALLINT nullable;
    char result[1024];

    SQLRETURN ret = SQLDescribeCol(hstmt, column, colName, sizeof(colName), 
                                 &colNameLen, &dataType, &colSize,
                                 &decimalDigits, &nullable);
    
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNSTRX("-1");
    }

    snprintf(result, sizeof(result),
             "Name=%s;Type=%d;Size=%lu;Decimals=%d;Nullable=%d",
             colName, dataType, (unsigned long)colSize, 
             decimalDigits, nullable);
    
    RETURNSTRX(result);
    ENDPROC
}

// Get table list
PROCEDURE(odbc_tables)
{
    SQLCHAR table[256];
    SQLLEN length;
    char result[4096] = "";
    SQLRETURN ret;

    // Free the existing statement handle if any
    if (hstmt != NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
    }

    // Allocate a new statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNSTRX("Failed to allocate statement handle");
    }

    // Get the current database name
    ret = SQLExecDirect(hstmt, (SQLCHAR*)"SELECT DATABASE()", SQL_NTS);
    if (SQL_SUCCEEDED(ret) && SQL_SUCCEEDED(SQLFetch(hstmt))) {
        SQLCHAR dbname[256];
        SQLGetData(hstmt, 1, SQL_C_CHAR, dbname, sizeof(dbname), &length);
        printf("Current database: %s\n", dbname);
        
        // If no database selected, return an error
        if (strlen((char*)dbname) == 0) {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            RETURNSTRX("No database selected");
        }
    }

    // Free statement and create new one for table list
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // Try MySQL-specific SHOW TABLES
    ret = SQLExecDirect(hstmt, (SQLCHAR*)"SHOW TABLES", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        RETURNSTRX("Error executing table query");
    }

    // Get table names from result set
    while (SQL_SUCCEEDED(SQLFetch(hstmt))) {
        ret = SQLGetData(hstmt, 1, SQL_C_CHAR, table, sizeof(table), &length);
        if (SQL_SUCCEEDED(ret)) {
            if (result[0] != '\0') {
                strcat(result, " ");
            }
            strcat(result, (char*)table);
        }
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    hstmt = NULL;

    if (result[0] == '\0') {
        RETURNSTRX("No tables found in database");
    }

    RETURNSTRX(result);
    ENDPROC
}

// Get primary keys
PROCEDURE(odbc_primary_keys)
{
    char *table = GETSTRING(ARG0);
    SQLCHAR column[256];
    SQLLEN length;
    char result[4096] = "";
    SQLRETURN ret;

    // Free existing statement handle if any
    if (hstmt != NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
    }
    
    // Allocate new statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!SQL_SUCCEEDED(ret)) {
        RETURNSTRX("Error allocating statement handle");
    }
    
    // Get primary keys
    ret = SQLPrimaryKeys(hstmt, NULL, 0, NULL, 0, (SQLCHAR*)table, SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
        RETURNSTRX("Error getting primary keys");
    }
    
    // Fetch primary key columns
    while (SQLFetch(hstmt) == SQL_SUCCESS) {
        SQLGetData(hstmt, 4, SQL_C_CHAR, column, sizeof(column), &length);
        if (result[0] != '\0') {
            strcat(result, " ");  // Use space as delimiter for words()
        }
        strcat(result, (char*)column);
    }
    
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    hstmt = NULL;
    
    RETURNSTRX(result);
    ENDPROC
}

// Execute multiple SQL statements
PROCEDURE(odbc_execute_batch)
{
    char *sql = GETSTRING(ARG0);
    char *delimiter = GETSTRING(ARG1);
    char *token;
    char *saveptr;
    SQLRETURN ret;
    
    token = strtok_r(sql, delimiter, &saveptr);
    while (token != NULL) {
        ret = SQLExecDirect(hstmt, (SQLCHAR*)token, SQL_NTS);
        if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
            RETURNINTX(-1);
        }
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    
    RETURNINTX(0);
    ENDPROC
}

// Get database info
PROCEDURE(odbc_get_info)
{
    char info[256];
    SQLSMALLINT length;
    
    SQLGetInfo(hdbc, SQL_DBMS_NAME, info, sizeof(info), &length);
    RETURNSTRX(info);
    ENDPROC
}

// Get connection attributes
PROCEDURE(odbc_get_connection_attr)
{
    SQLUINTEGER value;
    SQLINTEGER attr = GETINT(ARG0);
    
    SQLGetConnectAttr(hdbc, attr, &value, 0, NULL);
    RETURNINTX((int)value);
    ENDPROC
}

// Move cursor to specific row
PROCEDURE(odbc_move_to)
{
    SQLLEN row = GETINT(ARG0);
    
    SQLRETURN ret = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, row);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        RETURNINTX(-1);
    }
    RETURNINTX(0);
    ENDPROC
}

// Get row count
PROCEDURE(odbc_row_count)
{
    SQLLEN count;
    
    SQLRowCount(hstmt, &count);
    RETURNINTX((int)count);
    ENDPROC
}

// Get or set current database (MySQL only)
PROCEDURE(odbc_database)
{
    SQLCHAR dbname[256];
    SQLLEN length;
    SQLRETURN ret;
    char *newdb = GETSTRING(ARG0);

    // Check if we're setting a new database
    if (strlen(newdb) > 0) {
        // Try to change database using MySQL's USE command
        ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
        if (SQL_SUCCEEDED(ret)) {
            char sql[512];
            snprintf(sql, sizeof(sql), "USE %s", newdb);
            ret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
            
            // For MySQL, we need to process all results
            if (SQL_SUCCEEDED(ret)) {
                do {
                    while (SQLFetch(hstmt) == SQL_SUCCESS);
                } while (SQLMoreResults(hstmt) == SQL_SUCCESS);
            }
            
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = NULL;
            
            if (!SQL_SUCCEEDED(ret)) {
                RETURNSTRX("Error changing database");
            }
        }
    }

    // Try MySQL's DATABASE() function
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
    if (!SQL_SUCCEEDED(ret)) {
        RETURNSTRX("");
    }
    
    // Use COALESCE to handle NULL values
    ret = SQLExecDirect(hstmt, (SQLCHAR*)"SELECT COALESCE(DATABASE(), '') AS current_db", SQL_NTS);
    
    if (!SQL_SUCCEEDED(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
        RETURNSTRX("");
    }
    
    if (SQL_SUCCEEDED(SQLFetch(hstmt))) {
        ret = SQLGetData(hstmt, 1, SQL_C_CHAR, dbname, sizeof(dbname), &length);
        if (SQL_SUCCEEDED(ret)) {
            SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
            hstmt = NULL;
        //    printf("Current database: '%s'\n", dbname);  // Debug output
            RETURNSTRX((char*)dbname);
        }
    }
    
    if (hstmt != NULL) {
        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
    }

    RETURNSTRX("");
    ENDPROC
}

PROCEDURE(show_message) {
    char *line1 = GETSTRING(ARG0);  // Get the first line
    char *line2 = GETSTRING(ARG1);  // Get the second line
    char *line3 = GETSTRING(ARG2);  // Get the third line
    char *line4 = GETSTRING(ARG3);  // Get the fourth line

    // Create a buffer for the message
    char message[1024];  // Adjust size as needed
    snprintf(message, sizeof(message), "%s\n%s\n%s\n%s", line1, line2, line3, line4);

    // Show the message box
    MessageBox(NULL, message, "Information", MB_OK | MB_ICONINFORMATION);
    RETURNINTX(0);  // Return success
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