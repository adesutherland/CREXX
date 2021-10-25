/* rexx */
options levelb

say trunc(3.1415926,0)
say trunc(3.1415926,1)
say trunc(3.1415926,2)
say trunc(3.1415926,3)
say trunc(3141.5926,12)

return

/* trunc()  */
trunc: procedure = .string
  arg number = .float, fraction = 0
