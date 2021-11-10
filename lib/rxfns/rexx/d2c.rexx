/* rexx center text  */
options levelb

d2c: procedure = .string
  arg from = .int, slen=-1

  xstr=d2x(from)
return x2c(xstr)

x2c: procedure = .string
  arg hex = .string

d2x: procedure = .string
  arg xint = .int, slen=-1
