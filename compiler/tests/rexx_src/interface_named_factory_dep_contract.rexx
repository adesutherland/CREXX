options levelb
namespace interface_named_factory_dep_contract expose vehicle car

vehicle: interface
  *: factory
  arg name = .string
  from_name: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  from_name: factory
  arg name = .string
  _name = "dep:" || name
  return

  describe: method = .string
  return _name
