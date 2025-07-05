/**********************************************************************/
/* REXDIAG: Primary diagnostic package for REX  CMS/3.50 level.       */
/*  Invoke with arbitrary argument string or:                         */
/*     TRACE - to run with tracing on                                 */
/*     ERROR - to run error message test only                         */
/*     !!! string - to have specific string interpreted               */
/*********************************************************************/
/* Change activity:                                                  */
/*14 Sep 2010  jph Corrected to fix signal with mixed case.          */
/*                                                                   */
/* A  few changes have been made to get this test case to run (1) on */
/* current  z/VM  CMS,  and  (2) under Linux.  Of course, the latter */
/* requires an interpreter that can be cajoled into doing at least a */
/* minimum  of  CMS  commands  to the effect of setting the expected */
/* return  codes.   All  such  changes are identified by the comment */
/* jph.                                                              */
/**********************************************************************/
/*                                                                    */
/* Testing strategy used in REXDIAG EXEC.                             */
/*                                                                    */
/* The REX interpreter may be considered to be split into several     */
/* sub-systems, corresponding roughly to the modules forming the      */
/* interpreter (see internal structure overview included in REXINT):  */
/*                                                                    */
/*   Control structures   (REXINT)                                    */
/*   Stand-alone clauses  (REXEXEC)                                   */
/*   Expressions ..       (REXEVAL)                                   */
/*     .. and variables   (REXVAR)                                    */
/*     .. and conversions (REXCONV)                                   */
/*   Tracing              (REXTRACE)                                  */
/*   External interfaces  (REXMAIN & REXEXT)                          */
/*   Functions            (REXFUN - tested by FNSDIAG mainly)         */
/*                                                                    */
/* REXDIAG therefore follows the following overall strategy:          */
/*                                                                    */
/* 1) Ensure communication is OK by doing a trivial SAY               */
/* 2) Control structures                                              */
/*   a) Check basic control structure (ability to make a decision)    */
/*        i.e. IF-THEN-ELSE                                           */
/*      Includes trivial  (opand-op-opand) expressions                */
/*   b) Check DO groups and loops                                     */
/*   c) Check SELECTs                                                 */
/* 3) Expressions and operators                                       */
/*   a) Logical operators (used from here on) and equalities          */
/*   b) Comparisons and strict comparisons                            */
/*   c) Basic Arithmetic (+ - / * // % **)                            */
/*   d) Extended Arithmetic (>9 places etc)                           */
/*   e) Concatenation operators                                       */
/* 4) Stand-alone statements                                          */
/*   a) Drop/Nop                                                      */
/*   b) Stack (Push Pull Queue) and PARSE                             */
/*   c) Commands, Address                                             */
/*   d) Interpret                                                     */
/*   e) Signal and exceptions                                         */
/*   f) Call & subroutines                                            */
/*   g) Arrays, compound variables, continuations, Expose             */
/*   h) Function calls (built-in and external)                        */
/*   i) ARG, Internal functions (Includes all of the above,           */
/*      called recursively)                                           */
/* 5) Error messages - one of each                                    */
/*                                                                    */
/* With the exception of section (5), the tests only exercise syntax  */
/* which is correct (sometimes obscurely correct).                    */
/*                                                                    */
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
/* Testing areas NOT covered by this Exec are:                        */
/*                                                                    */
/* a) Built-in functions - see REXFUNDG EXEC                          */
/* b) Tracing - see TRACDIAG EXEC                                     */
/* c) Interactive debugging - see TRDIAG EXEC                         */
/*                                                                    */
/* Areas which require further specific testing are:                  */
/*                                                                    */
/* Obscure error paths: all error paths were tested when first        */
/*   written, but possibly may have become invalidated since that     */
/*   time.  Errors in REXDIAG are only generated by one path whereas  */
/*   in reality there may be several paths.  Locating  NMxxxx equates */
/*   in the source will identify all error exits.                     */
/*                                                                    */
/* General error conditions which have not been allowed for in the    */
/*   code.  i.e. user syntax or semantic errors which may not be      */
/*   correctly detected by the interpreter.  There are none of these  */
/*   known at present of course...                                    */
/*                                                                    */
/* External interfaces.  Some are tested herein (e.g. Host Commands)  */
/*   but most have been tested by being used (e.g. direct variables   */
/*   interface is exercised by FSX, IOX, etc.).  Specific test cases  */
/*   for these interfaces should be written.                          */
/*                                                                    */
/**********************************************************************/
parse arg spec String
  /* If called with 1st arg is '!!!' then execute rest of args  */
  /*   string to cause syntax error or whatever                 */
  if spec='!!!' then do; say; Interpret String; exit; end
/* drop through to here if any other call */
if spec='TRACE' then Trace ?rrrr; else Trace Off
parse version vers          /* Hope this works, at least */
parse source  source        /*  ditto */
parse source opsys fcall name .       /* ditto [jph]                 */
if spec='ERROR' then signal errors
if fcall='FUNCTION' then signal funccall
if fcall='SUBROUTINE' then signal subcall
trace o
'CLEAR'

say vers '/' source   /* say what level we are running under */

/* Next label (a) ensures lookaside is active, and (b) is start */
/* point when we call ourself as an internal function.          */
intfun:        /* Come here when self-call as internal function */

/*- - S A Y - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
say 'OK ! Say '
ok='OK'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/


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

/*- - D O - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
i=0
do 10
  i=i+1
  end
if i=10 then ok=ok ! 'Explicit Do'
        else say '*** Bad *** Explicit do'

j=0; m=0
do 5-2 while 1=1   /* possible DO block problem */
  m=m+2
  end
do while i<12
  i=i+1; j=j+1
  end
do k=1+2 to 3+1 while 1=1  /* checks storage allocation self-check */
  end
if i=12&j=2&k=5&m=6 then ok=ok ! 'Do While'
                    else say '*** Bad *** Do While'

j=0
do until i=15
  i=i+1; j=j+1
  end
if i=15&j=3 then ok=ok ! 'Do Until'
            else say '*** Bad *** Do Until'
say ok; ok='OK'

/* Test iterative Do loops of various kinds */
j=0; flag='1'
do i  =  1 to 12 by 2 while(i<=7)
  j=j+1; end
if j\=4 | i\=9 then flag='0'

j=0
do i=0-1 to(10-22)by 20-22 until i<-7
  j=j+1; end
if j\=5 | i\=-9 then flag='0'

j=0
do i=1 to 8 until i>3; j=j+1; end
if i><4 | j<>4 then flag='0'

j=0
do j+7 until j=12
  j=j+1; end
if j\=7 then flag='0'

j=0
do j+97 until j=12
  j=j+1; end
if j\=12 then flag='0'

do i=1 to 45; end
if i\=46 then flag='0'

do a.3 = 2 to 2  /* check loop var with substitution (was bug) */
  if a.3\=2 then flag='0'
  end
p=3
do a.p = 5 to 5
  if a.3\=5 then flag='0'
  end

j=0; do forever
   j=j+1; if j=6 then leave; end
