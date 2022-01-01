/* rexx test abs bif */
options levelb
say "test abbrev"
say '---1---'
say abbrev('quicker','quick')
say '---2---'
say abbrev('quicker','q',2)
say '---3---'
say abbrev('quicker','fast')
return

/* function prototype */
abbrev: procedure = .string
  arg string1 = .string, string2 = .string, len = 0