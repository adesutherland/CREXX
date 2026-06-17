options levelb

main: procedure = .int
  first = 10
  second = 20
  third = 30

  say countTail(first, second, third)
  say pickSecond(first, second, third)
  say hasSecond(first, second, third)
  say hasFifth(first, second, third)
  say mutateSecond(first, second, third)
  say first
  say second
  say third
  return 0

countTail: procedure = .int
  arg expose ... = .int
  return arg[]

pickSecond: procedure = .int
  arg expose ... = .int
  return arg[2]

hasSecond: procedure = .int
  arg expose ... = .int
  return arg(2, "E")

hasFifth: procedure = .int
  arg expose ... = .int
  return arg(5, "E")

bumpOne: procedure = .int
  arg expose value = .int
  value = value + 7
  return value

mutateSecond: procedure = .int
  arg expose ... = .int
  return bumpOne(arg[2])
