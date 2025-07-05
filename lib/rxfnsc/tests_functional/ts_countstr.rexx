/* rexx test abs bif */
options levelb
import rxfnsb

errors=0

if countstr('bc','abcabcabc') \= 3 then do
  errors=errors+1
  say 'COUNTSTR failed in test 1 '
end

if countstr('aa','aaaa') \= 2 then do
  errors=errors+1
  say 'COUNTSTR failed in test 2 '
end

if countstr('','a a') \= 0 then do
  errors=errors+1
  say 'COUNTSTR failed in test 3 '
end

/* These from the Rexx book. */
/* These from Mark Hessling. */
if countstr('','') \= 0 then do
  errors=errors+1
  say 'COUNTSTR failed in test 4 '
end

if countstr('a','abcdef') \= 1 then do
  errors=errors+1
  say 'COUNTSTR failed in test 5 '
end

if countstr(0,0) \= 1 then do
  errors=errors+1
  say 'COUNTSTR failed in test 6 '
end

if countstr('a','def') \= 0 then do
  errors=errors+1
  say 'COUNTSTR failed in test 7 '
end

if countstr('a','') \= 0 then do
  errors=errors+1
  say 'COUNTSTR failed in test 8 '
end

if countstr('','def') \= 0 then do
  errors=errors+1
  say 'COUNTSTR failed in test 9 '
end

if countstr('abc','abcdef') \= 1 then do
  errors=errors+1
  say 'COUNTSTR failed in test 10 '
end

if countstr('abcdefg','abcdef') \= 0 then do
  errors=errors+1
  say 'COUNTSTR failed in test 11 '
end

if countstr('abc','abcdefabccdabcd') \= 3 then do
  errors=errors+1
  say 'COUNTSTR failed in test 12 '
end

if countstr('o','the quick brown fox jumps over the lazy dog') \= 4 then do
  errors=errors+1
  say 'COUNTSTR failed in test 13 '
end

/* 12345678901234567890123456789012  */

return errors<>0
