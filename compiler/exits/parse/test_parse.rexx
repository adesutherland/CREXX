options levelb
import rxfnsb

main: procedure
/* --------------------------------------------------------------
 * test_parse.rexx
 *
 * Regression tests for custom parse implementation
 * -------------------------------------------------------------- */
ccod = .int[]

fred = 'Now is the time for all good men '
parse into abc var fred  first second
Say 'Display extracted Variables'
say '------------------------------'
say "first='"first"'"
say "second='"second"'"
say " "
Say 'Display array from INTO Clause'
say '------------------------------'
do i=1 to abc[0]
   say 'Array['i']="'abc[i]'"'
end
say "  "
Say "Start all test cases"
say "--------------------"
/* -------------------------------------------------------------- */
/* TEST 1: blank delimiter        */
/* -------------------------------------------------------------- */
fred = 'Now is the time for all good men'
parse fred q ' ' y

call assert_equal '1a', 'Now', q,ccod
call assert_equal '1b', 'is the time for all good men', y, ccod

/* -------------------------------------------------------------- */
/* TEST 2: numeric split */
/* -------------------------------------------------------------- */
fred = 'Now is the time for all good men'
parse fred 6 q +6 -3 y

call assert_equal '2a', 's the ', q , ccod
call assert_equal '2b', 'he time for all good men', y , ccod

/* -------------------------------------------------------------- */
/* TEST 3: fixed positions */
/* -------------------------------------------------------------- */
rec = '4711Alice Johnson   1249.19EUR London UK'
parse rec 1 id 5 name 21 amount 28 currency city country

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
parse value '1,22,333,4444,55555' with 1 first ',' second ',' third ',' fourth ',' fifth

call assert_equal '4a', '1', first , ccod
call assert_equal '4b', '22', second , ccod
call assert_equal '4c', '333', third , ccod
call assert_equal '4d', '4444', fourth , ccod
call assert_equal '4e', '55555', fifth , ccod

/* -------------------------------------------------------------- */
/* TEST 5: implicit with drop */
/* -------------------------------------------------------------- */
fred = 'Awfulcome its time'
parse fred h1 .

call assert_equal '5a', 'Awfulcome', h1 , ccod

/* -------------------------------------------------------------- */
/* TEST 6: multiple implicit      */
/* -------------------------------------------------------------- */
fred = 'l  m  n o'
parse fred f1 f2 f3

call assert_equal '6a', 'l', f1 , ccod
call assert_equal '6b', 'm', f2 , ccod
call assert_equal '6c', ' n o', f3 , ccod

/* -------------------------------------------------------------- */
/* TEST 7: repeated absolute position (your custom semantics) */
/* -------------------------------------------------------------- */
x = ' abc '
parse var x 1 w1 1 w2 1 w3

call assert_equal '7a', ' abc ', w1 , ccod
call assert_equal '7b', ' abc ', w2 , ccod
call assert_equal '7c', ' abc ', w3 , ccod

/* -------------------------------------------------------------- */
/* TEST 8: positional blanks                 */
/* -------------------------------------------------------------- */
x = ' abc '
parse var x 1 a 2 b 3 c

call assert_equal '8a', ' ', a, ccod
call assert_equal '8b', 'a', b, ccod
call assert_equal '8c', 'bc ', c, ccod

/* -------------------------------------------------------------- */
/* TEST 9: literal not found */
/* -------------------------------------------------------------- */
line = 'abcdef'
parse value line left ',' right

call assert_equal '9a', 'abcdef', left, ccod
call assert_equal '9b', '', right, ccod

/* -------------------------------------------------------------- */
/* TEST 10: relative controls */
/* -------------------------------------------------------------- */
fred = 'abcdefghijklmno'
## parse fred 3 a +4 b -2 c
parse fred 3 a +4 c
call assert_equal '10a', 'cdef', a, ccod
call assert_equal '10b', 'ghijklmno', c, ccod

/* -------------------------------------------------------------- */
/* TEST 11: suppressed middle */
/* -------------------------------------------------------------- */
fred = 'one two three'
parse fred a b c

