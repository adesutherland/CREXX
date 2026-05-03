options levelb
call myproc()
return 0

myproc: procedure
  keys = .string[10]
  keys[1] = "hello"
  say keys[1]
  return
