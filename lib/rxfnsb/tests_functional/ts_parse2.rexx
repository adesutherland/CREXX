/* RXPP */
/* ----------------------------------------------------------------------
 * PRE Compiled on 21 Aug 2025  at 11:41:53
 * ----------------------------------------------------------------------
 */
options levelb
import rxfnsb
errors=0;debug=0
c=','
string='1,22,333,4444,55555'
/* first','second','third','fourth','fifth','six */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=string
   _parsetemplate="first','second','third','fourth','fifth','six"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
first=_pass_variable_content.1
second=_pass_variable_content.2
third=_pass_variable_content.3
fourth=_pass_variable_content.4
fifth=_pass_variable_content.5
six=_pass_variable_content.6
## ---------- parse variables set ----------
if first  <> '1' then do
  errors=errors+1
  say 'Parse failed in test 1 '
end
if debug then say 'after 1'
if second <> '22' then do
  errors=errors+1
  say 'Parse failed in test 2 '
end
if debug then say 'after 2'
if third  <> '333' then do
    errors=errors+1
    say 'Parse failed in test 3 '
  end
if debug then say 'after 3'
if fourth <> '4444' then do
  errors=errors+1
  say 'Parse failed in test 4 '
end
if debug then say 'after 4'
if fifth  <> '55555' then do
  errors=errors+1
  say 'Parse failed in test 5 '
end
if debug then say 'after 5'
if six    <> '' then do
  errors=errors+1
  say 'Parse failed in test 6 '
end
if debug then say 'after 6'
h='come'
h='Awful'h 'its time'
/* h1 . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=h
_parsetemplate="h1 ."
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
h1=_pass_variable_content.1
## ---------- parse variables set ----------
if h <> "Awfulcome its time" then do
  errors=errors+1
  say 'Parse failed in test 7 '
end
if debug then say 'after 7'
if h1 <> "Awfulcome" then do
  errors=errors+1
  say 'Parse failed in test 8 '
end
if debug then say 'after eight'
/*  f1 f2 f3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='l  m  n o'
   _parsetemplate="f1 f2 f3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
f1=_pass_variable_content.1
f2=_pass_variable_content.2
f3=_pass_variable_content.3
## ---------- parse variables set ----------
if f1 <> 'l' then do
    errors=errors+1
  say 'Parse failed in test 9 '
end
if debug then say 'after 9'
if f2 <> 'm' then do
    errors=errors+1
  say 'Parse failed in test 10 '
end
if debug then say 'after 10'
if f3 <> ' n o' then do
  errors=errors+1
  say 'Parse failed in test 11 '
end
if debug then say 'after 11'
/*  e1 e2 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='l m n o'
   _parsetemplate="e1 e2"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
e1=_pass_variable_content.1
e2=_pass_variable_content.2
## ---------- parse variables set ----------
if e1 <> 'l' then do
  errors=errors+1
  say 'Parse failed in test 12 '
end
if debug then say 'after 12'
if e2 <> 'm n o' then do
  errors=errors+1
  say 'Parse failed in test 13 '
end
if debug then say 'after 13'
/*  e3 e4 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=''
   _parsetemplate="e3 e4"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
e3=_pass_variable_content.1
e4=_pass_variable_content.2
## ---------- parse variables set ----------
if e3 <> '' then do
  errors=errors+1
  say 'Parse failed in test 14 '
end
if debug then say 'after 14'
if e4 <> '' then do
  errors=errors+1
  say 'Parse failed in test 15 '
end
if debug then say 'after 15'
x='aa' 1+2+3 'bb'
/* e5 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=x
   _parsetemplate="e5"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
e5=_pass_variable_content.1
## ---------- parse variables set ----------
if e5 <> 'aa 6 bb' then do
  errors=errors+1
  say 'Parse failed in test 16 '
end
if debug then say 'after 16'
delim='x'
/*  verb 2 delim +1 str (delim) rest */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='c/my string/1 10'
   _parsetemplate="verb 2 delim +1 str (delim) rest"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
verb=_pass_variable_content.1
delim=_pass_variable_content.2
str=_pass_variable_content.3
rest=_pass_variable_content.4
## ---------- parse variables set ----------
if verb <> 'c' then do
  errors=errors+1
  say 'Parse failed in test 17 '
end
if debug then say 'after 17'
if delim<>'/' then do
    errors=errors+1
  say 'Parse failed in test 18 '
end
if debug then say 'after 18'
if str<>'my string' then do
    errors=errors+1
  say 'Parse failed in test 19 '
end
if debug then say 'after 19'
if rest<>'1 10' then do
    errors=errors+1
  say 'Parse failed in test 20 '
