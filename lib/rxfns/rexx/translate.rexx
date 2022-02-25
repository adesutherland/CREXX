/* rexx */
options levelb
/* translate  */

translate: procedure = .string
  arg source = .string, tochar = "?????", fromchar = "?????",pad=" "

  slen=0
  flen=0
  tlen=0
  result=""
  schar=0

  /* !!! fromchar or tochar containing '?????' mean it is not set, while '' mean it is empty */
  if pad=' ' then do
     if fromchar="?????" & tochar="?????" then return upper(source)
     if fromchar="" & tochar="" then return source
  end
  assembler strlen slen,source                 /* get length of souce string     */
  if fromchar="?????" then do
     if tochar="?????" then  return copies(pad,slen)
     if tochar="" then return copies(pad,slen)
     fromchar=" "
  end
  if tochar="?????" then tochar=" "
  if fromchar="?????" then fromchar=" "

  assembler strlen flen,fromchar               /* get length of from list        */
  assembler strlen tlen,tochar                 /* get length of to   list        */
  if flen>tlen then tochar=left(tochar,flen,pad)  /* from and to list must be equal */
  else if tlen>flen then fromchar=left(fromchar,tlen,pad)

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
   copies: procedure = .string
     arg string1 = .string, count = .int



