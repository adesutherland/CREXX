/* rexx */
options levelb

namespace rxfnsb expose copies

copies: procedure=.string
  arg cstring=.string, count=.int
  result=''
  do i = 1 to count
     result = result || cstring
  end
return result