end
if debug then say 'after 20'
/* nl='\x15'; ffuts='abcd'nl'MFC rest' */
/* parse ffuts (nl) wordx . +20 */
/* if wordx<>nl'MFC' then do */
/*     errors=errors+1 */
/*   say 'Parse failed in test 21 ' */
/* end */
/* if debug then say 'after 21' */
/* missy='abcdefghijk'; null='' */
/* parse missy '' -4 last4 (null) -3 last3 */
/* parse missy '' -4 (null) -3 last3b +2 */
/* if last4<>'hijk' then do */
/*     errors=errors+1 */
/*   say 'Parse failed in test 22 ' */
/* end */
/* if debug then say 'after 22' */
/* if last3<>'ijk' then do */
/*     errors=errors+1 */
/*   say 'Parse failed in test 23 ' */
/* end */
/* if debug then say 'after 23' */
/* if last3b<>'ij' then do */
/*     errors=errors+1 */
/*   say 'Parse failed in test 24 ' */
/* end */
/* if debug then say 'after 24' */
fred=''
/* 6 x */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="6 x"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
## ---------- parse variables set ----------
if x<>'' then do
    errors=errors+1
  say 'Parse failed in test 25 '
end
if debug then say 'after 25'
fred='Now is the time for all good men'
/* 6 x 12 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="6 x 12"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
## ---------- parse variables set ----------
if x <> 's the ' then do
  errors=errors+1
  say 'Parse failed in test 26 '
end
if debug then say 'after 26'
/* 6 x +6 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="6 x +6"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
## ---------- parse variables set ----------
if x <> 's the ' then do
    errors=errors+1
  say 'Parse failed in test 27 '
end
if debug then say 'after 27'
/* x 1 y 1 z */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=333
   _parsetemplate="x 1 y 1 z"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
y=_pass_variable_content.2
z=_pass_variable_content.3
## ---------- parse variables set ----------
if x <> 333 then do
  errors=errors+1
  say 'Parse failed in test 28 '
end
if debug then say 'after 28'
/* x 1 y 1 z */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=333
   _parsetemplate="x 1 y 1 z"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
y=_pass_variable_content.2
z=_pass_variable_content.3
## ---------- parse variables set ----------
if x <> 333 then do
  errors=errors+1
  say 'Parse failed in test 28 '
end
if debug then say 'after 28'
if y <> 333 then do
  errors=errors+1
  say 'Parse failed in test 29 '
end
if debug then say 'after 29'
if z <> 333 then do
  errors=errors+1
  say 'Parse failed in test 30 '
end
if debug then say 'after 30'
/* 6 x +6  -3 y +4 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="6 x +6  -3 y +4"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
y=_pass_variable_content.2
## ---------- parse variables set ----------
if x<>'s the ' then do
      errors=errors+1
  say 'Parse failed in test 31 '
end
if debug then say 'after 31'
if y<>'he t' then do
  errors=errors+1
  say 'Parse failed in test 32 '
end
if debug then say 'after 32'
/* 12  -8 y +4  2 z 3 -5 a 10 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="12  -8 y +4  2 z 3 -5 a 10"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
y=_pass_variable_content.1
z=_pass_variable_content.2
a=_pass_variable_content.3
## ---------- parse variables set ----------
if y<> ' is ' then do
  errors=errors+1
  say 'Parse failed in test 33 '
end
if debug then say 'after 33'
if z<> 'o' then do
  errors=errors+1
  say 'Parse failed in test 34 '
end
if debug then say 'after 34'
if a<> 'Now is th' then do
  errors=errors+1
  say 'Parse failed in test 35 '
end
if debug then say 'after 35'
/* a 'the' . -6 y +4 . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
_parsetemplate="a'the'. -6 y +4 ."
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
a=_pass_variable_content.1
   
y=_pass_variable_content.3
## ---------- parse variables set ----------
if y<> 'ow i' then do
      errors=errors+1
  say 'Parse failed in test 36 --------------->' y 'should be ow i'
end
if debug then say 'after 36'
/* ' the' +1 x +8 y +8 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="' the'+1 x +8 y +8"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
y=_pass_variable_content.2
## ---------- parse variables set ----------
if x<> 'the time' then do
    errors=errors+1
  say 'Parse failed in test 38 --------------->' x 'should be the time'
end
if debug then say 'after 38'
if y <>' for all' then do
    errors=errors+1
  say 'Parse failed in test 39 --------------->' y 'should be for all'
end
if debug then say 'after 39'
/* parse fred 2 x +10 y + 10 z +10 a +4 */
/* if z<>'ll good me' then do */
/*   errors=errors+1 */
/*   say 'Parse failed in test 40 ' */
/* end */
/* if debug then say 'after 40' */
/* if  a<>'n' then do */
/*   errors=errors+1 */
/*   say 'Parse failed in test 41 ' */
/* end */
/* if debug then say 'after 41' */
  
