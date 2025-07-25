/* rexx */
options levelb

namespace rxfnsb expose right

right: procedure = .string
  arg rstring = .string, rlen = .int, pad = ' '

  if rlen = 0 then return ''

  slen=0
  assembler strlen slen,rstring
  if rlen = slen then return rstring

  plen = 1
  padchar = ''

  assembler substcut pad, plen              /* Ensure pad is one char */
  assembler strchar padchar, pad            /* Get UTF-8 codepoint */

  diff = rlen - slen                        /* is left padding necessary? */
  if diff > 0 then do                       /* yes */
     padstring = ''
     assembler padstr padstring, padchar, diff /* create left padding string */
     return padstring||rstring
  end
return substr(rstring, slen + 1 - rlen, rlen)
