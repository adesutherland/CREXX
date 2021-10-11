/* rexx test abs bif */
options levelb
x='the quick brown fox jumps over the lazy dog'
say "test Space bif"
say space(x,5,'.!#')
say space(x)
return

/* function prototype */
space: procedure = .str
arg string1 = .string, int2 = 2, string3 = " "


