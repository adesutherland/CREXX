options levelb

main: procedure = .int
  say "Starting exposed nested call inline test..."
  do i = 1 to 4
    result = compute(i)
    say "Computed" result
  end
  say "Exposed nested call inline test finished."
  return 0

compute: procedure = .int
  arg val = .int
  return 10 + addOne(val)

addOne: procedure = .int
  arg val = .int
  return val + 1
