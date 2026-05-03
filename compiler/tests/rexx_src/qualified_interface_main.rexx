options levelb
namespace qualified_interface_main
import qifa
import qifb

main: procedure
  left = .qifa..vehicle("one")
  right = .qifb..vehicle.from_name("two")

  say left.describe()
  say right.describe()
  return
