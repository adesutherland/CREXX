options levelb
namespace interface_named_factory_tiebreak

main: procedure
  current = .vehicle.from_name("duo")
  say current.describe()
  return

vehicle: interface
  *: factory = .vehicle
  arg name = .string
  from_name: factory = .vehicle
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: factory = .vehicle
  arg name = .string
  _name = name
  return

  from_name: factory = .vehicle
  arg name = .string
  _name = "car:" || name
  return

  describe: method = .string
  return _name

truck: class implements .vehicle
  _name = .string

  *: factory = .vehicle
  arg name = .string
  _name = name
  return

  from_name: factory = .vehicle
  arg name = .string
  _name = "truck:" || name
  return

  describe: method = .string
  return _name
