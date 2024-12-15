/* rexx */
options levelb
import rxfnsb

errors=0

if upper("The quick brown fox jumps over the lazy dog") \= 'THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG' then do
  errors=errors+1
  say 'UPPER failed in test      1'
end

if upper("é") \= 'É' then do
  errors=errors+1
  say 'UPPER failed in test      2'
end

return errors<>0
