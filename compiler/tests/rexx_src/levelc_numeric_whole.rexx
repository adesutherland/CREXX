options levelc
say 'defaults' digits() fuzz() form()
numeric digits 4
say 'arithmetic' 1234567+0 1.20+0 1.20*1.0 1.20/1
say 'format' format(1234567) trunc(1.23456,2)
say 'bifs' abs(-12345) abs('-12345') abs(12345) max(1.234,1.235) min(1.234,1.235) sign(-1)
numeric form engineering
say 'engineering' 12345+0 0.00000012345+0 form()
say 'engineering-bif' abs('-1.20E+4') abs('000123E+4')
numeric fuzz 1
say 'fuzz' fuzz() (1.234=1.235)
say 'bif-fuzz' max(1.234,1.233) min(1.234,1.233)
call child
say 'restored' digits() fuzz() form()
numeric digits
numeric fuzz
numeric form scientific
say 'reset' digits() fuzz() form()
formword='engineering'
numeric form value formword
say 'form-value' form()
numeric form scientific
digitword='+000000000000000000000000000000000000007.0'
numeric digits digitword
say 'leading-digits' digits()
digitword='9.99e2'
numeric digits digitword
say 'max-digits' digits()
numeric digits 9
count=0
numeric digits nextdigits()
say 'once' count digits()
if 1 then numeric digits 7
say 'nested' digits()
numeric digits 5
call recur 2
say 'recur-restored' digits()
exit
child: procedure
say 'inherited' digits() fuzz() form()
numeric digits 6
numeric fuzz 2
numeric form scientific
say 'child' digits() fuzz() form()
return
nextdigits:
count=count+1
return 5
recur: procedure
parse arg depth
say 'recur-enter' depth digits()
if depth > 0 then do
  numeric digits digits()-1
  call recur depth-1
  say 'recur-return' depth digits()
end
return
