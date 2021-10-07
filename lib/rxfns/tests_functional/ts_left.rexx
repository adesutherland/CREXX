/* rexx */
options levelb
/* These from TRL */
say "Look for LEFT OK"
if left('abc d',8) \= 'abc d   '  then say 'failed in test          1 '
if left('abc d',8,'.') \= 'abc d...'  then say 'failed in test          2 '
if left('abc  def',7) \= 'abc  de'  then say 'failed in test          3 '
/* These from Mark Hessling. */
if left("foobar",1) \=      "f"            then say 'failed in test          4 '
if left("foobar",0) \=      ""             then say 'failed in test          5 '
if left("foobar",6) \=      "foobar"       then say 'failed in test          6 '
if left("foobar",8) \=      "foobar  "     then say 'failed in test          7 '
if left("foobar",8,'*') \=  "foobar**"     then say 'failed in test          8 '
if left("foobar",1,'*') \=  "f"            then say 'failed in test          9 '
say "LEFT OK"

/* function prototype */
left: procedure = .string
arg string1 = .string, length = .int, pad = ' '


