# Programs and libraries

## Greeting the world

\rexx{} is designed as a small language, but there are already a lot of  things you can do without involving anything outside of it. One of those is to greet the world, as is the usual and obligatory first example of any programming language book.

```rexx <!--hello.rexx-->
/* rexx: welcoming the world */
say 'hello world!'
```

There is no need to include the starting comment, but by all means include it if you want to.[^1]

[^1]: Level B \crexx{} has a few ```options``` like ```hashcomments```, ```dashcomments``` or ```slashcomments``` to tune the comment format but the starting comment is meant for some operating systems to distinguish \rexx{} from the other, earlier command processors they have, like exec/exec2 for VM, CLIST for TSO on z/OS and the .bat language for DOS and OS/2.

\code{say} is a statement of the \rexx{} language and as such built-in. It does not matter if it is expressed as ```say```, ```Say```, or ```SAY````; in other words, the statements of the language are case-insensitive.

With the roots of the \rexx{} language (when it still went with one less 'x' at the end) as a command language, there is one other important thing we can do without any outside support, and that is sending commands to the environment.

```rexx <!--helloaddress.rexx-->
/* rexx: welcoming the world */
say 'hello world!'
'date'
```

As ```system``` is the standard environment before we change it (by specifying something else on the \code{address} statement), this is sent to this environment, most of the time the shell under which your program executes. If this has a ```date``` command, the output of it will be returned. The I/O to the environment using the ```address``` statement can also be redirected using *stem variables*. In fact, sending commands to an environment is one of the examples of Object Orientation using Classic \rexx{}; the same message can be sent to different objects, as long as these are implemented in the environment, and will potentially yield different answers.

## Calling a function

Whenever we are not executing the program in a linear way, we are calling functions and optionally returning data. The simplest form calls a label which is contained in the program source text itself. A label is indicated by placing a semicolon after a word, and using the ```call``` statement to jump to it and the built-in ```RESULT``` variable to fetch the answer the function deposited into it. Of course, the called function can also directly ```say``` things to output. A variant is the label with a ```procedure``` after it, which shields the global variables from the procedure, except the ones that are explicitly ```expose```d - the called function has access to those, and can update them, which normal procedures cannot.

Here is an example of calling something defined by a label:




And here a procedure with protected variables:


## Calling a built-in function

The \rexx{} language arrives with a number of built-in string functions which are the epitome of \rexx{}-ness; these are the common value in variants of the language. Now, in \crexx{} level B, we need to import a library with these functions.[^2] The reason for this, is that there are no automatic imports (for example, \rexx{} on TSO imports a library called \code{rxfns} automatically, and the other implementations also do that. Other function packages have specialized function libraries, which have system dependent calling conventions. The intention of the b level of the \crexx{} language is that the libraries are easy to add, can be written in \rexx{}, and all have the same way of being called; which library exactly is being used is specified in the ```import``` statement.

[^2]: \crexx{} level C, for compatibility with Classic \rexx{}, still grants these functions a special status.


```rexx <!--hellodate.rexx-->
/* rexx */
options levelb
import rxfnsb
say 'hello cRexx world!'
say 'today it''s' date('w')
```

Importing the standard ('bif') library gives us access to the Date() function, which has the advantage that we do not have to guess that the host platform has a ```date``` command and gives us all kinds of options, and date arithmetic. The ```rxfnsb``` library contains all the standard \rexx{} built-in functions, for the b-level implementation, which means that they use native types for their arguments. If we, for example, want to use a database in our application, we only need to import the ```odbc``` library.

```rexx <!--odbc.rexx-->
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
say "Connect to Database..."
  rc = odbc_connect("myCustomDB", "root", "pjjp")
  if rc < 0 then call Error_exit "Connection failed: "odbc_error_message(), "Full diagnostics: "odbc_get_diagnostics()
  dbinfo = odbc_get_info()
  say "Connected to database system:" dbinfo
/* -----------------------------------------------------------------
 * Explicitly set the database
 * -----------------------------------------------------------------
 */
  say "set/change Database..."
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
say  "SELECT * FROM "table"..."
  rc = odbc_execute("SELECT * FROM "table)
  if rc < 0 then call error_exit "Query failed: "odbc_error_message(),""
  /* Get row count */
  say "get row count..."
  row_count = odbc_row_count()
  say "Total rows:" row_count
```

Note that we have the choice of using the ODBC[^3] api for our database, which makes already for a robust architecture for an application, or we can *script* the access to the data by ```address```ing the database's shell program - it is up to you to make this choice; what is shown here, is that with one import a whole set of odbc functions is added to our arsenal, as easy as the standard \rexx{} functions. These will work with any database system that has an ODBC driver, even spreadsheets.

From the way of specifying the import, we cannot see if these function packages are made in \rexx{}, C or another programming languages. You can write your own libraries in \rexx{} or another language, and they can be distributed as binaries and linked into \crexx{} programs by specifying them on an ```import``` statement.[^4]

[^4]: The specifics of this are in the reference chapter, while the mechanics are shown in the Programming Guide and the inner workings in the VM Specification.

[^3]: Open DataBase Connectivity
