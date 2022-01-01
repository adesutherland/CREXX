/* rexx */
options levelb
/* returns sign of number  */

sign: procedure = .int
  arg number = .float
  if number>0 then return 1
  if number<0 then return -1
return 0

