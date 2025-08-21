/* rexx */
options levelb

namespace rxfnsb expose qwords

qwords: procedure=.int
  arg line=.string
  do i=1 to 99999999
     if qword(line,i)='' then return i-1
  end
return 0