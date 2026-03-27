options levelb

main: procedure = .int
  say "Starting object return assign inline test..."
  box = buildBox("seed")
  say describeBox(box)
  other = buildBox("temp")
  say describeBox(other)
  call replaceBoxRef(box)
  say box.getName()
  say "Object return assign inline test finished."
  return 0

buildBox: procedure = .Box
  arg initial = .string
  value = .Box(initial)
  return value

describeBox: procedure = .string
  arg value = .Box
  return value.getName()

replaceBoxRef: procedure = .void
  arg expose value = .Box
  value = .Box("ref")
  return

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  getName: method = .string
    return name
