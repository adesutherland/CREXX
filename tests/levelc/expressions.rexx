options levelb
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
