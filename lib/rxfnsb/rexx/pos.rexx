/* rexx */
options levelb

namespace rxfnsb expose pos

/* pos(sub-string in string beginning at) */
pos: procedure = .int
  arg needle = .string, haystack=.string, start=1
  if needle='' then return 0
  if haystack='' then return 0
  foundpos=start   ## use return variable to input start position
  assembler strpos foundpos,needle,haystack
return foundpos