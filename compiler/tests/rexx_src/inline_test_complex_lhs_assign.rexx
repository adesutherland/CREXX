options levelb

main: procedure = .int
  values = .int[3]
  ix = .int
  ix = 1
  values[ix + 1] = addOne(4)
  say values[2]
  return 0

addOne: procedure = .int
  arg value = .int
  return value + 1
