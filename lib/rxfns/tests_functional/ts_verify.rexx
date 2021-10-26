/* rexx */
options levelb

say verify('123456','0123456789')
say verify('12w36','0123456789')
say verify('12w36','0123456789',,4)
say verify('a12w36','0123456789','M')

return

/* verify()  */
verify: procedure = .int
  arg instring = .string, intab = .string, match = 'N',pos=1
