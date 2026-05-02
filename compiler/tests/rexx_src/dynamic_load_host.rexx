options levelb
namespace dynamic_load_host expose dynamic_load_host
import rxfnsb
import dynamic_load_provider

dynamic_load_host: procedure = .int
  arg provider_path = .string

  module_num = .int

  module_num = loadmodule(provider_path)
  if module_num <= 0 then return 10

  if dynamic_value("ok") \= "dynamic:ok" then return 20

  return 0
