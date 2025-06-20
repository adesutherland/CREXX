/* rexx */
options levelb

namespace rxfnsb expose parse

/* ----------------------------------------------------------------------
 * Compile the given template, split it in tokens and token.ktypes
 * ----------------------------------------------------------------------
 */
parse: Procedure=.int
  arg string2Parse=.string, template=.string, expose variable=.string[], expose variable_content=.string[]
  token.1=''          ## init token array
  token_type.1=''     ## init types array
  rc=parseCompile(template,token,token_type)
  call parseString string2Parse, token, token_type,variable,variable_content
return variable.0
