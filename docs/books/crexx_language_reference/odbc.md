# ODBC Plugin Documentation


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

## Windows ODBC Configuration

### Setting Up ODBC Data Sources

1. **Open ODBC Data Source Administrator**
   - Press Windows + R
   - Type: `odbcad32.exe` for 32-bit or
   - Navigate to: Control Panel > Administrative Tools > ODBC Data Sources

2. **Choose the Correct Administrator**
   - Use 32-bit: `C:\Windows\SysWOW64\odbcad32.exe`
   - Use 64-bit: `C:\Windows\System32\odbcad32.exe`
   - Note: Match the bitness of your REXX environment

### Configuring Database Connections

1. **MySQL Configuration**

   Driver: MySQL ODBC Driver  
   Server: localhost (or your server)  
   Database: your_database_name  
   User: your_username  
   Password: your_password  
   Port: 3306 (default)  


2. **CSV File Configuration**
   - Driver: `Microsoft Access Text Driver (*.txt, *.csv)`
   - Directory: Path to your CSV files
   - Extensions: CSV
   - Format: Delimited(;)

### Required ODBC Drivers

1. **For MySQL**
   - Download MySQL Connector/ODBC from MySQL website
   - Run installer and select correct bitness (32/64)
   - Verify in ODBC Administrator

2. **For CSV Files**
   - Microsoft Access Text Driver (built into Windows)
   - Microsoft Excel Driver (optional, for backup)

### Testing the Connection

1. **Using ODBC Administrator**
   - Click "Test Connection" after setup
   - Verify connectivity before using in REXX

2. **Using REXX**
```rexx <!--testconnect.rexx-->
   /* Test connection */
   rc = odbc_connect("myDSN", "user", "pass")
   if rc < 0 then do
       say "Connection failed:" odbc_error_message()
       exit
   end
   say "Connection successful"
   call odbc_disconnect
```

### Troubleshooting Windows ODBC

1. **Common Issues**
   - Driver not found: Install correct ODBC driver
   - Wrong bitness: Match 32/64-bit versions
   - Path issues: Use double backslashes in paths
   - Permission denied: Check Windows permissions

2. **Driver Verification**
```rexx <!--listdrivers.rexx-->
   /* List available drivers */
   rc = odbc_connect("", "", "")  /* Will fail but show drivers */
```

3. **Path Format Examples**
```rexx <!--pathformat.rexx-->
   /* CSV directory path formats */
   rc = odbc_connect("C:\\Data\\CSV", "", "")     /* Double backslash */
   rc = odbc_connect("C:/Data/CSV", "", "")       /* Forward slash */
```

### Windows-Specific Notes

1. **File Permissions**
   - Ensure read/write access to database/files
   - Run as administrator if needed
   - Check Windows User Account Control (UAC)

2. **System DSN vs. User DSN**
   - System DSN: Available to all users
   - User DSN: Only for current user
   - File DSN: Portable between systems

3. **Registry Settings**
   - ODBC drivers listed in:
     - `HKEY_LOCAL_MACHINE\SOFTWARE\ODBC\ODBCINST.INI`
   - DSN configurations in:
     - `HKEY_LOCAL_MACHINE\SOFTWARE\ODBC\ODBC.INI`

4. **Environment Variables**
   - ODBC_PATH: Path to ODBC installation
   - ODBCINI: Path to odbc.ini file
   - ODBCINST: Path to odbcinst.ini file

## Linux ODBC Configuration

### Installing ODBC Components

1. **Install Required Packages**
```bash
   # Debian/Ubuntu
   sudo apt-get install unixodbc unixodbc-dev
   
   # RedHat/CentOS
   sudo yum install unixODBC unixODBC-devel
   
   # For MySQL
   sudo apt-get install libmyodbc   # Debian/Ubuntu
   sudo yum install mysql-connector-odbc   # RedHat/CentOS
```

2. **Configuration Files**
   - `/etc/odbc.ini`: System DSN definitions
   - `/etc/odbcinst.ini`: Driver definitions
   - `~/.odbc.ini`: User-specific DSN definitions

### Setting Up Drivers

1. **MySQL Driver Setup**
```ini <!--mysqlodbcinst.ini-->
   # /etc/odbcinst.ini
   [MySQL]
   Description = MySQL ODBC Driver
   Driver = /usr/lib/x86_64-linux-gnu/odbc/libmyodbc.so
   Setup = /usr/lib/x86_64-linux-gnu/odbc/libodbcmyS.so
   FileUsage = 1
```

2. **CSV Driver Setup**
```ini <!--csvinst.ini-->
   # /etc/odbcinst.ini
   [Text]
   Description = Text Driver
   Driver = /usr/lib/x86_64-linux-gnu/odbc/libtxtS.so
   Setup = /usr/lib/x86_64-linux-gnu/odbc/libtxtS.so
   FileUsage = 1
```

### Configuring Data Sources

1. **MySQL DSN Configuration**
```ini <!--mysql.ini-->
   # /etc/odbc.ini or ~/.odbc.ini
   [MyDB]
   Driver = MySQL
   Server = localhost
   Database = test
   Port = 3306
```

