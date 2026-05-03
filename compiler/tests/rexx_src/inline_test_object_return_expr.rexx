options levelb

main: procedure = .int
  say "Starting object return expr inline test..."
  say describeBox(buildBox("seed"))
  say describeBox(buildBox("temp"))
  say "Object return expr inline test finished."
  return 0

buildBox: procedure = .Box
  arg initial = .string
  value = .Box(initial)
  return value

describeBox: procedure = .string
  arg value = .Box
  return value.getName()

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  getName: method = .string
    return name
