# ODBC Programming


## Overview
The ODBC plugin provides a bridge between CREXX and database systems through ODBC (Open Database Connectivity). It supports both SQL databases and CSV files.

## Quick Start
```rexx <!--basicquery.rexx-->
/* Basic database query */
rc = odbc_connect("myDB", "user", "pass")
if rc < 0 then exit

/* Explicitly set the database */
newdb = odbc_database("CompanyDB")
if newdb = "Error changing database" then exit

rc = odbc_execute("SELECT * FROM mytable")
if rc < 0 then do
    say "Error:" odbc_error_message()
    exit
end

do while odbc_fetch() >= 0
    value = odbc_getcolumn(1)
    say value
end

call odbc_disconnect
```

## Functions

### Connection Management

#### odbc_connect
```rexx <!--ocbdconnect.rexx-->
rc = odbc_connect(dsn, username, password)
```
Establishes a connection to a database or CSV file.
- **Parameters:**
  - dsn: Database name or directory path for CSV
  - username: Database username (empty for CSV)
  - password: Database password (empty for CSV)
- **Returns:**
  - 0: Success
  - -1: Environment handle allocation failed
  - -2: Connection handle allocation failed
  - -3: Connection failed

#### odbc_disconnect
```rexx <!--odbcdisconnect.rexx-->
call odbc_disconnect
```
Closes the database connection and frees resources.

### Query Execution

#### odbc_execute
```rexx <!--odbcexecute.rexx-->
rc = odbc_execute(sql)
```
Executes an SQL statement.
- **Parameters:**
  - sql: SQL query string
- **Returns:**
  - 0: Success
  - -1: Statement handle allocation failed
  - -2: Query execution failed

### Result Set Navigation

#### odbc_fetch
```rexx <!--odbcfetch.rexx-->
rc = odbc_fetch()
```
Fetches the next row from the result set. This function should be called in a loop to process each row of the result set.

#### odbc_getcolumn
```rexx <!--odbcgetcloumn.rexx-->
value = odbc_getcolumn(column)
```
Retrieves the value from a specific column in the current row.
- **Parameters:**
  - column: Column number (1-based)
- **Returns:**
  - Column value as string
  - "-1" on error

### Metadata Functions

#### odbc_columns
```rexx <!--odbccolumns.rexx-->
cols = odbc_columns()
```
Gets the number of columns in the result set.
- **Returns:**
  - Number of columns
  - -1 on error

#### odbc_colname
```rexx <!--odbcolname.rexx-->
name = odbc_colname(column)
```
Gets the name of a specific column.
- **Parameters:**
  - column: Column number (1-based)
- **Returns:**
  - Column name
  - Empty string on error

#### odbc_coltype
```rexx <!--odbccoltype.rexx-->
type = odbc_coltype(column)
```
Gets the SQL type of a specific column.
- **Parameters:**
  - column: Column number (1-based)
- **Returns:**
  - SQL type code (see SQL Data Types section)
  - -1 on error

### Transaction Management

#### odbc_begin_transaction
```rexx <!--odbcbegintransaction.rexx-->
call odbc_begin_transaction
```
Starts a new transaction by turning off autocommit.
- **Returns:**
  - 0: Success
  - -1: Failed to set autocommit off

#### odbc_commit
```rexx <!--odbccommit.rexx-->
call odbc_commit
```
Commits the current transaction.
- **Returns:**
  - 0: Success
  - -1: Commit failed

#### odbc_rollback
```rexx <!--odbcrollback-->
call odbc_rollback
```
Rolls back the current transaction.
- **Returns:**
  - 0: Success
  - -1: Rollback failed

### Error Handling

#### odbc_error_message
```rexx <!--odbcerrmsg.rexx-->
message = odbc_error_message()
```
Gets the last error message from ODBC.
- **Returns:** Detailed error message string

### Diagnostic Functions

