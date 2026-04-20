/* rexx */
options levelb

namespace rxfnsb expose _copies

_copies: procedure=.string
  arg cstring=.string, count=.int
  result=''
  do i = 1 to count
     result = result || cstring
  end
return result
