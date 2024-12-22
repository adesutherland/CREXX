/* GETPI Plugin Test */
options levelb
import arrays
import fileio

import rxfnsb
files.1=""
dirs.1=""
say readdir(files,dirs,"c:\temp\")
Say "Read Temp Directory"
do i=1 to files.0
   say "file "files.i
end
do i=1 to dirs.0
   say "DIR  "dirs.i
end


array.1=""

say readall(array,"c:\temp\test.rexx",11)
do i=1 to array.0
   say right(i,6) array.i
end
say 'now write it'
say writeall(array,"c:\temp\testrx2.rexx",9)
say appendall(array,"c:\temp\testrx2.rexx",9)
say appendall(array,"c:\temp\testrx2.rexx",9)
