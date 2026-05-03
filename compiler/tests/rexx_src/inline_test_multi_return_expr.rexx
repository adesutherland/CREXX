options levelb

main: procedure = .int
  say "Starting multi return expr inline test..."
  do i = -1 to 1
    say 10 + classify(i)
  end
  say "Multi return expr inline test finished."
  return 0

classify: procedure = .int
  arg val = .int
  if val < 0 then return -1
  if val > 0 then return 1
  return 0
