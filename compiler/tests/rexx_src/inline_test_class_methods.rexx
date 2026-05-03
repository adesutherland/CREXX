options levelb
namespace inline_test_class_methods

main: procedure = .int
  box = .Box("alpha")
  say box.label()
  call box.setLabel("beta")
  say box.label()
  say box.prefix("p")
  call update(box, "gamma")
  say box.label()
  saved = box.setAndReport("delta")
  say saved
  say box.label()
  say box.setAndReport("epsilon")
  say box.label()
  say .Box("direct").prefix("q")
  return 0

update: procedure = .void
  arg item = .Box, value = .string
  call item.setLabel(value)
  return

Box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  label: method = .string
    return name

  setLabel: method = .void
    arg next = .string
    name = next
    return

  prefix: method = .string
    arg tag = .string
    return tag || ":" || label()

  setAndReport: method = .string
    arg next = .string
    name = next
    return name
