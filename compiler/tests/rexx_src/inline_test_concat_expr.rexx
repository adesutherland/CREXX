options levelb

main: procedure = .int
  say "Starting concat inline expr test..."
  do i = 1 to 3
    msg = "Value=" || asString(i)
    say msg
  end
  say "Concat inline expr test finished."
  return 0

asString: procedure = .string
  arg val = .int
  return val
