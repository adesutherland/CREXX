options levelb

main: procedure = .int
  say "Starting mutual recursive inline negative test..."
  say left(3)
  say "Mutual recursive inline negative test finished."
  return 0

left: procedure = .int
  arg val = .int
  return right(val - 1)

right: procedure = .int
  arg val = .int
  return left(val - 1)