#### odbc_get_diagnostics
```rexx <!--odbcdiag.rexx-->
info = odbc_get_diagnostics()
```
Gets detailed diagnostic information about the last error.
- **Returns:** String containing:
  - SQLSTATE code
  - Native error code
  - Detailed message

#### odbc_column_info
```rexx <!--odbccolinfo.rexx-->
info = odbc_column_info(column)
```
Gets comprehensive information about a column.
- **Parameters:**
  - column: Column number (1-based)
- **Returns:** Semicolon-delimited string containing:
  - Name: Column name
  - Type: SQL data type
  - Size: Column size
  - Decimals: Number of decimal places
  - Nullable: Whether column can contain NULL

### Example Usage
```rexx <!--odbccolinfoexample.rexx-->
/* Get column information */
do col = 1 to odbc_columns()
    info = odbc_column_info(col)
    parse var info 'Name='name';Type='type';Size='size';Decimals='dec';Nullable='null
    say "Column" col":" name
    say "  Type:" type
    say "  Size:" size
    say "  Decimals:" dec
    say "  Nullable:" null
end

/* Error handling with diagnostics */
if rc < 0 then do
    say "Error occurred:"
    say odbc_get_diagnostics()
    exit
end
```

## CSV File Support

### Connecting to CSV Files
```rexx <!--odbccsv.rexx-->
/* Connect to a directory containing CSV files */
rc = odbc_connect("C:\path\to\csv\directory", "", "")
if rc < 0 then exit

/* Query CSV file */
rc = odbc_execute("SELECT * FROM [mydata.csv]")
```

### CSV-Specific Notes
- Leave username and password empty for CSV connections
- The dsn parameter should be the directory containing CSV files
- CSV files should use semicolon (;) as delimiter
- First row can contain column headers
- File names are used as table names in queries
- Use square brackets around file names in queries: [filename.csv]

## SQL Data Types
Common SQL types returned by odbc_coltype:

| Constant            | Value | Description                                      |
|---------------------|-------|--------------------------------------------------|
| `SQL_CHAR`          | 1     | Fixed-length character string                    |
| `SQL_NUMERIC`       | 2     | Exact numeric value with precision and scale     |
| `SQL_DECIMAL`       | 3     | Exact numeric value with precision and scale     |
| `SQL_INTEGER`       | 4     | 32-bit integer                                   |
| `SQL_SMALLINT`      | 5     | 16-bit integer                                   |
| `SQL_FLOAT`         | 6     | Approximate numeric value                        |
| `SQL_REAL`          | 7     | Single precision floating point                  |
| `SQL_DOUBLE`        | 8     | Double precision floating point                  |
| `SQL_TYPE_TIMESTAMP`| 11    | Date and time                                    |
| `SQL_VARCHAR`       | 12    | Variable-length character string                 |
| `SQL_WVARCHAR`      | -9    | Unicode variable-length character string         |
| `SQL_WCHAR`         | -8    | Unicode fixed-length character string            |
| `SQL_WLONGVARCHAR`  | -10   | Unicode long variable-length character string    |

