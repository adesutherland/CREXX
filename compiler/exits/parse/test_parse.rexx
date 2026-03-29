options levelb
import rxfnsb

main: procedure
/* --------------------------------------------------------------
 * test_parse.rexx
 *
 * Regression tests for custom parse log implementation
 * -------------------------------------------------------------- */
errors = 0
/* -------------------------------------------------------------- */
/* TEST 1: blank delimiter */
/* -------------------------------------------------------------- */
fred = 'Now is the time for all good men'
parse log fred q ' ' y

call assert_equal '1a', 'Now', q,errors
call assert_equal '1b', 'is the time for all good men', y, errors

/* -------------------------------------------------------------- */
/* TEST 2: numeric split */
/* -------------------------------------------------------------- */
fred = 'Now is the time for all good men'
parse log fred 6 q +6 -3 y

call assert_equal '2a', 's the ', q , errors
call assert_equal '2b', 'he time for all good men', y , errors

/* -------------------------------------------------------------- */
/* TEST 3: fixed positions */
/* -------------------------------------------------------------- */
rec = '4711Alice Johnson   1249.19EUR London UK'
parse log rec 1 id 5 name 21 amount 28 currency city country

call assert_equal '3a', '4711', id , errors
call assert_equal '3b', 'Alice Johnson   ', name , errors
call assert_equal '3c', '1249.19', amount , errors
call assert_equal '3d', 'EUR', currency , errors
call assert_equal '3e', 'London', city , errors
call assert_equal '3f', 'UK', country , errors

/* -------------------------------------------------------------- */
/* TEST 4: comma delimited                   */
/* -------------------------------------------------------------- */
line = '1,22,333,4444,55555'
parse log value '1,22,333,4444,55555' with 1 first ',' second ',' third ',' fourth ',' fifth

call assert_equal '4a', '1', first , errors
call assert_equal '4b', '22', second , errors
call assert_equal '4c', '333', third , errors
call assert_equal '4d', '4444', fourth , errors
call assert_equal '4e', '55555', fifth , errors

/* -------------------------------------------------------------- */
/* TEST 5: implicit with drop */
/* -------------------------------------------------------------- */
fred = 'Awfulcome its time'
parse log fred h1 .

call assert_equal '5a', 'Awfulcome', h1 , errors

/* -------------------------------------------------------------- */
/* TEST 6: multiple implicit */
/* -------------------------------------------------------------- */
fred = 'l  m  n o'
parse log fred f1 f2 f3

call assert_equal '6a', 'l', f1 , errors
call assert_equal '6b', 'm', f2 , errors
call assert_equal '6c', 'n o', f3 , errors

/* -------------------------------------------------------------- */
/* TEST 7: repeated absolute position (your custom semantics) */
/* -------------------------------------------------------------- */
x = ' abc '
parse log var x 1 w1 1 w2 1 w3

call assert_equal '7a', ' abc ', w1 , errors
call assert_equal '7b', ' abc ', w2 , errors
call assert_equal '7c', ' abc ', w3 , errors

/* -------------------------------------------------------------- */
/* TEST 8: positional blanks                 */
/* -------------------------------------------------------------- */
x = ' abc '
parse log var x 1 a 2 b 3 c

call assert_equal '8a', ' ', a, errors
call assert_equal '8b', 'a', b, errors
call assert_equal '8c', 'bc ', c, errors

/* -------------------------------------------------------------- */
/* TEST 9: literal not found */
/* -------------------------------------------------------------- */
line = 'abcdef'
parse log value line left ',' right

call assert_equal '9a', 'abcdef', left, errors
call assert_equal '9b', '', right, errors

/* -------------------------------------------------------------- */
/* TEST 10: relative controls */
/* -------------------------------------------------------------- */
fred = 'abcdefghijklmno'
## parse log fred 3 a +4 b -2 c
parse log fred 3 a +4 c
call assert_equal '10a', 'cdef', a, errors
call assert_equal '10b', 'ghijklmno', c, errors

/* -------------------------------------------------------------- */
/* TEST 11: suppressed middle */
/* -------------------------------------------------------------- */
fred = 'one two three'
parse log fred a . c

call assert_equal '11a', 'one', a, errors
call assert_equal '11b', 'three', c, errors

/* -------------------------------------------------------------- */
/* RESULT */
/* -------------------------------------------------------------- */
say '----------------------------------------'
if errors = 0 then
do
   say 'All parse log tests passed'
   say 'SUCCESS'
end
else
   say errors 'test(s) failed'
say '----------------------------------------'
return


/* -------------------------------------------------------------- */
/* helper   */
/* -------------------------------------------------------------- */
assert_equal: procedure
  arg testname=.string, expected=.string, actual=.string, expose errors=.int
  if actual \= expected then do
     errors = errors + 1
     say '+++ FAIL TEST:' testname
     say '  expected=<'expected'>'
     say '  actual  =<'actual'>'
     errors=errors+1
  end
  else do
     say '*** SUCCESS TEST:' testname
     say '  expected=<'expected'>'
     say '  actual  =<'actual'>'
  end
return

return