if j\=6 then flag=0

/* Test iterative Do loops with non-integer arguments */
k=1
/* These should all iterate once */
do 0.9999999999    /* legal as will round to 1 */
  k=k+1
  end
flag=flag&k=2
do i=0.9999999999 to 1 by -1
  k=k+1
  end
flag=flag&k=3
do i=1 to 0.9999999999
  k=k+1
  end
flag=flag&k=4
/* Now a bit more complicated */
k=0
do i=-0.5 to 0.5 by 0.1
  k=k+1
  end
flag=flag&k=11
do i=0+0.5 to 0-0.5 by ' - 1E-1 '
  k=k+1
  end
flag=flag&k=22
k=0
do i=-0.5 to 0.5 by 0.1 until i>0.3
  k=k+1
  end
flag=flag&i=0.4 &k=10
k=0
do i=-5E-1 to 5E-1 by 0.1 while i<0.3
  k=k+1
  end
flag=flag&i=0.3 &k=8
k=0
do i=-5E-1 to 5E-1 by 0.1 for 1
  k=k+1
  end
flag=flag&k=1
k=0
do i=1 to 10 for 9
  k=k+1
  end
flag=flag&k=9 &i=10
k=0
do i=1 to 10 for 10
  k=k+1
  end
flag=flag&k=10&i=11
k=0
do i=1 to 10 for 11
  k=k+1
  end
flag=flag&k=10&i=11

trace ooo
if flag then ok=ok ! 'Do I=nn..'
        else say '*** Bad *** Do I=nn..'

i=1
j=3; do j=1 to j
   i=i+1
   end

if i=4 then ok=ok ! 'Do j=1 to j'
       else say '*** Bad *** Do J=1 to J'

j=0; k=0
do i=1 by 1 until j=10
  j=j+1
  if j=5 then leave
  if j=3 then do
    k=9
    iterate
    end
  k=k+1
  end
if j=5 & k=10 then flag=1
              else flag=0
k=0; l=0
do i=1 to 10
   k=k+1
   do j=6.0 to 1*1+(1-1)by -1
     l=l-1
     if j>3 then iterate j
     l=l-1
     if i=2 then leave i
            else leave j
     l=124 dead
     end
   k=k+5
   end
if l\=-10 | k\=7 then flag=0

if flag then ok=ok ! 'Leave/Iterate'
        else say '*** Bad *** Leave/Iterate'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - S E L E C T - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
i=4
select
  when i=1 then say '*** Bad *** When'
  when i=4 then ok=ok '! When'
  when i=6 then say '*** Bad *** When'
end;
select
  when i=1 then say '*** Bad *** When/Otherwise'
  when i=6 then say '*** Bad *** When/Otherwise'
  otherwise ok=ok '! Otherwise'
end;
j=3
select
  when i=1 then say '*** Bad *** When'
  when i=4 then select
     when j=1 then say '*** Bad nested when ***'
     when j=3 then ok=ok '! nested Select'
     when j=5 then say '*** Bad nested when ***'
     otherwise say '*** Bad nested when ***'
    end
  otherwise say '*** Bad *** Otherwise'
end;
say ok; ok='OK'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/


/*- - E X P R E S S I O N S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/

/*** LOGICAL operators and comparisons ***/
if \'0'     then ok=ok ! '\'
            else say '*** Bad *** \'
if \'0'     then ok=ok ! '\'
            else say '*** Bad *** \'
if 1&1 &\0  then ok=ok ! '&'
            else say '*** Bad *** &'
if 1&&0 &\(1&&1)  then ok=ok ! '&&'
            else say '*** Bad *** &&'
if 0 | 1    then ok=ok ! '|'
            else say '*** Bad *** |'
If opsys='CMS'
   Then r='F0 c1'x='0A' & 'c7   d5'x='GN' & 'f0'x=0
   else r='30 41'x='0A' & '47   4e'x='GN' & '30'x=0
if 'a'='a' & 'a'='a ' & ' a'='a' & ''='  ',
 & 7=07 & 6=' + 6 ' & 5=' 5.00'  & 4.0=4.000,
 & r & ''x=''
            then ok=ok ! '='
            else say '*** Bad *** "="'
if 2==02 | 7==7.0 | ' a '=='a' | 'b'=='b ' | 'c'==' c',
   | \(2==2) | \('a'=='a')
            then say '*** Bad *** =='
            else ok=ok ! '=='
if 2\==02 & 7\==7.0 & ' a '\=='a' & 'b'\=='b ' & 'c'\==' c',
   & \(2\==2) & \('a'\=='a')
            then ok=ok ! '\=='
            else say '*** Bad *** \=='
if 1<>3 & 3><1 & 5\=6 & 8\=9,
 & 3>2 & 4<5 & 3<=3 & 7>=2 & 7\=4 & 3\>7 & 9\<6 & \(' '=='')
            then ok=ok ! 'Comparators'
            else say '*** Bad *** Comparators'

if 7<070 & 7>>070 & 7\>070 & 7\<<070,
 & 7<070 & 7>>=070 & 7\>070 & 007<<=070,
 & 77<<770 & 770>>77 & 07<<7 & 1>>01
            then ok=ok ! 'Stricts'
            else say '*** Bad *** Strict Comparators'

if 1+2*3==7      ,
 & (1+2)*3==9    ,
 & 3*7-1==20     ,
 & 1'2'==12      ,
 & '33'77==3377  ,
 & (67)76==6776  ,
 & (8)(9)==89    ,
 & (8)(3)*4==812 ,
 & (8)'3'*4==812 ,
 & 8'3'*4==812   ,
 & '8'3*4==812   ,
 & (12=(1)(2)) ==1    ,
 & ((1)(2)=12) ==1
            then ok=ok ! 'Precedence'
            else say '*** Bad *** Precedence'

trace o
say ok; ok='OK'

/** Arithmetic operators **/
flag=0
if 2+3==5  &  5.75+3.3==9.05  &  5+'-3'==2 & 0.7+0.3==1.0,
  & 1.23456789+1.00000000 == 2.23456789 then flag=1
flag= flag & 1.00000000     == 0.4444444444+0.5555555555
flag= flag & 0.444444444    == 0+0.4444444444999
flag= flag & 0.444444445    == 0+0.4444444445000
flag= flag & 2.500          == 1.250 + 1.250
flag= flag & 1.0000E+13     == 70      + 10000e9
flag= flag & 1.0000E+13     == 700     + 10000e+9
flag= flag & 1.0000E+13     == 7000    + 10000e9
flag= flag & 1.00000001E+13 == 70000   + 10000e9
flag= flag & 1.00000007E+13 == 700000  + 10000e+9
if flag then ok=ok ! '+'
        else say '*** Bad *** +'

if 2-3==-1 &  5.03-3.1==1.93  & '-5.0'-12.753==-17.753 & 1.3-0.3==1.0,
  & 1.23456789-1.00000000 == 0.23456789
  then flag=1
  else flag=0
