options levelb
namespace dynamic_load_provider expose dynamic_value

dynamic_value: procedure = .string
  arg text = .string
  return "dynamic:" || text
