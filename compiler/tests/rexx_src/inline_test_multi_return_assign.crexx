options levelb

main: procedure = .int
  say "Starting multi return assign inline test..."
  do i = -1 to 1
    result = classify(i)
    say result
  end
  say "Multi return assign inline test finished."
  return 0

classify: procedure = .int
  arg val = .int
  if val < 0 then return -1
  if val > 0 then return 1
  return 0
