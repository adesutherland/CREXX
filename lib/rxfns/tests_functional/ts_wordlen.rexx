/* rexx test abs bif */
options levelb
x='the quick brown fox jumps over the lazy dog'
wrds=words(x)
do i=1 to wrds
   say word(x,i) wordlen(x,i)
end
return

/* function prototype */
wordlen: procedure = .int
arg string1 = .string, int2 = .int

word: procedure = .string
arg string1 = .string, int2 = .int

words: procedure = .int
arg string1 = .string

