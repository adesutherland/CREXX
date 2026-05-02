/* Explicit runtime module/plugin loader. */
options levelb

namespace rxfnsb expose loadmodule

loadmodule: procedure = .int
  arg module_name = .string

  mod_num = 0
  assembler metaloadmodule mod_num, module_name
  return mod_num
