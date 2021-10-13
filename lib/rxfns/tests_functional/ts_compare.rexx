/* rexx test abs bif */
options levelb
say "test abbrev"
say '---1---'
/*                     12345  */
say compare('quicker','quick')
say '---2---'
say compare('quicker','quicker')
say '---3---'
say compare('quicker!','quicker',"!")
say '---4---'
say compare('quicker','fast')

return

/* function prototype */
compare: procedure = .string
  arg string1 = .string, string2 = .string, pad = " "