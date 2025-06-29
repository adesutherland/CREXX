/* rexx */
options levelb

namespace rxfnsb expose right

right: procedure=.string
  arg rstring=.string, rlen=.int, pad=' '
  if rlen=0 then return ''
  pad=substr(pad||' ',1,1)                    /* make sure there is at least one char available */
  padded = copies(pad, rlen)||rstring         /* Pad on the left */
return substr(padded, length(rstring) + 1, rlen)
