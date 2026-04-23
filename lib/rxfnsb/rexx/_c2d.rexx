/* rexx center text  */
options levelb
namespace rxfnsb expose _c2d

_c2d: procedure = .int
  arg from = .string
  len=0
  offset=0
  code=0
  assembler strlen len,from
  if len \= 1 then do
      assembler signal "CONVERSION_ERROR", "C2D expects exactly one character"
      return 0
  end
  assembler strchar code,from,offset
  return code

