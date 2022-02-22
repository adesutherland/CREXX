/*
 * crexx find
 * VM-TSO compatible implementation of wordpos()
 */

options levelb

find: procedure = .int
  arg expose needle = .string, haystack = .string, start = 1

return wordpos(needle,haystack)


wordpos: procedure = .int
  arg expose needle = .string, haystack = .string, start = 1

