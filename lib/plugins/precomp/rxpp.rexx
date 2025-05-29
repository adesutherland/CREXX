/*
 * crexx RXPP
 * CREXX Pre Compiler
 * Author Peter Jacob
 * Created May 2025
 */
options levelb
import precomp
namespace rxpp expose source stype callargs macros_mname macros_margs macros_mbody macros_varname macros_varvalue printgen_global outbuf lino rexxlines alphaN mExpanded
import rxfnsb
/* ------------------------------------------------------------------
 * CREXX Pre Compiler
 *   Currently supported Commands
 *   ##DEFINE
 *   ##INCLUDE  (nested)
 * ------------------------------------------------------------------
 */
arg command=.string[]

  say '['time('l')'] Pre-Compile started'
  do i=1 to command.0
     j=i+1
     if command.i='-i' then infile=command.j
     else if command.i='-o' then outfile=command.j
     else if command.i='-m' then maclib=command.j
  end

infile  = 'C:/Users/PeterJ/CLionProjects/CREXX/250606/lib/plugins/precomp/Macro1.rxpp'
outfile = 'C:/Users/PeterJ/CLionProjects/CREXX/250606/lib/plugins/precomp/\Macro1.rexx'
maclib  = 'C:/Users/PeterJ/CLionProjects/CREXX/250606/lib/plugins/precomp/\Maclib.rexx'


    say 'Input File:  ' infile
    say 'Output File: ' outfile
    say 'Macro Lib:   ' maclib


  call rxppinit infile                              ## init global and environment variables
  say '['time('l')'] Pre-Compile pass one'
  sourceLines=RXPPPassOne(infile,outfile,maclib)    ## load source file and macro library
  say '['time('l')'] Pre-Compile pass two'
  call RXPPPassTwo outfile,sourceLines              ## analyse source and expand macros
  say '['time('l')'] Pre-Compiled REXX saved'
  call list_array outbuf, -1,-1
  call writeall outbuf,outfile,-1                   ## write generated output to file
  say '['time('l')'] Pre-Compile completed, 'mexpanded' macro calls expanded, total source lines 'outbuf.0
return 0
/* ------------------------------------------------------------------
 * Pass one load source file and determine preprocessor statements
 * ------------------------------------------------------------------
 */
RXPPPassOne: procedure = .int
  arg expose infile=.string, outfile=.string,maclib=.string

  macnum=readSource(maclib)
  call GetPreComp macnum                 ## analyse maclib, source lines not needed just register macros
  say '['time('l')'] Maclib loaded:      'source.0' records'
  say '['time('l')'] Macros extracted:   'macros_mname.0

  ## clear source array, DROP doesn't work as expected
  do i=1 to source.0
     source.i=""
     stype.i='R'
  end

  rexxLines=readSource(infile)
  say '['time('l')'] Rexx Source loaded: 'rexxLines' records'
  maclibm=macros_mname.0
  call GetPreComp rexxlines              ## analyse source, source lines needed keep them
  say '['time('l')'] Macros extracted:   'macros_mname.0-maclibm
return rexxlines
/* ------------------------------------------------------------------
 * Analyse REXX program and expand Macros
 * ------------------------------------------------------------------
 */
RXPPPassTwo: procedure
  arg out=.string,lino=.int
  do lineNo=1 to lino
     line = source.lineNo
 	 if stype.LineNo='D' then do
 	    ## call writeLine line
 	    call printGen line,1
 	 end
 	  else if stype.LineNo='I' then do
           ## call writeLine line
      	   call printGen line,2
     end
     else if fstrip(line) \='' then do
  	    newline = expandRecursive(line)
  	    call writeline newline
 	 end
  end
  call writeline ''
  say '['time('l')'] Pre-Compiled REXX generated '
return
/* ------------------------------------------------------------------
 * Read Rexx Source
 * ------------------------------------------------------------------
 */
ReadSource: procedure=.int
  arg file=.string
  i=readall(source,file,-1)
return i
/* ------------------------------------------------------------------
 * Read inline Macros
 * ------------------------------------------------------------------
 */
GetPrecomp: procedure
   arg lino=.int
   lineNo = 0
   do lineNo=1 to lino
       lineMin=max(LineNo-1,1)
       line = source.lineNo
       ucmd=upper(fword(line,1))
       if ucmd      = '##DEFINE'  then call cmd_define  lineNo,line
       else if ucmd = '##INCLUDE' then call cmd_include lineNo,line
       else if ucmd = '##????' then do    ## for any new pre compile statement
       end
    end
