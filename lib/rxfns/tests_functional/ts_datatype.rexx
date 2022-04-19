/* test the datatype built-in function */
options levelb
/* DATATYPE */
errors=0
/* These from the Rexx book. */
  if datatype(' 12 ') \= 'NUM'      then exit
  if datatype('') \= 'CHAR'         then exit
/*   if datatype('123*') \= 'CHAR'     then exit */
/*   if datatype('12.3','N') \= 1       then exit */
/*   if datatype('12.3','W') \= 0       then exit */
/*   if datatype('Fred','M') \= 1       then exit */
/*   if datatype('','M')     \= 0       then exit */
/*   if datatype('Minx','L') \= 0       then exit */
/* /\* This one depends on which characters are "extra letters" *\/ */
/*   if datatype('3d?','s') \= 1        then exit */
/*   if datatype('BCd3','X') \= 1       then exit */
/*   if datatype('BC d3','X') \= 1      then exit */
/* /\* These from Mark Hessling. *\/ */
/*   if datatype("foobar") \=  "CHAR"               then exit */
/*   if datatype("foo bar") \=  "CHAR"              then exit */
/*   if datatype("123.456.789") \=  "CHAR"          then exit */
/*   if datatype("123.456") \=  "NUM"               then exit */
/*   if datatype("") \=  "CHAR"                     then exit */
/*   if datatype("DeadBeef" , 'A') \=  "1"          then exit */
/*   if datatype("Dead Beef",'A') \=  "0"           then exit */
/*   if datatype("1234ABCD",'A') \=  '1'            then exit */
/*   if datatype("",'A') \=  "0"                    then exit */
/*   if datatype("foobar",'B') \=  "0"              then exit */
/*   if datatype("01001101",'B') \=  "1"            then exit */
/*   if datatype("0110 1101",'B') \=  "1"           then exit */
/*   if datatype("",'B') \=  "1"                    then exit */
/*   if datatype("foobar",'L') \=  "1"              then exit */
/*   if datatype("FooBar",'L') \=  "0"              then exit */
/*   if datatype("foo bar",'L') \=  "0"             then exit */
/*   if datatype("",'L') \=  "0"                    then exit */
/*   if datatype("foobar",'M') \=  "1"              then exit */
/*   if datatype("FooBar",'M') \=  "1"              then exit */
/*   if datatype("foo bar",'M') \=  "0"             then exit */
/*   if datatype("FOOBAR",'M') \=  "1"              then exit */
/*   if datatype("",'M') \=  "0"                    then exit */
/*   if datatype("foo bar",'N') \=  "0"             then exit */
/*   if datatype("1324.1234",'N') \=  "1"           then exit */
/*   if datatype("123.456.789",'N') \=  "0"         then exit */
/*   if datatype("",'N') \=  "0"                    then exit */
/*   if datatype("foo bar",'S') \=  "0"             then exit */
/*   if datatype("??@##_Foo$Bar!!!",'S') \=  "1"    then exit */
/*   if datatype("",'S') \=  "0"                    then exit */
/*   if datatype("foo bar",'U') \=  "0"             then exit */
/*   if datatype("Foo Bar",'U') \=  "0"             then exit */
/*   if datatype("FOOBAR",'U') \=  "1"              then exit */
/*   if datatype("",'U') \=  "0"                    then exit */
/*   if datatype("Foobar",'W') \=  "0"              then exit */
/*   if datatype("123",'W') \=  "1"                 then exit */
/*   if datatype("12.3",'W') \=  "0"                then exit */
/*   if datatype("",'W') \=  "0"                    then exit */
/*   if datatype('123.123','W') \=  '0'             then exit */
/*   if datatype('123.123E3','W') \=  '1'           then exit */
/*   if datatype('123.0000003','W') \=  '1'         then exit */
/*   if datatype('123.0000004','W') \=  '1'         then exit */
/*   if datatype('123.0000005','W') \=  '0'         then exit */
/*   if datatype('123.0000006','W') \=  '0'         then exit */
/*   if datatype(' 23','W') \=  '1'                 then exit */
/*   if datatype(' 23 ','W') \=  '1'                then exit */
/*   if datatype('23 ','W') \=  '1'                 then exit */
/*   if datatype('123.00','W') \=  '1'              then exit */
/*   if datatype('123000E-2','W') \=  '1'           then exit */
/*   if datatype('123000E+2','W') \=  '1'           then exit */
/*   if datatype("Foobar",'X') \=  "0"              then exit */
/*   if datatype("DeadBeef",'X') \=  "1"            then exit */
/*   if datatype("A B C",'X') \=  "0"               then exit */
/*   if datatype("A BC DF",'X') \=  "1"             then exit */
/*   if datatype("123ABC",'X') \=  "1"              then exit */
/*   if datatype("123AHC",'X') \=  "0"              then exit */
/*   if datatype("",'X') \=  "1"                    then exit */
/*   if datatype('0.000E-2','w') \=  '1'            then exit */
/*   if datatype('0.000E-1','w') \=  '1'            then exit */
/*   if datatype('0.000E0','w') \=  '1'             then exit */
/*   if datatype('0.000E1','w') \=  '1'             then exit */
/*   if datatype('0.000E2','w') \=  '1'             then exit */
/*   if datatype('0.000E3','w') \=  '1'             then exit */
/*   if datatype('0.000E4','w') \=  '1'             then exit */
/*   if datatype('0.000E5','w') \=  '1'             then exit */
/*   if datatype('0.000E6','w') \=  '1'             then exit */
/*   if datatype('0E-1','w') \=  '1'                then exit */
/*   if datatype('0E0','w') \=  '1'                 then exit */
/*   if datatype('0E1','w') \=  '1'                 then exit */
/*   if datatype('0E2','w') \=  '1'                 then exit */

return errors <> 0

/* function prototype */
datatype: procedure = .string 
arg expose string_in = .string, Type = ""
