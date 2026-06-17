options levelb

main: procedure = .int
  say "Starting member receiver inline test..."
  say buildBox("seed").getName()
  say buildBox("temp").getName()
  say "Member receiver inline test finished."
  return 0

buildBox: procedure = .Box
  arg initial = .string
  value = .Box(initial)
  return value

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  getName: method = .string
    return name
