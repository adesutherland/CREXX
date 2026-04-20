/* rexx */
options levelb

namespace rxfnsb expose _upper

/* upper(string) translate to upper cases */
_upper: procedure = .string
  arg expose string = .string
  newstr=""
  assembler strupper newstr,string
return newstr
