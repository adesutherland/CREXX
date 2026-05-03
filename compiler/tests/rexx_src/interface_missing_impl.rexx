options levelb
namespace interface_missing_impl

vehicle: interface
  *: factory
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  *: factory
  arg name = .string
  return