return
/* ------------------------------------------------------------------
 * Process ##DEFINE command
 * Parses a macro definition and stores it in the macro tables
 * ------------------------------------------------------------------
 */
 CMD_define: procedure
   arg lino=.int,line=.string

   name    = ''
   arglist = ''
   body    = ''

   def = subword(line, 2)
   wrd=fword(def,1)
   ppi = fpos('(', wrd,1)    ## must be in macro name, not in later macro body

   if ppi > 1 then do  /* Format: name(arg1, arg2) body */
      name    = fstrip(fsubstr(def, 1, ppi - 1))
      ppi2    = fpos(')', def,ppi+1)
      if ppi2 = 0 then do
         say 'Error: missing closing parenthesis in macro definition: ' def
         exit 8
      end
      if ppi2 - ppi - 1>0 then arglist = fstrip(fsubstr(def, ppi + 1, ppi2 - ppi - 1))
      else arglist=''
      body    = fstrip(fsubstr(def, ppi2 + 1,0))
   end
   else do /* Format: name body (no arguments) */
      name = fword(def, 1)
      body = subword(def, 2)
      arglist=''
   end

/* Remove braces and trim */
   body = fstrip(body)
   len=length(body)-2
   body=fsubstr(body,2,len)
   if body = '' then do
      say 'Error: empty macro body or missing {} in macro: ' name
      exit 8
   end

/* Register macro */
   i = macros_mname.0
   if macros_mname.i \= '' then i = i + 1

   macros_mname.i = upper(name)'('
   macros_margs.i = fstrip(arglist)
   macros_mbody.i = body
   stype.lino     = 'D'

return
/* ------------------------------------------------------------------
 * Print pre compiler statements and Macro Calls
 * ------------------------------------------------------------------
 */
printGen: procedure=.string
  arg line=.string, type=.int
  if printgen_global='NONE' then return ''   ## suppress all macro call definitions
  if type=1 then return '/* 'line' D*/'   ## DEFINE clause
  if type=2 then return '/* 'line' I*/'   ## INCLUDE clause
  if type=3 then return '/* 'line' S*/'   ## SET clause
  else return '/* rxpp: 'line' */'        ## Macro call
return '/* rxpp: 'line' U*/'
/* ------------------------------------------------------------------
 * Process ##SET command
 * ------------------------------------------------------------------
 */
CMD_set: procedure
  arg lino=.int,incl=.string
  stype.lino= 'S'
  varn='{'fword(incl,2)'}'
  i = macros_varname.0
  do j=1 to macros_varname.0
     if macros_varname.j=varn then do
        macros_varvalue.j=fstrip(subword(incl,3))
        if varn='{printgen}' then printgen_global=upper(macros_varvalue.j)   ## set printgen_global additionally directly will be used often
        return
     end
  end

  i = i + 1
  macros_varname.i =varn
  macros_varvalue.i=fstrip(subword(incl,3))
 return
/* ------------------------------------------------------------------
 * Process ##INCLUDE command
 * ------------------------------------------------------------------
 */
CMD_include: procedure
  arg lino=.int,incl=.string
  stype.lino= 'I'
  file=fword(incl,2)
  include.1=''
  new=readall(include,file,-1)
  rc=insert_array(source,lino,new)
  do j=1 to new
     lino=lino+1
     source.lino=include.j
  end
return
/* ------------------------------------------------------------------
 * Write a line to the output buffer, splitting on backslashes (\)
 * ------------------------------------------------------------------
 */
