options levelb
namespace interface_default_dep_contract expose shape box

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
    return "dep"