call assert_equal '11a', 'one', a, ccod
call assert_equal '11b', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 12: empty source with implicit variables                  */
/* -------------------------------------------------------------- */
x = ''
parse var x a b c

call assert_equal '12a', '', a, ccod
call assert_equal '12b', '', b, ccod
call assert_equal '12c', '', c, ccod

/* -------------------------------------------------------------- */
/* TEST 13: blank-only source with implicit variables             */
/* -------------------------------------------------------------- */
x = '   '
parse var x a b

call assert_equal '13a', '', a, ccod
call assert_equal '13b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 14: leading blank with blank delimiter                    */
/* -------------------------------------------------------------- */
x = ' Now'
parse var x a ' ' b

call assert_equal '14a', '', a, ccod
call assert_equal '14b', 'Now', b, ccod

/* -------------------------------------------------------------- */
/* TEST 15: trailing blank with blank delimiter                   */
/* -------------------------------------------------------------- */
x = 'Now '
parse var x a ' ' b

call assert_equal '15a', 'Now', a, ccod
call assert_equal '15b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 16: double blank after delimiter                          */
/* -------------------------------------------------------------- */
x = 'Now  is'
parse var x a ' ' b

call assert_equal '16a', 'Now', a, ccod
call assert_equal '16b', ' is', b, ccod

/* -------------------------------------------------------------- */
/* TEST 17: comma with empty middle field                         */
/* -------------------------------------------------------------- */
x = 'a,,c'
parse var x a ',' b ',' c

call assert_equal '17a', 'a', a, ccod
call assert_equal '17b', '', b, ccod
call assert_equal '17c', 'c', c, ccod

/* -------------------------------------------------------------- */
/* TEST 18: comma with all empty fields                           */
/* -------------------------------------------------------------- */
x = ',,'
parse var x a ',' b ',' c

call assert_equal '18a', '', a, ccod
call assert_equal '18b', '', b, ccod
call assert_equal '18c', '', c, ccod

/* -------------------------------------------------------------- */
/* TEST 19: leading delimiter                                     */
/* -------------------------------------------------------------- */
x = ',leading'
parse var x a ',' b

call assert_equal '19a', '', a, ccod
call assert_equal '19b', 'leading', b, ccod

/* -------------------------------------------------------------- */
/* TEST 20: trailing delimiter                                    */
/* -------------------------------------------------------------- */
x = 'trailing,'
parse var x a ',' b

call assert_equal '20a', 'trailing', a, ccod
call assert_equal '20b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 21: multi-character literal delimiter                     */
/* -------------------------------------------------------------- */
x = 'abc--def--ghi'
parse var x a '--' b '--' c

call assert_equal '21a', 'abc', a, ccod
call assert_equal '21b', 'def', b, ccod
call assert_equal '21c', 'ghi', c, ccod

/* -------------------------------------------------------------- */
/* TEST 22: suppressed first target                               */
/* -------------------------------------------------------------- */
x = 'one two three'
parse var x . b c

call assert_equal '22a', 'two', b, ccod
call assert_equal '22b', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 23: double suppressed target                              */
/* -------------------------------------------------------------- */
x = 'one two three'
parse var x . . c

call assert_equal '23a', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 24: suppressed last target                                */
/* -------------------------------------------------------------- */
x = 'one two three'
parse var x a .

call assert_equal '24a', 'one', a, ccod

/* -------------------------------------------------------------- */
/* TEST 25: single word plus remainder                            */
/* -------------------------------------------------------------- */
x = 'single'
parse var x a b

call assert_equal '25a', 'single', a, ccod
call assert_equal '25b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 26: implicit remainder preserves trailing blanks          */
/* -------------------------------------------------------------- */
x = '  Now   is   the   time  '
parse var x a b c d

call assert_equal '26a', 'Now', a, ccod
call assert_equal '26b', 'is', b, ccod
call assert_equal '26c', 'the', c, ccod
call assert_equal '26d', 'time  ', d, ccod

/* -------------------------------------------------------------- */
/* TEST 27: last variable bounded by suppressed target            */
/* -------------------------------------------------------------- */
x = '  Now   is   the   time  '
parse var x a b c d .

