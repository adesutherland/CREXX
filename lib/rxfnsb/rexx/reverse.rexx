/* rexx */
options levelb

namespace rxfnsb expose reverse


reverse: procedure=.string
  arg str=.string
  len = length(str)
  rev = ''

  do i = len to 1 by -1
    rev = rev || substr(str, i, 1)
  end

return rev