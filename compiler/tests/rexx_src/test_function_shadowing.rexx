/* Test Function Shadowing */
options levelb
import rxfnsb

/* This should call the local procedure */
say translate('AAP NOOT MIES')

/* This should bypass the local procedure and call the BIF */
say 'TRANSLATE'('AAP NOOT MIES', ' ', 'O')

return 0

Translate: procedure = .string
arg inString = string
/* Local shadowing procedure */
return "LOCAL_TRANSLATE:" inString
