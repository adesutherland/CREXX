options levelb
namespace interface_method_object_arg_same_module

main: procedure
  current = .runner
  item = .payload

  current = .runner()
  item = .payload("hello")
  say current.execute(item)
  return

runner: interface
  *: factory
  execute: method = .string
  arg item = .payload

payload: class
  _text = .string

  *: factory
    arg text = .string
    _text = text
    return

  text: method = .string
    return _text

echo_runner: class implements .runner
  *: factory
    return

  execute: method = .string
    arg item = .payload
    return item.text()
