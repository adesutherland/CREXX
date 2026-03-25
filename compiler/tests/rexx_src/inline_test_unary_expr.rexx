options levelb

main: procedure = .int
  say "Starting unary inline expr test..."
  do i = 1 to 3
    say -addOne(i)
    say +addOne(i)
  end
  say "Unary inline expr test finished."
  return 0

addOne: procedure = .int
  arg val = .int
  return val + 1
