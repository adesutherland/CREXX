/* index (=pos with switched needle, haystack) */
options levelb
errors=0
/* These from the Rexx book. */
if index('Saturday','day') \= 6 then do
  errors=errors+1
  say 'INDEX failed in test 1 '
end
return errors<>0

/* function prototype */
index: procedure = .int
arg string1 = .string, string2 = .string, start = 1

