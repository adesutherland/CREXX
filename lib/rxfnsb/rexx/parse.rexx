/* rexx */
options levelb

namespace rxfnsb expose parse

/* ----------------------------------------------------------------------
 * Compile the given template, split it in tokens and token.ktypes
 * ----------------------------------------------------------------------
 */
parse: Procedure=.int
  arg string2Parse=.string, template=.string, expose variable=.string[], expose variable_content=.string[],option=0,upper=0
  token.1=''          ## init token array
  token_type.1=''     ## init types array
  count=parseCompile(template,token,token_type)

  call parseString string2Parse, count, token, token_type,variable,variable_content,template
  if option=1 | upper>0 then do i=1 to variable_content.0
     if option=1 then variable_content.i=strip(variable_content.i)
     if upper=1 then variable_content.i=upper(variable_content.i)
     else if upper=2 then variable_content.i=lower(variable_content.i)
  end
return variable.0
