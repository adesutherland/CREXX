/*
 * crexx RXPP
 * CREXX Pre Compiler
 * Author Peter Jacob
 * Created May 2025
 * Version 31. May 2025
 */
options levelb

import precomp
namespace rxpp expose source stype callargs macros_mname macros_margs macros_mbody macros_varname macros_varvalue cflags printgen_flags outbuf lino rexxlines included_files syspath alphaN mExpanded expandLevel ifblock elapsedTime verbose
import rxfnsb

/* ------------------------------------------------------------------
 * CREXX Pre Compiler
 *   Currently supported Commands
 *   ##DEFINE
 *   ##INCLUDE  (nested)
 * ------------------------------------------------------------------
 */
arg command=.string[]
internal_testing=0    ## activate only for rxpp internal tests
verbose=0

  ElapsedTime=time('us')
  if verbose then say 'CRX0010I ['time('l')'] Pre-Compile started'
  do i=1 to command.0
     j=i+1
     command.i=upper(command.i)
     if command.i     ='-I' then infile=command.j
     else if command.i='-IN' then infile=command.j
     else if command.i='-O' then outfile=command.j
     else if command.i='-OUT' then outfile=command.j
     else if command.i='-M' then maclib=command.j
  end

if internal_testing=1 then do
   infile  = 'C:/Users/PeterJ/CLionProjects/CREXX250701/lib/plugins/system/treemap_test.rxpp'
   outfile = 'C:/Users/PeterJ/CLionProjects/CREXX250701/lib/plugins/system/treemap_test.rexx'
   maclib  = 'C:/Users/PeterJ/CLionProjects/CREXX250701/lib/plugins/precomp/Maclib.rexx'