writeline: procedure
  arg oline=.string
  do while oline \= ''
     do while fpos('{',oline,1)>0 & fpos('}',oline,1)>0
        cmt=fsubstr(fstrip(oline),1,2)
        if cmt='/*' | cmt='##' then leave
        do i=1 to macros_varname.0
           oline=ChangeStr(macros_varname.i,oline,macros_varvalue.i)
        end
     end
     ppi = pos('\', oline)
     if ppi > 0 then do    /* Write part before the backslash */
        lino = lino + 1
        outbuf.lino = fsubstr(oline, 1, ppi - 1)
     /* Continue with the rest of the line */
        oline = fsubstr(oline, ppi + 1,0)
     end
     else do   /* Write the remaining content */
        lino = lino + 1
        outbuf.lino = oline
        leave
     end
  end
  return
/* ------------------------------------------------------------------
 * Expand recursively
 * ------------------------------------------------------------------
 */
expandRecursive:  procedure=.string
  arg line=.string
  old = ''
  do until old = line | line=''   /* No change → stop */
     old = line
     line = expandLine(line)  /* Expand one level */
  end
return line
/* ------------------------------------------------------------------
 * Expand single line
 * ------------------------------------------------------------------
 */
expandLine:  procedure=.string
  arg line=.string
  ucmd=upper(fword(line,1))
  if ucmd = '##SET' then do
     call cmd_set 9999,line
     return printGen(line,3)
  end
  ## if cmt='##' then return line
  ## if cmt='/*' then return line
  ## if fpos('(',line,1)=0 then return line   ## if there is no "(" there can't be a macro call
  i=hasMacro(line,macros_mname)    ## checks also for ## /*
  level=0
  do while i>0
     line=resolveMacro(i,line,level)
     level=level+1
     i=hasMacro(line,macros_mname)
  end
return line
/* ------------------------------------------------------------------
 * Try to resolve macro by available macro definition
 * ------------------------------------------------------------------
 */
 resolveMacro: procedure=.string
   arg i=.int, line=.string,level=.int
   uline   = upper(line)
   callPos = 0
   name    = macros_mname.i
   args    = macros_margs.i
   body    = macros_mbody.i
   do while fpos('{',body,1)>0 & fpos('}',body,1)>0
      do i=1 to macros_varname.0
         body=ChangeStr(macros_varname.i,body,macros_varvalue.i)
      end
   end
   mexpanded=mexpanded+1

   do while fpos(name, uline, callPos + 1) > 0
    ## test Macro is variadic
       isVariadic = 0
       if fpos('...', args,1) > 0 then do
          isVariadic = 1
          args = fstrip(changestr('...', '', args))
       end

       if level=0 & printgen_global='NNEST' then call writeline printGen(fstrip(line),0)
       else if printgen_global='ALL' then call writeline printGen(fstrip(line),0)

       callPos = fpos(name, uline, callPos + 1)
       remain  = fsubstr(line, callPos + length(name),0)    ## set to parameter part, macro has format name( +length positions into it
    ## extract argument list
       argtext = fetchArguments(remain)
       callargcount = parseArgList(argtext)

       bodyExp = body
       xargs   = translate(args, , ',')
       if isVariadic then return variadic(bodyExp, callargcount, argtext)
       else bodyExp = replaceFixArg(bodyExp, xargs)
       callLen = length(name) + 1 + length(argtext)  /* inkl. len(name()+1 for ) */
       line=insertatc(bodyexp,line,callpos,callLen)
       uline   = upper(line)         /* update uline for repetition */
       callPos = callPos - 1        /* set next start point */
   end
return line
/* ------------------------------------------------------------------
 * Replace fixed define arguments
 * ------------------------------------------------------------------
 */
replaceFixArg: Procedure=.string
  arg bodyexp=.string,xargs=.string
  wrds=words(xargs)
  do k=1 to wrds
      aname = fword(xargs, k)
      aval  = callargs.k
      bodyExp = replaceArg(bodyExp, aname, aval)
  end
return bodyExp
/* ------------------------------------------------------------------
 * Process Variadic macros
 * ------------------------------------------------------------------
 */
variadic: procedure=.string
  arg bodyExp=.string, callArgCount=.int, argText=.string

  /* Normalize argument text */
  argText  = translate(argText, , ',')
  varCount = callArgCount - 1

  /* Extract left-hand side (e.g., result = ...) */
  tempBody = translate(bodyExp, , '.=')
  lhs      = fword(tempBody, 1)
  ix       = fword(tempBody, 2)

  /* Extract the remaining expression */
  tempBody = translate(bodyExp, , '=')
  remain   = subword(tempBody, 2)

  /* Replace the primary macro parameters */
  bodyExp  = replaceArg(bodyExp, lhs, fword(argText, 1))      /* fixed first argument */
  bodyExp  = replaceArg(bodyExp, 'argcount', varCount)

  originalBody = bodyExp  /* store original for each iteration */

  /* Loop through variadic arguments */
  do k = 1 to varCount
     value   = fword(argText, k + 1)
     bodyExp = replaceArg(bodyExp, '$indx', k)
     bodyExp = replaceArg(bodyExp, 'arglist.'k, value)
     call writeline bodyExp

     bodyExp = originalBody  /* reset to base form for next iteration */
  end

  /* Final run with index = 0 (optional placeholder reset) */
  bodyExp = replaceArg(bodyExp, '$indx', 0)

  /* Extract final left-hand side for return */
  finalBody = translate(bodyExp, , '=')
  lhs       = fword(finalBody, 1)
  remain    = fword(finalBody, 2)

  if remain = '' then return ''  /* empty body check */
  return ''
  ## return lhs '=' varCount   ## set variable.0 not allowed in level b
/* ------------------------------------------------------------------
 * Debug routine
 * ------------------------------------------------------------------
 */
debugArgs:  procedure
  arg name=.string,args=.string,callargcount=.int
  say '--- DEBUG ---'
  say 'Macro Name: ' name
  say 'Macro Args: ' args
  say 'Argtext:     ' argtext
  do k = 1 to callargcount
     say 'callargs.'k' = 'callargs.k
  end
  say 'Before Replace: ' body
return
/* ------------------------------------------------------------------
 * Extract full argument list from macro call (handles nested parentheses)
 * ------------------------------------------------------------------
 */
fetchArguments: procedure=.string
  arg remain=.string

  argText = ''
  depth   = 1  /* Initial open parenthesis already passed before this call */

  do j = 1 to length(remain)
     c = fsubstr(remain, j, 1)

     if c = '(' then depth = depth + 1
     if c = ')' then do
        depth = depth - 1
        if depth = 0 then leave  /* finished parsing full argument expression */
     end

     argText = argText || c
  end
  return fstrip(argText)
/* ------------------------------------------------------------------
 * Replace Argument in Macro Call
 * ------------------------------------------------------------------
 */
replaceArg: procedure=.string
  arg str=.string, name=.string, value=.string

  posn = 1
  p = fpos(name, str, 1)
  do while p>0
     before = ''
     if p > 1 then before = fsubstr(str, p - 1, 1)
     after  = fsubstr(str, p + length(name), 1)
    /* check before and after chars to change only full variable names */
	verbb=verify(before, alphaN, 'N')
    verba=verify(after, alphaN, 'N')
    if length(before)=0 then verbb=1
	if length(after) =0 then verba=1
    if verbb=0 | verba=0 then do
	   posn = p + 1
       iterate        /* no, not a valid variable name */
     end
    str=insertatc(value,str,p,length(name))    ## use the C-function
  ##    str=insertat(value,str,p,length(name))
 ##    if cstr \=str then say "***** '"cstr"' '"str"'"
    posn = p + length(value)
    p = fpos(name, str, posn)
  end
return str
/* ------------------------------------------------------------------
 * Insert/replace string in string
 * ------------------------------------------------------------------
 */
InsertAt: Procedure=.string
arg needle=.string,haystack=.string,at=.int,len=.int
  if at+len>length(haystack) then rhs=''
  else rhs=fsubstr(haystack, at+len,0)
  if at>1 then lhs=fsubstr(haystack, 1, at-1)
  else lhs=''
return lhs || needle || rhs
/* ------------------------------------------------------------------
 * Determine argument list
 * ------------------------------------------------------------------
 */
parseArgList: procedure=.int
  arg argstr=.string
  callargs.1 = ''
  i = 1
  depth = 0
  token = ''
  do j = 1 to length(argstr)
    c = fsubstr(argstr, j, 1)
    if c = ',' & depth = 0 then do
      callargs.i = fstrip(token)
      token = ''
      i = i + 1
    end
    else do
      if c = '(' then depth = depth + 1
      if c = ')' then depth = depth - 1
      token = token || c
    end
  end
  if fstrip(token) <> '' then callargs.i = fstrip(token)
return i
/* ------------------------------------------------------------------
 * Init RXPP environment
 * ------------------------------------------------------------------
 */
rxppinit: procedure
  arg rexxname=.string
  alphaN='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_'
  source.1=''
  stype.1='R'
  macros_mname.1=''
  lino=0
  outbuf.1 = ""
  mexpanded=0
  rexxname=translate(rexxname,,'/\')
  wrds=words(rexxname)
  rexxname=word(rexxname,wrds)
  macros_varname.1 ='{mainrexx}'
  macros_varvalue.1=rexxname
  macros_varname.2 ='{printgen}'
  macros_varvalue.2=0
  printgen_global='NNEST'
  macros_varname.3 ='{RXXP_DATE}'
  macros_varvalue.3=date()
return