flag= flag & -0.0000001     == 10.23456784-10.23456789
flag= flag & 0              == 10.23456785-10.23456789
flag= flag & 0              == 10.23456786-10.23456789
flag= flag & 0              == 10.23456787-10.23456789
flag= flag & 0              == 10.23456788-10.23456789
flag= flag & 0              == 10.23456789-10.23456789
flag= flag & 0              == 10.23456790-10.23456789
flag= flag & 0              == 10.23456791-10.23456789
flag= flag & 0              == 10.23456792-10.23456789
flag= flag & 0              == 10.23456793-10.23456789
flag= flag & 0.0000001      == 10.23456794-10.23456789

flag= flag & -0.0000001     == 10.23456781-10.23456786
flag= flag & 0              == 10.23456782-10.23456786
flag= flag & 0              == 10.23456783-10.23456786
flag= flag & 0              == 10.23456784-10.23456786
flag= flag & 0              == 10.23456785-10.23456786
flag= flag & 0              == 10.23456786-10.23456786
flag= flag & 0              == 10.23456787-10.23456786
flag= flag & 0              == 10.23456788-10.23456786
flag= flag & 0              == 10.23456789-10.23456786
flag= flag & 0              == 10.23456790-10.23456786
flag= flag & 0.0000001      == 10.23456791-10.23456786

flag= flag & 0              == 1-0.999999999
flag= flag & 0              == 0.999999999-1
if flag then ok=ok ! '-'
        else say '*** Bad *** -'

flag=1
flag=flag&    5+3*2         == 11  /* checks precedence */
if flag & 2*3==6  &  5.09*7.1==36.139  & 2.5*4==10.0,
  & 1.23456789*1.00000000 == 1.23456789,
  & 9.999999999 * 9.9999999999 == 100.000000
          then ok=ok ! '*'
          else say '*** Bad *** "*"'
flag=1
flag=flag& 1     / 3        == 0.333333333
flag=flag& 2.4   / 1        == 2.4
flag=flag& 2.40  / 1        == 2.4
flag=flag& 2.400 / 1        == 2.4
flag=flag& 2.4   / 2        == 1.2
flag=flag& 2     / 2        == 1
flag=flag& 20    / 20       == 1
flag=flag& 187   / 187      == 1
flag=flag& 2.400 / 2        == 1.2
flag=flag& 5     / 2.000    == 2.5
flag=flag& 5     / 0.200    == 25
flag=flag& 5     / 2        == 2.5
if flag then ok=ok ! '/'
        else say '*** Bad *** /'
flag=1
flag=flag& 101.0 % 3       == 33
flag=flag& 2.4   % 1       == 2
flag=flag& 2.400 % 1       == 2
flag=flag& 18    % 18      == 1
flag=flag& 1120  % 1000    == 1
flag=flag& 2.4   % 2       == 1
flag=flag& 2.400 % 2       == 1
flag=flag& 0.5   % 2.000   == '0'
flag=flag& 8.005 % 7       == 1
flag=flag& 5     % 2       == 2
if flag then ok=ok ! '%'
        else say '*** Bad *** %'
flag=1
flag=flag&  5//5      == 0
flag=flag&  13//10    == 3
flag=flag&  13//50    == 13
flag=flag&  13//100   == 13
flag=flag&  13//1000  == 13
flag=flag&  .133//1   == 0.133
flag=flag&  .1033//1  == 0.1033
flag=flag&  1.033//1  == 0.033
flag=flag&  10.33//1  == 0.33
flag=flag&  10.33//10 == 0.33
flag=flag&  103.3//1  == 0.3
flag=flag&  133//10   == 3
flag=flag&  1033//10  == 3
flag=flag&  133//50   == 33
flag=flag& 101.0 // 3       == '2.0'
flag=flag& 102.0 // 3       == '0'
flag=flag& 2.4   // 1       == 0.4
flag=flag& 2.40  // 1       == 0.40
flag=flag& 2.400 // 1       == 0.400
flag=flag& 2.4   // 2       == 0.4
flag=flag& 2.400 // 2       == 0.400
flag=flag& 0.5   // 2.001   == 0.5
flag=flag& 0.03  // 7       == 0.03
flag=flag& 5     // 2       == 1
flag=flag& 1.2   // .7345   == 0.4655
flag=flag& 0.8   // 1       == 0.8
flag=flag& 0.800 // 1.7     == 0.800
trace ooo
if flag    then ok=ok ! '//'
           else say '*** Bad *** //'

flag=1
flag=flag&     0**0         == 1
flag=flag&     0**1         == 0
flag=flag&     0**1         == 0
flag=flag&     1**0         == 1
flag=flag&   0.3**0         == 1
flag=flag&    10**999999999 == 1e+999999999
flag=flag&    10**999999998 == 1e+999999998
flag=flag&    10**999999997 == 1e+999999997
flag=flag&    10**333333333 == 1e+333333333
flag=flag&    10**77        == 1e+77
flag=flag&    10**22        == 1e+22
flag=flag&    10**-77       == 1e-77
flag=flag&    10**-22       == 1e-22
flag=flag&     2**-1        == 0.5
flag=flag&     2**-2        == 0.25
flag=flag&     2**-4        == 0.0625
flag=flag&    5*3**2        == 45  /* checks precedence */
x=0.50; temp=1
do n=1 to 10
  temp=temp*x/1
  flag=flag&   x**n         == temp
  end
x=2; temp=1

/* The   30th  test  case  shows  different  rounding  for  the  two */
/* algorithms.   The power result is 9313225746, but the temp one is */
/* 9313225780;  both  before  rounding.  This leads to 5 vs 8 on the */
/* last  digit after rounding.  The infinite precision (digits 1000) */
/* result  is  '0.000000000931322574615478515625,  thus  **  is more */
/* correct.  jph                                                     */

do n=1 to 29                          /* jph                         */
  temp=temp*x/1
  flag=flag&   x**n         == temp
  flag=flag&   x**-n        == 1/temp
  /* Note that rounding errors are possible for larger "n" */
  /* due to the information content of the exponent        */
  end
flag=flag & x**-30          == '0.000000000931322575'         /* jph */
flag=flag & 1/(temp*x/1)    == '0.000000000931322578'         /* jph */
if flag    then ok=ok ! '**'
           else say '*** Bad *** "**"'

if \0==1 &,
   \1==0    then ok=ok ! 'Prefix \'
            else say '*** Bad *** Prefix \'
if ++7E1==70 &,
   +3==3    then ok=ok ! 'Prefix +'
            else say '*** Bad *** Prefix +'
if - -7E1==70 &,
   -9=='-9' then ok=ok ! 'Prefix -'
            else say '*** Bad *** Prefix -'
say ok; ok='OK'

