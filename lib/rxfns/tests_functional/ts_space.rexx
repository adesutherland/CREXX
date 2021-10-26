/* rexx test abs bif */
options levelb
/* x='the quick brown fox jumps over the lazy dog' */
/* say "test Space bif" */
/* say space(x,5,'.!#') */
/* say space(x) */
/* return */


  say "Look for SPACE OK"
/* These from the Rexx book. */
  if space('abc  def  ') \= 'abc def'         then say 'failed in line          1 '
  if space('  abc  def',3) \= 'abc   def'     then say 'failed in line          2 '
  if space('abc  def  ',1) \= 'abc def'       then say 'failed in line          3 '
  if space('abc  def  ',0) \= 'abcdef'        then say 'failed in line          4 '
  if space('abc  def  ',2,'+') \= 'abc++def'  then say 'failed in line          5 '
/* These from Mark Hessling. */
  if space(" foo ")                \= "foo"                then say 'failed in line          6 '
  if space("  foo")                \= "foo"                then say 'failed in line          7 '
  if space("foo  ")                \= "foo"                then say 'failed in line          8 '
  if space("  foo  ")              \= "foo"                then say 'failed in line          9 '
  if space(" foo bar ")            \= "foo bar"            then say 'failed in line         10 '
  if space("  foo  bar  ")         \= "foo bar"            then say 'failed in line         11 '
  if space(" foo bar " , 2)          \= "foo  bar"         then say 'failed in line         12 '
  if space(" foo bar ",,"-")       \= "foo-bar"            then say 'failed in line         13 '
  if space("  foo  bar  ",2,"-")   \= "foo--bar"           then say 'failed in line         14 '
  if space(" f-- b-- ",2,"-")      \= "f----b--"           then say 'failed in line         15 '
  if space(" f o o   b a r ",0)    \= "foobar"             then say 'failed in line         16 '
  say "SPACE OK"




/* function prototype */
space: procedure = .string
arg string1 = .string, int2 = 2, string3 = " "


