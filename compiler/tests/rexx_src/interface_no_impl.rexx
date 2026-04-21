options levelb
namespace interface_no_impl

main: procedure
  current = .vehicle
  current = .vehicle("solo")
  return

vehicle: interface
  *: factory = .vehicle
  arg name = .string
  describe: method = .string
