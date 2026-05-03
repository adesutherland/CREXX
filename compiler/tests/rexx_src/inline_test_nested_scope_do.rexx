options levelb

main: procedure = .int
  say "Starting nested scope do inline test..."
  do i = 1 to 3
    say scopedAdd(i)
  end
  say "Nested scope do inline test finished."
  return 0

scopedAdd: procedure = .int
  arg val = .int
  do
    step = .int
    step = val + 2
    return step
  end
  return 0
