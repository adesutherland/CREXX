options levelb

main: procedure = .int
  say "Starting ref inline negative test..."
  values = .int[3]
  values[1] = 10
  say refBump(values[1])
  say values[1]
  say "Ref inline negative test finished."
  return 0

refBump: procedure = .int
  arg expose value = .int
  value = value + 1
  return value
