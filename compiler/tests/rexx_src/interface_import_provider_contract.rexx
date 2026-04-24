options levelb
namespace interface_import_provider_contract expose environment

environment: interface
  *: factory = .environment
    arg name = .string
  describe: method = .string

contractanchor: class
  *: factory
  return
