/* REXX Levelb VERSION Implementation */
options levelb

namespace rxfnsb expose version

version: procedure = .string

string=''
assembler rxvers string

return string