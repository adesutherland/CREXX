/* rexx */
options levelb

/* SUBSTR */
say "Look for SUBSTR OK"
/* These from the Rexx book. */
/* say '|'substr('abc',2)'|' */
if substr('abc',2) \= 'bc'            then say 'failed in test          1 '
if substr('abc',2,4) \= 'bc  '        then say 'failed in test          2 '
if substr('abc',2,6,'.') \= 'bc....'  then say 'failed in test          3 '
/* These from Mark Hessling. */
if substr("foobar",2,3) \=   "oob"    then say 'failed in test          4 '
/* say '|'substr('foobar',3)'|' */
if substr("foobar",3) \=   "obar"     then say 'failed in test          5 '
if substr("foobar",3,6) \=   "obar  " then say 'failed in test          6 '
if substr("foobar",3,6,'*') \=   "obar**"  then say 'failed in test          7 '
if substr("foobar",6,3) \=   "r  "    then say 'failed in test          8 '
if substr("foobar",8,3) \=   "   "    then say 'failed in test          9 '
if substr('1234567890',5) \= '567890' then say 'failed in test          10 '
/* say '|'substr('1234567890',5)'|' */
if substr('1234567890',6,6,'.') \= '67890.' then say 'failed in test          11 '
if substr('abc',2,4,'.') \= 'bc..'    then say 'failed in test          12 '
if substr('abcdefgh',1,2,'.') \= 'ab' then say 'failed in test          13 '
if substr('abcdefgh',2,3,'é') \= 'bcd'then say 'failed in test          14 '
if substr("René Vincent Jansen",1,4,".") \= 'René' then say 'failed in test          15 '
if substr("René Vincent Jansen",6,7,"") \= 'Vincent' then say 'failed in test          16 '
if substr("12345678",5,6,"é") \= '5678éé' then say 'failed in test          17 '
/* say substr("12345678",10,6,"é") */
/* if substr("12345678",10,6,"éé") \= 'éééééé' then say 'need exceptions for this' */

say "SUBSTR OK"

return

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
  arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '


