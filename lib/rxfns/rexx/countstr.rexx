/* rexx */
options levelb

/* countstr(needle,haystack) returns number occurrences of needle in haystack */
countstr: procedure = .int
  arg expose needle = .string, expose haystack = .string /* Pass by reference */
  count=0
  offset=0

  do i=1 to 8192      /* use a large do loop until we get a do forever */
     offset=pos(needle,haystack,offset+1)
     if offset=0 then return count
     count=count+1
  end
return count

/* pos() Procedure */
pos: procedure = .int
arg needle = .string, haystack = .string, offset = 1
