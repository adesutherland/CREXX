options levelb

main: procedure = .int
  say "Starting return inline expr test..."
  do i = 1 to 3
    say wrap(i)
  end
  say "Return inline expr test finished."
  return 0

wrap: procedure = .int
  arg val = .int
  return addOne(val)

addOne: procedure = .int
  arg val = .int
  return val + 1
