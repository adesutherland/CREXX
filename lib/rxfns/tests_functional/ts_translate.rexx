/* rexx */
options levelb
/* translate  */

say translate('The quick brown fox jumpst over the lazy dog','xy','eo')
say translate('The quick brown fox jumpst over the lazy dog','.#$','azy')
say translate('The quick brown fox jumpst over the lazy dog')
say translate('1234567890','.!','1468','+')

return

translate: procedure = .string
  arg expose source = .string, tochar = "", fromchar = "", pad=" "
