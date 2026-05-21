options levelb

main: procedure
  signal on other call handle_other
  signal other "handled payload"
  signal off other
  say "SUCCESS"
  return

handle_other: procedure = .signalaction
  arg condition = .signal
  if condition.name() = "OTHER" then say condition.message()
  return .signalaction.skip()
