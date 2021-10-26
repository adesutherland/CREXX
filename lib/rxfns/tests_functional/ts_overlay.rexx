/* rexx test abs bif */
options levelb
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
  say "Look for OVERLAY OK"
  if overlay('.','abcdef',3,2) \= 'ab. ef'         then say 'failed in test          1 '
  if overlay(' ','abcdef',3) \= 'ab def'           then say 'failed in test          2 '
  if overlay('.','abcdef',3,2) \= 'ab. ef'         then say 'failed in test          3 '
  /* if overlay('qq','abcd') \= 'qqcd'                then say 'failed in test          4 ' */
  if overlay('qq','abcd',4) \= 'abcqq'             then say 'failed in test          5 '
  if overlay('123','abc',5,6,'+') \= 'abc+123+++'  then say 'failed in test          6 '
/* These from Mark Hessling. */
  if overlay('foo', 'abcdefghi',3,4,'*') \=  'abfoo*ghi'  then say 'failed in test          7 '
  if overlay('foo', 'abcdefghi',3,2,'*') \=  'abfoefghi'  then say 'failed in test          8 '
  if overlay('foo', 'abcdefghi',3,4,) \=  'abfoo ghi'     then say 'failed in test          9 '
  if overlay('foo', 'abcdefghi',3) \=  'abfoofghi'        then say 'failed in test         10 '
  /* if overlay('foo', 'abcdefghi',,4,'*') \=  'foo*efghi'   then say 'failed in test         11 ' */
  if overlay('foo', 'abcdefghi',9,4,'*') \=  'abcdefghfoo*'  then say 'failed in test         12 '
  if overlay('foo', 'abcdefghi',10,4,'*') \=  'abcdefghifoo*'  then say 'failed in test         13 '
  if overlay('foo', 'abcdefghi',11,4,'*') \=  'abcdefghi*foo*'  then say 'failed in test         14 '
  if overlay('', 'abcdefghi',3) \=  'abcdefghi'                 then say 'failed in test         15 '
  if overlay('foo', '',3) \=  '  foo'                           then say 'failed in test         16 '
  if overlay('', '',3,4,'*') \=  '******'                       then say 'failed in test         17 '
  /* if overlay('', '') \=  ''                                     then say 'failed in test         18 ' */
  say "OVERLAY OK"


/* function prototype */
overlay: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = " "



