/* System Plugin Test */
options levelb
import system
import rxfnsb
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

/* addition functions
  say "getEnv(PATH) "getEnv("PATH")
  say "getDIR       "getdir()
  say "setDIR(C:/temp) "setdir("c:/Temp")
  say "getDIR       "getdir()
*/
