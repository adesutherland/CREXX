options levelb

main: procedure
  signal on other call handle_other
  return

handle_other: procedure = .string
  arg condition = .signal
  return "skip"
