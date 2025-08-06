/* rexx */
options levelb

namespace rxfnsb expose fnv

fnv: procedure = .int
arg input_value = .string

len=0
hashr=0

assembler strlen len,input_value
assembler rxhash hashr,input_value,len
return hashr