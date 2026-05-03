options levelb

main: procedure = .int
  say "Starting call arg inline expr test..."
  do i = 1 to 4
    say keepValue(addOne(i))
  end
  say "Call arg inline expr test finished."
  return 0

keepValue: procedure = .int
  arg val = .int
  if val > 100 then return 100
  return val

addOne: procedure = .int
  arg val = .int
  return val + 1
