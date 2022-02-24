/* rexx test abs bif */
options levelb
errors=0
/* say "test insert" */
/* x='CREXX is faster  than BREXX' */

/* say "'"overlay('quicker ',x,10)"'" */

/* say '--2----' */
/* say "'"overlay('as',x,17,5)"'" */

/* say '--3----' */
/* say "'"overlay('The new system ',x,1)"'" */

/* say '--4----' */
/* say "'"overlay(" ,isnt it?",x,27)"'" */

/* return */

/* These from TRL */

  

if overlay('.','abcdef',3,2) \= 'ab. ef'         then do
  errors=errors+1
  say 'OVERLAY failed in test          1 '
end
if overlay(' ','abcdef',3) \= 'ab def'           then do
  errors=errors+1
  say 'OVERLAY failed in test          2 '
end
if overlay('.','abcdef',3,2) \= 'ab. ef'         then do
  errors=errors+1
  say 'OVERLAY failed in test          3 '
end
  /* 
end
if overlay('qq','abcd') \= 'qqcd'                then do
errors=errors+1
say 'OVERLAY failed in test          4 ' */
  

if overlay('qq','abcd',4) \= 'abcqq'             then do
  errors=errors+1
  say 'OVERLAY failed in test          5 '
end
if overlay('123','abc',5,6,'+') \= 'abc+123+++'  then do
  errors=errors+1
  say 'OVERLAY failed in test          6 '
end
  /* These from Mark Hessling. */

if overlay('foo', 'abcdefghi',3,4,'*') \=  'abfoo*ghi'  then do
  errors=errors+1
  say 'OVERLAY failed in test          7 '
end
if overlay('foo', 'abcdefghi',3,2,'*') \=  'abfoefghi'  then do
  errors=errors+1
  say 'OVERLAY failed in test          8 '
end
if overlay('foo', 'abcdefghi',3,4,) \=  'abfoo ghi'     then do
  errors=errors+1
  say 'OVERLAY failed in test          9 '
end
if overlay('foo', 'abcdefghi',3) \=  'abfoofghi'        then do
  errors=errors+1
  say 'OVERLAY failed in test         10 '
end
/* 
   end
   if overlay('foo', 'abcdefghi',,4,'*') \=  'foo*efghi'   then do
   errors=errors+1
   say 'OVERLAY failed in test         11 ' */


if overlay('foo', 'abcdefghi',9,4,'*') \=  'abcdefghfoo*'  then do
  errors=errors+1
  say 'OVERLAY failed in test         12 '
end
if overlay('foo', 'abcdefghi',10,4,'*') \=  'abcdefghifoo*'  then do
  errors=errors+1
  say 'OVERLAY failed in test         13 ' overlay('foo', 'abcdefghi',10,4,'*')
end

if overlay('foo', 'abcdefghi',11,4,'*') \=  'abcdefghi*foo*'  then do
  errors=errors+1
  say 'OVERLAY failed in test         14 '
end
if overlay('', 'abcdefghi',3) \=  'abcdefghi'                 then do
  errors=errors+1
  say 'OVERLAY failed in test         15 '
end
if overlay('foo', '',3) \=  '  foo'                           then do
  errors=errors+1
  say 'OVERLAY failed in test         16 '
end
if overlay('', '',3,4,'*') \=  '******'                       then do
  errors=errors+1
  say 'OVERLAY failed in test         17 '
end
/* 
   end
   if overlay('', '') \=  ''                                     then do
   errors=errors+1
   say 'OVERLAY failed in test         18 ' */

return errors<>0


/* function prototype */
overlay: procedure = .string
arg insstr = .string, string = .string, position = .int, len = 0, pad = ""



