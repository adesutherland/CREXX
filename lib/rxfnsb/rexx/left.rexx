/* rexx */
options levelb

namespace rxfnsb expose left

left: procedure=.string
  arg lstring=.string, leftlen=0, pad=' '
  llen=0
  assembler strlen llen,lstring

  if leftlen<=0 | llen=0 then return ''     /* one of the length is zero, return empty string */
  if leftlen = llen then return lstring     /* no length change, return original */
  if leftlen < llen then do                 /* requested length is < then string length, just cut it off */
     assembler substcut lstring,leftlen     /* just cut off string */
     return lstring                         /* and return it */
  end
 /* left length > string length, this needs padding */
  plen=1
  padchar=''
  padstring=''
  assembler substcut pad,plen              /* ensure pad is just one char */
  assembler strchar padchar, pad           /* padchar= utf8codepoint(pad) */
  leftlen=leftlen-llen                     /* how many padding chars needed */
  assembler padstr padstring, padchar, leftlen  /* create pad string */
return lstring||padstring                  /* return padded string */