/** Arithmetic and PARSE NUMERIC **/
nflag=1  /* Parse numeric */
flag=1   /* Arithmetic */
parse numeric pn
nflag=nflag&  pn='9 0 SCIENTIFIC'  /* check defaults */
flag= flag & 0              == (1.23456784=1.23456789)
flag= flag & 0              == (1.23456785=1.23456789)
flag= flag & 0              == (1.23456786=1.23456789)
flag= flag & 0              == (1.23456787=1.23456789)
flag= flag & 0              == (1.23456788=1.23456789)
flag= flag & 1              == (1.23456789=1.23456789)
flag= flag & 0              == (1.23456790=1.23456789)
flag= flag & 0              == (1.23456791=1.23456789)
flag= flag & 0              == (1.23456792=1.23456789)
flag= flag & 0              == (1.23456793=1.23456789)
flag= flag & 0              == (1.23456794=1.23456789)
flag= flag & 0              == (1         =0.99999999)
flag= flag & 0              == (0.99999999=1          )
/* Check guard digit rounding... */
flag= flag & 0              == (1.123456784=1.123456789)
flag= flag & 1              == (1.123456785=1.123456789)
flag= flag & 1              == (1.123456786=1.123456789)
flag= flag & 1              == (1.123456787=1.123456789)
flag= flag & 1              == (1.123456788=1.123456789)
flag= flag & 1              == (1.123456789=1.123456789)
flag= flag & 1              == (1.123456790=1.123456789)
flag= flag & 1              == (1.123456791=1.123456789)
flag= flag & 1              == (1.123456792=1.123456789)
flag= flag & 1              == (1.123456793=1.123456789)
flag= flag & 0              == (1.123456794=1.123456789)
flag= flag & 1              == (1         =0.999999999)
flag= flag & 1              == (0.999999999=1          )
numeric fuzz 1
parse numeric pn
nflag=nflag&  pn='9 1 SCIENTIFIC'  /* check fuzz change */
flag= flag & 0              == (1.23456784=1.23456789)
flag= flag & 1              == (1.23456785=1.23456789)
flag= flag & 1              == (1.23456786=1.23456789)
flag= flag & 1              == (1.23456787=1.23456789)
flag= flag & 1              == (1.23456788=1.23456789)
flag= flag & 1              == (1.23456789=1.23456789)
flag= flag & 1              == (1.23456790=1.23456789)
flag= flag & 1              == (1.23456791=1.23456789)
flag= flag & 1              == (1.23456792=1.23456789)
flag= flag & 1              == (1.23456793=1.23456789)
flag= flag & 0              == (1.23456794=1.23456789)
flag= flag & 1              == (1         =0.99999999)
flag= flag & 1              == (0.99999999=1          )
numeric fuzz 0
flag=flag&  1e3=1000
flag=flag&  1e-3=0.001
numeric digits 3
parse numeric pn
nflag=nflag&  pn='3 0 SCIENTIFIC'
flag=flag&  1/3=0.333
flag=flag&  1/3\=0.33
numeric fuzz 1
parse numeric pn
nflag=nflag&  pn='3 1 SCIENTIFIC'
flag=flag&  1/3=0.33
flag=flag&  1/3\=0.34
numeric digits 9
flag=flag&  1/3==0.333333333
flag=flag&  0+3e10 =='3E+10'
numeric digits 100
parse numeric pn
nflag=nflag&  pn='100 1 SCIENTIFIC'
flag=flag&  1/3=='0.'||copies('3',100)
flag=flag&  2/3=='0.'||copies('6',99)||'7'
flag=flag&  (1/3)*3=='0.'||copies('9',100)
flag=flag&  ((1/3)*3)/3=='0.'||copies('3',100)
numeric digits 6
numeric fuzz   3
numeric form engineering
parse numeric pn
nflag=nflag&  pn='6 3 ENGINEERING'
/* Check defaulting mechanism... */
numeric digits
numeric fuzz
numeric form
parse numeric pn
nflag=nflag&  pn='9 0 SCIENTIFIC'
/* Check Sci/Eng switch */
numeric form engineering
parse numeric pn
nflag=nflag&  pn='9 0 ENGINEERING'
flag=flag& 0+3e10 == "30E+9"
flag=flag& 0-0.02 =='-0.02'
flag=flag& \(0.02<0)
flag=flag& \(0.02=0)
flag=flag&   0.02>0
flag=flag& \('-0.02'>0)
flag=flag&   '-0.02'<0
flag=flag& \('-0.02'>'-0.00001')
flag=flag&   '-0.02'<'-0.00001'
flag=flag&   0\='.'

If flag then ok=ok ! 'Arithmetic'
        else say '*** Bad *** Arithmetic'
If nflag then ok=ok ! 'Numeric'
        else say '*** Bad *** Numeric/Parse Numeric'

/* Now check NUMERIC settings properly saved across routines */
numeric digits 10
numeric fuzz    0
numeric form   scientific
parse numeric all1
a1=1/3
all2=setnumup()
parse numeric all3
a3=1/3
if all1==all3 & all3=='10 0 SCIENTIFIC',
  & a1==0.3333333333,
  & a1==a3 & all2==,
  '40 20 ENGINEERING 0.3333333333333333333333333333333333333333'
     then ok=ok ! 'Numeric save'
     else say '*** Bad *** Numeric save across call'

/* Reset back to defaults */
numeric fuzz
numeric digits
numeric form
trace o
say ok; ok='OK'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - C O N C A T E N A T I O N - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
if 5  1 =='5 1' then ok=ok ! '<blank>'
                else say '*** Bad *** <blank> operator'
if 6||2 =='62'  then ok=ok ! '||'
                else say '*** Bad *** || operator'
if 'X'123.4'y' =='X123.4y' then ok=ok ! '<implicit Concat>'
                else say '*** Bad *** <implicit concat> operator'
if 'X'123.4/10'y' =='X12.34y' then ok=ok ! '<compound Concat>'
                else say '*** Bad *** <compound Concat> operator'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - D R O P ,  N O P , A R R A Y  I N I T - -- -- -- -- -- -- -- -- -- -- -- -- --*/
temp='I'
a.temp='x'
a.2='y'; a.4='4'
p.1=1; p.2=2; p.3=3
i=2; ii=3
drop a.i i a.i p.
if ii==3 & i=='I' & a.2=='A.2' & a.temp=='A.I' ,
 & a.4='4' & p.1=='P.1' & p.2='P.2' & p.3='P.3'
         then ok=ok ! 'Drop'
         else say '*** Bad *** Drop'

/* Array initialisation etc... */
flag=1
a.=7; parse value '999' with bbb.
flag=flag & a.1==7
flag=flag & a.uu==7
flag=flag & symbol('a.88')=='VAR'
drop a.
flag=flag & symbol('a.88')=='LIT'
flag=flag & bbb.8==999
flag=flag & symbol('bbb.8')=='VAR'
bbb.8='New'
flag=flag & symbol('bbb.8')=='VAR'
flag=flag & bbb.8=='New'
flag=flag & symbol('bbb.8')=='VAR'
drop bbb.8 bbb.4
flag=flag & bbb.8=='BBB.8'          /* should now be totally reset ... */
flag=flag & symbol('bbb.8')=='LIT'  /* ... and with original value */
flag=flag & bbb.4=='BBB.4'          /* should now be totally reset ... */
flag=flag & symbol('bbb.4')=='LIT'  /* ... and with original value */
flag=flag & bbb.9==999
trace o
if flag then ok=ok ! 'Array init'
        else say '*** Bad *** Array init'
