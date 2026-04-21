options levelb
namespace interface_missing_impl

vehicle: interface
  *: factory = .vehicle
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  *: factory = .vehicle
  arg name = .string
  return
