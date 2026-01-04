/* rexx */
options levelb

namespace rxfnsb expose countstr

/* countstr(needle,haystack) returns number occurrences of needle in haystack */
countstr: procedure = .int
  arg expose needle = .string, expose haystack = .string /* Pass by reference */
  count=0
  offset=1   ## strpos is 1-based offset for best performance
  nlen=0
  hlen=0
  assembler strlen nlen,needle
  if nlen<1 then return 0   /* empty needle, nothing to count */
  assembler strlen hlen,haystack
  if hlen<1 then return 0   /* empty haystack, nothing to count */


  do i=1 to 8192      /* use a large do loop until we get a do forever */
     assembler strpos offset,needle,haystack
     if offset<=0 then return count
     offset=offset+nlen
     count=count+1
  end
return count
