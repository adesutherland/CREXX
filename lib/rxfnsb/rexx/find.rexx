/*
 * crexx find
 * VM-TSO compatible implementation of wordpos()
 */
options levelb

namespace rxfnsb expose find

find: procedure = .int
  arg expose needle = .string, haystack = .string, start = 1

return wordpos(needle,haystack)
