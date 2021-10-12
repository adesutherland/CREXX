/* rexx test center bif */
options levelb
/* 1234567890123456789012345678901234567890123 */
x='the quick brown fox jumps over the lazy dog'
say "'"center(x,43,)"'"
say "'"center(x,50,"!")"'"
say "'"center(x,9)"'"

say "'"centre(x,43,)"'"
say "'"centre(x,50,"!")"'"
say "'"centre(x,9)"'"

return

/* function prototype */
center: procedure = .str
arg string1 = .string, int2 = .int, char = " "

centre: procedure = .str
arg string1 = .string, int2 = .int, char = " "

