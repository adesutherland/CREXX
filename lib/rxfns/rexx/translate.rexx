/* rexx */
options levelb
/* translate  */

translate: procedure = .string
  arg expose source = .string, tochar = "", fromchar = "",pad=" "

  slen=0
  flen=0
  result=""
  schar=0

  if fromchar="" & tochar="" then return upper(source)

  assembler strlen slen,source                 /* get length of souce string     */
  assembler strlen flen,fromchar               /* get length of from list        */

  tochar=left(tochar,flen,pad)                 /* from and to list must be equal */

  do i=0 to slen-1                             /* run through from source string */
     assembler strchar schar,source,i
     assembler transchar schar,tochar,fromchar /* translate source char (if necessary) */
     assembler appendchar result,schar         /* append it to result  */
  end

return result

/* Prototypes */
   left: procedure = .string
         arg string = .string, length1 = .int, pad = ' '
   upper: procedure = .string
         arg expose string1 = .string



