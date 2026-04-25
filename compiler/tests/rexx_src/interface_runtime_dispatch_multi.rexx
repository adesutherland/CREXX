options levelb
namespace interface_runtime_dispatch_multi

main: procedure
  current = .vehicle

  current = .car("mini")
  say current.type()
  say current.describe()

  current = .truck("rig")
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

truck: class implements .vehicle
  _name = .string

  *: factory
  arg name = .string
  _name = name
  return

  type: method = .string
  return "truck"

  describe: method = .string
  return type() || ":" || _name
