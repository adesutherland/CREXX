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
  return observe(val * 2)

observe: procedure = .int
  arg value = .int
  if value < 0 then return 0
  say "Observed return" value
  return value
