/* ODBC Sample */
options levelb
import odbc
import rxfnsb
database='CompanyDB'
table   =database'.Employees'
/* Connect to Database */
/* -----------------------------------------------------------------
 * Connect to Database
 * -----------------------------------------------------------------
 */
say "*** Connect to Database..."
  rc = odbc_connect("myCustomDB", "root", "pjjp")
  if rc < 0 then call Error_exit "Connection failed: "odbc_error_message(), "Full diagnostics: "odbc_get_diagnostics()
  dbinfo = odbc_get_info()
  say "Connected to database system:" dbinfo
/* -----------------------------------------------------------------
 * Explicitly set the database
 * -----------------------------------------------------------------
 */
say "*** set/change Database..."
  newdb = odbc_database('customer')
  if newdb = "Error changing database" then call error_exit "Failed to set database: "odbc_error_message(),""
  say "Database set to: '"newdb"'"
  /* Explicitly set the database */
  newdb = odbc_database(database)
  if newdb = "Error changing database" then call error_exit "Failed to set database: "odbc_error_message(),""
  say "Database set to: '"newdb"'"
/* -----------------------------------------------------------------
 * Execute SELECT * FROM table
 * -----------------------------------------------------------------
 */
say "*** SELECT * FROM "table"..."
  rc = odbc_execute("SELECT * FROM "table)
  if rc < 0 then call error_exit "Query failed: "odbc_error_message(),""
  /* Get row count */
  say "*** get row count..."
  row_count = odbc_row_count()
  say "Total rows:" row_count
  /* Get tables */
  say "*** Retrieving tables..."
/* -----------------------------------------------------------------
 * Query all tables in a database and its primary keys
 * -----------------------------------------------------------------
 */
tables = odbc_tables()
  if tables = "Error executing SQLTables" | ,
     tables = "Error fetching table names" | ,
     tables = "Failed to allocate statement handle" then call error_exit "Failed to get tables: "odbc_error_message(), "Full diagnostics: "odbc_get_diagnostics()
  if tables = '' then call error_exit "No tables found in database",""
  /* Process table list */
  say "*** processing tables..."
  tabcount=words(tables)
  do i=1 to tabcount
     table=word(tables,i)
     /* Get primary keys for this table */
     keys = odbc_primary_keys(table)
     keycount=words(keys)
     do j=1 to keycount
        say "  Primary key:" word(keys,j)
     end
     if keycount=0 then say "  No primary keys found"
  end
/* -----------------------------------------------------------------
 * Execute SELECT * FROM table and retrieve column information
 * -----------------------------------------------------------------
 */
say "*** Column Information Example..."
  rc = odbc_execute("SELECT * FROM "table)
  if rc < 0 then call error_exit "Query failed: "odbc_error_message(),""

  cols = odbc_columns()
  do col = 1 to cols
     colname = odbc_colname(col)
     coltype = odbc_coltype(col)
     say "Column" col": Name =" colname "Type =" coltype
  end
/* -----------------------------------------------------------------
 * Transaction Management
 * -----------------------------------------------------------------
 */
say "*** Transaction Example..."
  call odbc_begin_transaction
  /* Try to insert two records - all or nothing */
  rc = odbc_execute("INSERT INTO "table" (Employee_ID, Name, Salary) VALUES (600, 'John2', 50000)")
  if rc < 0 then do
     say "First insert failed:" odbc_error_message()
     call odbc_rollback
     say "Transaction rolled back"
  end
  else do
     rc = odbc_execute("INSERT INTO "table" (Employee_ID, Name, Salary) VALUES (610,'Jane2', 55000)")
     if rc < 0 then do
        say "Second insert failed:" odbc_error_message()
        call odbc_rollback
        say "Transaction rolled back"
     end
     else do
        call odbc_commit
        say "Transaction committed successfully"
     end
  end
/* -----------------------------------------------------------------
 * Query with Error Handling: SELECT Name, Salary FROM 'table' WHERE Salary > 60000
 * -----------------------------------------------------------------
 */
say "=== Query with Error Handling Example ==="
  say '    SELECT Name, Salary FROM 'table' WHERE Salary > 60000)'
  rc = odbc_execute("SELECT Name, Salary FROM "table" WHERE Salary > 60000")
  if rc < 0 then do
     say "Query failed:" odbc_error_message()
     exit
  end
/* Print column headers */
  cols = odbc_columns()
  coln=""
  do col = 1 to cols
     coln=coln||right(odbc_colname(col),16)" "
  end
  say coln
  say copies('-',33)
  call odbc_fetch(5)
  /* Fetch and display results */
  do while odbc_fetch(0) >= 0
     colx=''
     do coli = 1 to cols
        colx=colx||right(odbc_getcolumn(coli),16)" "
    end
    say  colx
end
/* -----------------------------------------------------------------
 * Example 4: Query with Error Handling: SELECT DATABASE()
 * -----------------------------------------------------------------
 */
say "=== Query with Error Handling Example ==="
  say '    SELECT DATABASE()'
  rc = odbc_execute("SELECT DATABASE()")
  if rc < 0 then do
     say "Query failed:" odbc_error_message()
     exit
  end
  /* Print column headers */
  cols = odbc_columns()
  coln=""
  do col = 1 to cols
     coln=coln||right(odbc_colname(col),16)" "
  end
  say coln
  say copies('-',33)
  /* Fetch and display results */
  do while odbc_fetch(0) >= 0
     colx=''
     do coli = 1 to cols
        colx=colx||right(odbc_getcolumn(coli),16)" "
     end
     say  colx
  end
/* -----------------------------------------------------------------
 * Cleanup
 * -----------------------------------------------------------------
 */
  call odbc_disconnect
exit
/* -----------------------------------------------------------------
 * Helper function Error Handling
 * -----------------------------------------------------------------
 */
Error_exit: procedure
arg msg1=.string, msg2=.string
   say msg1
   say msg2
exit
return