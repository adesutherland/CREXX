options levelb

main: procedure = .int
  say "Starting nested call inline expr test..."
  do i = 1 to 3
    say identity(addOne(i))
  end
  say "Nested call inline expr test finished."
  return 0

identity: procedure = .int
  arg val = .int
  return val

addOne: procedure = .int
  arg val = .int
  return val + 1
