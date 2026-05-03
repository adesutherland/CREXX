options levelb
namespace qifa expose vehicle

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
    _name = "A-" || name
    return

  from_name: factory
    arg name = .string
    _name = "A:named-" || name
    return

  describe: method = .string
    return _name
