/* ODBC Sample */
options levelb
import odbc
import rxfnsb
database='CompanyDB'
table   =database'.Employees'
/* ---------------------------------------------------------------------
 * Connect to Database via ODBC
 * ---------------------------------------------------------------------
 */
rc = odbc_connect("myCustomDB", "root", "pjjp")
if rc = 0 then say "ODBC connection established with rc=" rc
else do
   say "Connection failed with rc=" rc
   exit
end
/* ---------------------------------------------------------------------
 * Determine rows of the table
 * ---------------------------------------------------------------------
 */
rc=odbc_run('SELECT  COUNT(*) FROM 'table)
result = odbc_fetch()                  ## address result row
say "table contains "odbc_getcolumn(1)" rows"   ## fetch 1. row, 1. column, there are the number of rows stored
/* ---------------------------------------------------------------------
 * Print all rows of the table
 *   1. Select table
 *   2. retrieve number of columns
 *   3. report for each row the columns and output them
 * ---------------------------------------------------------------------
 */
rc = odbc_run("SELECT * FROM "table)   ## SELECT *   the table, pointer is set to first record

cols = odbc_columns()                  ## Get number of columns
say 'number available columns 'cols

/* Fetch and process results */
row=0
do forever
    row=row+1
    result = odbc_fetch()              ## fetch first, and subsequently next row
    if result = "-1" then leave
    say "Retrieve Row: "row
    do col = 1 to cols
       value = odbc_getcolumn(col)     ## get each column separately
       say "  Column" col": '"value"'"
    end
end
/* Disconnect */
call odbc_disconnect
exit
/* ---------------------------------------------------------------------
 * Execute the SQL statements with error checking
 * ---------------------------------------------------------------------
 */
odbc_run: procedure=.int
arg sql=.string
  rc=odbc_execute(sql)
  if rc = 0 then say "SQL "sql" successfully executed"
  else do
    say "Query "sql" failed with rc=" rc
    call odbc_disconnect
    exit
  end
return 0
