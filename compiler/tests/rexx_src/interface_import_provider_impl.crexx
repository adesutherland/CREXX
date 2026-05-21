options levelb
namespace interface_import_provider_impl expose cmsenvironment
import interface_import_provider_contract

cmsenvironment: class implements .environment
  _name = .string

  *: match
    arg name = .string
    if name = "cms" then return 100
    return 0

  *: factory
    arg name = .string
    _name = name
    return

  describe: method = .string
    return "cms:" || _name
