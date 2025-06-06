/*
 * crexx RXPP
 * CREXX Pre Compiler
 * Author Peter Jacob
 * Created May 2025
 * Version 31. May 2025
 */
options levelb
import precomp
namespace rxpp expose source stype callargs macros_mname macros_margs macros_mbody macros_varname macros_varvalue cflags printgen_flags outbuf lino rexxlines included_files alphaN mExpanded expandLevel ifblock
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
     if upper(command.i)     ='-I' then infile=command.j
     else if upper(command.i)='-O' then outfile=command.j
     else if upper(command.i)='-M' then maclib=command.j
  end

/*
infile  = 'C:/Users/PeterJ/CLionProjects/CREXX/250606/lib/plugins/precomp/Macro1.rxpp'
outfile = 'C:/Users/PeterJ/CLionProjects/CREXX/250606/lib/plugins/precomp/Macro1.rexx'
maclib  = 'C:/Users/PeterJ/CLionProjects/CREXX/250606/lib/plugins/precomp/Maclib.rexx'
*/

  if fstrip(infile)='' then do
       say 'Error: no source file specified'
       exit 8
  end

    say 'Input File:  ' infile
    say 'Output File: ' outfile
    say 'Macro Lib:   ' maclib

  call rxppinit infile                              ## init global and environment variables
  say '['time('l')'] Pre-Compile pass one'
  sourceLines=RXPPPassOne(infile,outfile,maclib)    ## load source file and macro library
  ## !!! to early is as picked up later-> is in pass 2 now !! if pos(' 1buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 1'
  call RXPPPassTwo                                  ## pass 2 to pre-expand certain elements (##ELSE)
     if pos(' 2buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 2'
   say '['time('l')'] Pre-Compile pass two'
  call RXPPPassThree outfile                        ## analyse source and expand macros
  say '['time('l')'] Pre-Compiled REXX saved'
     if pos(' 3buf',cflags)>0 then call list_array outbuf, -1,-1,'Source Buffer after Pass 3'
  call writeall outbuf,outfile,-1                   ## write generated output to file
  say '['time('l')'] Pre-Compile completed, 'mexpanded' macro calls expanded, total source lines 'outbuf.0
  if pos(' vars',cflags)>0 then call printvars
  if pos(' maclist',cflags)>0 then call printmacs
   if pos(' includes',cflags)>0 then call list_array included_files,-1,-1,'Include Files'
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
  if rexxLines<0 then do
     say 'Error: source file missing: ' infile
     exit 8
  end

  say '['time('l')'] Rexx Source loaded: 'rexxLines' records'
  maclibm=macros_mname.0
  call GetPreComp rexxlines              ## analyse source, source lines needed keep them
  say '['time('l')'] Macros extracted:   'macros_mname.0-maclibm
  call sort_bylen macros_mname,macros_margs,macros_mbody ## sort macro names by length do avoid unintented substitution (e.g. quote dquote)
  say '['time('l')'] Macros arranged:    'macros_mname.0
  call sort_bylen macros_mname,macros_margs,macros_mbody ## sort macro names by length do avoid unintented substitution (e.g. quote dquote)

return rexxlines
/* ------------------------------------------------------------------
 * Pass 2 pre-expand certain elements
 * ------------------------------------------------------------------
 */
RXPPPassTwo: procedure
  stack.1 = ''         /* simulate stack using indexed stem */
  depth = 0            /* current depth */
  ifblock.1=0  ifblock.1=0
  which=0
  i=fsearch(source,1,'##IF ','##IFN ',"##CFLAG",which)   ## search as first word in the source string
  do while i>0
     if which<3 then condition=word(source.i,2)
      else flagsset=early_flag_pick_up(i) /* <<<!!!!!!!!!!!! ugly construct to pick up potential compiler flags immediately after pass 1 */
     i=fsearch(source,i+1,'##IF ','##IFN ','##ELSE',which)
     if which \=3 then iterate
     call insert_array source,i,1
     call insert_array stype,i,1
     source.i='##endif'
     stype.i='R'
     j=i+1
     source.j='##ifn 'condition
     i=fsearch(source,j+1,'##IF ','##IFN ',"",which)   ## search as first word in the source string
  end

  which=0
  i=fsearch(source,1,'##IF ','##IFN ',"",which)   ## search as first word in the source string
  do while i>0
     if which=1 then stype.i='IF'
     else if which=2 then stype.i='IFN'
     lnk=findMatchingEndif(i+1)
     ifblock.i=lnk
     i=fsearch(source,i+1,'##IF ','##IFN ','',which)
  end
  if pos(' iflink',cflags)=0 then return
  if ifblock.0=0 then say "+++ no Pre Source Expansion occurred"
  else do i=1 to ifblock.0
     if ifblock.i=0 then iterate
     lnk=ifblock.i
     say right(i,4,'0')' 'left(fstrip(source.i),16)' --> linked to --> 'right(lnk,4,'0')' 'source.lnk'   <+++ following lines skipped based on condition +++>'
     do j=i+1 to lnk
        say '? skipped: 'right(j,4,'0')' 'fstrip(source.j)
     end
  end
return
/* ------------------------------------------------------------------
 * Pick up the compiler flg setup in an early stage
 * ------------------------------------------------------------------
 */
early_flag_pick_up: procedure=.int
  arg i=.int
  source.i=lower(source.i)
  cflags=' '||subword(source.i,2)   ## add blank to later search for ' flags', this avoids unwanted hit of e.g. 1buf in n1buf
  call setvar 'cflags',cflags
  source.i='/* 'source.i' */'
  if pos(' 1buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 1'
  if pos(' iflink',cflags)>0 then do
     say "Pre Source Expansion Pass 1"
     say "-------------------------------------------------------"
  end
return 1
/* ------------------------------------------------------------------
 * for ##IF / ##IFN find associated ##ENDIF
 * ------------------------------------------------------------------
 */
findMatchingEndif: procedure=.int
  arg startline=.int
  nest = 1  /* Start at 1 because the current ##IF is level 1 */
  which = 0
  i = fsearch(source, startline, '##IF ','##IFN ','##END', which)
  do while i > 0
    if which = 1 | which=2 then nest = nest + 1
    else if which = 3 then do
      nest = nest - 1
      stype.i = 'X'  /* mark for removal or annotation */
      if nest = 0 then return i  /* finally matched ##ENDIF */
    end
    i = fsearch(source, i + 1, '##IF ', '##IFN ','##END', which)
  end

  say '**ERROR: No matching ##ENDIF found after line' startline
return 0
/* ------------------------------------------------------------------
 * Analyse REXX program and expand Macros
 * ------------------------------------------------------------------
 */
RXPPPassThree: procedure
  arg out=.string
  do lineNo=1 to source.0
     line = source.lineNo
  	 if stype.LineNo='D' then do
 	    if pos(' ndef',cflags)>0 then iterate
 	    call writeLine printGen(line,1)
  	 end
     else if stype.LineNo='I' then do
       if pos(' nset',cflags)>0 then iterate
   	   call writeLine printGen(line,2)
     end
     else if stype.LineNo='IF' then do
       ##  call writeLine printGen(line,4)
       if findvar(fword(line,2))=0 & ifblock.lineno>0 then do
           lineno=ifblock.lineno
        end
     end
     else if stype.LineNo='IFN' then do
       ##  call writeLine printGen(line,4)
        if findvar(fword(line,2))>0 & ifblock.lineno>0 then do
           lineno=ifblock.lineno
        end
     end
     else if stype.LineNo='X' then iterate    ## suppress any ##ELSE ##ENDIF
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
   do lineNo=1 to source.0
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
  if printgen_flags=' none' then return ''   ## suppress all macro call definitions
  if type=1 then return '/* 'line' D*/'   ## DEFINE clause
  if type=2 then return '/* 'line' I*/'   ## INCLUDE clause
  if type=3 then return '/* 'line' S*/'   ## SET clause
  if type=4 then return '/* 'line' IF*/'  ## IF clause
  else return '/* rxpp: 'line' */'        ## Macro call
return '/* rxpp: 'line' U*/'
/* ------------------------------------------------------------------
 * Process ##SET command
 * ------------------------------------------------------------------
 */
CMD_set: procedure
  arg lino=.int,incl=.string
  stype.lino= 'S'
  varn=fword(incl,2)
  vind=setvar(varn,DropComment(subword(incl,3)))
  if varn='cflags' then cflags=fword(lower(macros_varvalue.vind),1) ## set cflags additionally directly will be used often
  return
/* ------------------------------------------------------------------
 * Drop comments at the end of a line
 * ------------------------------------------------------------------
 */
dropComment: procedure=.string
  arg varvalue=.string
  fp1=fpos('##',varvalue,1)
  if fp1>1 then varvalue=fsubstr(varvalue,1,fp1-1)
  fp1=fpos('/*',varvalue,1)
  if fp1>1 then varvalue=fsubstr(varvalue,1,fp1-1)
return fstrip(varvalue)
/* ------------------------------------------------------------------
 * Process ##UNSET command
 * ------------------------------------------------------------------
 */
CMD_unset: procedure
  arg lino=.int,incl=.string
  stype.lino= 'S'
  varn=fword(incl,2)
  varn=lower(varn)
  if varn='printgen'  then return
  if varn='rxpp_rexx' then return
  if varn='rxpp_date' then return
  i=setvar(varn,'')  ## reset flag
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
  if new<0 then do
     say 'Error: missing include file: 'file
     exit 8
  end
  if search_array(included_files,file,1,1)>0 then return
  imax=included_files.0
  if included_files.imax\='' then imax=imax+1
  included_files.imax=file
  rc=insert_array(source,lino+1,new)
  rc=insert_array(stype,lino+1,new)
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
     oline=injectVariable(oline)
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
  expandlevel=0
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
  if ucmd = '##SET' | ucmd = '##UNSET' then do
     call cmd_set 9999,line
     if pos(' nset',cflags)>0 then return ''
     return printGen(line,3)
  end
  ## if cmt='##' then return line
  ## if cmt='/*' then return line
  ## if fpos('(',line,1)=0 then return line   ## if there is no "(" there can't be a macro call
  i=hasMacro(line,macros_mname,1)      ## checks also for ## /*
   do while i>0
     line=resolveMacro(i,line,expandLevel)
     expandlevel=expandlevel+1
     i=hasMacro(line,macros_mname,i+1)
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
   body=injectVariable(body)    ## inject pre-compiler variables, if there are any
   mexpanded=mexpanded+1
   do while fpos(name, uline, callPos + 1) > 0
    ## test Macro is variadic
       isVariadic = 0
       if fpos('...', args,1) > 0 then do
          isVariadic = 1
          args = fstrip(changestr('...', '', args))
       end
       if printgen_flags='all' then call writeline printGen(fstrip(line),0)
       else if level=0 & printgen_flags='nnest' then call writeline printGen(fstrip(line),0)
       level=level+1     ## increase level in case a second instance of macro is found
       callPos = fpos(name, uline, callPos + 1)
       if callpos>1 then do
          status_before=verify(substr(uline,callpos-1,1), alphaN, 'N')
          if status_before=0 then iterate
       end
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
 * Inject pre-compiler variable into line
 * ------------------------------------------------------------------
 */
injectVariable: procedure=.string
  arg line2change=.string
   fp1=fpos('{',line2change,1)>0
   fp2=fpos('}',line2change,fp1+1)
   do while fp1>0 & fp2>0
      cmt=fsubstr(fstrip(line2change),1,2)
       if cmt='/*' | cmt='##' then leave
      do i=1 to macros_varname.0
         if macros_varname.i='' then iterate
         Line2change=ChangeStr(macros_varname.i,line2change,macros_varvalue.i)
      end
      fp1=fpos('{',line2change,fp2+1)>0
      fp2=fpos('}',line2change,fp2+1)
   end
return line2change
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
 * Dump pre-compiler variables
 * ------------------------------------------------------------------
 */
printvars: procedure
  say 'Pre-Compiler Variable List'
  say '-------------------------------------------------------'
  do i=1 to macros_varname.0
     if macros_varname.i='' then iterate
     say right(i,3,'0')' 'macros_varname.i"='"macros_varvalue.i"'"
  end
  say macros_varname.0' variables defined'
return
/* ------------------------------------------------------------------
 * Dump macros
 * ------------------------------------------------------------------
 */
printmacs: procedure
  say 'Macro List'
  say '-------------------------------------------------------'
  do i=1 to macros_mname.0
     say left(macros_mname.i||macros_margs.i')',35) '{'macros_mbody.i'}'
  end
  say macros_mname.0' macros defined'
return
/* ------------------------------------------------------------------
 * Return pre-compiler variable index
 * ------------------------------------------------------------------
 */
getvarindx: procedure=.int
  arg varname=.string
  do i=1 to macros_varname.0
     if macros_varname.i='' then iterate
     if macros_varname.i=varname then return i
  end
  return 0
/* ------------------------------------------------------------------
 * Return pre-compiler variable content
 * ------------------------------------------------------------------
 */
findvar: procedure=.string
  arg varname=.string
  varname='{'lower(varname)'}'
return getvarindx(varname)
/* ------------------------------------------------------------------
 * Return pre-compiler variable content
 * ------------------------------------------------------------------
 */
getvar: procedure=.string
  arg varname=.string
  varname='{'lower(varname)'}'
  i=getvarindx(varname)
  if i>0 then return macros_varvalue.i
return ''
/* ------------------------------------------------------------------
 * Set pre-compiler variable
 * ------------------------------------------------------------------
 */
setvar: procedure=.int
  arg varname=.string,varvalue=.string
  varname='{'lower(varname)'}'
  i=getvarindx(varname)
  if i=0 then i=macros_varname.0+1
  macros_varname.i=varname
  macros_varvalue.i=varvalue
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
  macros_varname.1=''
  macros_varvalue.1=''
  included_files.1=''
  lino=0
  outbuf.1 = ""
  mexpanded=0
  rexxname=translate(rexxname,,'/\')
  wrds=words(rexxname)
  rexxname=word(rexxname,wrds)
  call setvar 'rxpp_rexx',rexxname
  cflags=' ndef nset svars siflink n1buf n2buf n3buf nvars'
    call setvar 'cflags',cflags
  printgen_flags='all'
    call setvar 'printgen',printgen_flags
  call setvar 'rxpp_date',date()' 'time()
 return
