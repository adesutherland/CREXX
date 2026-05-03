options levelb
namespace interface_multi_interface_same_module

main: procedure
  by_name = .named
  by_size = .measured
  item = .widget("gear", 4)

  by_name = item
  by_size = item

  say by_name.name()
  say by_size.size()
  return

named: interface
  *: factory
  arg label = .string, length = .int
  name: method = .string

measured: interface
  size: method = .int

widget: class implements .named .measured
  _label = .string
  _length = .int

  *: factory
    arg label = .string, length = .int
    _label = label
    _length = length
    return

  name: method = .string
    return _label

  size: method = .int
    return _length