/* ' ti' +1 time  ' '  1 ' pr' +1 print ' ' */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred
   _parsetemplate="' ti'+1 time' '1' pr'+1 print' '"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
time=_pass_variable_content.1
print=_pass_variable_content.2
## ---------- parse variables set ----------
if time<> 'time' then do
    errors=errors+1
  say 'Parse failed in test 42 --------------->' time 'should be time'
end
if debug then say 'after 42'
if print<> '' then do
  errors=errors+1
  say 'Parse failed in test 43 '
end
if debug then say 'after 43'
/*  p ',' q +1 r */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='a,b,c'
   _parsetemplate="p','q +1 r"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
p=_pass_variable_content.1
q=_pass_variable_content.2
r=_pass_variable_content.3
## ---------- parse variables set ----------
/*  s '123' t +5 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='12345678'
   _parsetemplate="s'123't +5"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
s=_pass_variable_content.1
t=_pass_variable_content.2
## ---------- parse variables set ----------
if p<>'a' then do
  errors=errors+1
  say 'Parse failed in test 44 '
end
if debug then say 'after 44'
if q<>',' then do
  errors=errors+1
  say 'Parse failed in test 45  --------------->' y 'should be ,'
end
if debug then say 'after 45'
if r<>'b,c' then do
  errors=errors+1
  say 'Parse failed in test 46 '
end
if debug then say 'after 46'
if s<>'' then do
  errors=errors+1
  say 'Parse failed in test 47 '
end
if debug then say 'after 47'
if t<>'12345' then do
  errors=errors+1
  say 'Parse failed in test 48 '
end
if debug then say 'after 48'
/*  '34' me +3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='12345678'
   _parsetemplate="'34'me +3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
me=_pass_variable_content.1
## ---------- parse variables set ----------
if me<>'345' then do
  errors=errors+1
  say 'Parse failed in test 49 '
end
if debug then say 'after 49'
/* we skipped for the moment: parse with comment patterns in the template */
/* we skipped for the moment: parse with arrays in the template           */
/* these are from NRL */
  /* Introductory examples */
