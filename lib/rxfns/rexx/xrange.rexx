/* rexx */
options levelb

namespace rxfnsb

/*
 * XRANGE: deprecated
 * (XRANGE is limited to single byte characters 00-FF and wraps around) 
 */ 
xrange: procedure = .string
arg from = .string, tos = .string

say 'xrange is deprecated; using sequence'

return sequence(from, tos)

