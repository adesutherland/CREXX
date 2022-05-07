/* rexx test abs bif */
options levelb
say translate(ABCDEF,1e+17,'ac','1') "\= '1bEdef '"
return

translate: procedure = .string
arg source = .string, tochar = "?????", fromchar = "?????", pad=" "
