/* rexx */
options levelb


/* These from TRL */
say "Look for RIGHT OK"
if right('abc  d',8) \= '  abc  d'        then say 'failed in test          1 '
if right('abc def',5) \= 'c def'          then say 'failed in test          2 '
if right('12',5,'0') \= '00012'           then say 'failed in test          3 '
/* These from Mark Hessling. */
if right("",4) \=            "    "                    then say 'failed in test          4 '
if right("foobar",0) \=      ""                        then say 'failed in test          5 '
if right("foobar",3) \=      "bar"                     then say 'failed in test          6 '
if right("foobar",6) \=      "foobar"                  then say 'failed in test          7 '
if right("foobar",8) \=      "  foobar"                then say 'failed in test          8 '
if right("foobar",8,'*') \=  "**foobar"                then say 'failed in test          9 '
if right("foobar",4,'*') \=  "obar"                    then say 'failed in test         10 '

/* these from us in crexx */
/* say "Test of Right function, 3 args" */
if right("abc  d",8,'.') \=  "..abc  d"                    then say 'failed in test         11 '
/* say "Test of Right function, 3 args" */
if right("12",5,'0') \=  "00012"                    then say 'failed in test         12 '
/* say "Test of Right function, non-copies concatenate version" */
if right("the quick brown fox",3) \=  "fox"                    then say 'failed in test         13 '
/* say "Test of Right function, non-copies concatenate version" */
if right("abcdefghijklmnopqrstuvwxyz",6) \=  "uvwxyz"                    then say 'failed in test         14 '
say "RIGHT OK"

/* function prototype */
right: procedure = .string
arg string1 = .string, start = .int, pad = ' '

