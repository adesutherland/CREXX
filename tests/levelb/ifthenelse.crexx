options levelb

/*- - I F / T H E N / E L S E - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
if 1 then ok=ok ! 'If Then'
     else say '*** Bad *** If Then'
if 0 then say '*** Bad *** If Else'
     else ok=ok ! 'If Else'
if 1; then flag=1
      else flag=0
if flag;
  /* Test dummy truly Null clauses before THEN */;
      then flag=1
      else flag=0
if 0; then flag=0
      else
      i=i
if flag=1  then ok=ok ! 'If ..;Then'
           else say '*** Bad *** If ..;then'

/* Test THEN special processing */
IF 1=1
  then a1=1
IF 1=1
  then
  a2=1
then='OK1'
a3 = then='OK1'
then = 'ok2'
a4 = then='ok2'
then /* comment */ = 'ok3'
a5 = then='ok3'
then: a6=1
IF 1=1
  then,
  a7=1
if 1=1 then,
  a8=1
if 1=1,
  then  a9=1
if a1=1 & a2=1 & a3=1 & a4=1 & a5=1 & a6=1 & a7=1 & a8=1 & a9=1
  then ok=ok ! 'Then'
  else say '*** Bad *** Then processing'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
