options levelb

main: procedure = .int
  say "main-count=" || arg()
  say "main-bracket=" || arg[]
  say "main-exists=" || arg(1,'E')
  say "helper-count=" || helperCount()
  say "helper-exists=" || helperExists()
  return 0

helperCount: procedure = .int
  return arg[]

helperExists: procedure = .int
  return arg(1,'E')
