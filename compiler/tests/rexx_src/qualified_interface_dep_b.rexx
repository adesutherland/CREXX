options levelb
namespace qifb expose vehicle

vehicle: interface
  *: factory = .vehicle
  arg name = .string
  from_name: factory = .vehicle
  arg name = .string
  describe: method = .string

truck: class implements .vehicle
  _name = .string

  *: factory = .vehicle
    arg name = .string
    _name = "B-" || name
    return

  from_name: factory = .vehicle
    arg name = .string
    _name = "B:named-" || name
    return

  describe: method = .string
    return _name
