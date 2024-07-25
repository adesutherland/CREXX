/* */
options levelb
import rxfnsb


x='CREXX is faster than BREXX'
# say hello('much',x,10,10)
say insert('much ',x,9)

return 0

hello: procedure = .string
arg insstr = .string, string = .string, position = -1, len = -1, pad = " "
say "insstr =" insstr
say "string =" string
say "position =" position
say "len =" len
say "pad =" pad

return "Hello World"
