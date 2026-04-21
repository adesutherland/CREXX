options levelb
namespace interface_match_dep_contract expose vehicle car bike truck

vehicle: interface
  *: factory = .vehicle
  arg name = .string
  from_name: factory = .vehicle
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: match
    arg name = .string
    if name = "car gamma" then return 50
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
    if name = "bike beta" then return 90
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
