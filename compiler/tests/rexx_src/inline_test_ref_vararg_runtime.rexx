options levelb

main: procedure = .int
  first = 10
  second = 20
  third = 30

  say countTail(first, second, third)
  say readTail(2, first, second, third)
  say mutateTailIndirect(2, first, second, third)
  say first
  say second
  say third
  return 0

countTail: procedure = .int
  arg expose ... = .int
  return arg[]

readTail: procedure = .int
  arg which = .int, expose ... = .int
  return arg[which]

bumpOne: procedure = .int
  arg expose value = .int
  value = value + 7
  return value

mutateTailIndirect: procedure = .int
  arg which = .int, expose ... = .int
  return bumpOne(arg[which])
