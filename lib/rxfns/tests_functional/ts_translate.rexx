/* rexx */
options levelb
/* translate  */
errors=0
/* TRANSLATE */

/* These from the Rexx book. */
if translate('abcdef') \= 'ABCDEF'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          1 '
end
if translate('abbc','&','b') \= 'a&&c'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          2 '
end
if translate('abcdef','12','ec') \= 'ab2d1f'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          3 '
end
if translate('abcdef','12','abcd','.') \= '12..ef'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          4 '
end
if translate('4123','abcd','1234') \= 'dabc'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          5 '
end
/* These from Mark Hessling. */

if translate("Foo Bar") \=  "FOO BAR"        then do
  errors=errors+1
  say 'TRANSLATE failed in test          6 '
end
if translate("Foo Bar",,"") \=  "Foo Bar"    then do
  errors=errors+1
  say 'TRANSLATE failed in test          7 '
end
if translate("Foo Bar","",) \=  "       "    then do
  errors=errors+1
  say 'TRANSLATE failed in test          8 '
end
if translate("Foo Bar","",,'*') \=  "*******"  then do
  errors=errors+1
  say 'TRANSLATE failed in test          9 '
end
/*  
end
if translate("Foo Bar",xrange('01'x,'ff'x)) \==  "Gpp!Cbs"  then do
errors=errors+1
say 'TRANSLATE failed in test         10 ' ascii/ebcic */

if translate("","klasjdf","woieruw") \=  ""   then do
  errors=errors+1
  say 'TRANSLATE failed in test         11 '
end
if translate("foobar","abcdef","fedcba") \=  "aooefr"  then do
  errors=errors+1
  say 'TRANSLATE failed in test         12 '
end
if translate('The quick brown fox jumpst over the lazy dog','xy','eo') \= 'Thx quick brywn fyx jumpst yvxr thx lazy dyg'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          13 '
end
if translate('The quick brown fox jumpst over the lazy dog','.#$','azy')  \= 'The quick brown fox jumpst over the l.#$ dog'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          14 '
end
if translate('The quick brown fox jumpst over the lazy dog')  \= 'THE QUICK BROWN FOX JUMPST OVER THE LAZY DOG'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          15 '
end
if translate('1234567890','.!','1468','+')  \= '.23!5+7+90'   then do
  errors=errors+1
  say 'TRANSLATE failed in test          16 '
end

if translate('abc') \= 'ABC' then do
  errors=errors+1
  say 'TRANSLATE failed in test          17 '
end

/* TODO if translate('00010203'x,'123',,'$') \= '123$' then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test          18 ' */
/* end */

/* TODO if translate('00010203'x,'123') then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test          19 ' */
/* end */

vit = copies('0',100000)||'b'
vot = copies('1',100000)||'B'

if translate('abc',vot,vit) \= 'aBc' then do
  errors=errors+1
  say 'TRANSLATE failed in test          20 '
end

if translate('abcdef','123456','aaabbbcc','.') \= '14.def' then do
  errors=errors+1
  say 'TRANSLATE failed in test 21: translate(''abcdef'',''123456'',''aaabbbcc'',''.'')' translate('abcdef','123456','aaabbbcc','.') 'but must be 14.def'
end

if translate('abcdef ',,,'$') \= '$$$$$$$' then do
  errors=errors+1
  say "TRANSLATE failed in test 22: translate('abcdef ',,,'$') " "'"translate('abcdef ',,,'$')"'" "but must be '$$$$$$$' " 
end

if translate('abcdef ',1e+17,'ac','1') \= '1bEdef ' then do
  errors=errors+1
  say 'TRANSLATE failed in test 23: translate('abcdef ',1e+17,'ac','1') ' "'"translate('abcdef ',1e+17,'ac','1')"'" "but must be '1bEdef '"
end

if translate('abcdef ',1e+17,'ac','1') \= '1bEdef ' then do
  errors=errors+1
  say 'TRANSLATE failed in test 24: translate('abcdef ',1e+17,'ac','1') ' "'"translate('abcdef ',1e+17,'ac','1')"'" "but must be '1bEdef '"
end

if translate(translate('abcd','&','b'),translate('y','z','y'),translate('x','&','x')) \= 'azcd' then do
  errors=errors+1
  say 'TRANSLATE failed in test 25'
end

if TRANSLATE('abcdef','12','ec') \= 'ab2d1f' then do
  errors=errors+1
  say 'TRANSLATE failed in test 26'
end

if TRANSLATE('abcdef','12','abcd','.') \= '12..ef' then do
  errors=errors+1
  say 'TRANSLATE failed in test 27'
end

if TRANSLATE('APQRV', ,'PR') \= 'A Q V' then do
  errors=errors+1
  say 'TRANSLATE failed in test 28: TRANSLATE('APQRV', ,'PR')' "'"TRANSLATE('APQRV', ,'PR')"'" "but must be 'A Q V'"
end

if translate('español francés inglés') \= 'ESPAÑOL FRANCÉS INGLÉS' then do
  errors=errors+1
  say 'TRANSLATE failed in test 29: translate('español francés inglés')' "'"translate('español francés inglés')"'" "but should be 'ESPAÑOL FRANCÉS INGLÉS'"
end

if TRANSLATE('4123','abcd','1234') \= 'dabc' then do
  errors=errors+1
  say 'TRANSLATE failed in test 30'
end

/* TODO THIS NEEDS XRANGE if TRANSLATE('APQRV',XRANGE('00'X,'Q')) \= 'APQ  ' then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test 31' */
/* end */


/* TODO these are ooRexx extensions - decide */
/* TODO if translate('abc',,,,2) \= 'aBC' then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test 32' */
/* end */

/* TODO if translate('abc',,,,2,1) \= 'aBc' then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test 33' */
/* end */

/* TODO if translate('abc',,,,,1) \= 'Abc' then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test 34' */
/* end */
  
/* TODO if translate('abcdef','123456','aaabbbcc','.', 2, 3) \= 'a4.def' then do */
/*   errors=errors+1 */
/*   say 'TRANSLATE failed in test 35' */
/* end */
  
return errors<>0

translate: procedure = .string
arg source = .string, tochar = "?????", fromchar = "?????", pad=" "

/* copies function prototype */
copies: procedure = .string
arg string1 = .string, times= .int
