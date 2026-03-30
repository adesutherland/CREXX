options levelb
import rxfnsb

main: procedure
/* --------------------------------------------------------------
 * test_parse.rexx
 *
 * Regression tests for custom parse log implementation
 * -------------------------------------------------------------- */
ccod = .int[]
/* -------------------------------------------------------------- */
/* TEST 1: blank delimiter */
/* -------------------------------------------------------------- */
fred = 'Now is the time for all good men'
parse log fred q ' ' y

call assert_equal '1a', 'Now', q,ccod
call assert_equal '1b', 'is the time for all good men', y, ccod

/* -------------------------------------------------------------- */
/* TEST 2: numeric split */
/* -------------------------------------------------------------- */
fred = 'Now is the time for all good men'
parse log fred 6 q +6 -3 y

call assert_equal '2a', 's the ', q , ccod
call assert_equal '2b', 'he time for all good men', y , ccod

/* -------------------------------------------------------------- */
/* TEST 3: fixed positions */
/* -------------------------------------------------------------- */
rec = '4711Alice Johnson   1249.19EUR London UK'
parse log rec 1 id 5 name 21 amount 28 currency city country

call assert_equal '3a', '4711', id , ccod
call assert_equal '3b', 'Alice Johnson   ', name , ccod
call assert_equal '3c', '1249.19', amount , ccod
call assert_equal '3d', 'EUR', currency , ccod
call assert_equal '3e', 'London', city , ccod
call assert_equal '3f', 'UK', country , ccod

/* -------------------------------------------------------------- */
/* TEST 4: comma delimited                   */
/* -------------------------------------------------------------- */
line = '1,22,333,4444,55555'
parse log value '1,22,333,4444,55555' with 1 first ',' second ',' third ',' fourth ',' fifth

call assert_equal '4a', '1', first , ccod
call assert_equal '4b', '22', second , ccod
call assert_equal '4c', '333', third , ccod
call assert_equal '4d', '4444', fourth , ccod
call assert_equal '4e', '55555', fifth , ccod

/* -------------------------------------------------------------- */
/* TEST 5: implicit with drop */
/* -------------------------------------------------------------- */
fred = 'Awfulcome its time'
parse log fred h1 .

call assert_equal '5a', 'Awfulcome', h1 , ccod

/* -------------------------------------------------------------- */
/* TEST 6: multiple implicit */
/* -------------------------------------------------------------- */
fred = 'l  m  n o'
parse log fred f1 f2 f3

call assert_equal '6a', 'l', f1 , ccod
call assert_equal '6b', 'm', f2 , ccod
call assert_equal '6c', 'n o', f3 , ccod

/* -------------------------------------------------------------- */
/* TEST 7: repeated absolute position (your custom semantics) */
/* -------------------------------------------------------------- */
x = ' abc '
parse log var x 1 w1 1 w2 1 w3

call assert_equal '7a', ' abc ', w1 , ccod
call assert_equal '7b', ' abc ', w2 , ccod
call assert_equal '7c', ' abc ', w3 , ccod

/* -------------------------------------------------------------- */
/* TEST 8: positional blanks                 */
/* -------------------------------------------------------------- */
x = ' abc '
parse log var x 1 a 2 b 3 c

call assert_equal '8a', ' ', a, ccod
call assert_equal '8b', 'a', b, ccod
call assert_equal '8c', 'bc ', c, ccod

/* -------------------------------------------------------------- */
/* TEST 9: literal not found */
/* -------------------------------------------------------------- */
line = 'abcdef'
parse log value line left ',' right

call assert_equal '9a', 'abcdef', left, ccod
call assert_equal '9b', '', right, ccod

/* -------------------------------------------------------------- */
/* TEST 10: relative controls */
/* -------------------------------------------------------------- */
fred = 'abcdefghijklmno'
## parse log fred 3 a +4 b -2 c
parse trace fred 3 a +4 c
call assert_equal '10a', 'cdef', a, ccod
call assert_equal '10b', 'ghijklmno', c, ccod

/* -------------------------------------------------------------- */
/* TEST 11: suppressed middle */
/* -------------------------------------------------------------- */
fred = 'one two three'
parse log fred a . c

