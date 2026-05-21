/* GUI Sample */
options levelb
import odbc
import rxfnsb
database='CompanyDB'
table   ='Employees.'database    

/* Connect to CSV file via ODBC */
## rc = odbc_connect("C:\\Temp\\CREXX", "", "")  /* CSV file Use double backslashes in Windows paths */
 rc = odbc_connect("myCustomDB", "root", "pjjp")
if rc = 0 then say "ODBC connection established with rc=" rc
else do
    say "Connection failed with rc=" rc
    exit
end

/* Execute a query on the CSV file */
## rc = odbc_execute("SELECT * FROM [ODBC_test.csv]")   /* CSV Use simple SELECT * */

rc = odbc_execute("SELECT  COUNT(*) FROM CompanyDB.Employees")
if rc = 0  then say "Table selected with rc=" rc
else do
    say "Query failed with rc=" rc
    call odbc_disconnect
    exit
end
## SELECT  COUNT(*) returns a single line, fetch it
result = odbc_fetch()

rc = odbc_execute("SELECT * FROM CompanyDB.Employees")  /* Use simple SELECT * */
if rc = 0  then say "Table selected with rc=" rc
else do
    say "Query failed with rc=" rc
    call odbc_disconnect
    exit
end
rc = odbc_execute("SELECT  COUNT(*) FROM CompanyDB.Employees")
if rc = 0  then say "Table selected with rc=" rc
else do
    say "Query failed with rc=" rc
    call odbc_disconnect
    exit
end
/* Get number of columns */
cols = odbc_columns()
say 'number available columns 'cols

/* Fetch and process results */
row=0
do forever
    row=row+1
    result = odbc_fetch()
    if result = "-1" then leave
    say "Retrieve Row: "row
    do col = 1 to cols
       value = odbc_getcolumn(col)
       say "  Column" col": '"value"'"
    end
end

/* Disconnect */
call odbc_disconnect