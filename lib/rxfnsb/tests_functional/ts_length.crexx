/* LENGTH */
options levelb
import rxfnsb

errors=0
/* These from the Rexx book. */

if length('abcdefgh') \= 8 then do
  errors=errors+1
  say 'LENGTH failed in test 1 '
end
if length('') \= 0 then do
  errors=errors+1
  say 'LENGTH failed in test 2 '
end
/* These from Mark Hessling. */

if length("") \= 0 then do
  errors=errors+1
  say 'LENGTH failed in test 3 '
end
if length("a") \= 1 then do
  errors=errors+1
  say 'LENGTH failed in test 4 '
end
if length("abc") \= 3 then do
  errors=errors+1
  say 'LENGTH failed in test 5 '
end
if length("abcdefghij") \= 10 then do
  errors=errors+1
  say 'LENGTH failed in test 6 '
end
/*Unicode*/
if length("René") \= 4 then do
  errors=errors+1
  say 'LENGTH failed in test 7 '
end

if length(René) \= 4 then do
  errors=errors+1
  say 'LENGTH failed in test 8 '
end

return errors<>0
