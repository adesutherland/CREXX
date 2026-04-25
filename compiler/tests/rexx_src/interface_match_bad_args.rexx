options levelb
namespace interface_match_bad_args

vehicle: interface
  *: factory
  arg name = .string

car: class implements .vehicle
  *: match
    arg name = .int
    return 1

  *: factory
    arg name = .string
    return
