/* rexx */
options levelb
/* Length() Procedure */
length: procedure = .int
  arg expose string1 = .string /* Pass by reference */
  result = 0
  assembler strlen result,string1
  return result
