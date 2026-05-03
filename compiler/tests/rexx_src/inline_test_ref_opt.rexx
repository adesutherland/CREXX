options levelb

main: procedure = .int
  say "Starting ref/opt inline test..."
  x = 10
  say refBump(x)
  say x
  say optAdd()
  say optAdd(3)
  say "Ref/opt inline test finished."
  return 0

refBump: procedure = .int
  arg expose value = .int
  value = value + 1
  return value

optAdd: procedure = .int
  arg ?a = 10, ?b = 5
  return a + b
