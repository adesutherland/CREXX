options levelb
namespace interface_factory_tiebreak

main: procedure
  current = .vehicle
  current = .vehicle("duo")
  say current.describe()
  return

vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  describe: method = .string
  return "car:" || _name

truck: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  describe: method = .string
  return "truck:" || _name
