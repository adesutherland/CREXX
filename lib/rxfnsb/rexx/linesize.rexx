/* rexx linesize bif */
options levelb

namespace rxfnsb expose linesize

/*
 * here mainly because it needs a native implementation on z/VM
 * and probably other OS; this one returns 999999999 to be consistent
 * with the Rexx compiler for zSeries 
 */  

linesize: procedure = .int
  result = 999999999
  return result
