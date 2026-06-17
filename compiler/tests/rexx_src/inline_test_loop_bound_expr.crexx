options levelb

main: procedure = .int
  say "Starting loop bound inline test..."

  do repeatCount(2)
    say "for"
  end

  do i = startAt() to stopAt() by stepSize()
    say i
  end

  say "Loop bound inline test finished."
  return 0

repeatCount: procedure = .int
  arg value = .int
  return value

startAt: procedure = .int
  return 1

stopAt: procedure = .int
  return 5

stepSize: procedure = .int
  return 2
