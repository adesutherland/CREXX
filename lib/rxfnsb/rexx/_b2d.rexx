/* rexx */
options levelb
namespace rxfnsb expose _b2d
import _rxsysb

_b2d: procedure = .int
  arg bin = .string
  return reradix(bin,2,10)

