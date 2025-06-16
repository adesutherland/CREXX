/* rexx */
options levelb

namespace rxfnsb expose parseString

/* ----------------------------------------------------------------------
 * Parse the string, using the found tokens and types
 * ----------------------------------------------------------------------
 */
parsestring: procedure
  arg parse_string=.string, token=.string[],token_type=.string[],expose variable=.string[],expose variable_content=.string[]
  pointer = 1
  j = 0
  t = parse_string
/* variable content containers */
  j = 0
  pointer = 1
  lastVarIndex = 0

  do k = 1 to token.0
     if token_type.k = 1 then do   /* type 1 is variable name **/
        j = j + 1
        variable.j = token.k
        lastVarIndex = k
     end
     else if token_type.k>=3 then do           /* type 3 set to fixed column, 4 re-position column */
          newpos = token.k
	      if token_type.k= 4 then newpos = newpos+pointer
          if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
             slen=newpos - pointer
             if newpos>pointer then variable_content.j = substr(parse_string, pointer, slen)
		     else variable_content.j=substr(parse_string, pointer)
             lastVarIndex = 0
          end
          pointer = newpos
     end
     else if token_type.k= 2 then do           /* type 2 is template */
          chunk = substr(parse_string, pointer)
          pos = pos(token.k, chunk)
          if pos = 0 then pos = length(chunk) + 1
          if lastVarIndex > 0 & token_type.lastVarIndex = 1 then do
             variable_content.j = substr(parse_string, pointer, pos - 1)
             lastVarIndex = 0
          end
          pointer = pointer + pos + length(token.k) - 1
     end
  end
/* Final fallback */
  if lastVarIndex > 0 & token_type.lastVarIndex = 1 then variable_content.j = substr(parse_string, pointer)
return