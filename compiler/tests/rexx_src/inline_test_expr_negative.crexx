options levelb

main: procedure = .int
  say "Starting nested short-circuit inline negative test..."
  do i = 1 to 3
    if identity(addOne(i)) | 0 then say "truthy" i
    else say "falsey" i
  end
  say "Nested short-circuit inline negative test finished."
  return 0

identity: procedure = .int
  arg val = .int
  return val

addOne: procedure = .int
  arg val = .int
  return val + 1
