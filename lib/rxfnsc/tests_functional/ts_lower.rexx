/* rexx */
options levelb
import rxfnsb

errors=0
if lower("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG") \= "the quick brown fox jumps over the lazy dog" then do
errors=errors+1
say 'LOWER Failed in test 1'
end
if lower("É") \= "é" then do
errors=errors+1
say 'LOWER Failed in test 2'
end

return errors<>0
