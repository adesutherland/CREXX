/* LASTPOS */
options levelb
errors=0
/* These from the Rexx book. */
if lastpos(' ','abc def ghi') \= 8 then do
  errors=errors+1
  say 'LASTPOS failed in test 1 '
end
if lastpos(' ','abcdefghi') \= 0 then do
  errors=errors+1
  say 'LASTPOS failed in test 2 '
end
if lastpos(' ','abc def ghi',7) \= 4 then do
  errors=errors+1
  say 'LASTPOS failed in test 3 '
  /* These from Mark Hessling. */
end
if lastpos('b', 'abc abc') \= 6 then do
  errors=errors+1
  say 'LASTPOS failed in test 4 '
end
if lastpos('b', 'abc abc',5) \= 2 then do
  errors=errors+1
  say 'LASTPOS failed in test 5 '
end
if lastpos('b', 'abc abc',6) \= 6 then do
  errors=errors+1
  say 'LASTPOS failed in test 6 '
end
if lastpos('b', 'abc abc',7) \= 6 then do
  errors=errors+1
  say 'LASTPOS failed in test 7 '
end
if lastpos('x', 'abc abc') \= 0 then do
  errors=errors+1
  say 'LASTPOS failed in test 8 '
end
if lastpos('b', 'abc abc',20) \= 6 then do
  errors=errors+1
  say 'LASTPOS failed in test 9 '
end
/* 
   end
   if lastpos('b', '') \= 0 then do
   errors=errors+1
   say 'LASTPOS failed in test 10 ' */
/* 
   end
   if lastpos('', 'c') \= 0 then do
   errors=errors+1
   say 'LASTPOS failed in test 11 ' */
/* 
   end
   if lastpos('', '') \= 0 then do
   errors=errors+1
   say 'LASTPOS failed in test 12 ' */
if lastpos('b', 'abc abc',20) \= 6 then do
  errors=errors+1
  say 'LASTPOS failed in test 13 '
end
if lastpos('bc', 'abc abc') \= 6 then do
  errors=errors+1
  say 'LASTPOS failed in test 14 '
end
if lastpos('bc ', 'abc abc',20) \= 2 then do
  errors=errors+1
  say 'LASTPOS failed in test 15 '
end
if lastpos('abc', 'abc abc',6) \= 1 then do
  errors=errors+1
  say 'LASTPOS failed in test 16 '
end
if lastpos('abc', 'abc abc') \= 5 then do
  errors=errors+1
  say 'LASTPOS failed in test 17 '
end
if lastpos('abc', 'abc abc',7) \= 5 then do
  errors=errors+1
  say 'LASTPOS failed in test 18 '
  /* These from elsewhere. */
end
if lastpos('abc','abcdefabccdabcd',4) \= 1 then do
  errors=errors+1
  say 'LASTPOS failed in test 19 '
end
return errors<>0
/* function prototype */
lastpos: procedure = .int
arg needle = .string, haystack = .string, start = 0