say ok; ok='OK'

rc=0; if 1=1 then nop
if rc=0  then ok=ok ! 'Nop'
         else say '*** Bad *** Nop'

/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - P U S H,  Q U E U E,  P U L L - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CONWAIT'   /* CONWAITs are for debug CP commands in REX */
push 'Now  is  the   time for all '
queue 'good men to come'
pull a b c d
pull e f g .
if c=='THE' & g=='TO' & b=='IS' & d=='  TIME FOR ALL '
           then ok=ok '! Pull ! Push ! Queue'
           else say '*** Bad *** Push, Queue, or Pull'
if .='.'   then ok=ok '! . in template'
           else say '*** Bad *** "." in template'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - P A R S E   V A L U E,  P A R S E   V A R - -- -- -- -- -- -- -- -- -- -- --*/
parse value'l m n o'with e1 e2
parse upper value'l  m  n o'with f1 f2 f3
parse value with e3 e4
parse value 'aa' 1+2+3 'bb' with e5
parse value 'c/my string/1 10' with verb 2 delim +1 str (delim) rest
nl='15'x; ffuts='abcd'nl'MFC rest'
parse var ffuts (nl) wordx . +20
if wordx\==nl'MFC' then delflag=0; else delflag=1
h='come'
h='Awful'h 'its time'
parse var h h1 .
missy='abcdefghijk'; null=''
parse var missy '' -4 last4 (null) -3 last3
parse var missy '' -4 (null) -3 last3b +2
/* Now some comments in parsing templates... */
j='1 2 3 4 5'
parse var j j1 j2
parse/**/var j j3 j4
parse    var/**/j j5 j6/**/
parse    var    j/**/j7 j8
parse    var    j    j9/**/j10
jf= j1==1 & j3==1 & j5==1 & j7==1 & j9==1 & j10=='2 3 4 5'

/* test PARSE UPPER is safe:*/
parse value pup('lower') with pupret

if h1='Awfulcome' & f1=='L' & f2=='M' & f3==' N O',
 & verb=='c' & delim=='/' & str=='my string' & rest=='1 10',
 & e1='l' & e2='m n o' & e3='' & e4='' & e5='aa 6 bb' ,
 & last4='hijk' & last3='ijk' & last3b='ij',
 & pupret=='lower UPPER' & jf & delflag
           then ok=ok '! Parse'
           else say '*** Bad *** Parse'

flag=1
fred=''
parse var fred  6 x  /* has unearthed a bug in the past */
flag=flag & x=''
fred='Now is the time for all good men'
parse var fred  6 x 12
flag=flag & x=='s the '
parse var fred  6 x +6
flag=flag & x=='s the '
parse value 333 with x 1 y 1 z
flag=flag & x==333 & y==333 & z==333
parse var fred  6 x +6  -3 y +4
flag=flag & x=='s the ' & y=='he t'
parse var fred  12  -8 y +4  2 z 3 -5 a 10
flag=flag & y==' is ' & z=='o' & a='Now is th'
parse var fred a 'the' b -6 y +4 c
flag=flag & y=='ow i'
parse var fred  ' the' +1 x +8 y +8
flag=flag & x=='the time' & y ==' for all'
parse var fred 2 x +10 y + 10 z +10 a +4
flag=flag & z=='ll good me' & a=='n'
parse var fred   ' ti' +1 time  ' '  ,
               1 ' pr' +1 print ' '
flag=flag & time=='time' & print==''
parse value "a,b,c" with p ',' q +1 r
parse value "12345678" with s '123' t +5
flag=flag & p='a' & q==',' & s=='' & t=='12345'

push   'abc def ghi jkl'     /* check backtrack/uppercase interaction */
                             /* (historical, not relevant in REX3)    */
pull   x y z 1 x2 y2
flag=flag &  x=='ABC' & y=='DEF' & z='GHI JKL',
          & x2=='ABC' & y2=='DEF GHI JKL'
/* Check mixed string/numeric trigger (the famous SP3/SP4 bug fix) */
parse value '12345678' with '34' me +3
flag=flag & me=='345'

if flag  then ok=ok '! Numeric Parse '
         else say '*** Bad *** Numeric Parse'
trace 'o'
say ok; ok='OK'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - C O M M A N D S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
flag=1
'CONWAIT'        /* This must work to get here! */
if rc\=0 then flag=0
cmd='CONWAIT'
cmd
if rc\=0 then flag=0
cmd=left('CONWAIT',1000) /* Long command, buffer needed */
cmd                      /*  ... */
if rc\=0 then flag=0
left('CONWAIT',1000)     /* Long command, no buffer needed */
if rc\=0 then flag=0
trace ooo

/* Now see if SP2 CP translation bites us ... */
'CP SET EMSG OFF'
'CP SET MSG OFF'
cmd='CP M * Hi there'
save=cmd
cmd; src=cmd
'CP SET EMSG TEXT'
'CP SET MSG ON'
if save\=cmd | rc\=0 then flag=0

if flag  then ok=ok '! Host Commands'
         else say '*** Bad *** Host Commands'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - A D D R E S S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CONWAIT'
/* EXECCOMM will always be set up, so send it invalid request */
ADDRESS 'EXECCOMM'; rc=0
'!                                                ' /** err **/
if rc=-1 then sofar=1
         else sofar=0
ADDRESS CMS  /* back to CMS */
'CONWAIT'
if rc\=0 then sofar=0
/**/
ADDRESS 'EXECCOMM'; rc=0
'!                                                ' /** err **/
if rc\=-1 then sofar=0
ADDRESS  /* back to CMS */
'CONWAIT'
if rc\=0 then sofar=0
/**/
fred='EXECCOMM'
address value fred
'!                                           '
if rc\=-1 then sofar=0
address
'$Nonxist'
if rc\=-3 then sofar=0
address 'GARbage' boo
if rc\=-3 then sofar=0
if sofar then ok=ok ! 'Address'
         else say '*** Bad *** "Address"'

flag=0
address 'ME2'
address 'ME1'
a1=address()      /* ME1 */
address
a2=address()      /* ME2 */
a3=setaddress()   /* WOBBLE */
a4=address()      /* ME2 */
address
a5=address()      /* ME1 */
flag=      a1=='ME1'
flag=flag& a2=='ME2'
flag=flag& a3=='WOBBLE'
flag=flag& a4=='ME2'
flag=flag& a5=='ME1'

/* now check address toggle handled properly over I-fun call */
address last
address current
address foo addresstest()
a1=address()  /* should be CURRENT */
address
a2=address()  /* should be LAST    */
flag=flag& a1=='CURRENT'
flag=flag& a2=='LAST'
trace o
if flag then ok=ok ! 'Address save'
        else say '*** Bad *** Address save across CALL'
address ' ' /* set back to CMS */
say ok; ok='OK'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - I N T E R P R E T - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CONWAIT'
i=0; do 3
  interpret 'interpret "i=i+1; pq=zappa"'
  end
