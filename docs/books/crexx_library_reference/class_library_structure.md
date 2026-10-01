# Class library structure

As mentioned, the intention is to leave the naming structure relatively flat. The reason for this is to limit the need
for convoluted import statements which are hard to remember and take up a lot of space in program source files. It should be easy to remember which part of the library to include and the decision to determine from which part of the library a specific class is should be straightforward. The currently active parts are:

|Namespace   |Domain   | Example Classes  |
|---|---|---|
|System  | OS related functionality; things that are dependent or specific for the OS   |beep, wait, listdir,  |
|Data | Collection classes and data interchange (I/O and formats)  | Sets, Maps, Lists, Stems, Arrays, json, xml, yaml, ODBC|
|Time | Time, Date, Calender oriented functionality  | Time, Date, Calendar, ISO Date, Locale Calendars  |
|Math | Mathematics functions (Trigonometry, Statistics, Conversions)  | sin, cos, mean, etc.  |
|GUI  | Graphical User Interfaces, Text based or bitmapped  | curses, GTK  |

Table: Library Namespaces. {#tbl:id}

