/* rexx */
options levelb

namespace rxfnsb expose _lower

/* upper(string) translate to upper cases */
_lower: procedure = .string
  arg expose string = .string
  newstr=""
  assembler strlower newstr,string
return newstr
