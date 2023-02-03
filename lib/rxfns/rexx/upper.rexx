/* rexx */
options levelb

namespace rxfnsb expose upper

/* upper(string) translate to upper cases */
upper: procedure = .string
  arg expose string = .string
  newstr=""
  assembler strupper newstr,string
return newstr
