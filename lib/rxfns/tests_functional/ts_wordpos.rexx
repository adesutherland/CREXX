/* rexx test WORDPOS bif */
options levelb
/* /\* 1234567890123456789012345678901234567890123 *\/ */
/* x='the quick brown fox jumps over the lazy dog' */
/* say wordpos('jum',x) */
/* say wordpos('the',x) */
/* say wordpos('the',x,5) */
/* return */

/*  */
say "Look for WORDPOS OK"
/* These from the Rexx book. */
if wordpos('the','Now is the time') \= 3 then say 'failed in test 1 '
if wordpos('The','Now is the time') \= 0 then say 'failed in test 2 '
if wordpos('is the','Now is the time') \= 2 then say 'failed in test 3 '
if wordpos('is the','Now is the time') \= 2 then say 'failed in test 4 '
if wordpos('be','To be or not to be') \= 2 then say 'failed in test 5 '
if wordpos('be','To be or not to be',3) \= 6 then say 'failed in test 6 '
/* These from Mark Hessling. */
if wordpos('This','This is a small test') \= 1 then say 'failed in test 7 '
if wordpos('test','This is a small test') \= 5 then say 'failed in test 8 '
if wordpos('foo','This is a small test') \= 0 then say 'failed in test 9 '
if wordpos(' This ','This is a small test') \= 1 then say 'failed in test 10 '
if wordpos('This',' This is a small test') \= 1 then say 'failed in test 11 '
if wordpos('This','This is a small test') \= 1 then say 'failed in test 12 '
if wordpos('This','this is a small This') \= 5 then say 'failed in test 13 '
if wordpos('This','This is a small This') \= 1 then say 'failed in test 14 '
if wordpos('This','This is a small This', 2) \= 5 then say 'failed in test 15 '
if wordpos('is a ','This is a small test') \= 2 then say 'failed in test 16 '
if wordpos('is a ','This is a small test') \= 2 then say 'failed in test 17 '
if wordpos(' is a ','This is a small test') \= 2 then say 'failed in test 18 '
if wordpos('is a ','This is a small test', 2) \= 2 then say 'failed in test 19 '
if wordpos('is a ','This is a small test',3) \= 0 then say 'failed in test 20 '
if wordpos('is a ','This is a small test',4) \= 0 then say 'failed in test 21 '
if wordpos('test ','This is a small test') \= 5 then say 'failed in test 22 '
if wordpos('test ','This is a small test',5) \= 5 then say 'failed in test 23 '
if wordpos('test ','This is a small test',6) \= 0 then say 'failed in test 24 '
if wordpos('test ','This is a small test ') \= 5 then say 'failed in test 25 '
if wordpos(' test','This is a small test ',6) \= 0 then say 'failed in test 26 '
if wordpos('test ','This is a small test ',5) \= 5 then say 'failed in test 27 '
if wordpos(' ','This is a small test') \= 0 then say 'failed in test 28 '
if wordpos(' ','This is a small test',3) \= 0 then say 'failed in test 29 '
if wordpos('','This is a small test',4) \= 0 then say 'failed in test 30 '
if wordpos('test ','') \= 0 then say 'failed in test 31 '
if wordpos('','') \= 0 then say 'failed in test 32 '
if wordpos('',' ') \= 0 then say 'failed in test 33 '
if wordpos(' ','') \= 0 then say 'failed in test 34 '
if wordpos(' ','', 3) \= 0 then say 'failed in test 35 '
if wordpos(' a ','') \= 0 then say 'failed in test 36 '
if wordpos(' a ','a') \= 1 then say 'failed in test 37 '
say "WORDPOS OK"

/* function prototype */
wordpos: procedure = .int
arg string1 = .string, string2 = .string, int3 = 1


