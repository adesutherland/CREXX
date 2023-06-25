/* rexx */
options levelb

namespace rxfnsb expose fnv

fnv: procedure = .string
arg input = .string

len=0
hashr=""

assembler strlen len,input
assembler rxhash hashr,input,len

return hashr