/* rexx */
/* These from TRL */
options levelb
say "Look for COPIES OK"
if copies('abc',3) \= 'abcabcabc' then say 'failed in test 1 '
if copies('abc',0) \= '' then say 'failed in test 2 '
/* These from Mark Hessling. */
if copies("foo",3) \= "foofoofoo" then say 'failed in test 3 '
if copies("x", 10) \= "xxxxxxxxxx" then say 'failed in test 4 '
if copies("", 50) \= "" then say 'failed in test 5 '
if copies("", 0) \= "" then say 'failed in test 6 '
if copies("foobar",0 ) \= "" then say 'failed in test 7 '
say "COPIES OK"

/* function prototype */
copies: procedure = .string
arg string1 = .string, times= .int

