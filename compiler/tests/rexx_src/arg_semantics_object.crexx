options levelb

main: procedure = .int
  box = .Box("alpha")
  say "object-read=" || readBox(box)
  say "object-mut=" || replaceBox(box)
  say "object-after=" || box.getName()
  say "object-temp=" || replaceBox(.Box("temp"))
  call replaceBoxRef(box)
  say "object-ref=" || box.getName()
  return 0

readBox: procedure = .string
  arg value = .Box
  return value.getName()

replaceBox: procedure = .string
  arg value = .Box
  value = .Box("local")
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
