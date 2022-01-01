/* rexx */
options levelb
/* translate  */
  say "Look for TRANSLATE OK"
/* TRANSLATE */

/* These from the Rexx book. */
  if translate('abcdef') \= 'ABCDEF'   then say 'failed in test          1 '
  if translate('abbc','&','b') \= 'a&&c'   then say 'failed in test          2 '
  if translate('abcdef','12','ec') \= 'ab2d1f'   then say 'failed in test          3 '
  if translate('abcdef','12','abcd','.') \= '12..ef'   then say 'failed in test          4 '
  if translate('4123','abcd','1234') \= 'dabc'   then say 'failed in test          5 '
/* These from Mark Hessling. */
  if translate("Foo Bar") \=  "FOO BAR"        then say 'failed in test          6 '
  if translate("Foo Bar",,"") \=  "Foo Bar"    then say 'failed in test          7 '
  if translate("Foo Bar","",) \=  "       "    then say 'failed in test          8 '
  if translate("Foo Bar","",,'*') \=  "*******"  then say 'failed in test          9 '
/*  if translate("Foo Bar",xrange('01'x,'ff'x)) \==  "Gpp!Cbs"  then say 'failed in test         10 ' ascii/ebcic */
  if translate("","klasjdf","woieruw") \=  ""   then say 'failed in test         11 '
  if translate("foobar","abcdef","fedcba") \=  "aooefr"  then say 'failed in test         12 '
  if translate('The quick brown fox jumpst over the lazy dog','xy','eo') \= 'Thx quick brywn fyx jumpst yvxr thx lazy dyg'   then say 'failed in test          13 '
if translate('The quick brown fox jumpst over the lazy dog','.#$','azy')  \= 'The quick brown fox jumpst over the l.#$ dog'   then say 'failed in test          14 '
if translate('The quick brown fox jumpst over the lazy dog')  \= 'THE QUICK BROWN FOX JUMPST OVER THE LAZY DOG'   then say 'failed in test          15 '
if translate('1234567890','.!','1468','+')  \= '.23!5+7+90'   then say 'failed in test          16 '

say "TRANSLATE OK"

return

translate: procedure = .string
  arg source = .string, tochar = "?????", fromchar = "?????", pad=" "
