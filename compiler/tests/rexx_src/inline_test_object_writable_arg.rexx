options levelb

main: procedure = .int
  say "Starting object writable-arg inline test..."
  box = .Box("alpha")
  say replaceBox(box)
  say box.getName()
  say "Object writable-arg inline test finished."
  return 0

replaceBox: procedure = .string
  arg value = .Box
  value = .Box("local")
  return value.getName()

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  getName: method = .string
    return name
