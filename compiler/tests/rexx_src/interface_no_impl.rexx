options levelb
namespace interface_no_impl

main: procedure
  current = .vehicle
  current = .vehicle("solo")
  return

vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string
