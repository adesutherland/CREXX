/* rexx test abs bif */
options levelb
errors=0
/* x='the quick brown fox jumps over the lazy dog' */
/* say "test Space bif" */
/* say space(x,5,'.!#') */
/* say space(x) */
/* return */


/* These from the Rexx book. */
if space('abc  def  ') \= 'abc def'         then do
  errors=errors+1
  say 'SPACE failed in line          1 '
end
if space('  abc  def',3) \= 'abc   def'     then do
  errors=errors+1
  say 'SPACE failed in line          2 '
end
if space('abc  def  ',1) \= 'abc def'       then do
  errors=errors+1
  say 'SPACE failed in line          3 '
end
if space('abc  def  ',0) \= 'abcdef'        then do
  errors=errors+1
  say 'SPACE failed in line          4 '
end
if space('abc  def  ',2,'+') \= 'abc++def'  then do
  errors=errors+1
  say 'SPACE failed in line          5 '
end
/* These from Mark Hessling. */
if space(" foo ")                \= "foo"                then do
  errors=errors+1
  say 'SPACE failed in line          6 '
end
if space("  foo")                \= "foo"                then do
  errors=errors+1
  say 'SPACE failed in line          7 '
end
if space("foo  ")                \= "foo"                then do
  errors=errors+1
  say 'SPACE failed in line          8 '
end
if space("  foo  ")              \= "foo"                then do
  errors=errors+1
  say 'SPACE failed in line          9 '
end
if space(" foo bar ")            \= "foo bar"            then do
  errors=errors+1
  say 'SPACE failed in line         10 '
end
if space("  foo  bar  ")         \= "foo bar"            then do
  errors=errors+1
  say 'SPACE failed in line         11 '
end
if space(" foo bar " , 2)          \= "foo  bar"         then do
  errors=errors+1
  say 'SPACE failed in line         12 '
end
if space(" foo bar ",,"-")       \= "foo-bar"            then do
  errors=errors+1
  say 'SPACE failed in line         13 '
end
if space("  foo  bar  ",2,"-")   \= "foo--bar"           then do
  errors=errors+1
  say 'SPACE failed in line         14 '
end
if space(" f-- b-- ",2,"-")      \= "f----b--"           then do
  errors=errors+1
  say 'SPACE failed in line         15 '
end
if space(" f o o   b a r ",0)    \= "foobar"             then do
  errors=errors+1
  say 'SPACE failed in line         16 '
end

return errors<>0
  


/* function prototype */
space: procedure = .string
arg string1 = .string, int2 = 2, string3 = " "


