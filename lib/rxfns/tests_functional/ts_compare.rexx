/* rexx test abs bif */
options levelb
errors=0
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
say "Look for  OK"
/* These from the Rexx book. */
if compare('abc','abc') \= 0 then do
  errors=errors+1
  say 'COMPARE failed in test 1 '
end

if compare('abc','ak') \= 2 then do
  errors=errors+1
  say 'COMPARE failed in test 2 '
end

if compare('ab ','ab') \= 0 then do
  errors=errors+1
  say 'COMPARE failed in test 3 '
end

if compare('ab ','ab',' ') \= 0 then do
  errors=errors+1
  say 'COMPARE failed in test 4 '
end

if compare('ab ','ab','x') \= 3 then do
  errors=errors+1
  say 'COMPARE failed in test 5 '
end

if compare('ab-- ','ab','-') \= 5 then do
  errors=errors+1
  say 'COMPARE failed in test 6 '
end

/* These from Mark Hessling. */
if compare("foo", "bar") \= 1 then do
  errors=errors+1
  say 'COMPARE failed in test 7 '
end

if compare("foo", "foo") \= 0 then do
  errors=errors+1
  say 'COMPARE failed in test 8 '
end

if compare(" ", "" ) \= 0 then do
  errors=errors+1
  say 'COMPARE failed in test 9 '
end

if compare("foo", "f", "o") \= 0 then do
  errors=errors+1
  say 'COMPARE failed in test 10 '
end

if compare("foobar", "foobag") \= 6 then do
  errors=errors+1
  say 'COMPARE failed in test 11 '
end

return errors<>0

/* function prototype */
compare: procedure = .string
  arg string1 = .string, string2 = .string, pad = " "