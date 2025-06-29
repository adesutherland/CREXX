/* rexx */
options levelb

namespace rxfnsb expose left

left: procedure=.string
  arg lstring=.string, leftlen=.int, pad=' '
  if leftlen=0 then return ''
  pad=substr(pad||' ',1,1)                    /* make sure there is at least one char available */
  padded = lstring || copies(pad, leftlen)    /* Ensure it's long enough */
return substr(padded, 1, leftlen)             /* Extract exactly 'length' chars */
