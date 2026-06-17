options levelb
import rxfnsb
/* ------------------------------------------------------------------
 *  QPOS test suite  (plain Regina REXX) 
 *       Validates correctness across normal and edge cases.
 * ------------------------------------------------------------------ 
 */
say '--- QPOS TEST SUITE ---'
/* -----  Test 1 ----- */
tests_text.1     = 'say hello;world'
tests_needle.1   = ';'
tests_start.1    = 1
tests_expected.1 = 10
/* -----  Test 2 ----- */
tests_text.2    = 'say "hello;world"; next'
tests_needle.2 = ';'
tests_start.2 = 1
tests_expected.2= 18
/* -----  Test 3 ----- */
tests_text.3    = "say 'a;b;c'; end"
tests_needle.3 = ';'
tests_start.3 = 1
tests_expected.3= 12
/* -----  Test 4 ----- */
tests_text.4    = '"abc";"def";"ghi"'
tests_needle.4 = ';'
tests_start.4 = 1
tests_expected.4= 6
/* -----  Test 5 ----- */
tests_text.5    = "a='one;two;three'"
tests_needle.5 = ';'
tests_start.5 = 1
tests_expected.5= 0
/* -----  Test 6 ----- */
tests_text.6    = 'say "unfinished quote; test'
tests_needle.6 = ';'
tests_start.6 = 1
tests_expected.6= 0
/* -----  Test 7 ----- */
tests_text.7    = 'say "escaped ""quote""; end"'
tests_needle.7 = ';'
tests_start.7 = 1
tests_expected.7= 0
/* -----  Test 8 ----- */
tests_text.8    = 'say "hello"; say "goodbye"'
tests_needle.8 = ';'
tests_start.8 = 1
tests_expected.8= 12
/* -----  Test 9 ----- */
tests_text.9    = ';;;;'
tests_needle.9 = ';'
tests_start.9 = 3
tests_expected.9= 3
/* -----  Test 10 ----- */
tests_text.10    = 'nothing here'
tests_needle.10 = ';'
tests_start.10 = 1
tests_expected.10= 0
/* -----  Test 11 ----- */
tests_text.11    = 'a="b;c"d;e;f'
tests_needle.11 = ';'
tests_start.11 = 1
tests_expected.11= 9
/* -----  Test 12 ----- */
tests_text.12    = "'x';"'"'y"';'z'"
tests_needle.12 = ';'
tests_start.12 = 1
tests_expected.12= 4

/* ===== Run tests ===== */
passed = 0
do i = 1 to tests_text.0
   text    = tests_text.i
   needle = tests_needle.i
   start = tests_start.i
   expected= tests_expected.i
   result = qpos(needle, text, start)
   if result = expected then do
      say 'OK  ' right(i,2)':' result 'matches expected' expected
      passed = passed + 1
   end
   else say 'FAIL' right(i,2)':' result '(expected' expected")"
end
say ''
say 'Summary:' passed 'of' tests_text.0 'tests passed.'
exit