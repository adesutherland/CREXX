/* POS */
options levelb
import rxfnsb

errors=0
/* These from the Rexx book. */
if pos('day','Saturday') \= 6 then do
  errors=errors+1
  say 'POS failed in test 1 '
end
if pos('x','abc def ghi') \= 0 then do
  errors=errors+1
  say 'POS failed in test 2 '
end
if pos(' ','abc def ghi') \= 4 then do
  errors=errors+1
  say 'POS failed in test 3 '
end
if pos(' ','abc def ghi',5) \= 8 then do
  errors=errors+1
  say 'POS failed in test 4 '
end
/* These from Mark Hessling. */
if pos('foo','a foo foo b') \= 3 then do
  errors=errors+1
  say 'POS failed in test 5 '
end
if pos('foo','a foo foo',3) \= 3 then do
  errors=errors+1
  say 'POS failed in test 6 '
end
if pos('foo','a foo foo',4) \= 7 then do
  errors=errors+1
  say 'POS failed in test 7 '
end
if pos('foo','a foo foo b',30) \= 0 then do
  errors=errors+1
  say 'POS failed in test 8 '
end
if pos('foo','a foo foo b',1) \= 3 then do
  errors=errors+1
  say 'POS failed in test 9 '
end
if pos('','a foo foo b') \= 0 then do
  errors=errors+1
  say 'POS failed in test 10 ' /* segfaults */
end
if pos('foo','') \= 0 then do
  errors=errors+1
  say 'POS failed in test 11 ' /* segfaults */
end
if pos('','') \= 0 then do
  errors=errors+1
  say 'POS failed in test 12 ' /* segfaults */
end
if pos('b' , 'a') \= 0 then do
  errors=errors+1
  say 'POS failed in test 13 '
end
if pos('b','b') \= 1 then do
  errors=errors+1
  say 'POS failed in test 14 '
end
if pos('b','abc') \= 2 then do
  errors=errors+1
  say 'POS failed in test 15 '
end
if pos('b','def') \= 0 then do
  errors=errors+1
  say 'POS failed in test 16 '
end
if pos('foo','foo foo b') \= 1 then do
  errors=errors+1
  say 'POS failed in test 17 '
end
return errors<>0
