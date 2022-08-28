/* rexx */
options levelb

namespace rxfnsb

/* translate  */

translate: procedure = .string
  arg source = .string, tochar = "?????", fromchar = "?????",pad=" "

  slen=0
  flen=0
  result=""
  schar=0

  /* !!! fromchar or tochar containing '?????' mean it is not set, while '' mean it is empty */
  if fromchar="?????" & tochar="?????" then return upper(source)
  if fromchar="" & tochar="" then return source

  assembler strlen slen,source                 /* get length of souce string     */
  if fromchar="?????" & tochar="" then return left("",slen,pad)

  assembler strlen flen,fromchar               /* get length of from list        */

  tochar=left(tochar,flen,pad)                 /* from and to list must be equal */

  do i=0 to slen-1                             /* run through from source string */
     assembler strchar schar,source,i
     assembler transchar schar,tochar,fromchar /* translate source char (if necessary) */
     assembler appendchar result,schar         /* append it to result  */
  end

return result
