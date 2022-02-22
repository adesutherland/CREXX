/* rexx test abs bif */
options levelb
errors=0
/*                     12345  */
x='the quick brown fox jumps over the lazy dog'

if changestr('quick',x,'fast') \= 'the fast brown fox jumps over the lazy dog' then do
  errors=errors+1
  say 'CHANGESTR failed in test 1'
end


if changestr('the',x,'a') \= 'the quick brown fox jumps over a lazy dog quick brown fox jumps over a lazy dog' then do
  errors=errors+1
  say 'CHANGESTR failed in test 2'
end
  

if changestr('quicker',x,'fast') \= 'the quick brown fox jumps over the lazy dog' then do
  errors=errors+1
  say 'CHANGESTR failed in test 3'
end

return errors<>0

/* function prototype */
changestr: procedure = .string
  arg string1 = .string, string2 = .string, string3 = .string