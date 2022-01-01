/* WORD */
options levelb
say "Look for WORD OK"
/* These from the Rexx book. */
if word('Now is the time',3) \= 'the' then say 'failed in test 1 '
if word('Now is the time',5) \= '' then say 'failed in test 2 '
/* These from Mark Hessling. */
if word('This is certainly a test',1) \= 'This' then say 'failed in test 3 '
if word(' This is certainly a test',1) \= 'This' then say 'failed in test 4 '
if word('This is certainly a test',1) \= 'This' then say 'failed in test 5 '
if word('This is certainly a test',2) \= 'is' then say 'failed in test 6 '
if word('This is certainly a test',2) \= 'is' then say 'failed in test 7 '
if word('This is certainly a test',5) \= 'test' then say 'failed in test 8 '
if word('This is certainly a test ',5) \= 'test' then say 'failed in test 9 '
if word('This is certainly a test',6) \= '' then say 'failed in test 10 '
if word('',1) \= '' then say 'failed in test 11 '
if word('',10) \= '' then say 'failed in test 12 '
if word('test ',2) \= '' then say 'failed in test 13 '
say "WORD OK"

/* function prototype */
word: procedure = .string
arg string1 = .string, number = .int

