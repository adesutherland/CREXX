options levelb
namespace interface_default_codegen

main: procedure
  say .box().describe()
  return

shape: interface
  *: factory = .shape
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
