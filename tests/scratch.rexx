options levelb
import rxfnsb
#namespace hello
 hello = 3

call message "Welcome"

say "hello crexx!"
say 'hello crexx'
say hello crexx

/* some arithmetic */
say 1 + 1
say 0.2 - 0.1
say 800/81

say 'hello '||'Crexx'
return

message: procedure = .int
  arg message = .string
  say message
  return 0