call assert_equal '11a', 'one', a, ccod
call assert_equal '11b', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 12: empty source with implicit variables                  */
/* -------------------------------------------------------------- */
x = ''
parse log var x a b c

call assert_equal '12a', '', a, ccod
call assert_equal '12b', '', b, ccod
call assert_equal '12c', '', c, ccod

/* -------------------------------------------------------------- */
/* TEST 13: blank-only source with implicit variables             */
/* -------------------------------------------------------------- */
x = '   '
parse log var x a b

call assert_equal '13a', '', a, ccod
call assert_equal '13b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 14: leading blank with blank delimiter                    */
/* -------------------------------------------------------------- */
x = ' Now'
parse log var x a ' ' b

call assert_equal '14a', '', a, ccod
call assert_equal '14b', 'Now', b, ccod

/* -------------------------------------------------------------- */
/* TEST 15: trailing blank with blank delimiter                   */
/* -------------------------------------------------------------- */
x = 'Now '
parse log var x a ' ' b

call assert_equal '15a', 'Now', a, ccod
call assert_equal '15b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 16: double blank after delimiter                          */
/* -------------------------------------------------------------- */
x = 'Now  is'
parse log var x a ' ' b

call assert_equal '16a', 'Now', a, ccod
call assert_equal '16b', ' is', b, ccod

/* -------------------------------------------------------------- */
/* TEST 17: comma with empty middle field                         */
/* -------------------------------------------------------------- */
x = 'a,,c'
parse log var x a ',' b ',' c

call assert_equal '17a', 'a', a, ccod
call assert_equal '17b', '', b, ccod
call assert_equal '17c', 'c', c, ccod

/* -------------------------------------------------------------- */
/* TEST 18: comma with all empty fields                           */
/* -------------------------------------------------------------- */
x = ',,'
parse log var x a ',' b ',' c

call assert_equal '18a', '', a, ccod
call assert_equal '18b', '', b, ccod
call assert_equal '18c', '', c, ccod

/* -------------------------------------------------------------- */
/* TEST 19: leading delimiter                                     */
/* -------------------------------------------------------------- */
x = ',leading'
parse log var x a ',' b

call assert_equal '19a', '', a, ccod
call assert_equal '19b', 'leading', b, ccod

/* -------------------------------------------------------------- */
/* TEST 20: trailing delimiter                                    */
/* -------------------------------------------------------------- */
x = 'trailing,'
parse log var x a ',' b

call assert_equal '20a', 'trailing', a, ccod
call assert_equal '20b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 21: multi-character literal delimiter                     */
/* -------------------------------------------------------------- */
x = 'abc--def--ghi'
parse log var x a '--' b '--' c

call assert_equal '21a', 'abc', a, ccod
call assert_equal '21b', 'def', b, ccod
call assert_equal '21c', 'ghi', c, ccod

/* -------------------------------------------------------------- */
/* TEST 22: suppressed first target                               */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x . b c

call assert_equal '22a', 'two', b, ccod
call assert_equal '22b', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 23: double suppressed target                              */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x . . c

call assert_equal '23a', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 24: suppressed last target                                */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x a .

call assert_equal '24a', 'one', a, ccod

/* -------------------------------------------------------------- */
/* TEST 25: single word plus remainder                            */
/* -------------------------------------------------------------- */
x = 'single'
parse log var x a b

call assert_equal '25a', 'single', a, ccod
call assert_equal '25b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 26: implicit remainder preserves trailing blanks          */
/* -------------------------------------------------------------- */
x = '  Now   is   the   time  '
parse log var x a b c d

call assert_equal '26a', 'Now', a, ccod
call assert_equal '26b', 'is', b, ccod
call assert_equal '26c', 'the', c, ccod
call assert_equal '26d', 'time  ', d, ccod

/* -------------------------------------------------------------- */
/* TEST 27: last variable bounded by suppressed target            */
/* -------------------------------------------------------------- */
x = '  Now   is   the   time  '
parse log var x a b c d .

call assert_equal '27a', 'Now', a, ccod
call assert_equal '27b', 'is', b, ccod
call assert_equal '27c', 'the', c, ccod
call assert_equal '27d', 'time', d, ccod

/* -------------------------------------------------------------- */
/* TEST 28: absolute start only                                   */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse log var x 3 a

call assert_equal '28a', 'cdef', a, ccod

