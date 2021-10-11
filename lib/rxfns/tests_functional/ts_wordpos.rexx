/* rexx test abs bif */
options levelb
/* 1234567890123456789012345678901234567890123 */
x='the quick brown fox jumps over the lazy dog'
say wordpos('jum',x)
say wordpos('the',x)
say wordpos('the',x,5)
return

/* function prototype */
wordpos: procedure = .int
arg string1 = .string, string2 = .string, int3 = 1