2. **CSV File Configuration**
```ini <!--csvfile.ini-->
   # /etc/odbc.ini or ~/.odbc.ini
   [MyCSV]
   Driver = Text
   Directory = /path/to/csv/files
   Extension = csv
   Delimiter = ;
```

### Testing Configuration

1. **Using isql**
```bash <!--bashtest.sh-->
   isql -v MyDB username password
```

2. **Using odbcinst**
   ```bash <!--bashtest2.sh-->
   odbcinst -q -d    # List drivers
   odbcinst -q -s    # List data sources
   ```

## macOS ODBC Configuration

### Installing ODBC Components

1. **Using Homebrew**
```bash
   brew install unixodbc
   brew install mysql-connector-odbc  # For MySQL
```

2. **Configuration Locations**
   - `/usr/local/etc/odbc.ini`
   - `/usr/local/etc/odbcinst.ini`
   - `~/Library/ODBC/odbc.ini`

### Driver Setup

1. **MySQL Driver**
   ```ini
   # /usr/local/etc/odbcinst.ini
   [MySQL]
   Description = MySQL ODBC Driver
   Driver = /usr/local/lib/libmyodbc5w.so
   Setup = /usr/local/lib/libodbcmyS.so
   ```

2. **CSV Support**
```ini <!--mysqlcsv.ini-->
   # /usr/local/etc/odbcinst.ini
   [Text]
   Description = Text Driver
   Driver = /usr/local/lib/libtextodbc.so
   Setup = /usr/local/lib/libtextodbc.so
```

### DSN Configuration

1. **MySQL Example**
```ini <!--mysqldsn.ini-->
   # /usr/local/etc/odbc.ini
   [MyDB]
   Driver = MySQL
   Server = localhost
   Database = test
   Port = 3306
```

2. **CSV Example**
```ini <!--msqlcsv2.ini-->
   # /usr/local/etc/odbc.ini
   [MyCSV]
   Driver = Text
   Directory = /Users/username/csv
   Extension = csv
   Delimiter = ;
```

### Testing and Troubleshooting

1. **Environment Variables**
```bash <!--bashtest3.sh-->
   export ODBCINI=/usr/local/etc/odbc.ini
   export ODBCSYSINI=/usr/local/etc
   export ODBCINSTINI=/usr/local/etc/odbcinst.ini
```

2. **Testing Tools**
```bash <!--bashtest4.sh-->
   # Test connection
   iodbctest "DSN=MyDB;UID=username;PWD=password"
   
   # List drivers
   iodbcadm-gtk    # GUI tool
   odbcinst -q -d  # Command line
```

### Common Issues on Unix-like Systems

1. **Driver Not Found**
   - Check library paths
   - Verify driver installation
   - Check file permissions

2. **Permission Issues**
```bash <!--permissions.sh-->
   # Fix permissions
   sudo chmod 644 /etc/odbc.ini
   sudo chmod 644 /etc/odbcinst.ini
```

3. **Library Path Issues**
```bash <!--add bashrc.sh-->
   # Add to .bashrc or equivalent
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

4. **SELinux Considerations (Linux)**
```bash <!--bashselinux.sh-->
   # If using SELinux
   sudo setsebool -P allow_odbc_connection 1
```

### Platform-Specific Notes

1. **Linux**
   - Different distributions may have different package names
   - Check distribution-specific documentation
   - Consider using package manager's search function

2. **macOS**
   - Homebrew is recommended for package management
   - M1/ARM64 may require different configurations
   - Check architecture-specific paths

## Database-Specific Configurations

### SQLite Configuration

1. **Installing SQLite ODBC Driver**
```bash <!--sqlite3.sh-->
   # Windows
   Download from http://www.ch-werner.de/sqliteodbc/

   # Linux (Debian/Ubuntu)
   sudo apt-get install libsqliteodbc

   # macOS
   brew install sqliteodbc
```

2. **Driver Setup**
```ini <!--sqlite3.ini-->
   # Windows: System DSN
   [SQLite3 ODBC Driver]
   Description=SQLite3 ODBC Driver
   Driver=sqliteodbc.dll
   Setup=sqliteodbc.dll
   
   # Linux/macOS: /etc/odbcinst.ini
   [SQLite3]
   Description=SQLite3 ODBC Driver
   Driver=/usr/lib/x86_64-linux-gnu/odbc/libsqlite3odbc.so
   Setup=/usr/lib/x86_64-linux-gnu/odbc/libsqlite3odbc.so
```

3. **DSN Configuration**
```ini <!--sqlitedsn.ini-->
   [MySQLiteDB]
   Driver=SQLite3
   Database=/path/to/database.db
   Timeout=2000
   NoTXN=0
   SyncPragma=NORMAL
```

### PostgreSQL Configuration

1. **Installing PostgreSQL ODBC Driver**
```bash <!--postrgenstall.sh-->
   # Windows
   Download from postgresql.org/ftp/odbc/versions/

   # Linux
   sudo apt-get install odbc-postgresql    # Debian/Ubuntu
   sudo yum install postgresql-odbc        # RedHat/CentOS

   # macOS
   brew install psqlodbc
