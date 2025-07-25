/* rexx test abs bif */
options levelb
import rxfnsb

/* RXVM ts_format format right left _itrunc _ftrunc copies pos substr length */
/* TODO */
errors=0

say 'format(-31415921111111,,4,1,1) "'format(-31415921111111.6,,4,1,1)'"'
say 'format(-31415.92111111,,4,1,1) "'format(-31415.9211111116,,4,5,1)'"'
/* Samples taken from the Regina manual, some seem wrong */
say "FORMAT(12.34,3,4) "FORMAT(12.34,3,4) ' 12.3400'
say "FORMAT(12.34,3,,3,0) "FORMAT(12.34,3,,3,0) ' 1.234E+001'
say "FORMAT(12.34,3,1) "FORMAT(12.34,3,1) ' 12.3400'
say "FORMAT(12.34,3,0) "FORMAT(12.34,3,0) ' 12.3' "why one fractional part, after say 0"
say "FORMAT(12.34,3,4) "FORMAT(12.34,3,4) ' 12'   "why no fractional part, after say 4"
say "FORMAT(12.34,,,,0) "FORMAT(12.34,,,,0) '1.234E+1'
say "FORMAT(12.34,,,0) "FORMAT(12.34,,,0) '12.34'
say "FORMAT(12.34,,,0,0) "FORMAT(12.34,,,0,0)"" '12.34'

say 'format(-3.14,,4) "'format('-3.14',,4)'"'
say 'format(-3.14,4,3) "'format('-3.14',4,3)'"'
say 'format(-3.14,1,4) "'format('-3.14',1,4)'"'

/* walter's latest */
_="    -0.05625000000000000000"
Say '_='_
Say 'format(_/1, 6)' format(_/1, 6)
Say 'format(_+0, 6)' format(_+0, 6)
Say 'format(_  , 6)' format(_  , 6)
_="     0.05625000000000000000"
Say 'format(_/1, 6)' format(_/1, 6)
Say 'format(_+0, 6)' format(_+0, 6)
Say 'format(_  , 6)' format(_  , 6)

return 0
