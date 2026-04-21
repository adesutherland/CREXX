options levelb
namespace interface_ambiguous

main: procedure
  current = .vehicle
  current = .vehicle("duo")
  return

vehicle: interface
  *: factory = .vehicle
  arg name = .string
  describe: method = .string

car: class implements .vehicle
  *: factory = .vehicle
  arg name = .string
  return

  describe: method = .string
  return "car"

truck: class implements .vehicle
  *: factory = .vehicle
  arg name = .string
  return

  describe: method = .string
  return "truck"
