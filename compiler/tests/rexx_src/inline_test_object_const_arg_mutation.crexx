options levelb

main: procedure = .int
  say "Starting object const-arg mutation inline test..."
  box = .Box("seed")
  call mutateBox(box)
  say box.getName()
  say "Object const-arg mutation inline test finished."
  return 0

mutateBox: procedure = .void
  arg value = .Box
  call value.setName("changed")
  return

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  setName: method = .void
    arg next = .string
    name = next
    return

  getName: method = .string
    return name
