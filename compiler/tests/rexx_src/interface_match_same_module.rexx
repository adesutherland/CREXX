options levelb
namespace interface_match_same_module

main: procedure
  say .vehicle("car alpha").describe()
  say .vehicle("bike beta").describe()
  say .vehicle("other").describe()
  say .vehicle.from_name("car gamma").describe()
  say .vehicle.from_name("bike delta").describe()
  say .vehicle.from_name("other").describe()
  return

vehicle: interface
  *: factory
  arg name = .string
  from_name: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: match
    arg name = .string
    if name = "car alpha" then return 50
    return 0

  *: factory
    arg name = .string
    _name = "car:" || name
    return

  from_name: match
    arg name = .string
    if name = "car gamma" then return 60
    return 0

  from_name: factory
    arg name = .string
    _name = "car:named:" || name
    return

  describe: method = .string
    return _name

bike: class implements .vehicle
  _name = .string

  *: match
    arg name = .string
    if name = "bike beta" then return 80
    return 0

  *: factory
    arg name = .string
    _name = "bike:" || name
    return

  from_name: match
    arg name = .string
    if name = "bike delta" then return 90
    return 0

  from_name: factory
    arg name = .string
    _name = "bike:named:" || name
    return

  describe: method = .string
    return _name

truck: class implements .vehicle
  _name = .string

  *: factory
    arg name = .string
    _name = "truck:" || name
    return

  from_name: factory
    arg name = .string
    _name = "truck:named:" || name
    return

  describe: method = .string
    return _name
