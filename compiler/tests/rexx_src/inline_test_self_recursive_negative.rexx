options levelb

main: procedure = .int
  say "Starting self recursive inline negative test..."
  say recurse(3)
  say "Self recursive inline negative test finished."
  return 0

recurse: procedure = .int
  arg val = .int
  return recurse(val - 1)
