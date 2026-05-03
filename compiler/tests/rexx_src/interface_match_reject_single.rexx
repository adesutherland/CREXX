options levelb
namespace interface_match_reject_single

main: procedure
  current = .vehicle("x")
  say current.describe()
  return

vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: match
    arg name = .string
    return 0

  *: factory
    arg name = .string
    _name = "car:" || name
    return

  describe: method = .string
    return _name
