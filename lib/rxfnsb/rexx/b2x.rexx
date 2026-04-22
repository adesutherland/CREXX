/* rexx */
options levelb numeric_classic
namespace rxfnsb expose b2x
import _rxsysb

b2x: procedure = .string
  arg bin = .string
  bin = space(bin, 0)

  if verify(bin, '01') \= 0 then
    return ''

  if bin = '' then
    return ''

  hexlen = (length(bin) + 3) % 4

  pad = length(bin) // 4
  if pad \= 0 then
    bin = copies('0', 4 - pad) || bin

  hex = translate(reradix(bin, 2, 16))
  return right(hex, hexlen, '0')

