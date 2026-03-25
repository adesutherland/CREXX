options levelb

main: procedure = .int
  say "Starting inline expr test..."
  do i = 1 to 4
    result = 10 + addOne(i)
    say "Computed" result
  end
  say "Inline expr test finished."
  return 0

addOne: procedure = .int
  arg val = .int
  return val + 1
