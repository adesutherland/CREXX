/* rexx */
options levelb

namespace rxfnsb expose lastpos

/* pos(sub-string in string beginning at) */
lastpos: procedure = .int
  arg needle = .string, haystack=.string, upto=0
  if upto<1 then do
     assembler strlen upto,haystack
  end
  nlen=0
  assembler strlen nlen,needle
  if nlen=0      then return 0
  if haystack='' then return 0
  nlen=nlen-1
  foundpos=1
  lastfound=0
  do forever
     assembler strpos foundpos,needle,haystack
     if foundpos = 0 then leave
     if foundpos + nlen> upto then leave    ## nlen-1 calculated above
     lastfound = foundpos
     foundpos=foundpos + 1
  end
return lastfound
