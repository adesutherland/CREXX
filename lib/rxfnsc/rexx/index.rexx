/*
 * crexx index
 * VM-TSO compatible implementation of pos()
 */

options levelb

namespace rxfnsb expose index

index: procedure = .int
  arg expose haystack = .string, needle = .string, start = 1

return pos(needle,haystack,start)
