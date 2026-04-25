options levelb
namespace interface_tracer

main: procedure
  current = .vehicle
  current = .vehicle("roadster")
  say current.type()
  say current.describe()
  return

vehicle: interface
  *: factory
  arg name = .string
  type: method = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  type: method = .string
  return "car"

  describe: method = .string
  return type() || ":" || _name
