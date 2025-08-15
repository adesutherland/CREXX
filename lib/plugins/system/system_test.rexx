/* RXPP */
/* ----------------------------------------------------------------------
 * PRE Compiled on 15 Aug 2025  at 15:04:22
 * ----------------------------------------------------------------------
 */
options levelb
import rxfnsb
import system
/* ##cflags def nset niflink 1buf 2buf 3buf parse nvars nmaclist includes  /* set early stage compiler flags */ */
##                                       nparse to switch off the parse debug information
/* 1. split string at ',' delimiter */
say "1. split string at ',' delimiter"
string='11,2222,3333,44444,555555'
/* rxpp: cparse(string,"first','second','third','fourth','fifth','six") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=string; parsetemplate="first','second','third','fourth','fifth','six" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : string"
say "#PARSE TEMPLATE: first','second','third','fourth','fifth','six"
## ---------- set parse variables ----------
first=_pass_variable_content.1
second=_pass_variable_content.2
third=_pass_variable_content.3
fourth=_pass_variable_content.4
fifth=_pass_variable_content.5
six=_pass_variable_content.6
## ---------- parse variables set ----------
say '1. parm 'first
say '2. parm 'second
say '3. parm 'third
say '4. parm 'fourth
say '5. parm 'fifth
say '6. parm "'six'"'
/* rxpp: cparse(string,"first','second','third','fourth','fifth','six") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=string; parsetemplate="first','second','third','fourth','fifth','six" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : string"
say "#PARSE TEMPLATE: first','second','third','fourth','fifth','six"
## ---------- set parse variables ----------
first=_pass_variable_content.1
second=_pass_variable_content.2
third=_pass_variable_content.3
fourth=_pass_variable_content.4
fifth=_pass_variable_content.5
six=_pass_variable_content.6
## ---------- parse variables set ----------
say '1. parm 'first
say '2. parm 'second
say '3. parm 'third
say '4. parm 'fourth
say '5. parm 'fifth
say '6. parm "'six'"'
/* 1. split string at ',' delimiter */
say "1. split string at ',' delimiter"
string='11,2222,3333,44444,555555'
/* rxpp: cparse(string,"first','second','third','fourth','fifth','six") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=string; parsetemplate="first','second','third','fourth','fifth','six" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : string"
say "#PARSE TEMPLATE: first','second','third','fourth','fifth','six"
## ---------- set parse variables ----------
first=_pass_variable_content.1
second=_pass_variable_content.2
third=_pass_variable_content.3
fourth=_pass_variable_content.4
fifth=_pass_variable_content.5
six=_pass_variable_content.6
## ---------- parse variables set ----------
say '1. parm 'first
say '2. parm 'second
say '3. parm 'third
say '4. parm 'fourth
say '5. parm 'fifth
say '6. parm "'six'"'
/* rxpp: cparse(string,"first','second','third','fourth','fifth','six") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=string; parsetemplate="first','second','third','fourth','fifth','six" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : string"
say "#PARSE TEMPLATE: first','second','third','fourth','fifth','six"
## ---------- set parse variables ----------
first=_pass_variable_content.1
second=_pass_variable_content.2
third=_pass_variable_content.3
fourth=_pass_variable_content.4
fifth=_pass_variable_content.5
six=_pass_variable_content.6
## ---------- parse variables set ----------
say '1. parm 'first
say '2. parm 'second
say '3. parm 'third
say '4. parm 'fourth
say '5. parm 'fifth
say '6. parm "'six'"'
/* 2. split string at '-' delimiter */
say "2. split string at '-' delimiter"
s = '2025-08-14'
/* rxpp: cparse(s,"year '-' . '-' day") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=s; parsetemplate="year '-' . '-' day" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : s"
say "#PARSE TEMPLATE: year '-' . '-' day"
## ---------- set parse variables ----------
year=_pass_variable_content.1
day=_pass_variable_content.2
## ---------- parse variables set ----------
say year month day   ## 2025 08 14
say " "
/* 3. split string at ';' delimiter */
say "3. split string at ';' delimiter"
s = 'host=example.com;port=5432;user=alice'
/* rxpp: cparse(s,"'host=' host ';' 'port=' port ';' 'user=' user") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=s; parsetemplate="'host=' host ';' 'port=' port ';' 'user=' user" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : s"
say "#PARSE TEMPLATE: 'host=' host ';' 'port=' port ';' 'user=' user"
## ---------- set parse variables ----------
host=_pass_variable_content.1
port=_pass_variable_content.2
user=_pass_variable_content.3
## ---------- parse variables set ----------
say host port user   ## example.com 5432 alice
say " "
/* 4. split string at certain position */
say "4. split string at certain position"
s = 'abcdef'
/* rxpp: cparse(s,"a 3 b 5 c") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=s; parsetemplate="a 3 b 5 c" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : s"
say "#PARSE TEMPLATE: a 3 b 5 c"
## ---------- set parse variables ----------
a=_pass_variable_content.1
b=_pass_variable_content.2
c=_pass_variable_content.3
## ---------- parse variables set ----------
say a b c        ## ab cd ef
say " "
/* 5. split string at relative position */
say "5. split string at relative position"
s = 'abcdef'
/* rxpp: cparse(s,"a +2 b +2 c") */
_pass_variable.1='' ; _pass_variable_content.1='' ; string2Parse=s; parsetemplate="a +2 b +2 c" ; call parse string2parse,parsetemplate,_pass_variable,_pass_variable_content ;
say "#PARSE STRING  : s"
say "#PARSE TEMPLATE: a +2 b +2 c"
## ---------- set parse variables ----------
a=_pass_variable_content.1
b=_pass_variable_content.2
c=_pass_variable_content.3
## ---------- parse variables set ----------
say a b c        ## ab cd ef
say " "
exit