call assert_equal '27a', 'Now', a, ccod
call assert_equal '27b', 'is', b, ccod
call assert_equal '27c', 'the', c, ccod
call assert_equal '27d', 'time', d, ccod

/* -------------------------------------------------------------- */
/* TEST 28: absolute start only                                   */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 3 a

call assert_equal '28a', 'cdef', a, ccod

/* -------------------------------------------------------------- */
/* TEST 29: absolute span split                                   */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 1 a 4 b

call assert_equal '29a', 'abc', a, ccod
call assert_equal '29b', 'def', b, ccod

/* -------------------------------------------------------------- */
/* TEST 30: second absolute span split                            */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 2 a 5 b

call assert_equal '30a', 'bcd', a, ccod
call assert_equal '30b', 'ef', b, ccod

/* -------------------------------------------------------------- */
/* TEST 31: literal not found in empty source                     */
/* -------------------------------------------------------------- */
x = ''
parse var x a ',' b

call assert_equal '31a', '', a, ccod
call assert_equal '31b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 32: repeated same absolute position                       */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 2 w1 2 w2 2 w3

/* custom semantic / non-advancing numeric fallback */
call assert_equal '32a', 'bcdef', w1, ccod
call assert_equal '32b', 'bcdef', w2, ccod
call assert_equal '32c', 'bcdef', w3, ccod

/* -------------------------------------------------------------- */
/* TEST 33: relative zero advance                                 */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 3 w1 +0 w2

/* custom semantic / non-advancing numeric fallback */
call assert_equal '33a', 'cdef', w1, ccod
call assert_equal '33b', 'cdef', w2, ccod

/* -------------------------------------------------------------- */
/* TEST 34: implicit after literal delimiter                      */
/* -------------------------------------------------------------- */
x = 'abc, def ghi'
parse var x a ',' b c

call assert_equal '34a', 'abc', a, ccod
call assert_equal '34b', 'def', b, ccod
call assert_equal '34c', 'ghi', c, ccod

/* -------------------------------------------------------------- */
/* TEST 35: literal delimiter with suppressed middle              */
/* -------------------------------------------------------------- */
x = 'abc,def,ghi'
parse var x a ',' . ',' c

call assert_equal '35a', 'abc', a, ccod
call assert_equal '35b', 'ghi', c, ccod

/* -------------------------------------------------------------- */
/* TEST 36: blank delimiter with suppressed middle                */
/* -------------------------------------------------------------- */
x = 'one two three'
parse var x a ' ' . ' ' c

call assert_equal '36a', 'one', a, ccod
call assert_equal '36b', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 37: multiple blanks in implicit parsing                   */
/* -------------------------------------------------------------- */
x = 'one  two   three'
parse var x a b c

call assert_equal '37a', 'one', a, ccod
call assert_equal '37b', 'two', b, ccod
call assert_equal '37c', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 38: remainder after comma                                 */
/* -------------------------------------------------------------- */
x = 'one,two,three'
parse var x a ',' rest

call assert_equal '38a', 'one', a, ccod
call assert_equal '38b', 'two,three', rest, ccod

/* -------------------------------------------------------------- */
/* TEST 39: blank delimiter chain                                 */
/* -------------------------------------------------------------- */
x = 'Now is the time'
parse var x a ' ' b ' ' c

call assert_equal '39a', 'Now', a, ccod
call assert_equal '39b', 'is', b, ccod
call assert_equal '39c', 'the time', c, ccod

/* -------------------------------------------------------------- */
/* TEST 40: source starts with blanks, implicit drop first        */
/* -------------------------------------------------------------- */
x = ' one two '
parse var x . a

call assert_equal '40a', 'two ', a, ccod

/* -------------------------------------------------------------- */
/* TEST 41: relative backward zero                                */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 3 w1 -0 w2

call assert_equal '41a', 'cdef', w1, ccod
call assert_equal '41b', 'cdef', w2, ccod

/* -------------------------------------------------------------- */
/* TEST 42: multi-char literal not found                          */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x a '--' b

