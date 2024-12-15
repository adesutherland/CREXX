/* rexx */
options levelb

namespace rxfnsb expose length

/* Length() Procedure */
length: procedure = .int
  arg expose string1 = .string /* Pass by reference - as a cludge we can mark it pass by value in the declaration */
  /* TODO - We need to have a better way to make the compiler know string 1 is a constant despite the
     assembler statement */
  result = 0
  assembler strlen result,string1
  return result
