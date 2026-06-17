options levelb
namespace type_ops_showcase

main: procedure
  vehicle = .vehicle
  vehicle = .car("roadster")

  generic = .object
  generic = vehicle as .object

  narrowed = generic as .vehicle
  concrete = narrowed as .car

  say typeof(1)
  say typeof(generic)
  say generic is .object
  say generic is .vehicle
  say generic is .car
  say generic is .truck
  say concrete.describe()
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
