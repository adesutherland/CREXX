options levelb

main: procedure = .int
  say "Starting vararg inline test..."
  say tailCount(10)
  say tailCount(10, 20, 30)
  total = 1 + secondOnly(5, 7, 11)
  say total
  say hasThird(1, 2)
  say hasThird(1, 2, 3)
  call logSecond(3, 9, 12)
  say "Vararg inline test finished."
  return 0

tailCount: procedure = .int
  arg base = .int, ... = .int
  return base + arg[]

secondOnly: procedure = .int
  arg base = .int, ... = .int
  return base + arg[2]

hasThird: procedure = .int
  arg ... = .int
  return arg(3, "E")

logSecond: procedure = .int
  arg base = .int, ... = .int
  say base + arg[2]
  return base
