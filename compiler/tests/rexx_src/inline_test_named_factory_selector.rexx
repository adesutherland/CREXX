options levelb
namespace inline_test_named_factory_selector

main: procedure = .int
  say directNamed().describe()
  say assignedNamed("beta").describe()
  say describedNamed("gamma")
  return 0

directNamed: procedure = .vehicle
  return .vehicle.from_name("alpha")

assignedNamed: procedure = .vehicle
  arg name = .string
  current = .vehicle.from_name(name)
  if name = "beta" then do
    marker = 1
  end
  return current

describedNamed: procedure = .string
  arg name = .string
  current = .vehicle.from_name(name)
  return current.describe()

vehicle: interface
  *: factory
  arg name = .string
  from_name: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  _name = .string

  *: factory
    arg name = .string
    _name = "default:" || name
    return

  from_name: factory
    arg name = .string
    _name = "named:" || name
    return

  describe: method = .string
    return _name
