options levelb

main: procedure = .int
  say "Starting compare inline expr test..."
  do i = 1 to 4
    if addOne(i) = 3 then say "matched" i
    else say "missed" i
  end
  say "Compare inline expr test finished."
  return 0

addOne: procedure = .int
  arg val = .int
  return val + 1