call assert_equal '42a', 'abcdef', a, ccod
call assert_equal '42b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 43: consecutive multi-char delimiters                     */
/* -------------------------------------------------------------- */
x = 'a----b'
parse var x a '--' b '--' c

call assert_equal '43a', 'a', a, ccod
call assert_equal '43b', '', b, ccod
call assert_equal '43c', 'b', c, ccod

/* -------------------------------------------------------------- */
/* TEST 44: multi-char delimiter at boundaries                    */
/* -------------------------------------------------------------- */
x = '--abc--'
parse var x a '--' b '--' c

call assert_equal '44a', '', a, ccod
call assert_equal '44b', 'abc', b, ccod
call assert_equal '44c', '', c, ccod

/* -------------------------------------------------------------- */
/* TEST 45: literal start control on first variable               */
/* -------------------------------------------------------------- */
x = 'x=1;y=22;z=333'
parse var x 'x=' a ';' 'y=' b ';' 'z=' c

call assert_equal '45a', '1', a, ccod
call assert_equal '45b', '22', b, ccod
call assert_equal '45c', '333', c, ccod

/* -------------------------------------------------------------- */
/* TEST 46: literal start not found                               */
/* -------------------------------------------------------------- */
x = 'abc=123'
parse var x 'xyz=' a

call assert_equal '46a', '', a, ccod

/* -------------------------------------------------------------- */
/* TEST 47: implicit with trailing suppressed target              */
/* -------------------------------------------------------------- */
x = 'one two three'
parse var x a b .

call assert_equal '47a', 'one', a, ccod
call assert_equal '47b', 'two', b, ccod

/* -------------------------------------------------------------- */
/* TEST 48: triple suppressed target                              */
/* -------------------------------------------------------------- */
x = 'one two three four'
parse var x . . . d

call assert_equal '48a', 'four', d, ccod

/* -------------------------------------------------------------- */
/* TEST 49: blank delimiter preserves exact remainder             */
/* -------------------------------------------------------------- */
x = 'one  two   three'
parse var x a ' ' b

call assert_equal '49a', 'one', a, ccod
call assert_equal '49b', ' two   three', b, ccod

/* -------------------------------------------------------------- */
/* TEST 50: absolute start beyond source length                   */
/* -------------------------------------------------------------- */
x = 'abc'
parse var x 10 a

call assert_equal '50a', '', a, ccod

/* -------------------------------------------------------------- */
/* TEST 51: relative forward beyond source length                 */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 3 a +20 b

call assert_equal '51a', 'cdef', a, ccod
call assert_equal '51b', '', b, ccod

/* -------------------------------------------------------------- */
/* TEST 52: empty final field after blank delimiter chain         */
/* -------------------------------------------------------------- */
x = 'one two '
parse var x a ' ' b ' ' c

call assert_equal '52a', 'one', a, ccod
call assert_equal '52b', 'two', b, ccod
call assert_equal '52c', '', c, ccod

/* -------------------------------------------------------------- */
/* TEST 53: implicit treats punctuation as part of word           */
/* -------------------------------------------------------------- */
x = 'abc,def ghi'
parse var x a b

call assert_equal '53a', 'abc,def', a, ccod
call assert_equal '53b', 'ghi', b, ccod

/* -------------------------------------------------------------- */
/* TEST 54: suppressed target leaves blank remainder              */
/* -------------------------------------------------------------- */
x = 'one '
parse var x a .

call assert_equal '54a', 'one', a, ccod

/* -------------------------------------------------------------- */
/* TEST 55: final implicit variable on blank-only remainder       */
/* -------------------------------------------------------------- */
x = 'one '
parse var x . a

call assert_equal '55a', '', a, ccod

/* -------------------------------------------------------------- */
/* TEST 100: comma-separated fields */
/* -------------------------------------------------------------- */
string='1,22,333,4444,55555'
parse string first','second','third','fourth','fifth','six

call assert_equal '100a', '1',      first,  ccod
call assert_equal '100b', '22',     second, ccod
call assert_equal '100c', '333',    third,  ccod
call assert_equal '100d', '4444',   fourth, ccod
call assert_equal '100e', '55555',  fifth,  ccod
call assert_equal '100f', '',       six,    ccod

