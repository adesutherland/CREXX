options levelb
namespace interface_method_arg_same_module

main: procedure
  current = .speaker

  current = .speaker("iface")
  say current.greet("world")
  return

speaker: interface
  *: factory = .speaker
  arg prefix = .string

  greet: method = .string
  arg name = .string

echospeaker: class implements .speaker
  _prefix = .string

  *: factory = .speaker
    arg prefix = .string
    _prefix = prefix
    return

  greet: method = .string
    arg name = .string
    return _prefix || ":" || name
