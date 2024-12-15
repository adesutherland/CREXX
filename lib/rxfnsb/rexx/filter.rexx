/* rexx */
options levelb

namespace rxfnsb expose filter

filter: procedure = .string
arg input_value = .string, filter = .string

strout=""

assembler dropchar strout,input_value,filter

return strout