/* rexx */
options levelb
/* These from TRL */
errors=0
if left('abc d',8) \= 'abc d   '  then do
  errors=errors+1
  say 'LEFT failed in test          1 '
end

if left('abc d',8,'.') \= 'abc d...'  then do
  errors=errors+1
  say 'LEFT failed in test          2 '
end
if left('abc  def',7) \= 'abc  de'  then do
  errors=errors+1
  say 'LEFT failed in test          3 '
  /* These from Mark Hessling. */
end
if left("foobar",1) \=      "f"            then do
  errors=errors+1
  say 'LEFT failed in test          4 '
end
if left("foobar",0) \=      ""             then do
  errors=errors+1
  say 'LEFT failed in test          5 '
end
if left("foobar",6) \=      "foobar"       then do
  errors=errors+1
  say 'LEFT failed in test          6 '
end
if left("foobar",8) \=      "foobar  "     then do
  errors=errors+1
  say 'LEFT failed in test          7 '
end
if left("foobar",8,'*') \=  "foobar**"     then do
  errors=errors+1
  say 'LEFT failed in test          8 '
end
if left("foobar",1,'*') \=  "f"            then do
  errors=errors+1
  say 'LEFT failed in test          9 '
end

return errors<>0

/* function prototype */
left: procedure = .string
arg string1 = .string, length = .int, pad = ' '


