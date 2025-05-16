/* rexx */
options levelb

namespace rxfnsb expose changestr

/* changestr(needle,haystack,new-needle) returns string with replaced string */
changestr: procedure = .string
  arg expose needle = .string, expose haystack = .string, expose nneedle = .string /* Pass by reference */
  offset=1
  nlen=0
  assembler strlen nlen,needle
  nnlen=0
  assembler strlen nnlen,nneedle
  newstr=haystack

  do i=1 to 8192      /* use a large do loop until we get a do forever */
     offset=pos(needle,newstr,offset)
     if offset=0 then return newstr
     if offset=1 then newstr=nneedle||substr(newstr,offset+nlen)
     else newstr=substr(newstr,1,offset-1)||nneedle||substr(newstr,offset+nlen)
     offset=offset+nnlen
  end
return newstr
