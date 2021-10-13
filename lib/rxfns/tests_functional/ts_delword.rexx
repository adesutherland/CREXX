/* rexx test abs bif */
options levelb
say "test delword"
x='CREXX is faster than BREXX'

say "delword(x,1) '"delword(x,1)"'"
say "delword(x,2) '"delword(x,2)"'"
say "delword(x,3) '"delword(x,3)"'"
say "delword(x,4) '"delword(x,4)"'"
say "delword(x,5) '"delword(x,5)"'"
say "delword(x,99) '"delword(x,99)"'"

say "delword(x,1,1) '"delword(x,1,1)"'"
say "delword(x,2,1) '"delword(x,2,1)"'"
say "delword(x,3,1) '"delword(x,3,1)"'"
say "delword(x,4,1) '"delword(x,4,1)"'"
say "delword(x,5,1) '"delword(x,5,1)"'"
say "delword(x,99,1) '"delword(x,99,1)"'"

say "delword(x,1,2) '"delword(x,1,2)"'"
say "delword(x,2,3) '"delword(x,2,3)"'"
say "delword(x,3,4) '"delword(x,3,4)"'"
say "delword(x,4,2) '"delword(x,4,2)"'"
say "delword(x,5,3) '"delword(x,5,3)"'"
say "delword(x,99,4) '"delword(x,99,4)"'"

return

/* function prototype */
delword: procedure = .string
arg string1 = .string, wnum = .int, wcount = 0


