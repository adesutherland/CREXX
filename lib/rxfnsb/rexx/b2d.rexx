/* rexx */
options levelb
namespace rxfnsb expose b2d
import _rxsysb

b2d: procedure = .int
  arg bin = .string
  return reradix(bin,2,10)

