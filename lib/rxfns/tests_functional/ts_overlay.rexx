/* rexx test abs bif */
options levelb
say "test insert"
x='CREXX is faster  than BREXX'

say "'"overlay('quicker ',x,10)"'"

say '--2----'
say "'"overlay('as',x,17,5)"'"

say '--3----'
say "'"overlay('The new system ',x,1)"'"

say '--4----'
say "'"overlay(" ,isnt it?",x,27)"'"

return

/* function prototype */
overlay: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = " "



