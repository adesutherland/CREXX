/* LENGTH */
options levelb
say "Look for LENGTH OK"
/* These from the Rexx book. */
if length('abcdefgh') \= 8 then say 'failed in test 1 '
if length('') \= 0 then say 'failed in test 2 '
/* These from Mark Hessling. */
if length("") \= 0 then say 'failed in test 3 '
if length("a") \= 1 then say 'failed in test 4 '
if length("abc") \= 3 then say 'failed in test 5 '
if length("abcdefghij") \= 10 then say 'failed in test 6 '
say "LENGTH OK"

/* function prototype */
length: procedure = .int
arg string1 = .string