/* -------------------------------------------------------------- */
/* TEST 101: dot placeholder */
/* -------------------------------------------------------------- */
h='come'
h='Awful'h 'its time'
parse h h1 .

call assert_equal '101a', 'Awfulcome its time', h,  ccod
call assert_equal '101b', 'Awfulcome',          h1, ccod

/* -------------------------------------------------------------- */
/* TEST 102: blank parsing */
/* -------------------------------------------------------------- */
parse "l  m  n o" f1 f2 f3

call assert_equal '102a', 'l',    f1, ccod
call assert_equal '102b', 'm',    f2, ccod
call assert_equal '102c', ' n o', f3, ccod

/* -------------------------------------------------------------- */
/* TEST 103: remainder field */
/* -------------------------------------------------------------- */
parse 'l m n o' e1 e2

call assert_equal '103a', 'l',     e1, ccod
call assert_equal '103b', 'm n o', e2, ccod

/* -------------------------------------------------------------- */
/* TEST 104: empty source */
/* -------------------------------------------------------------- */
parse '' e3 e4

call assert_equal '104a', '', e3, ccod
call assert_equal '104b', '', e4, ccod

/* -------------------------------------------------------------- */
/* TEST 105: expression source */
/* -------------------------------------------------------------- */
x='aa' 1+2+3 'bb'
parse x e5

call assert_equal '105a', 'aa 6 bb', e5, ccod

/* not available in CREXX PARSE
/* -------------------------------------------------------------- */
/* TEST 106: variable and literal patterns */
/* -------------------------------------------------------------- */
delim='x'
parse 'c/my string/1 10' verb 2 delim +1 str (delim) rest

call assert_equal '106a', 'c',         verb,  ccod
call assert_equal '106b', '/',         delim, ccod
call assert_equal '106c', 'my string', str,   ccod
call assert_equal '106d', '1 10',      rest,  ccod
*/

/***** disabled legacy tests 21-24 kept commented out in original *****/

/* -------------------------------------------------------------- */
/* TEST 106: absolute position past end */
/* -------------------------------------------------------------- */
fred=''
parse fred 6 x

call assert_equal '106a', '', x, ccod

/* -------------------------------------------------------------- */
/* TEST 107: absolute positions */
/* -------------------------------------------------------------- */
fred='Now is the time for all good men'
parse fred  6 x 12

call assert_equal '107a', 's the ', x, ccod

/* -------------------------------------------------------------- */
/* TEST 108: relative position */
/* -------------------------------------------------------------- */
parse fred  6 x +6

call assert_equal '108a', 's the ', x, ccod

/* -------------------------------------------------------------- */
/* TEST 109: repeated absolute 1 */
/* -------------------------------------------------------------- */
parse 333 x 1 y 1 z

call assert_equal '109a', 333, x, ccod
call assert_equal '109b', 333, y, ccod
call assert_equal '109c', 333, z, ccod

/* -------------------------------------------------------------- */
/* TEST 110: mixed absolute/relative/negative */
/* -------------------------------------------------------------- */
parse fred  6 x +6  -3 y +4

call assert_equal '110a', 's the ', x, ccod
call assert_equal '110b', 'he t',   y, ccod

/* -------------------------------------------------------------- */
/* TEST 111: chained position expressions */
/* -------------------------------------------------------------- */
## fred='Now is the time for all good men'
##       1234567890123456
parse fred  12  -8 y +4  2 z 3 -5 a 10

call assert_equal '111a', ' is ',      y, ccod
call assert_equal '111b', 'o',         z, ccod
call assert_equal '111c', 'Now is th', a, ccod

/* -------------------------------------------------------------- */
/* TEST 112: literal search with backward relative */
/* -------------------------------------------------------------- */
## fred='Now is the time for all good men'
##       1234567890123456

parse fred a 'the' b -6 y +4 .

call assert_equal '112a', 'Now is ', a, ccod
call assert_equal '112b', ' time ', b , ccod
call assert_equal '112c', ' tim', y , ccod

