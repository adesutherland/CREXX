/* rexx center text  */
/* CENTER and CENTRE are exact copies, any change must be reflected in both functions */

options levelb
center: procedure = .string
  arg expose string = .string, centlen = .int,  pad = " "

padstr=""
offset=0
slen=0
cpad=""

/* make sure just to take first char */
assembler strchar cpad,pad,offset
assembler load pad,""
assembler appendchar pad,cpad
/* calculate number of pad chars to added as prefix and suffix */
assembler strlen slen,string
padlen=centlen-slen
assembler idiv padlen,padlen,2
if padlen=0 then return string   /* if nothing to add return original string */

if padlen<0 then newstr=substr(string,-padlen+1,centlen," ")
else do
/* create padding string */
   padstr=copies(pad,padlen)
   newstr=padstr||string||padstr
   assembler strlen slen,newstr
   if slen<centlen then newstr=newstr||pad  /* in case of uneven center length */
end

return newstr

/* copies()  create copies of a char/string */
copies: procedure = .string
  arg string1 = .string, count = .int

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '