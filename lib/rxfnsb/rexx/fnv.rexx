/* rexx */
options levelb

namespace rxfnsb expose fnv

fnv: procedure = .string
arg input_value = .string

len=0
hashr=""

assembler strlen len,input_value
assembler rxhash hashr,input_value,len

return hashr