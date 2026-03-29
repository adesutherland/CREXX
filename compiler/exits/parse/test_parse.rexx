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
parse trace fred 3 a +4 c
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
/* TEST 12: empty source with implicit variables                  */
/* -------------------------------------------------------------- */
x = ''
parse log var x a b c

call assert_equal '12a', '', a, errors
call assert_equal '12b', '', b, errors
call assert_equal '12c', '', c, errors

/* -------------------------------------------------------------- */
/* TEST 13: blank-only source with implicit variables             */
/* -------------------------------------------------------------- */
x = '   '
parse log var x a b

call assert_equal '13a', '', a, errors
call assert_equal '13b', '', b, errors

/* -------------------------------------------------------------- */
/* TEST 14: leading blank with blank delimiter                    */
/* -------------------------------------------------------------- */
x = ' Now'
parse log var x a ' ' b

call assert_equal '14a', '', a, errors
call assert_equal '14b', 'Now', b, errors

/* -------------------------------------------------------------- */
/* TEST 15: trailing blank with blank delimiter                   */
/* -------------------------------------------------------------- */
x = 'Now '
parse log var x a ' ' b

call assert_equal '15a', 'Now', a, errors
call assert_equal '15b', '', b, errors

/* -------------------------------------------------------------- */
/* TEST 16: double blank after delimiter                          */
/* -------------------------------------------------------------- */
x = 'Now  is'
parse log var x a ' ' b

call assert_equal '16a', 'Now', a, errors
call assert_equal '16b', ' is', b, errors

/* -------------------------------------------------------------- */
/* TEST 17: comma with empty middle field                         */
/* -------------------------------------------------------------- */
x = 'a,,c'
parse log var x a ',' b ',' c

call assert_equal '17a', 'a', a, errors
call assert_equal '17b', '', b, errors
call assert_equal '17c', 'c', c, errors

/* -------------------------------------------------------------- */
/* TEST 18: comma with all empty fields                           */
/* -------------------------------------------------------------- */
x = ',,'
parse log var x a ',' b ',' c

call assert_equal '18a', '', a, errors
call assert_equal '18b', '', b, errors
call assert_equal '18c', '', c, errors

/* -------------------------------------------------------------- */
/* TEST 19: leading delimiter                                     */
/* -------------------------------------------------------------- */
x = ',leading'
parse log var x a ',' b

call assert_equal '19a', '', a, errors
call assert_equal '19b', 'leading', b, errors

/* -------------------------------------------------------------- */
/* TEST 20: trailing delimiter                                    */
/* -------------------------------------------------------------- */
x = 'trailing,'
parse log var x a ',' b

call assert_equal '20a', 'trailing', a, errors
call assert_equal '20b', '', b, errors

/* -------------------------------------------------------------- */
/* TEST 21: multi-character literal delimiter                     */
/* -------------------------------------------------------------- */
x = 'abc--def--ghi'
parse log var x a '--' b '--' c

call assert_equal '21a', 'abc', a, errors
call assert_equal '21b', 'def', b, errors
call assert_equal '21c', 'ghi', c, errors

/* -------------------------------------------------------------- */
/* TEST 22: suppressed first target                               */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x . b c

call assert_equal '22a', 'two', b, errors
call assert_equal '22b', 'three', c, errors

/* -------------------------------------------------------------- */
/* TEST 23: double suppressed target                              */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x . . c

call assert_equal '23a', 'three', c, errors

/* -------------------------------------------------------------- */
/* TEST 24: suppressed last target                                */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x a .

call assert_equal '24a', 'one', a, errors

/* -------------------------------------------------------------- */
/* TEST 25: single word plus remainder                            */
/* -------------------------------------------------------------- */
x = 'single'
parse log var x a b

call assert_equal '25a', 'single', a, errors
call assert_equal '25b', '', b, errors

/* -------------------------------------------------------------- */
/* TEST 26: implicit remainder preserves trailing blanks          */
/* -------------------------------------------------------------- */
x = '  Now   is   the   time  '
parse log var x a b c d

call assert_equal '26a', 'Now', a, errors
call assert_equal '26b', 'is', b, errors
call assert_equal '26c', 'the', c, errors
call assert_equal '26d', 'time  ', d, errors

