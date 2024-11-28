/* GETPI Plugin Test */
options levelb
import arrays
import fileio

import rxfnsb
files.1=""
dirs.1=""
say readdir(files,dirs,"c:\temp\")
do i=1 to files.0
   say "file "files.i
end
do i=1 to dirs.0
   say "DIR  "dirs.i
end
exit
/*
say readall(array,"c:\temp\test.rexx",11)
do i=1 to array.0
   say i array.i
end
say 'now write it'
say writeall(array,"c:\temp\testrx2",9)
*/