/* -------------------------------------------------------------- */
/* TEST 113: literal search with + offsets */
/* -------------------------------------------------------------- */
## fred='Now is the time for all good men'
##       1234567890123456
parse fred  ' the' x +9 y +9
call assert_equal '113a', ' time for', x, ccod
call assert_equal '113b', ' all good', y, ccod

/***** disabled legacy tests 40-41 kept commented out in original *****/

/* -------------------------------------------------------------- */
/* TEST 114: multiple literal/position combinations */
/* -------------------------------------------------------------- */
fred='Now is the time for all good men'
##       1234567890123456
parse var fred   ' ti' +1 time  ' '  1 ' go' +1 xx ' '

call assert_equal '114a', 'time', time,  ccod
call assert_equal '114b', 'good', xx, ccod

/* -------------------------------------------------------------- */
/* TEST 115: delimiter and +1     */
/* -------------------------------------------------------------- */
parse "a,b,c" p ',' q +1 r

call assert_equal '115a', 'a',     p, ccod
call assert_equal '115b', ',',     q, ccod
call assert_equal '115c', 'b,c',   r, ccod

/* -------------------------------------------------------------- */
/* TEST 116: literal start and +5 */
/* -------------------------------------------------------------- */
parse "12345678" s '123' t +5

call assert_equal '116a', '',      s, ccod
call assert_equal '116b', '12345', t, ccod

/* -------------------------------------------------------------- */
/* TEST 117: literal capture with +3 */
/* -------------------------------------------------------------- */
parse '12345678' '34' me +3

call assert_equal '117a', '345', me, ccod

/* -------------------------------------------------------------- */
/* TEST 118: introductory word parse */
/* -------------------------------------------------------------- */
parse 'This is a sentence.' v1 v2 v3

call assert_equal '118a', 'This',        v1, ccod
call assert_equal '118b', 'is',          v2, ccod
call assert_equal '118c', 'a sentence.', v3, ccod

/* -------------------------------------------------------------- */
/* TEST 119: literal comma delimiter */
/* -------------------------------------------------------------- */
parse 'To be, or not to be?' w1 ',' w2

call assert_equal '119a', 'To be',          w1, ccod
call assert_equal '119b', ' or not to be?', w2, ccod

/* -------------------------------------------------------------- */
/* TEST 120: comma delimiter plus word parsing */
/* -------------------------------------------------------------- */
parse 'To be, or not to be?' w1 ',' w2 w3 w4

call assert_equal '120a', 'To be',  w1, ccod
call assert_equal '120b', 'or',     w2, ccod
call assert_equal '120c', 'not',    w3, ccod
call assert_equal '120d', 'to be?', w4, ccod

/***** disabled legacy tests 59-61 kept commented out in original *****/

/* -------------------------------------------------------------- */
/* TEST 121: absolute split */
/* -------------------------------------------------------------- */
parse 'Flying pigs have wings' x1 5 x2

call assert_equal '121a', 'Flyi',               x1, ccod
call assert_equal '121b', 'ng pigs have wings', x2, ccod

/* -------------------------------------------------------------- */
/* TEST 122: two absolute splits */
/* -------------------------------------------------------------- */
parse 'Flying pigs have wings' x1 5 x2 10 x3

call assert_equal '122a', 'Flyi',          x1, ccod
call assert_equal '122b', 'ng pi',         x2, ccod
call assert_equal '122c', 'gs have wings', x3, ccod

/* -------------------------------------------------------------- */
/* TEST 123: absolute then relative */
/* -------------------------------------------------------------- */
parse 'Flying pigs have wings' x1 5 x2 +5 x3

call assert_equal '123a', 'Flyi',          x1, ccod
call assert_equal '123b', 'ng pi',         x2, ccod
call assert_equal '123c', 'gs have wings', x3, ccod

/***** disabled legacy tests 70-72 kept commented out in original *****/

/* -------------------------------------------------------------- */
/* TEST 124: comma-separated definitions */
/* -------------------------------------------------------------- */
in='This is  the text which, I think,  is scanned.'
parse in w1 ',' w2 ',' rest

