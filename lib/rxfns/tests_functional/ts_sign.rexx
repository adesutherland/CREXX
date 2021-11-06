/* rexx */
options levelb
say sign(3.14)
say sign("3.14")

say sign(-3.14)
say sign(-"3.14")

say sign(0)
say sign("0")

return

sign: procedure = .int
  arg expose number = .float