/*      v1 v2 v3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
_string2Parse='This is a sentence.'
   _parsetemplate="v1 v2 v3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
v1=_pass_variable_content.1
v2=_pass_variable_content.2
v3=_pass_variable_content.3
## ---------- parse variables set ----------
if v1<>'This' then do
  errors=errors+1
  say 'Parse failed in test 50 '
end
if debug then say 'after 50'
if v2<>'is' then do
  errors=errors+1
  say 'Parse failed in test 51 '
end
if debug then say 'after 51'
if v3<>'a sentence.' then do
  errors=errors+1
  say 'Parse failed in test 51 '
end
if debug then say 'after 51'
      
/*     w1 ',' w2 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='To be, or not to be?'
   _parsetemplate="w1','w2"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
## ---------- parse variables set ----------
if w1<>'To be' then do
  errors=errors+1 
  say 'Parse failed in test 52 '
end
if debug then say 'after 52'
if w2<>' or not to be?' then do
  errors=errors+1
  say 'Parse failed in test 53 '
end
if debug then say 'after 53'
/*     w1 ',' w2 w3 w4 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='To be, or not to be?'
   _parsetemplate="w1','w2 w3 w4"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
w4=_pass_variable_content.4
## ---------- parse variables set ----------
if w1<>'To be' then do
  errors=errors+1
  say 'Parse failed in test 54 '
end
if debug then say 'after 54'
if w2<>'or' then do
  errors=errors+1
  say 'Parse failed in test 55 '
end
if debug then say 'after 55'
if w3<>'not' then do
  errors=errors+1
  say 'Parse failed in test 56 '
end
if debug then say 'after 57'
if w4<>'to be?' then do
  errors=errors+1
  say 'Parse failed in test 58 '
end
if debug then say 'after 58'
/*     w1 (c) w2 w3 w4 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='To be, or not to be?'
   _parsetemplate="w1 (c) w2 w3 w4"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
w4=_pass_variable_content.4
## ---------- parse variables set ----------
if w1<>'To be' then do
    errors=errors+1
  say 'Parse failed in test 58 '
end
if debug then say 'after 58'
if w2<>'or' then do
    errors=errors+1
  say 'Parse failed in test 59 '
end
if debug then say 'after 59'
if w3<>'not' then do
    errors=errors+1
  say 'Parse failed in test 60 '
end
if debug then say 'after 60'
if w4<>'to be?' then do
  errors=errors+1
  say 'Parse failed in test 61 '
end
if debug then say 'after 61'
/*   x1 5 x2 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='Flying pigs have wings'
   _parsetemplate="x1 5 x2"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x1=_pass_variable_content.1
x2=_pass_variable_content.2
## ---------- parse variables set ----------
if x1<>'Flyi' then do
  errors=errors+1
  say 'Parse failed in test 62 '
end
if debug then say 'after 62'
if x2<>'ng pigs have wings' then do
  errors=errors+1
  say 'Parse failed in test 63 '
end
if debug then say 'after 63'
/*   x1 5 x2 10 x3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='Flying pigs have wings'
   _parsetemplate="x1 5 x2 10 x3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x1=_pass_variable_content.1
x2=_pass_variable_content.2
x3=_pass_variable_content.3
## ---------- parse variables set ----------
if x1<>'Flyi' then do
    errors=errors+1
  say 'Parse failed in test 64 '
end
if debug then say 'after 64'
if x2<>'ng pi' then do
    errors=errors+1
  say 'Parse failed in test 65 '
end
if debug then say 'after 65'
if x3<>'gs have wings' then do
    errors=errors+1
  say 'Parse failed in test 66 '
end
if debug then say 'after 66'
/*   x1 5 x2 +5 x3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='Flying pigs have wings'
   _parsetemplate="x1 5 x2 +5 x3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x1=_pass_variable_content.1
x2=_pass_variable_content.2
x3=_pass_variable_content.3
## ---------- parse variables set ----------
if x1<>'Flyi' then do
  errors=errors+1
  say 'Parse failed in test 67 '
end
if debug then say 'after 67'
if x2<>'ng pi' then do
  errors=errors+1
  say 'Parse failed in test 68 '
end
if debug then say 'after 68'
if x3<>'gs have wings' then do
  errors=errors+1
  say 'Parse failed in test 69 '
end
if debug then say 'after 69'
/* start=5; length=5; data='Flying pigs have wings' */
/* parse data x1 =(start) x2 +(length) x3 */
/* if x1<>'Flyi' then do */
/*   errors=errors+1 */
/*   say 'Parse failed in test 70 ' */
/* end */
/* if debug then say 'after 70' */
/* if x2<>'ng pi' then do */
/*   errors=errors+1 */
/*   say 'Parse failed in test 71 ' */
/* end */
/* if debug then say 'after 71' */
/* if x3<>'gs have wings') then do */
/*   errors=errors+1 */
/*   say 'Parse failed in test 72 ' */
/* end */
/* if debug then say 'after 72' */
/* Definition examples */
in='This is  the text which, I think,  is scanned.'
/* w1 ',' w2 ',' rest */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=in
   _parsetemplate="w1','w2','rest"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
rest=_pass_variable_content.3
## ---------- parse variables set ----------
if w1<>'This is  the text which' then do
      errors=errors+1
  say 'Parse failed in test 73 '
end
if debug then say 'after 73'
  
if w2<>' I think' then do
      errors=errors+1
  say 'Parse failed in test 74 '
end
if debug then say 'after 74'
if rest<>'  is scanned.' then do
  errors=errors+1
  say 'Parse failed in test 75 '
end
if debug then say 'after 75'
/* w1 ',' w2 ',' w3 ',' rest */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=in
   _parsetemplate="w1','w2','w3','rest"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
rest=_pass_variable_content.4
## ---------- parse variables set ----------
  if w1<>'This is  the text which' then do
      errors=errors+1
  say 'Parse failed in test 76 '
end
if debug then say 'after 76'
if w2<>' I think' then do
    errors=errors+1
  say 'Parse failed in test 77 '
end
if debug then say 'after 77'
if w3<>'  is scanned.' then do
    errors=errors+1
  say 'Parse failed in test 78 '
end
if debug then say 'after 78'
if rest<>'' then do
  errors=errors+1
  say 'Parse failed in test 79 '
end
if debug then say 'after 79'
/* w1 w2 w3 rest */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=in
   _parsetemplate="w1 w2 w3 rest"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
rest=_pass_variable_content.4
## ---------- parse variables set ----------
  if w1<>'This' then do
      errors=errors+1
  say 'Parse failed in test 80 '
end
if debug then say 'after 80'
if w2<>'is' then do
    errors=errors+1
  say 'Parse failed in test 81 '
