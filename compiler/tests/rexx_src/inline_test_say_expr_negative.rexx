options levelb

main: procedure = .int
  say "Starting say inline negative test..."
  do i = 1 to 3
    say asString(i)
  end
  say "Say inline negative test finished."
  return 0

asString: procedure = .string
  arg val = .int
  return val
