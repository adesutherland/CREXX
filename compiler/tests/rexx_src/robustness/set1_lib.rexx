options levelb
namespace set1_lib expose glob_int glob_string get_val set_val is_true is_false

glob_int = 42
glob_string = "Hello"

get_val: procedure = .int
  return glob_int

set_val: procedure
  arg x = .int
  glob_int = x
  return

is_true: procedure = .boolean
  return 1

is_false: procedure = .boolean
  return 0
