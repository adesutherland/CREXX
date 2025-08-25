/* rexx wordlength finds length of certain word */
options levelb

namespace rxfnsb expose qwordlength

qwordlength: procedure = .string
  arg expose string = .string, wordnum = .int
  wlen=0
  wordstr=qword(string,wordnum)
  if wrdstr="" then return 0
  assembler strlen wlen,wordstr
return wlen