if pq=zappa & I=3 then ok=ok ! 'Interpret'
                  else say '*** Bad *** Interpret'

/* try Interpret on an ELSE clause (tricky to implement): */
n=1;
if n = 0 then say '*** Bad IF ***'
         else interpret 'n=awful'
n=ok
if n=ok then ok=ok ! 'Else Interpret'
        else say '*** Bad *** Else Interpret'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - L A B E L S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
/* Some sneaky labels and parsing trickies here */
parse=0
fredaleble,
: nop
fedaleewew,
,
       : /* continuations galore */
       parse=1
fedaleewew,
/* * */ ,  /**/
,   /* comments too */
/* yahoo */ ,  /* fee */
,
       : /* more galore */
double_label: nop
trace 'o'
if parse=1 then ok=ok '! Labels'
           else say '*** Bad Label parsing ***'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - S I G N A L - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CONWAIT'
animal='Garbage'  /* Should not pick this up */
signal animal
 exit 104
animal:
signal 'LITERALSIG'
 exit 174
literalsig:
signal on syntax name mixsynt         /* jph                         */
signal 'mixedliteral'
 exit 184
mixedliteral:
say '*** Bad *** signalmixed'         /* jph                         */
mixsynt:                              /* jph                         */
signal off syntax                     /* jph                         */
varsig='VARIABLE'
signal value varsig||'SIG'  /* Allow an expression */
 exit 164
variablesig:

i=0
/* Try and confuse it with a leading split comment  ....
... */ sigloop:
  i=i+1
  if I<7  then do; interpret 'signal sigloop'; end

if i=7  then ok=ok ! 'Signal Loop'
        else say '*** Bad *** Signal Loop'

/* Check duplicate labels handled correctly */
signal sigcarryon
duplabel:  value=1; signal sigend
duplabel:  value=2; signal sigend
sigcarryon: signal duplabel
sigend: if value=1 then ok=ok '! Duplabel'
                   else say '*** Bad duplicate label handling ***'

say ok; ok='OK'

signal on SYNTAX ; /* Literal string for label is not acceptable on 5.2 jph */
 /* Note, this should not be scanning error, or lookaside will be
    turned off for rest of diagnostics pack, and in any case we won't
    be able to get to the label without re-raising the error! */
 say 3+
 if 1 then say '*** Bad Signal on Syntax ***'
   else
syntax: ok = ok ! 'Signal on Syntax'

signal on error
 'NUCEXT ZABBA'
 if 1 then say '*** Bad Signal on Error ***'
   else
error: ok=ok ! 'Signal on Error'

signal on failure
 'ZAB$BA'  /* should be a failure */
 if 1 then say '*** Bad Signal on Failure ***'
   else
failure: ok=ok ! 'Signal on Failure'
say ok; ok='OK'

signal on halt
 'TRACER HALT QUIET'
 if 1 then say '*** Bad Signal on Halt ***'
   else
halt: ok=ok ! 'Signal on Halt'

nov=0
signal on novalue
fred=123 /* better not signal! */
nov=1
fred=fred /* better not signal! */
nov=2
signal on novalue
fred=zabbalquaolsl /* This one should go */
  if 1 then say '*** Bad Signal on novalue ***'
    else
novalue: if nov=2  then ok=ok ! 'Signal on Novalue'
                   else say '(NOV was' nov')'
say ok
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - C A L L - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CONWAIT'
ok='OK'
i=1; result='Should be dropped'
call noret hi; res1=result; res1type=symbol('RESULT')
i=i+1
call BERT 'hi'; res2=result; res2type=symbol('RESULT')
i=i+1
call mult 'now','is','the','time','son'||'ny'; res3=result
i=i+1
call fred now is the time', son'  /* This also tests RETURN cleanup */
i=i+1
call rexdiag 'Cloggy'; res4=result /* External Exec call */
parse pull fred  /* it put data on the stack */
i=i+1
interpret 'call fred now is the time; j=78'
if result='THE IS NOW : This is bert',
 & i=6,
 & res1='RESULT' & res1type='LIT',
 & res2='hi bert' & res2type='VAR',
 & res3='is now the time  sonny',
 & fred='REXDIAG Cloggy works',
 & res4='RESULT',
 & j=78 then ok=ok '! Call/Return'
        else say '*** Bad *** Call/Return'
trace 'o'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - P R O C E D U R E   E X P O S E - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CONWAIT'
/* don't clean the slate - we need "OK" */
flag=1; drop x.; long=copies('1',100)
i=1; j=2; k=4; x.2='ho'
flag=flag & exposimple() == 'I 2 4 X.1 ho' long
k=1; x.1='ha'; x.3='hi'
flag=flag & exposimple() == 'I 2 1 ha ho' long
j=1; x.1='ha'; x.3='hi'
flag=flag & exposimple() == 'I 1 1 ha X.2' long
/* Now check for two-level+ remote extensions */
line='aaaaa'; length=0
call exposeadd
/* On return, LENGTH should have value 24 */
flag=flag & length=24

drop x.; x.3=55; i=7; x.i='00';
flag=flag & exposmult() == 'X.1 55 00'
flag=flag & x.3         == 'X.3'  /* has been dropped */

i=1; x.i=0; x.2=0; y.1='';  null=''; x.null='hi'
call expo 1  /* single level X. */
call expo 3  /* triple level X. */
flag=flag & x.1=4 & y.1='' & x.null='hihihihihi'

/* now ensure that exposing a single compound var has no side-effect */
a.=0
both=testexpose()
flag=flag & a.77 a.88 both == '0 0 0 A.88'

/* now ensure that exposing a single compound var is not affected by */
/* initialise.                                                       */
a.=0
a.77=123 a.99=123
call testexpose2 456
flag=flag & a.77 a.88 a.99 a.1 == '456 99  0'
trace o

if flag then ok=ok '! Expose'
        else say '*** Bad *** Expose'
trace 'o'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - A R R A Y S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
/* Arrays and substituted variables */
drop a.; null=''
k.null ='abc'
a.null='def'
kitt.='ghi'
a.1='a'
a.2='b'
a.3='c'
i=3; j=2; k=1
a.2.3=k.null a.null kitt.null a.i a.j a.k a.3 a.4
flag= a.j.i=='abc def ghi c b a c A.4'

/* Check proper handling of set/locate stems */
drop a.
temp1=a. a.null
a.=12; null=''
temp2=a. a.null
a.null=7; a.1=9
temp3=a. a.null   /* Now not the same thing! */
flag=flag & temp1=='A. A.',
          & temp2=='12 12',
          & temp3=='12 7',
          & a.1=='9' & a.7='12'

if flag then ok=ok '! Compound Vars'
        else say '*** Bad *** Compound Vars'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - C O N T I N U A T I O N S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
if 1=1 then okflag,
             =1
       else okflag=,     /* Note comment is allowed here */  /**/
              0
do 2,
  while 1=1
  end
if,
  1\=1 then okflag=0
