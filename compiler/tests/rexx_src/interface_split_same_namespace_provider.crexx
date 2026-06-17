options levelb
namespace automotive expose mycar

mycar: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  describe: method = .string
  return "dep:" || _name
