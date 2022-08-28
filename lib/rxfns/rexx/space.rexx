/* rexx space adds n padding chars between words */
options levelb

namespace rxfnsb

space: procedure = .string
  arg expose string = .string, spacenr = 1,  pad = " "

wrds=words(string)
if wrds<2 then return word(string,1)

padstr=""
newstr=""
offset=0
cpad=""
/* make sure just to take first char */
assembler strchar cpad,pad,offset
assembler load pad,""
assembler appendchar pad,cpad

/* create padding string */
do i=1 to spacenr
   padstr=padstr||pad
end

/* add padding string between words */
do i=1 to wrds-1
   newstr=newstr||word(string,i)||padstr
end
/* add last word */
newstr=newstr||word(string,wrds)

return newstr
