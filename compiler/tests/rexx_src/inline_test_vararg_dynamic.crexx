options levelb

main: procedure = .int
  say pick(2, 10, 20, 30)
  say pick(1 + 2 - 1, 40, 50)
  say hasArg(3 - 1, 7, 8)
  say hasArg(5 - 1, 7, 8)
  return 0

pick: procedure = .int
  arg ix = .int, ... = .int
  return arg(ix)

hasArg: procedure = .int
  arg ix = .int, ... = .int
  return arg(ix, "E")
