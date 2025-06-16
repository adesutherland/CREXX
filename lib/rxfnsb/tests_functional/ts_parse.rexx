/*
 * crexx RXPP
 * CREXX Pre Compiler
 */
options levelb
import rxfnsb
  token.1=''          ## init token array
  token_type.1=''     ## init types array
  variable.1=''
  variable_content.1=''
## 1. Example
## ----------
  say '-------- 1. Example --------------'
  template = "dsn'(' member "'")"mode'
  string2Parse = "  ms.mspdm.slib(crexx) SHR"
  say time('l')' Compile Template'
  rc=parseCompile(template,token,token_type)
     call token_print token,token_type                              ## print tokens
  call parseString string2Parse, token, token_type,variable,variable_content
     call variable_print string2Parse, variable,variable_content    ## print variables
## 2. Example
## ----------
  say '-------- 2. Example --------------'
  template = "name 10 job +8 surname 'is-a' skill"
  string2Parse = "René     Captain Jansen is-a REXX wizard"
  say time('l')' Compile Template'
  rc=parseCompile(template,token,token_type)
    call token_print token,token_type                              ## print tokens
  call parseString string2Parse, token, token_type,variable,variable_content
     call variable_print string2Parse, variable,variable_content   ## print variables
## 2. Example
## ----------
  say '-------- 3. Example --------------'
  template = "first','second','third','fourth','fifth"
  string2Parse = "1. think,2. overthink,3. redesign,4. code in rexx and pray,5. blame the user"
   say time('l')' Compile Template'
  rc=parseCompile(template,token,token_type)
    call token_print token,token_type                              ## print tokens
  call parseString string2Parse, token, token_type,variable,variable_content
     call variable_print string2Parse, variable,variable_content   ## print variables
exit
/* ----------------------------------------------------------------------
 *  Print created tokens
 * ----------------------------------------------------------------------
 */
token_print: procedure
  arg token=.string[],token_type=.string[]
  say 'Template='template
  say time('l')' Compile completed'
  say 'Created Tokens, type=1: variable, 2=quoted-string, 3=column-set, 4=column-reposition'
  do j = 1 to token.0
     say "Token" j ": '"token.j"' Type:" token_type.j
  end
  say time('l')' Parse String using compiled Template'
return
/* ----------------------------------------------------------------------
 * Output results, there is no variable assignment yet, just where it
 *        should go. Maybe I'll knock something up in the pre-compiler
 * ----------------------------------------------------------------------
 */
variable_print: procedure
   arg string2parse=.string,variable=.string[],variable_content=.string[]
   say time('l')' Parse completed'
   say "Template="template
   say "         1...5....0....5....0....5....0....5....0"
   say "Parse    "string2parse
   say "Extracted variables"
   do i = 1 to variable.0
      say left(variable.i,8) " = '"variable_content.i"'"
   end
return