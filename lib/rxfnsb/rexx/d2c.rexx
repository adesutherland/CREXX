/* rexx center text  */
options levelb

namespace rxfnsb expose d2c

d2c: procedure = .string
  arg from = .int, slen=-1
  if slen=0 then return ''
  if slen > 1 | slen < -1 then do
     assembler signal "CONVERSION_ERROR", "D2C supports only omitted length, 0, or 1 with Unicode semantics"
     return ''
  end
  if from < 0 | from > 1114111 | (from >= 55296 & from <= 57343) then do
     assembler signal "CONVERSION_ERROR", "D2C invalid Unicode code point"
     return ''
  end
  xstr=''
  assembler appendchar xstr,from
 return xstr
