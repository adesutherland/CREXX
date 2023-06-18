/* rexx */
options levelb

namespace rxfnsb expose filter

filter: procedure = .string
arg input = .string, filter = .string

strout=""

assembler dropchar strout,input,filter

return strout