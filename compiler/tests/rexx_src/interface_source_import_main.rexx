options levelb
namespace interface_source_import_main
import interface_dep_contract

main: procedure
  current = .vehicle
  current = .vehicle("mini")
  say current.describe()
  return
