options levelb
/* d2b( decimal to bit string)  returns bit combination of decimal string */

d2b: procedure = .string
   arg dec = .int
   hex=d2x(dec)
   bit=x2b(hex)
return bit

x2b: procedure = .string
  arg hex = .string, blen=-1

d2x: procedure = .string
  arg int1 = .int, slen = -1


