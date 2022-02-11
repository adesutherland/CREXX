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


return errors<>0

translate: procedure = .string
arg source = .string, tochar = "?????", fromchar = "?????", pad=" "
