/* LASTPOS */
options levelb
say "Look for LASTPOS OK"
/* These from the Rexx book. */
if lastpos(' ','abc def ghi') \= 8 then say 'failed in test 1 '
if lastpos(' ','abcdefghi') \= 0 then say 'failed in test 2 '
if lastpos(' ','abc def ghi',7) \= 4 then say 'failed in test 3 '
/* These from Mark Hessling. */
if lastpos('b', 'abc abc') \= 6 then say 'failed in test 4 '
if lastpos('b', 'abc abc',5) \= 2 then say 'failed in test 5 '
if lastpos('b', 'abc abc',6) \= 6 then say 'failed in test 6 '
if lastpos('b', 'abc abc',7) \= 6 then say 'failed in test 7 '
if lastpos('x', 'abc abc') \= 0 then say 'failed in test 8 '
if lastpos('b', 'abc abc',20) \= 6 then say 'failed in test 9 '
/* if lastpos('b', '') \= 0 then say 'failed in test 10 ' */
/* if lastpos('', 'c') \= 0 then say 'failed in test 11 ' */
/* if lastpos('', '') \= 0 then say 'failed in test 12 ' */
if lastpos('b', 'abc abc',20) \= 6 then say 'failed in test 13 '
if lastpos('bc', 'abc abc') \= 6 then say 'failed in test 14 '
if lastpos('bc ', 'abc abc',20) \= 2 then say 'failed in test 15 '
if lastpos('abc', 'abc abc',6) \= 1 then say 'failed in test 16 '
if lastpos('abc', 'abc abc') \= 5 then say 'failed in test 17 '
if lastpos('abc', 'abc abc',7) \= 5 then say 'failed in test 18 '
/* These from elsewhere. */
if lastpos('abc','abcdefabccdabcd',4) \= 1 then say 'failed in test 19 '
say "LASTPOS OK"

/* function prototype */
lastpos: procedure = .int
arg needle = .string, haystack = .string, start = 0
