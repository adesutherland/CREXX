options levelb

main: procedure = .int
  say "Starting inline call test..."
  do i = 1 to 3
    call logDouble(i)
  end
  say "Inline call test finished."
  return 0

logDouble: procedure = .int
  arg val = .int
  say "Double for" val "is" val * 2
  return val * 2
