/* rexx test abs bif */
options levelb
say "test insert"
x='CREXX is faster than BREXX'

say "'"insert('much',x,10,10)"'"
say '------'
say "'"insert('much ',x,10)"'"

say '------'
say "'"insert('The new ',x,1)"'"

say '------'
say "'"insert(' ,isn"t it?',x,27)"'"

return

/* function prototype */
insert: procedure = .string
  arg expose insstr = .string, expose string = .string, position = .int, len = 0, pad = ' '



