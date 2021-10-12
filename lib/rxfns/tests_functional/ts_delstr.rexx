/* rexx test abs bif */
options levelb
say "test delstr"
x='CREXX is faster than BREXX'
say "6,9 " "'"delstr(x,6,100)"'"
say "5   " "'"delstr(x,5)"'"
say "1   " "'"delstr(x,1)"'"
say "99  " "'"delstr(x,99)"'"
say "0   " "'"delstr(x,0)"'"

return

/* function prototype */
delstr: procedure = .string
arg string1 = .string, pos = .int, dlen = 0


