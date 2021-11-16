/* rexx test abs bif */
options levelb
/* say "test abbrev" */
/* say '---1---' */
/* /\*                     12345  *\/ */
/* say compare('quicker','quick') */
/* say '---2---' */
/* say compare('quicker','quicker') */
/* say '---3---' */
/* say compare('quicker!','quicker',"!") */
/* say '---4---' */
/* say compare('quicker','fast') */

/* COMPARE */
say "Look for COMPARE OK"
/* These from the Rexx book. */
if compare('abc','abc') \= 0 then say 'failed in test 1 '
if compare('abc','ak') \= 2 then say 'failed in test 2 '
if compare('ab ','ab') \= 0 then say 'failed in test 3 '
if compare('ab ','ab',' ') \= 0 then say 'failed in test 4 '
if compare('ab ','ab','x') \= 3 then say 'failed in test 5 '
if compare('ab-- ','ab','-') \= 5 then say 'failed in test 6 '
/* These from Mark Hessling. */
say "COMPARE OK"
if compare("foo", "bar") \= 1 then say 'failed in test 7 '
if compare("foo", "foo") \= 0 then say 'failed in test 8 '
if compare(" ", "" ) \= 0 then say 'failed in test 9 '
if compare("foo", "f", "o") \= 0 then say 'failed in test 10 '
if compare("foobar", "foobag") \= 6 then say 'failed in test 11 '
return

/* function prototype */
compare: procedure = .string
  arg string1 = .string, string2 = .string, pad = " "