Table: ODBC SQL Data Types  {#tbl:id}

## Usage Examples

### Basic Query Example
```rexx <!--csvconnectex.rexx-->
/* Connect to database */
rc = odbc_connect("myDB", "user", "pass")
if rc < 0 then do
    say "Connection failed:" odbc_error_message()
    exit
end

/* Explicitly set the database */
newdb = odbc_database("CompanyDB")
if newdb = "Error changing database" then do
    say "Failed to set database:" odbc_error_message()
    exit
end

/* Execute query */
rc = odbc_execute("SELECT * FROM employees")
if rc < 0 then do
    say "Query failed:" odbc_error_message()
    call odbc_disconnect
    exit
end

/* Get and display column information */
cols = odbc_columns()
do col = 1 to cols
    name = odbc_colname(col)
    type = odbc_coltype(col)
    say "Column" col":" name "Type:" type
end

/* Fetch and display results */
do while odbc_fetch() >= 0
    do col = 1 to cols
        value = odbc_getcolumn(col)
        say "Column" col":" value
    end
    say "-----------------"
end

call odbc_disconnect
```

### Transaction Example
```rexx <!--tranacionexample.rexx-->
/* Start transaction */
call odbc_begin_transaction

/* First operation */
rc = odbc_execute("UPDATE employees SET salary = salary * 1.1")
if rc < 0 then do
    say "Update failed:" odbc_error_message()
    call odbc_rollback
    call odbc_disconnect
    exit
end

/* Second operation */
rc = odbc_execute("INSERT INTO audit_log VALUES ('Salary update')")
if rc < 0 then do
    say "Audit failed:" odbc_error_message()
    call odbc_rollback
    call odbc_disconnect
    exit
end

/* Commit if both operations succeeded */
call odbc_commit
call odbc_disconnect
```

### Advanced Usage Examples

1. **Table and Schema Information**
```rexx <!--schemainfo.rexx-->
/* Get list of tables */
tables = odbc_tables()
do while tables \= ''
    parse var tables table ';' tables
    say "Found table:" table
end

/* Get primary keys for a table */
keys = odbc_primary_keys("employees")
do while keys \= ''
    parse var keys key ';' keys
    say "Primary key column:" key
end
```

2. **Batch Operations**
```rexx <!--odbcbatch.rexx-->
/* Execute multiple SQL statements */
sql = "UPDATE employees SET salary = salary * 1.1 WHERE department = 'IT';"
sql = sql || "INSERT INTO audit_log VALUES ('Salary update');"
sql = sql || "UPDATE budget SET amount = amount - 50000 WHERE dept = 'IT';"

rc = odbc_execute_batch(sql, ";")
if rc < 0 then do
    say "Batch execution failed:" odbc_error_message()
    exit
end
```

3. **Database Information**
```rexx <!--odbcdbinfo.rexx-->
/* Get database system info */
dbms = odbc_get_info()
say "Database system:" dbms

/* Get connection attributes */
autocommit = odbc_get_connection_attr(SQL_ATTR_AUTOCOMMIT)
say "Autocommit mode:" autocommit
```

4. **Result Set Navigation**
```rexx <!--odbcresultset.rexx-->
/* Execute a query */
rc = odbc_execute("SELECT * FROM employees ORDER BY salary DESC")

/* Get total rows */
rows = odbc_row_count()
say "Total rows:" rows

/* Move to specific row */
rc = odbc_move_to(5)  /* Move to 5th row */
if rc >= 0 then do
    name = odbc_getcolumn(1)
    salary = odbc_getcolumn(2)
    say "5th highest salary:" name salary
end

/* Move to last row */
rc = odbc_move_to(rows)
if rc >= 0 then do
    name = odbc_getcolumn(1)
    salary = odbc_getcolumn(2)
    say "Lowest salary:" name salary
end
```

5. **Combined Example**
```rexx <!--odbccombined.rexx-->
/* Connect and start transaction */
rc = odbc_connect("MyDB", "user", "pass")
call odbc_begin_transaction

/* Get database info */
say "Connected to" odbc_get_info()

/* Get tables and their primary keys */
tables = odbc_tables()
do while tables \= ''
    parse var tables table ';' tables
    say "Table:" table
    
    keys = odbc_primary_keys(table)
    do while keys \= ''
        parse var keys key ';' keys
        say "  Primary key:" key
    end
end

/* Execute query with row navigation */
rc = odbc_execute("SELECT * FROM employees")
rows = odbc_row_count()
say "Found" rows "employees"

/* Move through result set */
do row = 1 to rows
    rc = odbc_move_to(row)
    if rc >= 0 then do
        name = odbc_getcolumn(1)
        say "Employee" row":" name
    end
end

call odbc_commit
call odbc_disconnect
```

These examples demonstrate:
- Table and schema metadata retrieval
- Batch SQL execution
- Database system information
- Result set navigation
- Row counting
- Combined usage patterns

Note: Some functions may not be available with all ODBC drivers or database systems. Always check return codes for errors.

