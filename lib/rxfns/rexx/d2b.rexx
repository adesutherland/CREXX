options levelb
/* d2b( decimal to bit string)  returns bit combination of decimal string */

namespace rxfnsb

d2b: procedure = .string
   arg dec = .int
   hex=d2x(dec)
   bit=x2b(hex)
return bit
