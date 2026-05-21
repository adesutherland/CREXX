options levelb

main: procedure = .int
  say "Starting nested scope if-do inline test..."
  do i = 0 to 2
    say classify(i)
  end
  say "Nested scope if-do inline test finished."
  return 0

classify: procedure = .int
  arg val = .int
  if val > 0 then do
    tmp = .int
    tmp = val * 10
    return tmp
  end
  return val
