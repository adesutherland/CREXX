/* rexx test center bif */
options levelb
import rxfnsb

errors=0
/* These from TRL */

if  '  ABC  ' \= centre(abc,7)             then do
  errors=errors+1
  say 'CENTER failed in test 1 '
end

if center(abc,7) \= '  ABC  '              then do
  errors=errors+1
  say 'CENTER failed in test 2 '
end

if center(abc,8,'-') \= '--ABC---'         then do
  errors=errors+1
  say 'CENTER failed in test 3 '
end

if center('The blue sky',8) \= 'e blue s'  then do
  errors=errors+1
  say 'CENTER failed in test 4 '
end

if center('The blue sky',7) \= 'e blue '   then do
  errors=errors+1
  say 'CENTER failed in test 5 '
end

/* These from Mark Hessling. */
if center('****',8,'-')      \='--****--'   then do
  errors=errors+1
  say 'CENTER failed in test 6 '
end

if center('****',7,'-')      \='-****--'    then do
  errors=errors+1
  say 'CENTER failed in test 7 '
end

if center('*****',8,'-')     \='-*****--'   then do
  errors=errors+1
  say 'CENTER failed in test 8 '
end

if center('*****',7,'-')     \='-*****-'    then do
  errors=errors+1
  say 'CENTER failed in test 9 '
end

if center('12345678',4,'-')  \='3456'       then do
  errors=errors+1
  say 'CENTER failed in test 10 '
end

if center('12345678',5,'-')  \='23456'      then do
  errors=errors+1
  say 'CENTER failed in test 11 '
end

if center('1234567',4,'-')   \='2345'       then do
  errors=errors+1
  say 'CENTER failed in test 12 '
end

if center('1234567',5,'-')   \='23456'      then do
  errors=errors+1
  say 'CENTER failed in test 13 '
end

/* 1234567890123456789012345678901234567890123 */
x='the quick brown fox jumps over the lazy dog'
if center(x,43,) \= 'the quick brown fox jumps over the lazy dog ' then do
  errors=errors+1
  say 'CENTER failed in test 14 '
end

say '|'center(x,43,)'|'
say "'"center(x,50,"!")"'"
say "'"center(x,9)"'"

say "'"centre(x,43,)"'"
say "'"centre(x,50,"!")"'"
say "'"centre(x,9)"'"

return 0