if 'abc def c b a'=,
 'abc def c b a' & okflag
                             then ok=ok '! Continuation'
                             else say '*** Bad *** Continuation'
trace ooo
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - U P P E R - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
/* toupper()  in  the  western  European locales also uppercases the */
/* accented  characters from 'c0'x and on.  So we limit ourselves to */
/* 7-bit ASCII.  jph                                                 */
If opsys='CMS'
   Then
      Do
         string=xrange('00'x,'ff'x)   /* Full house.  jph            */
         ups=overlay('ABCDEFGHI',string,130)
         ups=overlay('JKLMNOPQR',ups   ,146)
         ups=overlay( 'STUVWXYZ',ups   ,163)  /* Ups is now lower->upper TTAB */
      end
   Else
      Do
         string=xrange('00'x,'7f'x)   /* 7-bit ASCII only.  jph      */
         ups=overlay('ABCDEFGHIJKLMNOPQRSTUVWXYZ',string,98)  /* jph */
      end
l=length(string)                      /* jph                         */

a='Woe is me'; b='Hi!'; c=''
upper a b c string
a.='g'       /* test UPPER of unused compound variable */
null=''
upper a.null
if a. a.null a.3 \== 'g G g' then flag=0
                             else flag=1
if flag & a='WOE IS ME' & b='HI!' & c='' & string=ups
            then ok=ok ! 'Upper'
            else say '*** Bad *** Upper stmt'
trace oo
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/


/*- - C O M M E N T S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
cflag=1; abc='ABC'
cflag=cflag & 'abc'/* */'def'   == 'abcdef'
cflag=cflag & 'abc',
/* */'def'   == 'abc def'
cflag=cflag & 'abc'/* */,
'def'   == 'abc def'
cflag=cflag & abc/* */'def'   == 'ABCdef'
cflag=cflag & abc,
/* */'def'   == 'ABC def'
cflag=cflag & abc/* */,
'def'   == 'ABC def'

cflag=cflag & 'abc' /* */'def'  == 'abc def'
cflag=cflag & 'abc' ,
/* */'def'  == 'abc def'
cflag=cflag & 'abc' /* */,
'def'  == 'abc def'
cflag=cflag & abc /* */'def'  == 'ABC def'
cflag=cflag & abc ,
/* */'def'  == 'ABC def'
cflag=cflag & abc /* */,
'def'  == 'ABC def'

/* check for invalid addition of blank... */
cflag=cflag & /* */,
'abc def'  == 'abc def'
cflag=cflag & /* */ ,
'abc def'  == 'abc def'
cflag=cflag & , /* */
'abc def'  == 'abc def'
cflag=cflag & , /* */  /* extra blanks between comments */
'abc def'  == 'abc def'
cflag=cflag & , /* *//* touching comments */
'abc def'  == 'abc def'

cflag=cflag & 'abc'/* */ 'def'  == 'abc def'
cflag=cflag & 'abc',
/* */ 'def'  == 'abc def'
cflag=cflag & 'abc'/* */,
 'def'  == 'abc def'

cflag=cflag & 'abc' /* */ 'def' == 'abc def'
cflag=cflag & 'abc',
 /* */ 'def' == 'abc def'
cflag=cflag & 'abc' /* */,
 'def' == 'abc def'
cflag=cflag & 'abc', /* */
 'def' == 'abc def'
cflag=cflag & 'abc', /* */ /* extra blank between */
 'def' == 'abc def'
cflag=cflag & 'abc', /* *//* adjacent */
 'def' == 'abc def'
cflag=cflag & abc /* */ 'def' == 'ABC def'
cflag=cflag & abc,
 /* */ 'def' == 'ABC def'
cflag=cflag & abc /* */,
 'def' == 'ABC def'

/* Check nesting of comments */
/* /* simple nest */ */
/* /* double nest */  /* xx */ */
/* /* double nest squash *//* xx */ */
/*/* squash left */ */
/*/*/* squash left */ */ */
/* /* squash right */*/
/* /* /* squash right */*/*/
/*/* squash both */*/
/*/*/* squash both */*/*/
/* adjacent comments *//* second *//* Third */

/* Now check for spurious continuation character */
cff=0; x='2b'x
cff=1  /* this will be treated as continuation of previous, maybe */
cflag=cflag & cff

if cflag then ok=ok ! 'Comments'
         else say '*** Bad *** Comments'
trace oo
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
say ok; ok='OK'

/*- - F U N C T I O N   C A L L S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
if substr('abcdef',2,3)='bcd' then ok=ok '! Builtin function'
                              else say '*** Bad *** Builtin function'
'CONWAIT'
fred=rexdiag('Clobby');
if fred='REXDIAG Clobby works' then ok=ok '! Implicit Exec function'
                   else say '*** Bad *** Implicit Exec function'
fred=rexdiagmefoo('Happy');
trr1=right(1,2,3)  /* special */
trr2='RIGHT'(1,2)  /* special, ensure no usurp */
trr3=right(1,2,3)  /* special */
trace 'o'
testvar=123
if testvar ifunproc() testvar = 123 5 123,
 & testvar ifunproci() testvar = 123 5 123,
 & trr1=='3  1'  &  trr2==' 1' & trr3=trr1,
 & fred='mefoo Happy' then ok=ok '! Simple I-function'
                else say '*** Bad *** Simple I-function'
say ok; ok='OK'
if 0 then /* skip */ do
   funccall:  /* REXDIAG is being called as a function */
              parse arg rest
              parse source . . . . . en .
              return en rest 'works'
   subcall:   /* REXDIAG is being called as a subroutine */
              parse arg rest
              parse source . . . . . en .
              push en rest 'works' /* return via stack */
              return /* check no data returned works OK */
   end
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - A R G   and  P A R S E   S O U R C E - -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
'CONWAIT'
PARSE SOURCE . . fn ft fm cname addr .
Parse ARG First ,  second
PARSE ARG rest
flag= first=rest
if first='IFUN' then flag= second='arg2'
if second\=''   then flag=flag& first='IFUN'

if \flag then say '*** Bad Arg or SOURCE ***'
 else say 'OK ! Arg ! Args were' '/' cname '/' rest '/' second
trace 'o'
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
/*  Now decide whether to:  stop                                      */
/*                          call all the above as internal function   */
/*                          try the error messages                    */
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/
reask:
say 'Please check that every line above (except the first) begins with "OK".'
say 'Then:'
say '  hit ENTER to stop the test,'
say '  type "F" to call test so far as an internal function,'
say '  any other character for test of all possible error messages ...'
pull ans .
if ans='P' then 'CRASH' /* for development use only */
if ans='' | ans='P' then signal terminate
if ans='F' then do
  say 'Calling as internal function...'
  do 1 /* Put in a repetititive DO to provoke lurking insects */
    say intfun('IFUN','arg2') '! internal function'
    end
  signal reask;
  end
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - E R R O R S - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
'CLEAR'
errors:
say; say 'Now trying one of each possible terminal error ...'
'CP SET EMSG ON'
If opsys='CMS'
   Then
      Do
         PARSE SOURCE . . fn ft fm cname addr .
         name='EXEC' fn '!!!'

         'EXEC'                                        /* no name              */
         'EXEC Nonexist'                               /* bad name             */
         'CP SET EMSG OFF'; 'CP DET 191'; 'CP SET EMSG ON'
         'EXEC PROFILE EXEC'                           /* Unreadable file */
         'CP SET EMSG OFF'
         'CP LINK' userid() 191 191
         'CP SL 1 SEC'
         'ACC 191 A'
         'CP SET EMSG ON';'RT'
      End
   Else
      Do
         PARSE SOURCE . . fn ft fm cname addr .
         parse var fn fn '.'
         name=fn '!!!'
      End

