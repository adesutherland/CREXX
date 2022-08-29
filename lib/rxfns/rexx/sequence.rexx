/* rexx */
options levelb

namespace rxfnsb
import _rxsysb

/* Built-in function Sequence is a modern day equivalent of XRANGE,
 * that can deal with Unicode. First turned up in NetRexx.
 * (XRANGE is limited to single byte characters 00-FF and wraps around) 
 */ 
sequence: procedure = .string
arg from = .string, tos = .string

/* TODO: support hex notation */

fromVal=c2d(from)
toVal  =c2d(tos)

/* as opposed to xrange, there is no wraparound */
if fromVal > toVal then do
  say 'starting value must be less than end value'
  return "BAD"
end

resultString = ""
diff=0

assembler isub diff,toVal,fromVal

loop i=0 to diff
  assembler itos fromVal
  val=reradix(fromVal,10,16)
  resultString=resultString||x2c(val)
  assembler inc fromVal
end

return resultString

