/* rexx test abs bif */
options levelb
x='the quick brown fox jumps over the lazy dog'
/* 12345678901234567890123456789012  */
say countstr('o',x)
return

/* function prototype */
countstr: procedure = .int
arg string1 = .string, string2 = .string


