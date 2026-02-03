/* */
options levelb
import rxfnsb

x='CREXX is faster than BREXX'
say hello(x)

return 0

hello: procedure = .string
arg message = .string

return "message =" message

