#ifndef CREXX_TEST_MOCK_ODBC_H
#define CREXX_TEST_MOCK_ODBC_H

#include <stddef.h>
#include <stdint.h>

typedef void *SQLHANDLE;
typedef SQLHANDLE SQLHENV;
typedef SQLHANDLE SQLHDBC;
typedef SQLHANDLE SQLHSTMT;
typedef void *SQLPOINTER;
typedef unsigned char SQLCHAR;
typedef int16_t SQLSMALLINT;
typedef uint16_t SQLUSMALLINT;
typedef int32_t SQLINTEGER;
typedef uint32_t SQLUINTEGER;
typedef int64_t SQLLEN;
typedef uint64_t SQLULEN;
typedef int64_t SQLBIGINT;
typedef SQLSMALLINT SQLRETURN;

#define SQL_NULL_HANDLE NULL
#define SQL_NULL_HENV NULL
#define SQL_NULL_HDBC NULL
#define SQL_NULL_HSTMT NULL

#define SQL_SUCCESS 0
#define SQL_SUCCESS_WITH_INFO 1
#define SQL_ERROR (-1)
#define SQL_NO_DATA 100
#define SQL_SUCCEEDED(rc) ((rc) == SQL_SUCCESS || (rc) == SQL_SUCCESS_WITH_INFO)

#define SQL_HANDLE_ENV 1
#define SQL_HANDLE_DBC 2
#define SQL_HANDLE_STMT 3
#define SQL_ATTR_ODBC_VERSION 200
#define SQL_OV_ODBC3 3
#define SQL_ATTR_AUTOCOMMIT 102
#define SQL_AUTOCOMMIT_OFF 0
#define SQL_AUTOCOMMIT_ON 1
#define SQL_ATTR_CURRENT_CATALOG 109
#define SQL_COMMIT 0
#define SQL_ROLLBACK 1
#define SQL_NTS (-3)
#define SQL_NULL_DATA (-1)
#define SQL_PARAM_INPUT 1
#define SQL_C_CHAR 1
#define SQL_C_SBIGINT (-25)
#define SQL_C_DOUBLE 8
#define SQL_VARCHAR 12
#define SQL_BIGINT (-5)
#define SQL_DOUBLE 8
#define SQL_DESC_NAME 1011
#define SQL_DESC_TYPE 1002
#define SQL_FETCH_ABSOLUTE 5
#define SQL_CLOSE 0
#define SQL_RESET_PARAMS 3
#define SQL_DBMS_NAME 17
#define SQL_MAX_MESSAGE_LENGTH 512

SQLRETURN SQLAllocHandle(SQLSMALLINT handle_type, SQLHANDLE input,
                         SQLHANDLE *output);
SQLRETURN SQLFreeHandle(SQLSMALLINT handle_type, SQLHANDLE handle);
SQLRETURN SQLSetEnvAttr(SQLHENV environment, SQLINTEGER attribute,
                        SQLPOINTER value, SQLINTEGER length);
SQLRETURN SQLConnect(SQLHDBC connection, SQLCHAR *dsn, SQLSMALLINT dsn_length,
                     SQLCHAR *user, SQLSMALLINT user_length,
                     SQLCHAR *password, SQLSMALLINT password_length);
SQLRETURN SQLDisconnect(SQLHDBC connection);
SQLRETURN SQLExecDirect(SQLHSTMT statement, SQLCHAR *sql,
                        SQLINTEGER length);
SQLRETURN SQLPrepare(SQLHSTMT statement, SQLCHAR *sql, SQLINTEGER length);
SQLRETURN SQLBindParameter(SQLHSTMT statement, SQLUSMALLINT parameter,
                           SQLSMALLINT input_output_type,
                           SQLSMALLINT value_type,
                           SQLSMALLINT parameter_type,
                           SQLULEN column_size, SQLSMALLINT decimal_digits,
                           SQLPOINTER value, SQLLEN buffer_length,
                           SQLLEN *indicator);
SQLRETURN SQLExecute(SQLHSTMT statement);
SQLRETURN SQLFreeStmt(SQLHSTMT statement, SQLUSMALLINT option);
SQLRETURN SQLFetch(SQLHSTMT statement);
SQLRETURN SQLFetchScroll(SQLHSTMT statement, SQLSMALLINT orientation,
                         SQLLEN offset);
SQLRETURN SQLNumResultCols(SQLHSTMT statement, SQLSMALLINT *columns);
SQLRETURN SQLGetData(SQLHSTMT statement, SQLUSMALLINT column,
                     SQLSMALLINT target_type, SQLPOINTER buffer,
                     SQLLEN buffer_length, SQLLEN *indicator);
SQLRETURN SQLColAttribute(SQLHSTMT statement, SQLUSMALLINT column,
                          SQLUSMALLINT field, SQLPOINTER character,
                          SQLSMALLINT buffer_length,
                          SQLSMALLINT *string_length, SQLLEN *numeric);
SQLRETURN SQLDescribeCol(SQLHSTMT statement, SQLUSMALLINT column,
                         SQLCHAR *name, SQLSMALLINT buffer_length,
                         SQLSMALLINT *name_length, SQLSMALLINT *type,
                         SQLULEN *size, SQLSMALLINT *digits,
                         SQLSMALLINT *nullable);
SQLRETURN SQLSetConnectAttr(SQLHDBC connection, SQLINTEGER attribute,
                            SQLPOINTER value, SQLINTEGER length);
SQLRETURN SQLGetConnectAttr(SQLHDBC connection, SQLINTEGER attribute,
                            SQLPOINTER value, SQLINTEGER buffer_length,
                            SQLINTEGER *string_length);
SQLRETURN SQLEndTran(SQLSMALLINT handle_type, SQLHANDLE handle,
                     SQLSMALLINT completion);
SQLRETURN SQLGetDiagRec(SQLSMALLINT handle_type, SQLHANDLE handle,
                        SQLSMALLINT record, SQLCHAR *state,
                        SQLINTEGER *native_error, SQLCHAR *message,
                        SQLSMALLINT buffer_length, SQLSMALLINT *length);
SQLRETURN SQLTables(SQLHSTMT statement, SQLCHAR *catalogue,
                    SQLSMALLINT catalogue_length, SQLCHAR *schema,
                    SQLSMALLINT schema_length, SQLCHAR *table,
                    SQLSMALLINT table_length, SQLCHAR *type,
                    SQLSMALLINT type_length);
SQLRETURN SQLPrimaryKeys(SQLHSTMT statement, SQLCHAR *catalogue,
                         SQLSMALLINT catalogue_length, SQLCHAR *schema,
                         SQLSMALLINT schema_length, SQLCHAR *table,
                         SQLSMALLINT table_length);
SQLRETURN SQLGetInfo(SQLHDBC connection, SQLUSMALLINT info_type,
                     SQLPOINTER value, SQLSMALLINT buffer_length,
                     SQLSMALLINT *string_length);
SQLRETURN SQLRowCount(SQLHSTMT statement, SQLLEN *count);

int crexx_mock_odbc_maximum_execute_overlap(void);
void crexx_mock_odbc_reset_execute_overlap(void);

#endif