call assert_equal '124a', 'This is  the text which', w1,   ccod
call assert_equal '124b', ' I think',                w2,   ccod
call assert_equal '124c', '  is scanned.',           rest, ccod

/* -------------------------------------------------------------- */
/* TEST 125: extra field before remainder */
/* -------------------------------------------------------------- */
in='This is  the text which, I think,  is scanned.'
parse in w1 ',' w2 ',' w3 ',' rest

call assert_equal '125a', 'This is  the text which', w1,   ccod
call assert_equal '125b', ' I think',                w2,   ccod
call assert_equal '125c', '  is scanned.',           w3,   ccod
call assert_equal '125d', '',                        rest, ccod

/* -------------------------------------------------------------- */
/* TEST 126: word parsing with remainder */
/* -------------------------------------------------------------- */
in='This is  the text which, I think,  is scanned.'
parse in w1 w2 w3 rest

call assert_equal '126a', 'This',                              w1,   ccod
call assert_equal '126b', 'is',                                w2,   ccod
call assert_equal '126c', 'the',                               w3,   ccod
call assert_equal '126d', 'text which, I think,  is scanned.', rest, ccod

/* -------------------------------------------------------------- */
/* TEST 127: word parsing until comma */
/* -------------------------------------------------------------- */
in='This is  the text which, I think,  is scanned.'
parse in w1 w2 w3 rest ','

call assert_equal '127a', 'This',       w1,   ccod
call assert_equal '127b', 'is',         w2,   ccod
call assert_equal '127c', 'the',        w3,   ccod
call assert_equal '127d', 'text which', rest, ccod

/* -------------------------------------------------------------- */
/* TEST 128: explicit blank patterns */
/* -------------------------------------------------------------- */
in='This is  the text which, I think,  is scanned.'
parse in w1 ' ' w2 ' ' w3 ' ' rest ','

call assert_equal '128a', 'This',           w1,   ccod
call assert_equal '128b', 'is',             w2,   ccod
call assert_equal '128c', '',               w3,   ccod
call assert_equal '128d', 'the text which', rest, ccod

/* -------------------------------------------------------------- */
/* TEST 129: dot placeholders */
/* -------------------------------------------------------------- */
parse 'a bb ccc dddd eeeee' . . . word4 .

call assert_equal '129a', 'dddd', word4, ccod

/* -------------------------------------------------------------- */
/* TEST 130: fixed column slices */
/* -------------------------------------------------------------- */
in='This is  the text which, I think,  is scanned.'
parse in s1 10 s2 20 s3

call assert_equal '130a', 'This is  ',                   s1, ccod
call assert_equal '130b', 'the text w',                  s2, ccod
call assert_equal '130c', 'hich, I think,  is scanned.', s3, ccod

/* -------------------------------------------------------------- */
/* TEST 131: statement sequence positions */
/* -------------------------------------------------------------- */
a = '123456789'
parse a  3 w1 +3 w2 3 w3

call assert_equal '131a', '345',     w1, ccod
call assert_equal '131b', '6789',    w2, ccod
call assert_equal '131c', '3456789', w3, ccod

/* -------------------------------------------------------------- */
/* TEST 132: repeated absolute 1 with blanks */
/* -------------------------------------------------------------- */
x=' abc '
parse x 1 w1 1 w2 1 w3

call assert_equal '132a', x, w1, ccod
call assert_equal '132b', x, w2, ccod
call assert_equal '132c', x, w3, ccod
/* -------------------------------------------------------------- */
/* TEST 133: PARSE UPPER */
/* -------------------------------------------------------------- */
x = 'AbC dEf'
parse upper var x a b

call assert_equal '133a', 'ABC', a, ccod
call assert_equal '133b', 'DEF', b, ccod
/* -------------------------------------------------------------- */
/* TEST 134: PARSE LOWER */
/* -------------------------------------------------------------- */
x = 'AbC dEf'
parse lower var x a b

call assert_equal '134a', 'abc', a, ccod
call assert_equal '134b', 'def', b, ccod

