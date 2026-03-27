options levelb

main: procedure = .int
  say renameBox(makeBox())
  return 0

renameBox: procedure = .string
  arg box = .Box
  call box.setName("changed")
  return box.getName()

makeBox: procedure = .Box
  box = .Box("seed")
  return box

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  setName: method
    arg value = .string
    name = value
    return

  getName: method = .string
    return name
