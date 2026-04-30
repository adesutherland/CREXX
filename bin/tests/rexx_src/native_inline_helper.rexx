options levelb comments_dash
namespace native_inline_helper expose helper
import rxfnsb

main: procedure
  say helper()
  return

helper: procedure=.int
  return 7