/* -------------------------------------------------------------- */
/* TEST 27: last variable bounded by suppressed target            */
/* -------------------------------------------------------------- */
x = '  Now   is   the   time  '
parse log var x a b c d .

call assert_equal '27a', 'Now', a, errors
call assert_equal '27b', 'is', b, errors
call assert_equal '27c', 'the', c, errors
call assert_equal '27d', 'time', d, errors

/* -------------------------------------------------------------- */
/* TEST 28: absolute start only                                   */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse log var x 3 a

call assert_equal '28a', 'cdef', a, errors

/* -------------------------------------------------------------- */
/* TEST 29: absolute span split                                   */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse log var x 1 a 4 b

call assert_equal '29a', 'abc', a, errors
call assert_equal '29b', 'def', b, errors

/* -------------------------------------------------------------- */
/* TEST 30: second absolute span split                            */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse log var x 2 a 5 b

call assert_equal '30a', 'bcd', a, errors
call assert_equal '30b', 'ef', b, errors

/* -------------------------------------------------------------- */
/* TEST 31: literal not found in empty source                     */
/* -------------------------------------------------------------- */
x = ''
parse log var x a ',' b

call assert_equal '31a', '', a, errors
call assert_equal '31b', '', b, errors

/* -------------------------------------------------------------- */
/* TEST 32: repeated same absolute position                       */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse trace var x 2 w1 2 w2 2 w3

/* custom semantic / non-advancing numeric fallback */
call assert_equal '32a', 'bcdef', w1, errors
call assert_equal '32b', 'bcdef', w2, errors
call assert_equal '32c', 'bcdef', w3, errors

/* -------------------------------------------------------------- */
/* TEST 33: relative zero advance                                 */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse trace log var x 3 w1 +0 w2

/* custom semantic / non-advancing numeric fallback */
call assert_equal '33a', 'cdef', w1, errors
call assert_equal '33b', 'cdef', w2, errors

/* -------------------------------------------------------------- */
/* TEST 34: implicit after literal delimiter                      */
/* -------------------------------------------------------------- */
x = 'abc, def ghi'
parse log var x a ',' b c

call assert_equal '34a', 'abc', a, errors
call assert_equal '34b', 'def', b, errors
call assert_equal '34c', 'ghi', c, errors

/* -------------------------------------------------------------- */
/* TEST 35: literal delimiter with suppressed middle              */
/* -------------------------------------------------------------- */
x = 'abc,def,ghi'
parse log var x a ',' . ',' c

call assert_equal '35a', 'abc', a, errors
call assert_equal '35b', 'ghi', c, errors

/* -------------------------------------------------------------- */
/* TEST 36: blank delimiter with suppressed middle                */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x a ' ' . ' ' c

call assert_equal '36a', 'one', a, errors
call assert_equal '36b', 'three', c, errors

/* -------------------------------------------------------------- */
/* TEST 37: multiple blanks in implicit parsing                   */
/* -------------------------------------------------------------- */
x = 'one  two   three'
parse log var x a b c

call assert_equal '37a', 'one', a, errors
call assert_equal '37b', 'two', b, errors
call assert_equal '37c', 'three', c, errors

/* -------------------------------------------------------------- */
/* TEST 38: remainder after comma                                 */
/* -------------------------------------------------------------- */
x = 'one,two,three'
parse log var x a ',' rest

call assert_equal '38a', 'one', a, errors
call assert_equal '38b', 'two,three', rest, errors

/* -------------------------------------------------------------- */
/* TEST 39: blank delimiter chain                                 */
/* -------------------------------------------------------------- */
x = 'Now is the time'
parse log var x a ' ' b ' ' c

call assert_equal '39a', 'Now', a, errors
call assert_equal '39b', 'is', b, errors
call assert_equal '39c', 'the time', c, errors

/* -------------------------------------------------------------- */
/* TEST 40: source starts with blanks, implicit drop first        */
/* -------------------------------------------------------------- */
x = ' one two '
parse trace log var x . a

call assert_equal '40a', 'two ', a, errors

/* -------------------------------------------------------------- */
/* RESULT */
/* -------------------------------------------------------------- */
say '----------------------------------------'
if errors = 0 then
   say 'All parse log tests passed'
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