/* -------------------------------------------------------------- */
/* TEST 29: absolute span split                                   */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse log var x 1 a 4 b

call assert_equal '29a', 'abc', a, ccod
call assert_equal '29b', 'def', b, ccod

/* -------------------------------------------------------------- */
/* TEST 30: second absolute span split                            */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse log var x 2 a 5 b

call assert_equal '30a', 'bcd', a, ccod
call assert_equal '30b', 'ef', b, ccod

/* -------------------------------------------------------------- */
/* TEST 31: literal not found in empty source                     */
/* -------------------------------------------------------------- */
x = ''
parse log var x a ',' b

call assert_equal '31a', '', a, ccod
call assert_equal '31b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 32: repeated same absolute position                       */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse trace var x 2 w1 2 w2 2 w3

/* custom semantic / non-advancing numeric fallback */
call assert_equal '32a', 'bcdef', w1, ccod
call assert_equal '32b', 'bcdef', w2, ccod
call assert_equal '32c', 'bcdef', w3, ccod

/* -------------------------------------------------------------- */
/* TEST 33: relative zero advance                                 */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse trace log var x 3 w1 +0 w2

/* custom semantic / non-advancing numeric fallback */
call assert_equal '33a', 'cdef', w1, ccod
call assert_equal '33b', 'cdef', w2, ccod

/* -------------------------------------------------------------- */
/* TEST 34: implicit after literal delimiter                      */
/* -------------------------------------------------------------- */
x = 'abc, def ghi'
parse log var x a ',' b c

call assert_equal '34a', 'abc', a, ccod
call assert_equal '34b', 'def', b, ccod
call assert_equal '34c', 'ghi', c, ccod

/* -------------------------------------------------------------- */
/* TEST 35: literal delimiter with suppressed middle              */
/* -------------------------------------------------------------- */
x = 'abc,def,ghi'
parse log var x a ',' . ',' c

call assert_equal '35a', 'abc', a, ccod
call assert_equal '35b', 'ghi', c, ccod

/* -------------------------------------------------------------- */
/* TEST 36: blank delimiter with suppressed middle                */
/* -------------------------------------------------------------- */
x = 'one two three'
parse log var x a ' ' . ' ' c

call assert_equal '36a', 'one', a, ccod
call assert_equal '36b', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 37: multiple blanks in implicit parsing                   */
/* -------------------------------------------------------------- */
x = 'one  two   three'
parse log var x a b c

call assert_equal '37a', 'one', a, ccod
call assert_equal '37b', 'two', b, ccod
call assert_equal '37c', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 38: remainder after comma                                 */
/* -------------------------------------------------------------- */
x = 'one,two,three'
parse log var x a ',' rest

call assert_equal '38a', 'one', a, ccod
call assert_equal '38b', 'two,three', rest, ccod

/* -------------------------------------------------------------- */
/* TEST 39: blank delimiter chain                                 */
/* -------------------------------------------------------------- */
x = 'Now is the time'
parse log var x a ' ' b ' ' c

call assert_equal '39a', 'Now', a, ccod
call assert_equal '39b', 'is', b, ccod
call assert_equal '39c', 'the time', c, ccod

/* -------------------------------------------------------------- */
/* TEST 40: source starts with blanks, implicit drop first        */
/* -------------------------------------------------------------- */
x = ' one two '
parse log var x . a

call assert_equal '40a', 'two ', a, ccod

/* -------------------------------------------------------------- */
/* RESULT */
/* -------------------------------------------------------------- */
say copies('-',32)
say ' tests passed 'ccod.1
say ' tests failed 'ccod.2
say '        total 'ccod.1+ccod.2
say copies('-',32)
return


/* -------------------------------------------------------------- */
/* helper   */
/* -------------------------------------------------------------- */
assert_equal: procedure
  arg testname=.string, expected=.string, actual=.string, expose ccod=.int[]
  if actual \= expected then do
     say '+++ FAIL TEST:' testname
     say '  expected=<'expected'>'
     say '  actual  =<'actual'>'
     ccod.2=ccod.2+1
  end
  else do
     say '*** SUCCESS TEST:' testname
     say '  expected=<'expected'>'
     say '  actual  =<'actual'>'
     ccod.1=ccod.1+1
  end
return