options levelb

main: procedure = .int
  say "Starting nested scope exposed call inline test..."
  do i = 1 to 3
    say outer(i)
  end
  say "Nested scope exposed call inline test finished."
  return 0

outer: procedure = .int
  arg val = .int
  do
    tmp = .int
    tmp = addOne(val)
    return tmp * 2
  end
  return 0

addOne: procedure = .int
  arg val = .int
  return val + 1
