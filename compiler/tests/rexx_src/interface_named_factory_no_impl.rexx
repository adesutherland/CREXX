options levelb
namespace interface_named_factory_no_impl

main: procedure
  current = .vehicle.from_name("solo")
  say current.describe()
  return

vehicle: interface
  from_name: factory = .vehicle
  arg name = .string
  describe: method = .string
