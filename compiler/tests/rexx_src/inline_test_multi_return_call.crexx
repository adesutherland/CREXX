options levelb

main: procedure = .int
  say "Starting multi return call inline test..."
  do i = 0 to 2
    call maybeLog(i)
  end
  say "Multi return call inline test finished."
  return 0

maybeLog: procedure = .void
  arg val = .int
  if val = 0 then return
  say "value" val
  return
