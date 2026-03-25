options levelb

main: procedure = .int
  say "Starting bool inline negative test..."
  do i = 1 to 3
    if isTwo(i) | 0 then say "truthy" i
    else say "falsey" i
  end
  say "Bool inline negative test finished."
  return 0

isTwo: procedure = .int
  arg val = .int
  return val = 2
