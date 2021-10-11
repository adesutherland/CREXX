/* rexx test abs bif */
options levelb
x='the quick brown fox jumps over the lazy dog'
y=reverse(x)
say "reverse "y
z=reverse(y)
say "double reverse "z
say "Empty String '"reverse("")"'"

/* function prototype */
reverse: procedure = .string
arg string1 = .string
