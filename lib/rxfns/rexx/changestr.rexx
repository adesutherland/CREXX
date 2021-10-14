/* rexx */
options levelb

/* changestr(needle,haystack,new-needle) returns string with replaced string */
changestr: procedure = .string
  arg expose needle = .string, expose haystack = .string, expose nneedle = .string /* Pass by reference */
  offset=1
  nlen=0
  assembler strlen nlen,needle
  newstr=haystack

  do i=1 to 8192      /* use a large do loop until we get a do forever */
     offset=pos(needle,newstr,offset)
     if offset=0 then return newstr
     if offset=1 then newstr=nneedle||substr(newstr,offset+nlen)
     else newstr=substr(newstr,1,offset-1)||nneedle||substr(newstr,offset+nlen)
     offset=offset+nlen
  end
return newstr

/* pos() Procedure */
pos: procedure = .int
arg needle = .string, haystack = .string, offset = 1

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '
