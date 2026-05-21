options levelb
namespace interface_default_same_module

main: procedure
  iface_value = .shape()
  class_value = .box()
  say iface_value.describe()
  say class_value.describe()
  return

shape: interface
  *: factory
  describe: method = .string
    return prefix() || ":" || summary()
  prefix: method = .string
    return "iface"
  summary: method = .string

box: class implements .shape
  *: factory
    return

  *: match
    return 10

  summary: method = .string
    return "box"
