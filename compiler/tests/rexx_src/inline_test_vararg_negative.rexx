options levelb

main: procedure = .int
  say "Starting vararg inline negative test..."
  idx = 2
  a = 10
  b = 20
  c = 30
  say dynamicPick(idx, a, b, c)
  say refTailCount(a, b, c)
  say "Vararg inline negative test finished."
  return 0

dynamicPick: procedure = .int
  arg which = .int, ... = .int
  return arg[which]

refTailCount: procedure = .int
  arg expose ... = .int
  return arg[]
