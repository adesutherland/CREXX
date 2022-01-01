/* rexx test abs bif */
options levelb
say "test changestr"
/*                     12345  */
x='the quick brown fox jumps over the lazy dog'
say '---1---'
say changestr('quick',x,'fast')
say '---2---'
say changestr('the',x,'a')
say '---3---'
say changestr('quicker',x,'fast')

return

/* function prototype */
changestr: procedure = .string
  arg string1 = .string, string2 = .string, string3 = .string