/** For the other syntax errors, we generate a string to cause the    */
/** error, then call ourselves to execute it and blow up              */
/** this runs a lot faster if IEMP is active!                         */
name 'do 1000; a=a a a a a a a a; end'        /* Blow CMS storage     */
name '"'                                      /* unmatched '          */
name 'Select; rubbish'                        /* when or other expctd */
name 'else'                                   /* unexpected Else      */
name 'when'                                   /* unexpected When/Other*/
name 'end'                                    /* unexpected End       */
name 'a=; do 300;a=a||"do;";end; interpret a' /* Blow Control stack   */
name 'a=copies(xx,260); interpret a'          /* > 500 chars          */
name '111111'x                                /* bad data char        */
name 'do'                                     /* incomplete do        */
name '"1r"x'                                  /* bad hex              */
name 'signal cannotfindit'                    /* label not found      */
name 'procedure'                              /* Unexp. PROCEDURE     */
name 'if 1=1 say hi; say boo'                 /* THEN expected        */
name 'call;'                                  /* Sym/String expected  */
name 'numeric'                                /* Symbol expected      */
name 'select junky'                           /* junk                 */
name 'Trace scan'                             /* Bad TRACE request    */
name 'parse foo bar'                          /* Bad sub-keyword      */
name 'numeric fuzz yabba'                     /* Conversion           */
name 'do i=1 to 3 to 4'                       /* Invalid DO syntax    */
name 'leave'                                  /* Bad LEAVE/ITERATE    */
name 'address a1234567890123'                 /* Environment too long */
name 'a=copies(a,260); interpret a"=2"'       /* Name too long        */
name '34=35'                                  /* Name is number       */
name 'upper y.'                               /* Bad use of stem      */
name 'address value'                          /* Bad use of expr.     */
name 'if 3 then say !'                        /* Logic not 0 or 1     */
name '3***5'                                  /* Bad expr             */
name 'a=a*(45'                                /* Unmatch (            */
name 'a=4)'                                   /* Unexpected )         */
name 'parse var a (b'                         /* Bad template         */
name 'a=copies("a (",40); interpret "a="a'    /* Blow eval stack      */
name 'a=substr("")'                           /* Bad use of function  */
name 'a=a+238U'                               /* Arith Conversion     */
name 'a=7/0'                                  /* Arith Overflow       */
name 'a="Wonderx"(ss)'                        /* Unknown function     */
name '$='fn'(!!! Exit)'                       /* F must return data   */
name 'a=noret()'                              /* No data returned     */
/* not forceable.... */
                                              /* Failure of sys. serv */
                                              /* Interpreter error    */

'CP SET EMSG TEXT'
say; say 'Finished!, EMSG now set to TEXT.'

terminate:
  parse arg all
  if all='IFUN' then return 'OK'
  exit
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/*- - S U B R O U T I N E S   and   F U N C T I O N S - -- -- -- -- -- -- -- --*/

/* General purpose subroutine */
fred: /* sub 1 */
  parse arg all
  if i=1 then return 'Bert'
  do 3
    call bert  'This is'
    end
  parse arg a1 a2 a3 rest
  do iiii=1 to 3  /* Ensure  an active Doloop on return */
    if iiii=2 then return a3 a2 a1 ':' result
    end
  /** Never drop through **/
  exit 66666

/* Subroutine with simple PROCEDURE */
bert: /* sub 2 */procedure
  parse arg all
  ans=all 'bert'
  return ans

/* Function with simple PROCEDURE */
ifunproc: /* ... and interpreted RETURN */
  procedure
  testvar=5
  interpret "return testvar"

/* Internal procedure function, Interpreted RETURN */
ifunproci: procedure
  interpret "testvar=5; return testvar"

/* Subroutine/function with multiple args */
mult: parse arg a,b,c,d,e,f
      return b a c d f e

/* Simple internal function */
rexdiagmefoo:  /* Simple I-fun */
  parse arg stuff
  return 'mefoo' stuff

/* More complex mixed internal/external call */
right: procedure /* external call if 2 args, else fixed call */
  arg  a, b, c
  if c='' then return 'RIGHT'(a,b)
  return c 'RIGHT'(a,b)

/* subroutine with null return stmt */
noret:   /* Null comment */
  s=1
  return

/* Function to markedly change the DIGITS setting */
setnumup:  procedure
  numeric digits 40
  numeric fuzz   20
  numeric form   engineering
  parse numeric all
  return all 1/3

/* Internal function to try and mess up PARSE UPPER etc */
pup: procedure
  parse arg inp
  parse upper value "upper" with part2
  return inp part2

/* Function to change the ADDRESS settings */
setaddress:  procedure
  address 'gabble'
  address wobble
  return address()

/* Function to check toggling */
addresstest:  procedure
  if address()\='CURRENT' then say '*** Bad Address temporary change ***'
  'gabble'
  if address()\='CURRENT' then say '*** Bad Address temporary change ***'
  return address()

/* Procedure to expose to "n" levels the array "X." */
/* and do not expose "y." array */
expo: procedure expose x.
  arg n
  x.1=x.1+1; null=''; x.null=x.null||'hi'; y.1=y.1 'bad'
  if n>1 then call expo n-1
  return

/* Try multiple and mixed vars on EXPOSE to ensure dead children */
/* are freed... */
exposmult: procedure expose i i i j j x.1 x.2 x.i /* 3 children */ x. /**/
 x3=x.3; drop x.3
 return x.1 x3 x.7

/* Procedure to expose simple variables of various types */
/* Ensure that double exposure is OK, and stretch works... */
exposimple: procedure expose j j k x.j x.k
  x.3=copies('1',100)
  return i j k x.1 x.2 x.3

/* Routine to try EXPOSEing an individual compound variable */
/* (should have no side effects) */
testexpose:
  procedure expose a.77
  return a.77 a.88

/* Routine to initialise a compound variable while one element is */
/* exposed, then set that single element to the ARG passed.       */
/* (should have no side effects) */
testexpose2:
  procedure expose a.77 a.88 a.99
  drop a.
  a.=99
  a.99=''
  a.77=arg(1)
  return

/* Procedure to expose simple variables, and force extension at a */
/* level at least two removed from current generation */
exposeadd: procedure expose line length
if length(line)>7    then return
line=line||'a'
call exposeadd
/* On return, length of LINE should always be 8 */
length=length+length(line)
return
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

/* END of code */
