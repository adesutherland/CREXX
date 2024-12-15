/* rexx test abs bif */
options levelb
import rxfnsb

errors=0
/*                     12345  */
x='the quick brown fox jumps over the lazy dog'

if changestr('quick',x,'fast') \= 'the fast brown fox jumps over the lazy dog' then do
  errors=errors+1
  say 'CHANGESTR failed in test 1'
end


if changestr('the',x,'a') \= 'a quick brown fox jumps over a lazy dog' then do
  errors=errors+1
  say 'CHANGESTR failed in test 2'
end
  

if changestr('quicker',x,'fast') \= 'the quick brown fox jumps over the lazy dog' then do
  errors=errors+1
  say 'CHANGESTR failed in test 3'
end

return errors<>0
