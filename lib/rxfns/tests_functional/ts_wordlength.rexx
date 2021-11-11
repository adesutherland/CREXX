/* /\* rexx test abs bif *\/ */
options levelb
/* x='the quick brown fox jumps over the lazy dog' */
/* wrds=words(x) */
/* do i=1 to wrds */
/*    say word(x,i) wordlength(x,i) */
/* end */
/* return */
/* WORDLENGTH */
say "Look for WORDLENGTH OK"
/* These from the Rexx book. */
if wordlength('Now is the time',2) \=2 then say 'failed in test 1 '
if wordlength('Now comes the time',2) \=5 then say 'failed in test 2 '
if wordlength('Now is the time',6) \=0 then say 'failed in test 3 '
/* These from Mark Hessling. */
if wordlength('This is certainly a test',1) \= '4' then say 'failed in test 4 '
if wordlength('This is certainly a test',2) \= '2' then say 'failed in test 5 '
if wordlength('This is certainly a test',5) \= '4' then say 'failed in test 6 '
if wordlength('This is certainly a test ',5) \= '4' then say 'failed in test 7 '
if wordlength('This is certainly a test',6) \= '0' then say 'failed in test 8 '
if wordlength('',1) \= '0' then say 'failed in test 9 '
if wordlength('',10) \= '0' then say 'failed in test 10 '
say "WORDLENGTH OK"

/* function prototype */
wordlength: procedure = .int
arg string1 = .string, int2 = .int

word: procedure = .string
arg string1 = .string, int2 = .int

words: procedure = .int
arg string1 = .string

