options levelb
namespace type_ops_fail

main: procedure
  vehicle = .vehicle
  vehicle = .car("roadster")

  generic = .object
  generic = vehicle as .object

  wrong = generic as .truck
  say wrong.describe()
  return

vehicle: interface
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
