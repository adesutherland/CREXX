/* rexx */
options levelb
errors=0

/* SUBSTR */
/* do 1000000 */
/* These from the Rexx book. */
/* say '|'substr('abc',2)'|' */
if substr('abc',2) \= 'bc'            then
  do
    errors=errors+1
    say 'SUBSTR failed in test 1 '
  end
if substr('abc',2,4) \= 'bc  '        then
  do errors=errors+1
    say 'SUBSTR failed in test 2 '
  end
if substr('abc',2,6,'.') \= 'bc....'  then
  do errors=errors+1
    say 'SUBSTR failed in test 3 '
  end
/* These from Mark Hessling. */
if substr("foobar",2,3) \=   "oob"    then
  do errors=errors+1
    say 'SUBSTR failed in test 4 '
  end
	/* say '|'substr('foobar',3)'|' */
if substr("foobar",3) \=   "obar"     then
  do
    errors=errors+1
    say 'SUBSTR failed in test 5 '
  end
if substr("foobar",3,6) \=   "obar  " then
  do errors=errors+1
    say 'SUBSTR failed in test 6 '
  end
if substr("foobar",3,6,'*') \=   "obar**"  then
  do errors=errors+1
    say 'SUBSTR failed in test 7 '
  end
if substr("foobar",6,3) \=   "r  "    then
  do
    errors=errors+1
    say 'SUBSTR failed in test 8 '
  end
if substr("foobar",8,3) \=   "   "    then
  do errors=errors+1
    say 'SUBSTR failed in test 9 '
  end
if substr('1234567890',5) \= '567890' then
  do errors=errors+1
    say 'SUBSTR failed in test 10 '
  end
/* say '|'substr('1234567890',5)'|' */
if substr('1234567890',6,6,'.') \= '67890.' then
  do
    errors=errors+1
    say 'SUBSTR failed in test 11 '
  end
if substr('abc',2,4,'.') \= 'bc..'    then
  do
    errors=errors+1
    say 'SUBSTR failed in test 12 '
  end
if substr('abcdefgh',1,2,'.') \= 'ab' then
  do errors=errors+1
    say 'SUBSTR failed in test 13 '
  end
if substr('abcdefgh',2,3,'é') \= 'bcd' then
  do errors=errors+1
    say 'SUBSTR failed in test 14 '
  end
if substr("René Vincent Jansen",1,4,".") \= 'René' then
  do
    errors=errors+1
    say 'SUBSTR failed in test 15 '
  end
/* if substr("René Vincent Jansen",6,7,"") \= 'Vincent' then */
/*   do errors=errors+1 */
/*     say 'SUBSTR failed in test 16 ' */
/*   end */ /* this is actually a case of Error 40.23:  SUBSTR argument 4 must be a single character; found "". */ 
if substr("René Vincent Jansen",6,7,"") \= 'Vincent' then
  do errors=errors+1
    say 'SUBSTR failed in test 16 '
  end
if substr("12345678",5,6,"é") \= '5678éé' then
  do errors=errors+1
    say 'SUBSTR failed in test 17 '
  end
/* say substr("12345678",10,6,"é") */
/* if substr("12345678",10,6,"éé") \= 'éééééé' then do errors=errors+1;  say 'need exceptions for this'; end */
/* end */
return errors<>0

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
  arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '


