/* rexx center text  */
/* CENTER and CENTRE are exact copies, any change must be reflected in both functions */
/* todo fix this bit of ugliness when crexx allows */
options levelb

namespace rxfnsb

centre: procedure = .string
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