end
if debug then say 'after 81'
if w3<>'the' then do
  errors=errors+1
  say 'Parse failed in test 82 '
end
if debug then say 'after 82'
if rest<>'text which, I think,  is scanned.' then do
  errors=errors+1
  say 'Parse failed in test 83 '
end
if debug then say 'after 83'
  
/* w1 w2 w3 rest ',' */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=in
   _parsetemplate="w1 w2 w3 rest','"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
rest=_pass_variable_content.4
## ---------- parse variables set ----------
if w1<>'This' then do
  errors=errors+1
  say 'Parse failed in test 84 '
end
if debug then say 'after 84'
if w2<>'is' then do
  errors=errors+1
  say 'Parse failed in test 85 '
end
if debug then say 'after 85'
if w3<>'the' then do
  errors=errors+1
  say 'Parse failed in test 86 '
end
if debug then say 'after 86'
if rest<>'text which' then do
  errors=errors+1
  say 'Parse failed in test 87 '
end
if debug then say 'after 87'
/* w1 ' ' w2 ' ' w3 ' ' rest ',' */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=in
   _parsetemplate="w1' 'w2' 'w3' 'rest','"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
rest=_pass_variable_content.4
## ---------- parse variables set ----------
if w1<>'This' then do
  errors=errors+1
  say 'Parse failed in test 88 '
end
if debug then say 'after 88'
if w2<>'is' then do
  errors=errors+1
  say 'Parse failed in test 89 '
end
if debug then say 'after 89'
if w3<>'' then do
  errors=errors+1
  say 'Parse failed in test 90 '
end
if debug then say 'after 90'
if rest<>'the text which' then do
  errors=errors+1
  say 'Parse failed in test 91 '
end
if debug then say 'after 91'
/*  . . . word4 . */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse='a bb ccc dddd eeeee'
_parsetemplate=". . . word4 ."
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
   
   
   
word4=_pass_variable_content.4
## ---------- parse variables set ----------
if word4<>'dddd' then do
  errors=errors+1
  say 'Parse failed in test 92 '
end
if debug then say 'after 92'
/* s1 10 s2 20 s3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=in
   _parsetemplate="s1 10 s2 20 s3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
s1=_pass_variable_content.1
s2=_pass_variable_content.2
s3=_pass_variable_content.3
## ---------- parse variables set ----------
if s1<>'This is  ' then do
  errors=errors+1
  say 'Parse failed in test 93 '
end
if debug then say 'after 93'
if s2<>'the text w' then do
  errors=errors+1
  say 'Parse failed in test 94 '
end
if debug then say 'after 94'
if s3<>'hich, I think,  is scanned.' then do
  errors=errors+1
  say 'Parse failed in test 95 '
end
if debug then say 'after 95'
/* statement sequences ... */
a = '123456789'
/* 3 w1 +3 w2 3 w3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=a
   _parsetemplate="3 w1 +3 w2 3 w3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
## ---------- parse variables set ----------
if w1<>'345' then do
  errors=errors+1
  say 'Parse failed in test 96 '
end
if debug then say 'after 96'
if w2<>'6789' then do
  errors=errors+1
  say 'Parse failed in test 97 '
end
if debug then say 'after 97'
if w3<>'3456789' then do
  errors=errors+1
  say 'Parse failed in test 98 '
end
if debug then say 'after 98'
x=' abc '
/* 1 w1 1 w2 1 w3 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=x
   _parsetemplate="1 w1 1 w2 1 w3"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
w1=_pass_variable_content.1
w2=_pass_variable_content.2
w3=_pass_variable_content.3
## ---------- parse variables set ----------
if w1<>x then do
  errors=errors+1
  say 'Parse failed in test 99 '
end
if debug then say 'after 99'
if w2<>x then do
  errors=errors+1
  say 'Parse failed in test 99 '
end
if debug then say 'after 99'
if w3<>x then do
  errors=errors+1
  say 'Parse failed in test 100 '
end
if debug then say 'after 100'
fred='abc,def'
/* -1 x +1 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred','
   _parsetemplate="-1 x +1"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
## ---------- parse variables set ----------
if x<>'c' then do
  errors=errors+1
  say 'Parse failed in test 101 '
end
if debug then say 'after 101'
fred='abcdef'
/* -1 x +1 */
_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */
   _string2Parse=fred','
   _parsetemplate="-1 x +1"
   call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content
## ---------- set parse variables ----------
x=_pass_variable_content.1
## ---------- parse variables set ----------
if x<>'f' then do
  errors=errors+1
  say 'Parse failed in test 102 '
end
if debug then say 'after 102'
say errors
return errors<>0
