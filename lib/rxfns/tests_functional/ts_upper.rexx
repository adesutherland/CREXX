/* rexx */
options levelb

say 'Look for upper ok'
if upper("The quick brown fox jumps over the lazy dog") \= 'THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG' then say 'failed in test      1'

if upper("é") \= 'É' then say 'failed in test      2'



say 'upper OK'



/* upper()  */
upper: procedure = .string
  arg expose string1 = .string
