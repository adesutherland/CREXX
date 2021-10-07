/* POS */
options levelb
say "Look for POS OK"
/* These from the Rexx book. */
if pos('day','Saturday') \= 6 then say 'failed in test 1 '
if pos('x','abc def ghi') \= 0 then say 'failed in test 2 '
if pos(' ','abc def ghi') \= 4 then say 'failed in test 3 '
if pos(' ','abc def ghi',5) \= 8 then say 'failed in test 4 '
/* These from Mark Hessling. */
if pos('foo','a foo foo b') \= 3 then say 'failed in test 5 '
if pos('foo','a foo foo',3) \= 3 then say 'failed in test 6 '
if pos('foo','a foo foo',4) \= 7 then say 'failed in test 7 '
if pos('foo','a foo foo b',30) \= 0 then say 'failed in test 8 '
if pos('foo','a foo foo b',1) \= 3 then say 'failed in test 9 '
/* if pos('','a foo foo b') \= 0 then say 'failed in test 10 ' */ /* segfaults */
/* if pos('foo','') \= 0 then say 'failed in test 11 ' */ /* segfaults */
/* if pos('','') \= 0 then say 'failed in test 12 ' */ /* segfaults */
if pos('b' , 'a') \= 0 then say 'failed in test 13 '
if pos('b','b') \= 1 then say 'failed in test 14 '
if pos('b','abc') \= 2 then say 'failed in test 15 '
if pos('b','def') \= 0 then say 'failed in test 16 '
if pos('foo','foo foo b') \= 1 then say 'failed in test 17 '
say "POS OK"

/* function prototype */
pos: procedure = .int
arg string1 = .string, string2 = .string, start = 1

