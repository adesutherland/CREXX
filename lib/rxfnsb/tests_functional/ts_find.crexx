/* rexx test FIND bif */
options levelb
import rxfnsb

/* /\* 1234567890123456789012345678901234567890123 *\/ */
/* x='the quick brown fox jumps over the lazy dog' */
/* say wordpos('jum',x) */
/* say wordpos('the',x) */
/* say wordpos('the',x,5) */
/* return */
/*  */
errors=0
/* These from the Rexx book. */
if find('the','Now is the time') \= 3 then do
  errors=errors+1
  say 'find failed in test 1 '
end
/* if wordpos('The','Now is the time') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 2 ' */
/* end */
/* if wordpos('is the','Now is the time') \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 3 ' */
/* end */
/* if wordpos('is the','Now is the time') \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 4 ' */
/* end */
/* if wordpos('be','To be or not to be') \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 5 ' */
/* end */
/* if wordpos('be','To be or not to be',3) \= 6 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 6 ' */
/* end */
/* /\* These from Mark Hessling. *\/ */
/* if wordpos('This','This is a small test') \= 1 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 7 ' */
/* end */
/* if wordpos('test','This is a small test') \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 8 ' */
/* end */
/* if wordpos('foo','This is a small test') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 9 ' */
/* end */
/* if wordpos(' This ','This is a small test') \= 1 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 10 ' */
/* end */
/* if wordpos('This',' This is a small test') \= 1 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 11 ' */
/* end */
/* if wordpos('This','This is a small test') \= 1 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 12 ' */
/* end */
/* if wordpos('This','this is a small This') \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 13 ' */
/* end */
/* if wordpos('This','This is a small This') \= 1 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 14 ' */
/* end */
/* if wordpos('This','This is a small This', 2) \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 15 ' */
/* end */
/* if wordpos('is a ','This is a small test') \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 16 ' */
/* end */
/* if wordpos('is a ','This is a small test') \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 17 ' */
/* end */
/* if wordpos(' is a ','This is a small test') \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 18 ' */
/* end */
/* if wordpos('is a ','This is a small test', 2) \= 2 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 19 ' */
/* end */
/* if wordpos('is a ','This is a small test',3) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 20 ' */
/* end */
/* if wordpos('is a ','This is a small test',4) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 21 ' */
/* end */
/* if wordpos('test ','This is a small test') \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 22 ' */
/* end */
/* if wordpos('test ','This is a small test',5) \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 23 ' */
/* end */
/* if wordpos('test ','This is a small test',6) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 24 ' */
/* end */
/* if wordpos('test ','This is a small test ') \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 25 ' */
/* end */
/* if wordpos(' test','This is a small test ',6) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 26 ' */
/* end */
/* if wordpos('test ','This is a small test ',5) \= 5 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 27 ' */
/* end */
/* if wordpos(' ','This is a small test') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 28 ' */
/* end */
/* if wordpos(' ','This is a small test',3) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 29 ' */
/* end */
/* if wordpos('','This is a small test',4) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 30 ' */
/* end */
/* if wordpos('test ','') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 31 ' */
/* end */
/* if wordpos('','') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 32 ' */
/* end */
/* if wordpos('',' ') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 33 ' */
/* end */
/* if wordpos(' ','') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 34 ' */
/* end */
/* if wordpos(' ','', 3) \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 35 ' */
/* end */
/* if wordpos(' a ','') \= 0 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 36 ' */
/* end */
/* if wordpos(' a ','a') \= 1 then do */
/*   errors=errors+1 */
/*   say 'WORDPOS failed in test 37 ' */
/* end */
return errors<>0
