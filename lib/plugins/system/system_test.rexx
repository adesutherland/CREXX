/* System Plugin Test */
options levelb
import system
import rxfnsb

say 'GetGlobal  1 'getGlobal("fox")
say 'GetGlobal  1 'getGlobal("dog")
say 'SetClipboard 'setclipboard("Hello my dear clipboard")
say 'GetClipboard 'getclipboard()
say 'SetClipboard 'setclipboard("The quick brown fox jumps over the lazy dog")
say 'GetClipboard 'getclipboard()
say 'SetClipboard 'setclipboard("The quick brown fox jumps over the lazy dog")
say 'SetGlobal    'setGlobal("fox","The quick brown fox jumps over the lazy dog")
say 'SetGlobal    'setGlobal("dog","sleeps in the sun")
say 'GetGlobal  2 'getGlobal("fox")
say 'GetGlobal  2 'getGlobal("dog")
exit
variable.1='debug'
value.1=''
call parse '"("in brackets")" ',  ,
           '"("parm")"  fred',,
           variable,value
do i=1 to variable.0
   say variable.i"='"value.i"'"
end
/*
call parse 'any1"("in brackets")" nobody-knows-value":1:" belongs nowhere ',  ,   ## string to parse  //last comma is continuation char
           'v1"("parm")"xx',,   ## parse string     //last comma is continuation char
           variable,value       ## receving array: variable content value
do i=1 to variable.0
   say variable.i"='"value.i"'"
end
say copies("*",80)
call parse 'value-for-v1"<"parm-Value">"x-value',  ,   ## string to parse  //last comma is continuation char
           'v1"<"parm">"xx"="xyz',,   ## parse string     //last comma is continuation char
           variable,value      # receving array: variable content value
do i=1 to variable.0
   say variable.i"='"value.i"'"
end
do i=1 to variable.0
   say variable.i"='"value.i"'"
end

exit                                 ##0123another-valuenobody-knows-value123another-valuenobody-knows-value123another-value89
## ----------------------------------------------------
## Step 1: Create some new directories
## ----------------------------------------------------
rc=createdir("c:/Temp/crexxTEST")
  if rc=0 then say "Directory created"
  else if rc=-4 then say "Directory already exists"
  else say "Directory created failed"
rc=createdir("c:/Temp/crexxTEST/Data")
  if rc=0 then say "Directory created"
  else if rc=-4 then say "Directory already exists"
  else say "Directory created failed"
rc=createdir("c:/Temp/crexxTEST/Data/etc")
  if rc=0 then say "Directory created"
  else if rc=-4 then say "Directory already exists"
  else say "Directory created failed"
## ----------------------------------------------------
## Step 2: add new files
## ----------------------------------------------------
do i=1 to 10
   array.i='record 'i
end
say writeall(array,"c:/Temp/crexxTEST/Data/test1.txt",-1)' Records Written'
say writeall(array,"c:/Temp/crexxTEST/Data/test2.txt",-1)' Records Written'
say renameFile("c:/Temp/crexxTEST/Data/test1.txt","c:/Temp/crexxTEST/Data/test99.txt")' renamed'
## ----------------------------------------------------
## Step 3: List directory content
## ----------------------------------------------------
entries.1=''   ## init an array
call listdir "c:/Temp/crexxTEST/Data",entries
say 'directory of c:/Temp/crexxTEST/Data'
do i=1 to entries.0
  say entries.i
end
say 'directory contains 'entries.0' entries'
say 'prefix meaning'
say '>    directory entry'
say '+    file entry'
## ----------------------------------------------------
## Step 4: remove content of the directory
## ----------------------------------------------------
do i=1 to entries.0
  entry=substr(entries.i,3)
  type=substr(entries.i,1,1)
  if type='>' then say 'Delete RC of "'entry'" 'RemoveDir("c:/Temp/crexxTEST/Data/"entry)
  if type='+' then say 'Delete RC of "'entry'" 'DeleteFile("c:/Temp/crexxTEST/Data/"entry)
end
## ----------------------------------------------------
## Step 5: now this directory it is empty, delete it
## ----------------------------------------------------
say 'Delete RC of then "Data" dir 'RemoveDir("c:/Temp/crexxTEST/Data")
## ----------------------------------------------------
## Step 6: List directory content
## ----------------------------------------------------
new.1=''   ## init an array
call listdir "c:/Temp/crexxTEST",new
say 'directory of c:/Temp/crexxTEST'
do i=1 to new.0
  say new.i
end
say 'directory contains 'new.0' entries'

exit
*/

/* addition functions
  say "getEnv(PATH) "getEnv("PATH")
  say "getDIR       "getdir()
  say "setDIR(C:/temp) "setdir("c:/Temp")
  say "getDIR       "getdir()
*/
