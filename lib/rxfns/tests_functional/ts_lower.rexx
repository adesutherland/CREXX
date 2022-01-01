/* rexx */
options levelb

say 'Look for LOWER OK'
if lower("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG") \= "the quick brown fox jumps over the lazy dog" then say 'Failed in test 1'

if lower("É") \= "é" then say 'Failed in test 2'

say 'Lower OK'

/* lower()  */
lower: procedure = .string
  arg expose string1 = .string

/* upper()  */
upper: procedure = .string
  arg expose string1 = .string
