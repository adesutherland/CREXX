options levelb
import rxfnsb

errors=0;debug=0
c=','

string='1,22,333,4444,55555'
parse string first','second','third','fourth','fifth','six

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
parse h h1 .           
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

parse "l  m  n o" f1 f2 f3
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

parse 'l m n o' e1 e2
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

parse '' e3 e4
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
parse x e5
if e5 <> 'aa 6 bb' then do
  errors=errors+1
  say 'Parse failed in test 16 '
end
if debug then say 'after 16'

delim='x'
parse 'c/my string/1 10' verb 2 delim +1 str (delim) rest
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
parse fred 6 x
if x<>'' then do
    errors=errors+1
  say 'Parse failed in test 25 '
end
if debug then say 'after 25'

fred='Now is the time for all good men'
parse fred  6 x 12
if x <> 's the ' then do
  errors=errors+1
  say 'Parse failed in test 26 '
end
if debug then say 'after 26'

parse fred  6 x +6
if x <> 's the ' then do
    errors=errors+1
  say 'Parse failed in test 27 '
end
if debug then say 'after 27'

parse 333 x 1 y 1 z
if x <> 333 then do
  errors=errors+1
  say 'Parse failed in test 28 '
end
if debug then say 'after 28'

parse 333 x 1 y 1 z
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

parse fred  6 x +6  -3 y +4
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

parse fred  12  -8 y +4  2 z 3 -5 a 10
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

parse fred a 'the' . -6 y +4 .
if y<> 'ow i' then do
     errors=errors+1
 say 'Parse failed in test 36 --------------->' y 'should be ow i'
end
if debug then say 'after 36'

parse fred  ' the' +1 x +8 y +8
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
  
parse fred   ' ti' +1 time  ' '  1 ' pr' +1 print ' '
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

parse "a,b,c" p ',' q +1 r

parse "12345678" s '123' t +5
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

parse '12345678' '34' me +3
if me<>'345' then do
  errors=errors+1
  say 'Parse failed in test 49 '
end
if debug then say 'after 49'

/* we skipped for the moment: parse with comment patterns in the template */
/* we skipped for the moment: parse with arrays in the template           */

/* these are from NRL */
  /* Introductory examples */
parse 'This is a sentence.'     v1 v2 v3
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

      
parse 'To be, or not to be?'    w1 ',' w2
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

parse 'To be, or not to be?'    w1 ',' w2 w3 w4
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

/* Disabled: Legacy edge case not supported in cREXX
parse 'To be, or not to be?'    w1 (c) w2 w3 w4
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
*/

parse 'Flying pigs have wings'  x1 5 x2
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

parse 'Flying pigs have wings'  x1 5 x2 10 x3
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

parse 'Flying pigs have wings'  x1 5 x2 +5 x3
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
parse in w1 ',' w2 ',' rest
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

  parse in w1 ',' w2 ',' w3 ',' rest
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

  parse in w1 w2 w3 rest
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
  
parse in w1 w2 w3 rest ','
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

parse in w1 ' ' w2 ' ' w3 ' ' rest ','
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

parse 'a bb ccc dddd eeeee' . . . word4 .  
if word4<>'dddd' then do
 errors=errors+1
 say 'Parse failed in test 92 '
end
if debug then say 'after 92'

parse in s1 10 s2 20 s3
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
parse a  3 w1 +3 w2 3 w3
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
parse x 1 w1 1 w2 1 w3
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

/* Disabled: Legacy edge case not supported in cREXX
fred='abc,def'
parse fred',' -1 x +1
if x<>'c' then do
  errors=errors+1
  say 'Parse failed in test 101 '
end
if debug then say 'after 101'

fred='abcdef'
parse fred',' -1 x +1
if x<>'f' then do
  errors=errors+1
  say 'Parse failed in test 102 '
end
if debug then say 'after 102'
*/

say errors

return errors<>0
