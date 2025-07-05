/* rexx */
options levelb
import rxfnsb

errors=0

/* These from TRL */

if right('abc  d',8) \= '  abc  d'        then do
  errors=errors+1
  say 'RIGHT failed in test          1 '
end
if right('abc def',5) \= 'c def'          then do
  errors=errors+1
  say 'RIGHT failed in test          2 '
end
if right('12',5,'0') \= '00012'           then do
  errors=errors+1
  say 'RIGHT failed in test          3 '
end
/* These from Mark Hessling. */
if right("",4) \=            "    "                    then do
  errors=errors+1
  say 'RIGHT failed in test          4 '
end
if right("foobar",0) \=      ""                        then do
  errors=errors+1
  say 'RIGHT failed in test          5 '
end
if right("foobar",3) \=      "bar"                     then do
  errors=errors+1
  say 'RIGHT failed in test          6 '
end
if right("foobar",6) \=      "foobar"                  then do
  errors=errors+1
  say 'RIGHT failed in test          7 '
end
if right("foobar",8) \=      "  foobar"                then do
  errors=errors+1
  say 'RIGHT failed in test          8 '
end
if right("foobar",8,'*') \=  "**foobar"                then do
  errors=errors+1
  say 'RIGHT failed in test          9 '
end
if right("foobar",4,'*') \=  "obar"                    then do
  errors=errors+1
  say 'RIGHT failed in test         10 '
end

/* these from us in crexx */
/* say "Test of Right function, 3 args" */


if right("abc  d",8,'.') \=  "..abc  d"                    then do
  errors=errors+1
  say 'RIGHT failed in test         11 '
end
/* say "Test of Right function, 3 args" */
if right("12",5,'0') \=  "00012"                    then do
  errors=errors+1
  say 'RIGHT failed in test         12 '
end
/* say "Test of Right function, non-copies concatenate version" */
if right("the quick brown fox",3) \=  "fox"                    then do
  errors=errors+1
  say 'RIGHT failed in test         13 '
end
/* say "Test of Right function, non-copies concatenate version" */
if right("abcdefghijklmnopqrstuvwxyz",6) \=  "uvwxyz"                    then do
  errors=errors+1
  say 'RIGHT failed in test         14 '
end
return errors<>0
