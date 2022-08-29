/* index (=pos with switched needle, haystack) */
options levelb
import rxfnsb

errors=0
/* These from the Rexx book. */
if index('Saturday','day') \= 6 then do
  errors=errors+1
  say 'INDEX failed in test 1 '
end
return errors<>0