end

  if strip(infile)='' then do
     say 'CRX0100E+['time('l')'] no source file specified'
     exit 8
  end

  if verbose then say 'Input File:  ' infile
  if verbose then say 'Output File: ' outfile
  if verbose then say 'Macro Lib:   ' maclib

  syspath=rxppinit(infile,maclib)                   ## init global and environment variables
  if verbose then say 'CRX0100I ['time('l')'] Pre-Compile pass one'
  sourceLines=RXPPPassOne(infile,outfile,maclib)    ## load source file and macro library
  ## !!! to early is as picked up later-> is in pass 2 now !! if pos(' 1buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 1'
  call RXPPPassTwo                                  ## pass 2 to pre-expand certain elements (##ELSE)
     if pos(' 2buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 2'
  if verbose then say 'CRX0200I ['time('l')'] Pre-Compile pass two'
  call RXPPPassThree outfile                        ## analyse source and expand macros
  if verbose then say 'CRX0310I ['time('l')'] Pre-Compiled REXX saved'
     if pos(' 3buf',cflags)>0 then call list_array outbuf, -1,-1,'Source Buffer after Pass 3'
  call writeall outbuf,outfile,-1                   ## write generated output to file
  if verbose then say 'CRX0500I ['time('l')'] Pre-Compile completed, 'mexpanded' macro calls expanded, total source lines 'outbuf.0
  if pos(' vars',cflags)>0 then call printvars
  if pos(' maclist',cflags)>0 then call printmacs
  if pos(' includes',cflags)>0 then call list_array included_files,-1,-1,'Include Files'
  elapsedTime=(time('us')-elapsedtime)
  if verbose then say 'CRX0101I ['time('l')'] Elapsed Time 'elapsedTime/1000000' seconds, 'elapsedTime/1000' milliseconds'
return 0
/* ------------------------------------------------------------------
 * Pass one load source file and determine preprocessor statements
 * ------------------------------------------------------------------
 */
RXPPPassOne: procedure = .int
  arg expose infile=.string, outfile=.string,maclib=.string

  macnum=readSource(maclib)
  if macnum<0 then do
     if verbose then say 'CRX0900E+['time('l')'] Maclib not found, or not accessible: 'maclib
  end
  else do
     call GetPreComp macnum                 ## analyse maclib, source lines not needed just register macros
     if verbose then say 'CRX0110I ['time('l')'] Maclib loaded:      'source.0' records'
     if verbose then say 'CRX0120I ['time('l')'] Macros extracted:   'macros_mname.0
  end
  ## clear source array, DROP doesn't work as expected
  do i=1 to source.0
     source.i=""
     stype.i='R'
  end

  rexxLines=readSource(infile)
  if rexxLines<0 then do
     say 'CRX0910E+['time('l')'] source file missing: 'infile
     exit 8
  end

  if verbose then say 'CRX0130I ['time('l')'] Rexx Source loaded: 'rexxLines' records'
  maclibm=macros_mname.0
  call GetPreComp rexxlines              ## analyse source, source lines needed keep them
  if verbose then say 'CRX0140I ['time('l')'] Macros extracted:   'macros_mname.0-maclibm
  call sort_bylen macros_mname,macros_margs,macros_mbody ## sort macro names by length do avoid unintented substitution (e.g. quote dquote)
  if verbose then say 'CRX0150I ['time('l')'] Macros arranged:    'macros_mname.0
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
  i=ffind(source,1,'OOCREATE(')   ## search as first word in the source string
  do while i>0
     call oocreatedefs i
     i=ffind(source,i+1,'OOCREATE(')   ## search as next word in the source string
  end
  if pos(' iflink',cflags)=0 then return
  if ifblock.0=0 then say "+++ no Pre Source Expansion occurred"
  else do i=1 to ifblock.0
     if ifblock.i=0 then iterate
     lnk=ifblock.i
     if verbose then say right(i,4,'0')' 'left(strip(source.i),16)' --> linked to --> 'right(lnk,4,'0')' 'source.lnk'   <+++ following lines skipped based on condition +++>'
     do j=i+1 to lnk
        if verbose then say '? skipped: 'right(j,4,'0')' 'strip(source.j)
     end
  end
return
/* ------------------------------------------------------------------
 * Create the OOCreate definition
 * ------------------------------------------------------------------
 */
oocreatedefs: procedure
  arg lino=.int
  ipi=pos('=',source.lino)
  if ipi<=1 then return           ## no valid pointer=OOCREATE() statement
  lhs=strip(substr(source.lino,1,ipi-1))
  rhs=strip(substr(source.lino,ipi+1))
  parenPos = POS("(", rhs)        ## Find the opening parenthesis
  funcName = STRIP(SUBSTR(rhs, 1, parenPos-1))
  ## Extract parameter list (between parentheses)
  paramStr = STRIP(SUBSTR(rhs,parenPos + 1))
  paramStr = LEFT(paramStr, LENGTH(paramStr)- 1)    ## Remove closing ')'
/* drop first parameter, it's the prefix for the function calls */
  ppi=pos(',',paramStr)
  if ppi=0 then do
     prefix=paramstr
     paramstr=''
  end
  else do
     prefix=substr(paramstr,1,ppi-1)
     paramstr=substr(paramstr,ppi+1)
  end
  call setvar lhs'_prefix',prefix                 ## save the object handle: e.g. map=OOCREATE(tm) it will be map_prefix=tm
  source.lino=lhs'='prefix'CREATE('paramstr')'    ## for now the call function must be CREATE with the provided prefix
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
     if verbose then say "Pre Source Expansion Pass 1"
     if verbose then say "-------------------------------------------------------"
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
  if verbose then say 'CRX0920E+['time('l')'] No matching ##ENDIF found after line 'startline
return 0
/* ------------------------------------------------------------------
 * Analyse REXX program and expand Macros
 * ------------------------------------------------------------------
 */
RXPPPassThree: procedure
  arg out=.string
  lineNo=0
  do while lineNo<source.0
     lineNo=Lineno+1
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
       if findvar(word(line,2))=0 & ifblock.lineno>0 then do
           lineno=ifblock.lineno
        end
     end
     else if stype.LineNo='IFN' then do
       ##  call writeLine printGen(line,4)
        if findvar(word(line,2))>0 & ifblock.lineno>0 then do
           lineno=ifblock.lineno
        end
     end
  ##   else if stype.LineNo='PARSE' then call CMD_parse lineno,line ## doesn't make sense!
     else if stype.LineNo='X' then iterate    ## suppress any ##ELSE ##ENDIF
     else if strip(line) \='' then do
  	    newline = expandRecursive(line)
  	    call writeline newline
 	 end
  end

  call writeline ''
  if verbose then say 'CRX0300I ['time('l')'] Pre-Compiled REXX generated '
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
   do while lineNo < source.0      ## array might grow, so while is more reliable
      LineNo=LineNo+1
      lineMin=max(LineNo-1,1)
      line = source.lineNo
      ucmd=upper(word(line,1))
      if substr(ucmd,1,2)\='##' then iterate
      if ucmd      = '##DEFINE'  then lineno=cmd_define(lineNo,line)
      else if ucmd = '##INCLUDE' then call cmd_include lineNo,line,1
      else if ucmd = '##USE'     then call cmd_include lineNo,line,2
      else if ucmd = '##DATA'    then call cmd_data lineNo,line,word(line,2)
      else if ucmd = '##INPUT'   then call cmd_data lineNo,line,"input"
      else if substr(ucmd,1,5) = '##SYS'   then call cmd_data lineNo,line, substr(ucmd,3)
##    else if ucmd = '##PARSE' then stype.LineNo='PARSE'     ## is not worth it
##       else if ucmd = '##????' then do    ## for any new pre compile statement
##       end
   end
return
/* ------------------------------------------------------------------
 * Process ##DEFINE command
 * Parses a macro definition and stores it in the macro tables
 * ------------------------------------------------------------------
 */
 CMD_define: procedure=.int
   arg lino=.int,line=.string
   nlino=lino
   name    = ''
   arglist = ''
   body    = ''

   def = subword(line, 2)
   wrd=word(def,1)
   ppi = pos('(', wrd,1)    ## must be in macro name, not in later macro body
   if ppi > 1 then do  /* Format: name(arg1, arg2) body */
      name    = strip(substr(def, 1, ppi - 1))
      ppi2    = pos(')', def,ppi+1)
      if ppi2 = 0 then do
         say 'CRX0930E+['time('l')'] missing closing parenthesis in macro definition: ' def
         exit 8
      end
      if ppi2 - ppi - 1>0 then arglist = strip(substr(def, ppi + 1, ppi2 - ppi - 1))
      else arglist=''
      body    = strip(substr(def, ppi2 + 1))
   end
   else do /* Format: name body (no arguments) */
      name = word(def, 1)
      body = subword(def, 2)
      arglist=''
   end
/* Remove braces and trim */
   body = strip(body)
   stype.nlino='D'
   do forever
      if body='' then leave
      nlen=length(body)
      if substr(body,nlen,1) \= '\' then leave
      nlino=nlino+1
      if nlino>source.0 then do
         say 'CRX0950E+['time('l')'] macro definition is not closed: 'name
         exit 8
      end
      nlen=nlen-1
      if nlen>1 then body=strip(substr(body,1,nlen-1))' ; 'strip(source.nlino)
      else body=strip(source.nlino)
      stype.nlino='D'
   end
   opr=pos('{',body)
   lopr=lastpos('}',body)
   if lopr>0 then body=substr(body,opr+1,lopr-opr-1)
   if strip(body) = '' | lopr=0 then do
      say 'CRX0940E+['time('l')'] empty macro body or missing {} in macro: ' name
      exit 8
   end

/* Register macro */
   i = macros_mname.0
   if macros_mname.i \= '' then i = i + 1

   macros_mname.i = upper(name)'('
   macros_margs.i = strip(arglist)
   macros_mbody.i = body

return nlino
/* ------------------------------------------------------------------
 * Count braces to recognise multi line macros
 * ------------------------------------------------------------------
 */
countBraces: procedure=.int
  arg str=.string
  leftBraces = 0
  rightBraces = 0
  inQuote = ''
  len=length(str)
  i = 1
  do while i <= len
    ch = substr(str, i, 1)
    if inQuote = '' then do
       if ch = '"' | ch = "'" then inQuote = ch
       else if ch = '{' then leftBraces = leftBraces + 1
       else if ch = '}' then rightBraces = rightBraces + 1
    end
    else do
       if ch = inQuote then inQuote = ''  /* close quote */
       else if ch = '\' then do  /* skip escaped quote if next char matches */
          if i+1 <= len & substr(str, i+1, 1) = inQuote then i = i + 1
       end
    end
    i = i + 1
  end
  return leftBraces - rightBraces
/* ------------------------------------------------------------------
 * Print pre compiler statements and Macro Calls
 * ------------------------------------------------------------------
 */
printGen: procedure=.string
  arg line=.string, type=.int
  if printgen_flags='none' then return '' ## suppress all macro call definitions
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
  varn=word(incl,2)
  vind=setvar(varn,DropComment(subword(incl,3)))
  if varn='cflags' then cflags=word(lower(macros_varvalue.vind),1) ## set cflags additionally directly will be used often
return
/* ----------------------------------------------------------------------
 *  Print created tokens
 * ----------------------------------------------------------------------
 */
token_print: procedure
  arg template=.string,token=.string[],token_type=.string[]
  if verbose then say 'Template='template
  if verbose then say time('l')' Compile completed'
  if verbose then say 'Created Tokens, type=1: variable, 2=quoted-string, 3=column-set, 4=column-reposition'
  do j = 1 to token.0
     if verbose then say "Token" j ": '"token.j"' Type:" token_type.j
  end
  if verbose then say time('l')' Parse String using compiled Template'
  return
/* ------------------------------------------------------------------
 * Drop comments at the end of a line
 * ------------------------------------------------------------------
 */
dropComment: procedure=.string
  arg varvalue=.string
  fp1=pos('##',varvalue,1)
  if fp1>1 then varvalue=substr(varvalue,1,fp1-1)
  fp1=pos('/*',varvalue,1)
  if fp1>1 then varvalue=substr(varvalue,1,fp1-1)
return strip(varvalue)
/* ------------------------------------------------------------------
 * Process ##UNSET command
 * ------------------------------------------------------------------
 */
CMD_unset: procedure
  arg lino=.int,incl=.string
  stype.lino= 'S'
  varn=word(incl,2)
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
  arg lino=.int,incl=.string,mode=.int
  stype.lino= 'I'
  file=word(incl,2)
  file=normalisepath(syspath'/'file)
  include.1=''
  new=readall(include,file,-1)
  if new<0 then do
      say 'CRX0950E+['time('l')'] missing include file: 'file
     exit 8
  end
  if search_array(included_files,file,1,1)>0 then return
  imax=included_files.0
  if included_files.imax\='' then imax=imax+1
  included_files.imax=file

  if mode=1 then do
     rc=insert_array(source,lino+1,new)
     rc=insert_array(stype,lino+1,new)
     insertat=lino
  end
  else do
     linc=source.0
     rc=insert_array(source,linc+1,new)
     rc=insert_array(stype,linc+1,new)
     insertat=linc
  end
  do j=1 to new
     insertat=insertat+1
     source.insertat=include.j
  end
return
/* ------------------------------------------------------------------
 * Process ##DATA command
 * ------------------------------------------------------------------
 */
CMD_data: procedure
  arg lino=.int,line=.string,array=.string
  stype.lino= 'D'
  j=0
  do i=lino+1 to source.0
     line=source.i
     if upper(word(line,1))='##END' then leave
     j=j+1
     source.i=array'.'j'='safe_quote(line)
  end
  stype.i='D'   ## set D for ##END
return
/* ------------------------------------------------------------------
 * Write a line to the output buffer, splitting on backslashes (\)
 * ------------------------------------------------------------------
 */
writeline: procedure
  arg oline=.string
   oline=injectVariable(oline)
  ## if pos('~',oline) then oline=oo_translate(oline)
   if pos('.',oline) then oline=ooTranslate(oline)
/* not necessary - I hope
  do while oline \= ''
     oline=injectVariable(oline)
     ppi = pos('\', oline)
     if ppi > 0 then do    /* Write part before the backslash */
        lino = lino + 1
        outbuf.lino = substr(oline, 1, ppi - 1)
     /* Continue with the rest of the line */
        oline = substr(oline, ppi + 1)
     end
     else do   /* Write the remaining content */
        lino = lino + 1
        outbuf.lino = oline
        leave
     end
  end
  */
## just insert line to buffer
   if lino=1 then do
      lino = lino + 1
      outbuf.lino='/* 'copies('-',70)
      lino = lino + 1
      outbuf.lino = ' * PRE Compiled on 'date()' at 'time()
      lino = lino + 1
      outbuf.lino = ' * 'copies('-',70)
      lino = lino + 1
      outbuf.lino=' */'
   end

   lino = lino + 1
   outbuf.lino = oline
  return
/* ------------------------------------------------------------------
 * Translate OO Calls
 * ------------------------------------------------------------------
 */
oo_translate_tilde: procedure=.string
arg line=.string
line = strip(line)
result = ''
posStart = 1

do forever
    tildePos = pos('~', line, posStart)   ## Find object (left of ~)
    if tildePos=0 then leave

    i = tildePos - 1
    do while i > 0
       char = substr(line, i, 1)
       if verify(char, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') \= 0 then leave
       i = i - 1
    end
    objectStart = i + 1
    object = strip(substr(line, objectStart, tildePos - objectStart))

    ## Find method and args
    parenOpen = pos('(', line, tildePos)
    if parenOpen = 0 then leave  ## malformed, skip

    ## Find matching closing parent
    parenCount = 1
    j = parenOpen + 1
    do while j <= length(line) & parenCount > 0
        ch = substr(line, j, 1)
        if ch = '(' then parenCount = parenCount + 1
        if ch = ')' then parenCount = parenCount - 1
        j = j + 1
    end
    parenClose = j - 1

    method = strip(substr(line, tildePos + 1, parenOpen - tildePos - 1))
    args   = strip(substr(line, parenOpen + 1, parenClose - parenOpen - 1))

    ## Build transformed call
    upperMethod = translate(method)
    upperObject = translate(object)
 ##   callText = upperMethod || '_' || upperObject || '(' || object
    callText = upperMethod'(' || object
    if args \= '' then callText = callText || ',' || args
    callText = callText || ')'

    ## Append everything before the object~method and the transformed call
    result = result || substr(line, posStart, objectStart - posStart) || callText

    ## Move cursor past current match
    posStart = parenClose + 1
end

## Append remainder of line
if posStart <= length(line) then
    result = result || substr(line, posStart)

if verbose then say "Original:   " line
if verbose then say "Transformed:" result

return result

ooTranslate: procedure=.string
arg line=.string
line = strip(line)
result = ''
posStart = 1
if verbose then say 567 line
do while pos('.', line, posStart) > 0
    dotPos = pos('.', line, posStart)

    ##Detect method name after the dot
    methodStart = dotPos + 1
    methodEnd = methodStart

    do while methodEnd <= length(line) & verify(substr(line, methodEnd, 1), 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_') = 0
        methodEnd = methodEnd + 1
    end

    ##If not followed by a '(', this is a stem access
    if substr(line, methodEnd, 1) \= '(' then do
        stemAccess = substr(line, posStart, methodEnd - posStart)
        ##Preserve the stem text
        result = result || substr(line, posStart, methodEnd - posStart)
        posStart = methodEnd
        iterate
    end

    ##Backtrack to extract the object name
    objEnd = dotPos - 1
    objStart = objEnd

    do while objStart > 0 & ,
        verify(substr(line, objStart, 1), 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_') = 0
        objStart = objStart - 1
    end
    objStart = objStart + 1

    object = substr(line, objStart, objEnd - objStart + 1)
    method = substr(line, methodStart, methodEnd - methodStart)

    ##Extract argument list, handle nested parentheses
    parenStart = methodEnd
    parenCount = 1
    p = parenStart + 1

    do while p <= length(line) & parenCount > 0
        c = substr(line, p, 1)
        if c = '(' then parenCount = parenCount + 1
        if c = ')' then parenCount = parenCount - 1
        p = p + 1
    end
    parenEnd = p - 1
    args = strip(substr(line, parenStart + 1, parenEnd - parenStart - 1))

    ##Build the transformed function call
    upperMethod = translate(method)
    upperObject = translate(object)
  ##  callText = upperMethod || '_' || upperObject || '(' || object

    ooprefix=getvar(object'_prefix')
    if verbose then say 8888888888 ooprefix
    xyprefix=getvar(object'_prefix')
    callText = ooprefix||upperMethod'('object
    if args \= '' then callText = callText || ',' || args
    callText = callText || ')'

    ##Append unchanged part before object, then transformed call
    result = result || substr(line, posStart, objStart - posStart) || callText

    ##Continue after the transformed expression
    posStart = parenEnd + 1
end

##Append any remaining text
if posStart <= length(line) then result = result || substr(line, posStart)
return result

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
  ucmd=upper(word(line,1))
  if ucmd = '##SET' | ucmd = '##UNSET' then do
     call cmd_set 9999,line
     if pos(' nset',cflags)>0 then return ''
     return printGen(line,3)
  end
  ## if cmt='##' then return line
  ## if cmt='/*' then return line
  ## if pos('(',line,1)=0 then return line   ## if there is no "(" there can't be a macro call
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
   do while pos(name, uline, callPos + 1) > 0
   ## test Macro is variadic
      isVariadic = 0
      if pos('...', args,1) > 0 then do
         isVariadic = 1
         args = strip(changestr('...', '', args))
      end
       if printgen_flags='all' then call writeline printGen(strip(line),0)
       else if level=0 & printgen_flags='nnest' then call writeline printGen(strip(line),0)
       level=level+1     ## increase level in case a second instance of macro is found
       callPos = pos(name, uline, callPos + 1)
       if callpos>1 then do
          status_before=verify(substr(uline,callpos-1,1), alphaN, 'N')
          if status_before=0 then iterate
       end
       remain  = substr(line, callPos + length(name))    ## set to parameter part, macro has format name( +length positions into it
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
  fp1=pos('{',line2change,1)>0
  fp2=pos('}',line2change,fp1+1)
  do while fp1>0 & fp2>0
     cmt=substr(strip(line2change),1,2)
     if cmt='/*' | cmt='##' then leave
     do i=1 to macros_varname.0
        if macros_varname.i='' then iterate
        Line2change=ChangeStr(macros_varname.i,line2change,macros_varvalue.i)
     end
     fp1=pos('{',line2change,fp2+1)>0
     fp2=pos('}',line2change,fp2+1)
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
      aname = word(xargs, k)
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
  lhs      = word(tempBody, 1)
  ix       = word(tempBody, 2)

  /* Extract the remaining expression */
  tempBody = translate(bodyExp, , '=')
  remain   = subword(tempBody, 2)

  /* Replace the primary macro parameters */
  if pos('arglist',lhs)>0 then inc=0
  else do
     bodyExp  = replaceArg(bodyExp, lhs, word(argText, 1))      /* fixed first argument */
     bodyExp  = replaceArg(bodyExp, 'argcount', varCount)
     inc=1
  end
  originalBody = bodyExp  /* store original for each iteration */
  /* Loop through variadic arguments */
  do k = 1 to varCount
     value   = word(argText, k + 1)
     bodyExp = replaceArg(bodyExp, '$indx', k)
     bodyExp = replaceArg(bodyExp, 'arglist.'k, value)
     call writeline bodyExp
     bodyExp = originalBody  /* reset to base form for next iteration */
  end

  /* Final run with index = 0 (optional placeholder reset) */
  bodyExp = replaceArg(bodyExp, '$indx', 0)

  /* Extract final left-hand side for return */
  finalBody = translate(bodyExp, , '=')
  lhs       = word(finalBody, 1)
  remain    = word(finalBody, 2)

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
     c = substr(remain, j, 1)
     if c = '(' then depth = depth + 1
     if c = ')' then do
        depth = depth - 1
        if depth = 0 then leave  /* finished parsing full argument expression */
     end
     argText = argText || c
  end
  return strip(argText)
/* ------------------------------------------------------------------
 * Replace Argument in Macro Call
 * ------------------------------------------------------------------
 */
replaceArg: procedure=.string
  arg str=.string, name=.string, value=.string

  posn = 1
  p = pos(name, str, 1)
  do while p>0
     before = ''
     if p > 1 then before = substr(str, p - 1, 1)
     after  = substr(str, p + length(name), 1)
    /* check before and after chars to change only full variable names */
	verbb=verify(before, alphaN, 'N')
    verba=verify(after, alphaN, 'N')
    if length(before)=0 then verbb=1
	if length(after) =0 then verba=1
    if verbb=0 | verba=0 then do
	   posn = p + 1
	   p=posn
       iterate        /* no, not a valid variable name */
    end
    if p>length(str) then leave
    str=insertatc(value,str,p,length(name))    ## use the C-function
    posn = p + length(value)
    ## if posn>length(str) then say 'string exceeds length'
    p = pos(name, str, posn)
  end
return str
/* ------------------------------------------------------------------
 * Insert/replace string in string
 * ------------------------------------------------------------------
 */
InsertAt: Procedure=.string
arg needle=.string,haystack=.string,at=.int,len=.int
  if at+len>length(haystack) then rhs=''
  else rhs=substr(haystack, at+len)
  if at>1 then lhs=substr(haystack, 1, at-1)
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
    c = substr(argstr, j, 1)
    if c = ',' & depth = 0 then do
      callargs.i = strip(token)
      token = ''
      i = i + 1
    end
    else do
      if c = '(' then depth = depth + 1
      if c = ')' then depth = depth - 1
      token = token || c
    end
  end
  if strip(token) <> '' then callargs.i = strip(token)
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
  index=getvarindx(varname)
return index
/* ------------------------------------------------------------------
 * Return pre-compiler variable content
 * ------------------------------------------------------------------
 */
getvar: procedure=.string
  arg varname=.string
  varname='{'lower(varname)'}'
  vvalue=''
  i=getvarindx(varname)
  if i>0 then vvalue=macros_varvalue.i
return vvalue
/* ------------------------------------------------------------------
 * Set pre-compiler variable
 * ------------------------------------------------------------------
 */
setvar: procedure=.int
  arg varname=.string,varvalue=.string
  varname='{'lower(varname)'}'
  if varname='{printgen}' then printgen_flags=varvalue
  i=getvarindx(varname)
  if i=0 then i=macros_varname.0+1
  macros_varname.i=varname
  macros_varvalue.i=varvalue
return i
/* ------------------------------------------------------------------
 * Check for relative or absolute path
 * ------------------------------------------------------------------
 */
isAbsolutePath: procedure=.int
  arg path=.string
  path = translate(path, '/', '\')      /* Normalize slashes */
  if left(path, 1) = '/' then return 1  /* Check for Unix-style absolute path */
  if length(path) >= 2 then do          /* Check for Windows drive-letter absolute path (e.g. C:/folder) */
     if verify(substr(path, 1, 1),'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz')=0 & substr(path, 2, 1) = ':' then return 1
  end
  if left(path, 2) = '//' then return 1 /* Check for UNC path (Windows): starts with double backslash */
return 0
/* ------------------------------------------------------------------
 * normalizePath: resolves ../, ./ and slashes in mixed paths to normalised UNIX-style
 * Example: normalizePath("C:\\Users\\Peter/../Docs/./file.txt") -> "C:/Users/Docs/file.txt"
 * ------------------------------------------------------------------
 */
normalisePath: procedure=.string
  arg rawpath=.string

  path = translate(rawpath, '/', '\')    /* Convert backslashes to forward slashes */
  /* Split path into components by '/' */
  components.1=''
  compi = 0
  do while path \= ''
     ppi = pos('/', path)
     if ppi > 0 then do
        part = substr(path, 1, ppi - 1)
        path = substr(path, ppi + 1)
     end
     else do
        part = path
        path = ''
     end
     if part = '' then iterate
     compi = compi + 1
     components.compi = part
  end
  /* Stack for normalised components */
  parti = 0
  do i = 1 to compi
    part = components.i
    if part = '.' then nop
    else if part = '..' then do
       if parti > 0 then parti = parti - 1
    end
    else do
       parti = parti + 1
       norm.parti = part
    end
  end
  /* Check for drive letter prefix */
  drive = ''
  if substr(components.1, 2, 1) = ':' then do
     drive = components.1
     start = 2
  end
  else start = 1
/* Reconstruct the normalised path with forward slashes */
  normalised = ''
  if drive \= '' then normalised = drive
  do i = start to parti
     normalised = normalised || '/' || norm.i
  end
  if normalised = '' then normalised = '/'
return normalised

/* ------------------------------------------------------------------
 * Init RXPP environment
 * ------------------------------------------------------------------
 */
rxppinit: procedure=.string
  arg rexxname=.string,maclib=.string
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
  syspath=translate(maclib,,'/\')
  wrds=words(syspath)
  wlast=wordindex(syspath,wrds)
  syspath=substr(maclib,1,wlast-1)
  call setvar 'rxpp_rexx',rexxname
  cflags=' ndef nset svars siflink n1buf n2buf n3buf nvars'
    call setvar 'cflags',cflags
  printgen_flags='all'
  call setvar 'printgen',printgen_flags
  call setvar 'rxpp_date',date()' 'time()
 return strip(syspath)