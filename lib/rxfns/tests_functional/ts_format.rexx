/* rexx test abs bif */
options levelb
/* RXVM ts_format format right left _itrunc _ftrunc copies pos substr length */

say "test Format"
say 'format(-3.14,,4) "'format('-3.14',,4)'"'
say 'format(-3.14,4,3) "'format('-3.14',4,3)'"'
say 'format(-3.14,1,4) "'format('-3.14',1,4)'"'

return

format: procedure = .string
   arg innum = .string, before = 0, after = 0, expp = 0, expt=0