```

2. **Driver Setup**
```ini <!--pssql2.ini-->
   [PostgreSQL]
   Description=PostgreSQL ODBC Driver
   Driver=psqlodbcw.dll                    # Windows
   Driver=/usr/lib/psqlodbcw.so           # Linux
   Driver=/usr/local/lib/psqlodbcw.so     # macOS
   ```

3. **DSN Configuration**
```ini <!--mypostgres.ini-->
   [MyPostgres]
   Driver=PostgreSQL
   Server=localhost
   Port=5432
   Database=mydb
   Username=myuser
   Password=mypass
```

### Microsoft SQL Server Configuration

1. **Installing MSSQL ODBC Driver**
```bash <!--mssqlodbc.sh-->
   # Windows
   Download from Microsoft's website

   # Linux
   curl https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
   sudo apt-get install msodbcsql17

   # macOS
   brew tap microsoft/mssql-release
   brew install msodbcsql17
```

2. **Driver Setup**
```ini <!--mmssql.ini-->
   [MSSQL]
   Description=Microsoft SQL Server Driver
   Driver={ODBC Driver 17 for SQL Server}
```

3. **DSN Configuration**
```ini <!--mmsysql.ini-->
   [MyMSSQL]
   Driver=MSSQL
   Server=serveraddress,1433
   Database=mydatabase
```

### MariaDB Configuration

1. **Installing MariaDB ODBC Driver**
```bash <!--mariadb2.sh-->
   # Windows
   Download from mariadb.org/download/

   # Linux
   sudo apt-get install libmariadb3 libmariadb-dev
   sudo apt-get install mariadb-connector-odbc

   # macOS
   brew install mariadb-connector-odbc
```

2. **Driver Setup**
```ini <!--mariadb1.ini-->
   [MariaDB]
   Description=MariaDB ODBC Driver
   Driver=libmaodbc.so
```

### Usage Notes for Different Databases

1. **SQLite Specific**
```rexx <!--sqlitespec.rexx-->
   /* Connect to SQLite database */
   rc = odbc_connect("MySQLiteDB", "", "")  /* No username/password needed */
   
   /* Create table example */
   rc = odbc_execute("CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, name TEXT)")
```

2. **PostgreSQL Specific**
```rexx <!--postgresspec.rexx-->
   /* Handle schema */
   rc = odbc_execute("SET search_path TO myschema")
   
   /* Use prepared statements */
   rc = odbc_execute("PREPARE myplan (int) AS SELECT * FROM mytable WHERE id = $1")
   ```

3. **MSSQL Specific**
```rexx <!--mssqlspec.rexx-->
   /* Windows Authentication */
   rc = odbc_connect("MyMSSQL", "", "")  /* Empty credentials for Windows Auth */
   
   /* Handle schemas */
   rc = odbc_execute("USE mydatabase; SELECT * FROM dbo.mytable")
```

### Common Issues by Database Type

1. **SQLite**
   - File permissions
   - Database file locking
   - Journal file location

2. **PostgreSQL**
   - Schema handling
   - SSL certificate configuration
   - Network connectivity

3. **MSSQL**
   - Windows vs SQL authentication
   - Network protocols
   - Instance naming

4. **MariaDB/MySQL**
   - Character set configuration
   - SSL connection setup
   - Time zone handling

## Security Considerations

1. **Connection Security**
   - Avoid hardcoding credentials in scripts
   - Use environment variables or secure configuration files
   - Enable SSL/TLS where available

2. **File Permissions**
   - Secure database files with appropriate permissions
   - Protect configuration files containing credentials
   - Consider using system DSN instead of file DSN

3. **Network Security**
   - Use encrypted connections when possible
   - Configure firewalls appropriately
   - Consider using VPN for remote connections

## Performance Tips

1. **Query Optimization**
   - Use appropriate WHERE clauses
   - Consider indexing frequently queried columns
   - Avoid SELECT * when possible

2. **Connection Management**
   - Reuse connections when possible
   - Close connections properly
   - Use transactions for bulk operations

3. **Memory Considerations**
   - Handle large result sets in chunks
   - Clear statement handles when done
   - Monitor memory usage with large datasets

### Database Selection Functions

#### odbc_database
```rexx <!--odbcdb.rexx-->
curdb = odbc_database()          /* Get current database name */
newdb = odbc_database("mydb")    /* Switch to different database */
```
Gets or sets the current database. 

**Note:** This function is currently only supported for MySQL databases. For other database systems, use database-specific SQL commands via `odbc_execute()`.

**Important:** After connecting to a database, you should explicitly set the database using `odbc_database()` before performing any operations.

Parameters:
- newdb: (optional) Name of database to switch to

Returns:
- Current database name after operation
- Empty string if no database selected
- "Error changing database" if database switch failed

### Primary Key Retrieval

#### odbc_primary_keys
```rexx <!--odbcprimarykeys.rexx-->
keys = odbc_primary_keys(table)
```
Retrieves the primary keys for the specified table.

**Returns:**
- A string containing the primary key column names, separated by spaces.
- An error message if the primary keys cannot be retrieved.
