/* rexx test abs bif */
options levelb
import rxfnsb

say "Test MIN"
errors=0

if min(100,99,1) \= 1 then do
  errors=errors+1
  say 'MIN failed in test 1'
end

if min(47,42,129) \= 42 then do
  errors=errors+1
  say 'MIN failed in test 2'
end

if min(1,9,10) \= 1 then do
  errors=errors+1
  say 'MIN failed in test 3'
end

say "Test MAX"
if  max(100,99,1) \= 100 then do
  errors=errors+1
  say 'MAX failed in test 1'
end

if  max(47,42,129) \= 129 then do
  errors=errors+1
  say 'MAX failed in test 2'
end

if max(1,9,10) \= 10 then do
  errors=errors+1
  say 'MAX failed in test 3'
end

return errors<>0
