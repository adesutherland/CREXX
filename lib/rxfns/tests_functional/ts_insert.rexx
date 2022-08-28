/* rexx test abs bif */
options levelb
import rxfnsb

errors=0

x='CREXX is faster than BREXX'

if insert('much',x,10,10) \= 'CREXX is much      faster than BREXX' then do
  errors=errors+1
  say 'INSERT failed in test 1' insert('much',x,10,10)
end

if insert('much ',x,10) \= 'CREXX is much faster than BREXX' then do
  errors=errors+1
  say 'INSERT failed in test 2'
end

if insert('The new ',x,1) \= 'The new CREXX is faster than BREXX' then do
  errors=errors+1
  say 'INSERT failed in test 3'
end

if insert(' ,isn"t it?',x,27) \= 'CREXX is faster than BREXX ,isn"t it?' then do
  errors=errors+1
  say 'INSERT failed in test 4'
end

return errors<>0
