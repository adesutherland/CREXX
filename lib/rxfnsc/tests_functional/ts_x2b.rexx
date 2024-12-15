/* X2B */
options levelb
import rxfnsb

errors=0
/* These from the Rexx book. */
if x2b('C3') \= '11000011' then do
  errors=errors+1
  say 'X2B failed in test 1 '
end
if x2b('7') \= '0111' then do
  errors=errors+1
  say 'X2B failed in test 2 '
end
if x2b('1 C1') \= '000111000001' then do
  errors=errors+1
  say 'X2B failed in test 3 '
end
if x2b('C3') \= '11000011' then do
  errors=errors+1
  say 'X2B failed in test 4 '
end
if x2b(d2x('129')) \= '10000001' then do
  errors=errors+1
  say 'X2B failed in test 5 '
end
if x2b(d2x('12')) \= '1100' then do
  errors=errors+1
  say 'X2B failed in test 6 '
end
/* These from Mark Hessling. */
if x2b("416263") \= "010000010110001001100011" then do
  errors=errors+1
  say 'X2B failed in test 7 '
end
if x2b("DeadBeef") \= "11011110101011011011111011101111" then do
  errors=errors+1
  say 'X2B failed in test 8 '
end
if x2b("1 02 03") \= "00010000001000000011" then do
  errors=errors+1
  say 'X2B failed in test 9 '
end
if x2b("102 03") \= "00010000001000000011" then do
  errors=errors+1
  say 'X2B failed in test 10 '
end
if x2b("102") \= "000100000010" then do
  errors=errors+1
  say 'X2B failed in test 11 '
end
if x2b("11 2F") \= "0001000100101111" then do
  errors=errors+1
  say 'X2B failed in test 12 '
end
if x2b("") \= "" then do
  errors=errors+1
  say 'X2B failed in test 13 '
end
return errors<>0
