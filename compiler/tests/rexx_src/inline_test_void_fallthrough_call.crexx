options levelb

main: procedure = .int
  say "Starting void fallthrough inline test..."
  call maybeLog(0)
  call maybeLog(2)
  call tracePair(1, 3)
  say "Void fallthrough inline test finished."
  return 0

maybeLog: procedure = .void
  arg val = .int
  if val = 0 then return
  say "value" val

tracePair: procedure = .void
  arg left = .int, right = .int
  if left > right then return
  say "pair" left + right