/* -------------------------------------------------------------- */
/* TEST 135: PARSE VALUE WITH */
/* -------------------------------------------------------------- */
x = 'one two three'
parse value x with a b c
call assert_equal '135a', 'one', a, ccod
call assert_equal '135b', 'two', b, ccod
call assert_equal '135c', 'three', c, ccod

/* -------------------------------------------------------------- */
/* TEST 136: PARSE VAR leaves source unchanged */
/* -------------------------------------------------------------- */
x = 'alpha beta'
parse var x a b
call assert_equal '136a', 'alpha beta', x, ccod
call assert_equal '136b', 'alpha', a, ccod
call assert_equal '136c', 'beta', b, ccod

/* -------------------------------------------------------------- */
/* TEST 137: empty literal delimiter */
/* -------------------------------------------------------------- */
x = 'abc'
parse var x a '' b
call assert_equal '137a', '', a, ccod
call assert_equal '137b', 'abc', b, ccod
/* -------------------------------------------------------------- */
/* TEST 138: literal start not found before relative */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 'xyz' +1 a
call assert_equal '138a', '', a, ccod

/* -------------------------------------------------------------- */
/* TEST 139: absolute zero clamps to 1 */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x 0 a
call assert_equal '139a', 'abcdef', a, ccod

/* -------------------------------------------------------------- */
/* TEST 140: negative absolute clamps to 1 */
/* -------------------------------------------------------------- */
x = 'abcdef'
parse var x -3 a
call assert_equal '140a', 'abcdef', a, ccod

/* -------------------------------------------------------------- */
/* TEST 141: shared multi-char literal with +1 */
/* -------------------------------------------------------------- */
x = 'ab--cd--ef'
parse var x p '--' q +2 r
call assert_equal '141a', 'ab', p, ccod
call assert_equal '141b', '--', q, ccod
call assert_equal '141c', 'cd--ef', r, ccod

/* -------------------------------------------------------------- */
/* TEST 142: final drop after mixed controls */
/* -------------------------------------------------------------- */
x = 'abc,def,ghi'
parse var x a ',' b ',' .
call assert_equal '142a', 'abc', a, ccod
call assert_equal '142b', 'def', b, ccod

/* -------------------------------------------------------------- */
/* RESULT */
/* -------------------------------------------------------------- */
say copies('-',32)

say 'Test Cases (PARSE scenarios):'
say '  Total:             ' right(ccod.10,6)
say '  Passed:            ' right(ccod.10-ccod.11,6)
say '  Failed:            ' right(ccod.11,6)
say ''
say 'Assertions (value checks):'
say '  Passed:            ' right(ccod.1,6)
say '  Failed:            ' right(ccod.2,6)
say '  Trimmed OK:        ' right(ccod.3,6)
say '  Total:             ' right(ccod.1+ccod.2+ccod.3,6)

total = ccod.1+ccod.2+ccod.3
rate=.int
if total > 0 then do
   rate = (ccod.1+ccod.3) * 100 / total
   say ''
   say 'Success Rate:         'right(rate,6)' %'
end
say copies('-',32)

if ccod.11 = 0 & ccod.2 = 0 then do
    say 'SUCCESS'
end
return

/* -------------------------------------------------------------- */
/* helper   */
/* -------------------------------------------------------------- */
assert_equal: procedure
  arg testname=.string, expected=.string, actual=.string, expose ccod=.int[]
  casenum=substr(testname,1,length(testname)-1)
  if casenum\=ccod.9 then ccod.10=ccod.10+1
   if actual \= expected then do
     if strip(actual) = strip(expected) then ccod.3=ccod.3+1
     else do
        if casenum\=ccod.9 then ccod.11=ccod.11+1
        say '+++ FAIL TEST:' testname
        say '  expected=<'expected'>'
        say '  actual  =<'actual'>'
        ccod.2=ccod.2+1
     end
  end
  else do
     say '*** SUCCESS TEST:' testname
     say '  expected=<'expected'>'
     say '  actual  =<'actual'>'
     ccod.1=ccod.1+1
  end
   ccod.9=casenum     /* save case number */
return
