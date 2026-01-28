/*
 * crexx RXPP
 * CREXX Pre Compiler
 * Author Peter Jacob
 * Created May 2025
 * Version 31. May 2025
 */
options levelb

import precomp
namespace rxpp expose globaldef rxmodule source stype callargs macros_mname macros_margs macros_mbody macros_mspace macros_varname macros_varvalue cflags printgen_flags outbuf lino curlino rexxlines included_files syspath alphaN mExpanded expandLevel aifblock elapsedTime verbose imported_funcs select_count stem_scount stem_tempvar rxpp_lrc
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
verbose=1

  ElapsedTime=time('us')

  do i=1 to command.0
     j=i+1
     command.i=upper(command.i)
     if command.i     ='-I' then infile=command.j
     else if command.i='-IN' then infile=command.j
     else if command.i='-O' then outfile=command.j
     else if command.i='-OUT' then outfile=command.j
     else if command.i='-M' then maclib=command.j
     else if command.i='-VERBOSE0' then verbose=0
  end

  if verbose then say 'CRX0010I ['time('l')'] Pre-Compile started'
  
  if internal_testing=1 then do
     infile  = 'C:/Users/PeterJ/CLionProjects/CREXX250701/lib/plugins/system/treemap_test.rxpp'
     outfile = 'C:/Users/PeterJ/CLionProjects/CREXX250701/lib/plugins/system/treemap_test.rexx'
     maclib  = 'C:/Users/PeterJ/CLionProjects/CREXX250701/lib/plugins/precomp/Maclib.rexx'
  end

 if strip(infile)='' then do
     say 'CRX0100E+['time('l')'] no source file specified'
     exit 8
  end
  if verbose then do
     say 'Input File:  ' infile
     say 'Output File: ' outfile
     say 'Macro Lib:   ' maclib
  end

  syspath=rxppinit(infile,maclib)                   ## init global and environment variables
  if verbose then say 'CRX0100I ['time('l')'] Pre-Compile pass one'
  if verbose then say 'CRX0110I ['time('l')'] System Path 'syspath
/* --------------------------------------------------------------------
 * Pass One. Load Macros, Load Source, Expand Macros of Source
 * --------------------------------------------------------------------
 */
  sourceLines=RXPPPassOne(infile,outfile,maclib,syspath'/macsys.rexx')    ## load source file and macro library
  ## !!! to early is as picked up later-> is in pass 2 now !! if pos(' 1buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 1'
/* ------------------------------------------------------------------
 * Pass Two create ##IF/##IFN/##ELSE/##ENDIF reference table
 * ------------------------------------------------------------------
 */
  call RXPPPassTwo                                  ## pass 2 to pre-expand certain elements (##ELSE)
     if pos(' 2buf',cflags)>0 then call list_array source,-1,-1,'Source Buffer after Pass 2'
  if verbose then say 'CRX0200I ['time('l')'] Pre-Compile pass two'
/* ------------------------------------------------------------------
 *  Pass 3 keep/drop ##IF /##IFN blocks, build Final Output Buffer
 * ------------------------------------------------------------------
 */
  call RXPPPassThree outfile                        ## analyse source and expand macros
  if verbose then say 'CRX0310I ['time('l')'] Pre-Compiled REXX saved'
     if pos(' 3buf',cflags)>0 then call list_array outbuf, -1,-1,'Source Buffer after Pass 3'
  call writeall outbuf,outfile,-1                   ## write generated output to file
  call linkerInfo outfile,imported_funcs            ## linker definition will be calculated out of the normal output file
  if verbose then do
     say 'CRX0500I ['time('l')'] Pre-Compile completed, 'mexpanded' macro calls expanded, total source lines 'outbuf.0
     if pos(' vars',cflags)>0 then call printvars
     if pos(' maclist',cflags)>0 then call printmacs
     if pos(' includes',cflags)>0 then call list_array included_files,-1,-1,'Include Files'
     elapsedTime=(time('us')-elapsedtime)
     say 'CRX0101I ['time('l')'] Elapsed Time 'elapsedTime/1000000' seconds, 'elapsedTime/1000' milliseconds'
  end
return 0

/* ------------------------------------------------------------------
 * Pre Processor Pass One
 * 1. Load (and sort) Macros
 * 2. Load Source File
 * 3. Determine Pre-Processor Statements
 * ------------------------------------------------------------------
 */
RXPPPassOne: procedure = .int
  arg expose infile=.string, outfile=.string,maclib=.string, macsys=.string

/* ---- 1a. Load Macros from MACLIB  */
  macnum=readSource(maclib)
  if macnum<0 then do
     if verbose then say 'CRX0900E+['time('l')'] Maclib not found, or not accessible: 'maclib
  end
  else do
     call GetPreComp macnum                 ## analyse maclib, source lines not needed just register macros
     if verbose then say 'CRX0120I ['time('l')'] Maclib loaded:      'source.0' records'
     if verbose then say 'CRX0130I ['time('l')'] Macros extracted:   'macros_mname.0
     maxmac=macros_mname.0
  end
## clear source array
  call drop_Array source
  call drop_Array stype

/* ---- 1b. Load System Macros from MACSYS  */
  macnum=readSource(macsys)
  if macnum<0 then do
     if verbose then say 'CRX0900E+['time('l')'] System Maclib not found, or not accessible: 'macsys
  end
  else do
     call GetPreComp macnum                 ## analyse maclib, source lines not needed just register macros
     if verbose then say 'CRX0120I ['time('l')'] System Maclib loaded: 'source.0' records'
     if verbose then say 'CRX0130I ['time('l')'] Macros extracted:     'macros_mname.0-maxmac
  end
## clear source array
  call drop_Array source
  call drop_Array stype

/* ---- 1c. Load file to compile */
  rexxLines=readSource(infile)
  if rexxLines<0 then do
     say 'CRX0910E+['time('l')'] source file missing: 'infile
     exit 8
  end

/* ---- 2. find levelb, import and other option stuff  */
  which=0
  /* now pickup the flags, they might be used in PRECOMP */
  i=fsearch(source,1,"##CFLAG","","",which)   ## pickup CFLAG
  if i>0 then flagsset=early_flag_pick_up(i)

  linx=ffind(source,1,'options levelb')
  if linx>0 then stype.linx='X'         /* Skip line */
  linx=ffind(source,1,'import rxfnsb')
  if linx>0 then stype.linx='X'         /* Skip line */
  linx=ffind(source,1,'/* REXX */')
  if linx>0 then stype.linx='X'         /* Skip line */
  call insert_array source,1,10         /* insert header, and reserve some lines */
  call insert_array stype,1,10
  source.1='/* RXPP */'
  source.2='options levelb'
  source.3='import rxfnsb'
  source.4='/* reserved for namespace definition */'
  rexxlines=rexxlines+3
  if verbose then say 'CRX0140I ['time('l')'] Rexx Source loaded: 'rexxLines' records'

/* ---- 3. Pre-Compile source file,  */
  maclibm=macros_mname.0
  call GetPreComp rexxlines              ## analyse source, source lines needed keep them
  if verbose then say 'CRX0150I ['time('l')'] Macros extracted:   'macros_mname.0-maclibm
  call sort_bylen macros_mname,macros_margs,macros_mbody,macros_mspace ## sort macro names by length do avoid unintented substitution (e.g. quote dquote)
  if verbose then say 'CRX0160I ['time('l')'] Macros arranged:    'macros_mname.0
return rexxlines
/* ------------------------------------------------------------------
 * Pass 2 pre-expand certain elements
 * !! Macro expansions already took place in Pass one step 3 !!
 * create ##IF/##IFN/##ELSE/##ENDIF reference table
 * ------------------------------------------------------------------
 */
RXPPPassTwo: procedure
  stack = .string[]     /* simulate stack using indexed stem */
  depth = 0             /* current depth */
  aifblock=.int[]
  which=0
  i=fsearch(source,1,'namespace ','',"",which)            ## find any namespace instruction
  if i>0 then do
     globaldef=source.i' 'globaldef
     source.i=""
  end
  i=fsearch(source,1,'##IF ','##IFN ',"##CFLAG",which)   ## search as first word in the source string

/* 1. ##ELSE becomes    Translate ELSE clause
        ##ENDIF         close previous ##IF
        ##IFN condition open else as ##IFN    */
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

/* 2. ##IF / ##IFN build cross reference  */
  which=0
  i=fsearch(source,1,'##IF ','##IFN ',"",which)   ## search as first word in the source string
  do while i>0
     if which=1 then stype.i='IF'
     else if which=2 then stype.i='IFN'
     lnk=findMatchingEndif(i+1)
     aifblock.i=lnk
     i=fsearch(source,i+1,'##IF ','##IFN ','',which)
  end

/* 3. Build OO definition table */
  i=ffind(source,1,'OOCREATE(')   ## search as first word in the source string
  do while i>0
     call oocreatedefs i
     i=ffind(source,i+1,'OOCREATE(')   ## search as next word in the source string
  end

/* 4. output ##IF ##IFN / ##ENDIF Reference table */
  if verbose=0 then return
  if pos(' iflink',cflags)=0 then return
  if aifblock.0=0 then say "+++ no Pre Source Expansion occurred"
  else do i=1 to aifblock.0
     if aifblock.i=0 then iterate
     lnk=aifblock.i
     say right(i,4,'0')' 'left(strip(source.i),15)' linked to --> ' right(lnk,4,'0') ' ' strip(source.lnk)'  <+++ PASS2: IF/ENDIF linked; PASS3 decides emit or skip>'
     do j=i+1 to lnk-1
        say '? controlled by IF: 'right(j,4,'0')' 'strip(source.j)
     end
     say right(lnk,4,'0')' 'left(strip(source.lnk),15)' (block end)'
  end
return
/* ------------------------------------------------------------------
 *  Pass 3 Build Final Output Buffer
 *    1. activate ##IF /##IFN block if condition is true
 *    2. not active blocks are dropped stype.LineNo='X'
 *    3. handle ##PARSE, why ? maybe belongs to Pass one
 *    4. Write or Dropm Macro and pre-compiler statements
 *       to show original pre-process call
 * ------------------------------------------------------------------
 */
RXPPPassThree: procedure
  arg out=.string
  lineNo=0
  if globaldef='' then source.4=''
  else do
     if fpos('namespace',globaldef,1)>0 then source.4=globaldef
     else source.4='namespace 'rxmodule' expose 'globaldef
  end
  do while lineNo<source.0
     lineNo=Lineno+1
     curLino=LineNo           ## save into global current LineNo
     line = source.lineNo
     if stype.LineNo='X' then iterate    ## suppress any ##ELSE ##ENDIF
     if pos('/* reserved',line)>0 then iterate
  	 else if stype.LineNo='D' then do
 	    if pos(' ndef',cflags)>0 then iterate     ## ndef    : suppress original definition, just output expanded result
 	    call writeLine printGen(line,1)           ## printgen: prints pre-processor line with appropriate suffix comment
  	 end
     else if stype.LineNo='I' then do
       if pos(' nset',cflags)>0 then iterate      ## nset    : suppress original definition, just output expanded result
   	   call writeLine printGen(line,2)            ## printgen: prints pre-processor line with appropriate suffix comment
     end
     else if stype.LineNo='IF' then do
       ##  call writeLine printGen(line,4)
       if findvar(word(line,2))=0 & aifblock.lineno>lineNo then do   ## only forward pointer are permitted, else loop
           lineno=aifblock.lineno
        end
     end
     else if stype.LineNo='IFN' then do
       ##  call writeLine printGen(line,4)
        if findvar(word(line,2))>0 & aifblock.lineno>lineNo then do  ## only forward pointer are permitted, else loop
           lineno=aifblock.lineno
        end
     end
     else if stype.LineNo='PARSE' then call parsevar lineNo,line
     else if stype.LineNo='IMPORT' then do
        func=word(line,2)
        if pos("|"func,imported_funcs)=0 then do
           imported_funcs=imported_funcs"|"func   ## keep track of imported modules
           call writeline line                    ## also write line, else it gets lost
        end
     end
     else if stype.LineNo='ARRAY' then do
       if pos(' ndef',cflags)>0 then iterate      ## ndef    : suppress original definition, just output expanded result
       call writeLine printGen(line,5)            ## printgen: prints pre-processor line with appropriate suffix comment
     end
     else if stype.LineNo='GLOBAL' then do
       if pos(' ndef',cflags)>0 then iterate      ## ndef    : suppress original definition, just output expanded result
       call writeLine printGen(line,6)            ## printgen: prints pre-processor line with appropriate suffix comment
     end
  	 else if stype.LineNo='WHEN' then do
       if pos(' ndef',cflags)>0 then iterate      ## ndef    : suppress original definition, just output expanded result
       call writeLine printGen(line,3)            ## printgen: prints pre-processor line with appropriate suffix comment
   	 end
   	 else if stype.LineNo='FOR' then do
       if pos(' ndef',cflags)>0 then iterate      ## ndef    : suppress original definition, just output expanded result
       call writeLine printGen(line,3)            ## printgen: prints pre-processor line with appropriate suffix comment
     end
   	 else if stype.LineNo='SELECT' then do
       if pos(' ndef',cflags)>0 then iterate      ## ndef    : suppress original definition, just output expanded result
       call writeLine printGen(line,3)            ## printgen: prints pre-processor line with appropriate suffix comment
   	 end
     else if strip(line) \='' then do
   	    newline = expandRecursive(line)
   	    call writeline newline
   	 end
  end
  call writeline ''
  if verbose then say 'CRX0300I ['time('l')'] Pre-Compiled REXX generated '
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
 * Early_flag_pick_up: Reads ##CFLAG early and updates runtime flags used during subsequent passes.
 * ------------------------------------------------------------------
 */
early_flag_pick_up: procedure=.int
  arg i=.int
  if i=0 then return 0
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
 * for ##IF / ##IFN find associated ##ENDIF, handling nesting.
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
 * Read Rexx Source file into the global source[] buffer.
 * ------------------------------------------------------------------
 */
ReadSource: procedure=.int
  arg file=.string
  i=readall(source,file,-1)
return i
/* ------------------------------------------------------------------
 * Scans source for pre-compiler directives and macro definitions,
 * dispatches command handlers.
 * ------------------------------------------------------------------
 */
GetPrecomp: procedure
   arg lino=.int
   lineNo = 0
   do while lineNo < source.0      ## array might grow, so while is more reliable
      LineNo=LineNo+1
      if stype[LineNO]='noexp' | stype[LineNO]='cmt' then iterate
      hit=0
      depth = CommentScan(lineNo,0,hit)
      if hit then do
         cmtbeg=LineNo
         stype[lineNo] = 'cmt'
      /* if still inside a block comment, mark subsequent lines too */
         do while depth > 0 & lineNo < source.0
            lineNo = lineNo + 1
            stype[lineNo] = 'cmt'
            depth=CommentScan(lineNo, depth,hit)  /* hit will be 1 anyway because depth>0 at BOL, but keep it consistent */
         end
         if pos(' cmtblk',cflags)> 0 & LineNo-cmtbeg>15 then do ci=cmtbeg+1 to lineNo-1
            source[ci]='*>'source[ci]
         end
      end
      line = source.lineNo
      ucmd=word(line,1)
      if ucmd='' then iterate
      ucmd=upper(ucmd)

      if \(ucmd='##STEM' | ucmd='##DEFINE') then do
         if pos('.',line) then do
             if cmd_isstem(lineNo,line)>0 then iterate    ## deeper check if stem, if so start stem handling
         end
      end
      if ucmd = 'PARSE' then stype.LineNo='PARSE'                      ## short cut although it is a precompiler instruction
      else if ucmd = 'IMPORT' then stype.LineNo='IMPORT'               ## keep track of all imported functions plugins
      else if ucmd = 'SELECT' then call cmd_select lineNo,line,1
      else if ucmd = 'SWITCH' then call cmd_select lineNo,line,2
      else if ucmd = 'FOR'    then call cmd_for lineNo,line
      else if ucmd = 'WHEN'   then call cmd_when lineNo,line,1
      else if ucmd = 'OTHERWISE' then call cmd_when lineNo,line,2
      else if ucmd = 'CASE'    then call cmd_when lineNo,line,3
      else if ucmd = 'DEFAULT' then call cmd_when lineNo,line,2       ## same as OTHERWISE
      if substr(ucmd,1,2)\='##' then iterate
      if ucmd      = '##DEFINE'  then lineno=cmd_define(lineNo,line)
      else if ucmd = '##INCLUDE' then call cmd_include lineNo,line,1
      else if ucmd = '##USE'     then call cmd_include lineNo,line,2
      else if ucmd = '##DATA'    then call cmd_data lineNo,line,word(line,2)
      else if ucmd = '##INPUT'   then call cmd_data lineNo,line,"input"
      else if ucmd = '##PARSE'   then stype.LineNo='PARSE'
      else if ucmd = '##ARRAY'   then call cmd_array  lineNo,line
      else if ucmd = '##GLOBAL'  then call cmd_global lineNo,line
      else if ucmd = '##STEM'    then call cmd_stem lineNo,line
      else if substr(ucmd,1,5) = '##SYS'   then call cmd_data lineNo,line, substr(ucmd,3)
##       else if ucmd = '##????' then do    ## for any new pre compile statement
##       end
   end
return
/* ------------------------------------------------------------------
 * Process ##DEFINE command
 * Parses a macro definition and registers the macro in the macro tables.
 * ------------------------------------------------------------------
 */
CMD_define: procedure=.int
   arg lino=.int,line=.string
   nlino=lino
   name    = ''
   arglist = ''
   body    = ''
   dspace=0
   mtype=upper(word(line,2))
   if mtype = 'FUNCTION' | mtype='FKT' then def = subword(line, 3)
   if mtype = 'COMMAND' | mtype='CMD' then do
      def = subword(line, 3)
      dspace=1
   end
   else def = subword(line, 2)
   wrd=word(def,1)
   ppi = pos('(', wrd,1)    ## must be in macro name, not in later macro body
   if ppi > 1 then do  /* Format: name(arg1, arg2) body */
      name    = strip(substr(def, 1, ppi - 1))
      ppi2    = pos(')', def,ppi+1)
      if ppi2 = 0 then do
         say 'CRX0930E+['time('l')'] missing closing parenthesis in macro definition: ' def
         exit 8
      end
      if ppi2 - ppi - 1<=0 then arglist=''
      else do
         arglist = strip(substr(def, ppi + 1, ppi2 - ppi - 1))
         if dspace=1 then arglist=ConcParms(arglist)
      end
      body= strip(substr(def, ppi2 + 1))
   end
   else do /* Format: name body (no arguments) */
      name = wrd
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
      if nlen>1 then body=strip(substr(body,1,nlen))' ; 'strip(source.nlino)
      else body=strip(source.nlino)
      stype.nlino='D'
   end
   opr=pos('{',body)
   lopr=lastpos('}',body)
   if lopr>0 then body=substr(body,opr+1,lopr-opr-1)
   body=changestr('; ;',body,';')
   body=strip(body)
   if substr(body,1,1)=';' then body=substr(body,2)
   body=strip(body)
   if body = '' | lopr=0 then do
      say 'CRX0940E+['time('l')'] empty macro body or missing {} in macro: ' name
      exit 8
   end

 /* Register macro */
   i = macros_mname.0
 ## todo this is a sledgehammer approach, count the entries, if the counter says it is 0
   if i=0 then do i=1 to 150
      if macros_mname.i = '' then leave
      i=i+1
   end

   if macros_mname.i \= '' then i = i + 1
   if dspace=1 then macros_mname.i = upper(name)' '
   else macros_mname.i = upper(name)'('
   macros_margs.i = strip(arglist)
   macros_mbody.i = body
   macros_mspace.i= dspace
return nlino
/* ------------------------------------------------------------------
 * Concatenate all parameters into a comma-separated list.
 * ------------------------------------------------------------------
 */
ConcParms: Procedure=.string
   arg inparm=.string
   sargs=''
   do i=1 to words(inparm)
      sargs=sargs','word(inparm,i)
   end
   if sargs='' then return ''
return substr(sargs,2)
/* ------------------------------------------------------------------
 * Formats pre-compiler statements/macros into annotated comment lines for traceability.
 * ------------------------------------------------------------------
 */
printGen: procedure=.string
  arg line=.string, type=.int
  if printgen_flags='none' then return '' ## suppress all macro call definitions
  if type=1 then return '/* 'line' D*/'   ## DEFINE clause
  if type=2 then return '/* 'line' I*/'   ## INCLUDE clause
  if type=3 then return '/* 'line' S*/'   ## SET clause
  if type=4 then return '/* 'line' IF*/'  ## IF clause
  if type=5 then return '/* 'line' AR*/'  ## Array clause
  if type=6 then return '/* 'line' GL*/'  ## Global clause
  else return '/* rxpp: 'line' */'        ## Macro call
return '/* rxpp: 'line' U*/'
/* ------------------------------------------------------------------
 * Implements ##SET: assigns a pre-compiler variable and updates internal flags if needed.
 * ------------------------------------------------------------------
 */
CMD_set: procedure
  arg lino=.int,incl=.string
  stype.lino= 'S'
  varn=word(incl,2)
  vind=setvar(varn,DropComment(subword(incl,3)))
  if varn='cflags' then cflags=word(lower(macros_varvalue.vind),1) ## set cflags additionally directly will be used often
return

/* ------------------------------------------------------------------
 * Removes trailing line (##) or block (/*..*/) comments from a value.
 * ------------------------------------------------------------------
 */
dropComment: procedure=.string
  arg varvalue=.string,from=1
  fp1=pos('##',varvalue,from)
  if fp1>1 then varvalue=substr(varvalue,1,fp1-1)
  fp1=pos('/*',varvalue,from)
  if fp1>1 then varvalue=substr(varvalue,1,fp1-1)
return strip(varvalue)
/* ------------------------------------------------------------------
 * CMD_unset: Implements ##UNSET: clears a pre-compiler variable (with exemptions for protected vars).
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
 * CMD_include: Implements ##INCLUDE / ##USE: reads and inserts the included file into the source buffer.
 * ------------------------------------------------------------------
 */
CMD_include: procedure
  arg lino=.int,incl=.string,mode=.int
  stype.lino= 'I'
  file=word(incl,2)
  file=normalisepath(syspath'/'file)
  include=.string[]
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
     rc= insert_source(lino+1,new)
     insertat=lino
  end
  else do
     linc=source.0
     rc= insert_source(linc+1,new)
     insertat=linc
  end
  do j=1 to new
     insertat=insertat+1
     source.insertat=include.j
  end
return
/* ------------------------------------------------------------------
 * CMD_array: Implements ##ARRAY: expands array declarations into initialization statements.
 * ------------------------------------------------------------------
 */
CMD_array: procedure
   arg lino=.int,line=.string
   stype.lino= 'ARRAY'
   line=translate(line,,',')
   line=DropComment(line,3)
   atype=word(line,2)
   if substr(atype,1,1)\='.' then atype='.'atype
   defs=subword(line,3)
   new=words(defs)                       ## check how many arrays have been defined
   if new=0 then return
   rc= insert_source(lino+1,new)    ## insert new lines, shift buffer
   do i=1 to new                         ## now add the statements
      def=word(defs,i)
      source[lino+i]=def'='atype'[];'
   end
return
/* ------------------------------------------------------------------
 * CMD_global: Implements ##GLOBAL: expands globals and accumulates namespace exposure list.
 * ------------------------------------------------------------------
 */
CMD_global: procedure
   arg lino=.int,line=.string
   stype.lino= 'GLOBAL'
   line=translate(line,,',')
   line=DropComment(line,3)
   atype=upper(word(line,2))
   if substr(atype,1,1)='.' then atype=substr(atype,2)
   if atype='INT' | atype='FLOAT' | atype='STRING' | atype='INT[]' | atype='FLOAT[]' | atype='STRING[]'then nop
   else return
   defs=subword(line,3)
   new=words(defs)                       ## check how many arrays have been defined
   if new=0 then return
   rc= insert_source(lino+1,new)    ## insert new lines, shift buffer
   do i=1 to new                         ## now add the statements
      def=word(defs,i)
      vtemp=translate(def,,'=')
      ivar=word(vtemp,1)
      ival=word(vtemp,2)
      if ival='' then source[lino+i]=ivar'=.'atype
      else source[lino+i]=ivar'=.'atype'; 'def
      globaldef=globaldef' 'ivar
   end
return
/* ------------------------------------------------------------------
 * CMD_isstem: Fast detector to decide whether a line should be processed for stem expansion.
 * ------------------------------------------------------------------
 */
CMD_isstem: procedure=.int
   arg lino=.int,line=.string
   candidate=0
   tline=qstripComment('##','',line)
   tline=qstripComment('/*','*/',tline)
   tline=translate(tline,,'=;,')
   if pos(' dotisstem',cflags)>0 then mtails=1
   else mtails=2

   do i=1 to qwords(tline)
      tw=qword(tline,i)
      if countstr('.',tw)>=mtails then do
         if left(tw,1)\='.' & right(tw,1)\='.' then do
            candidate=1
            leave
         end
      end
   end
   if candidate=1 then call cmd_stem lino,line, mode=1
return candidate
/* ------------------------------------------------------------------
 * CMD_stem: Main stem preprocessor: rewrites stem reads/writes into getstem()/putstem() and temp variables.
 * Identifies a line containing stem operations (temporary for testing)
 * ------------------------------------------------------------------
 */
CMD_stem: procedure
   arg lino=.int,line=.string,mode=0
   stemlog=0
   oldhi=source.0
   oldlino=lino
   stype.lino= 'STEM'
   rxpp_lrc=''

   if isStemCandidate(line)=0 then return
   wrd1=upper(word(line,1))
   if wrd1='##DEFINE' then return
   assembler SETATTRS stem_tempvar,0
/* if we are called via the ##stem prefix, we need to drop it */
   if wrd1='##STEM' then line=subword(line,2)
   else do  /* else we need to comment out the stem line */
      source[lino]='/* 'copies('+',16)' 'line' */'
   end
/* now strip block comments from the line */
   nstmt=qstripComment("/*","*/",line)
   nstmt=qstripComment("##",,nstmt)
   if stemlog=1 then do
      say copies('-',72)
      say "Stem line   "right(lino,4,'0') line
      say "     line+1 "right(lino+1,4,'0') source[lino+1]
      say copies('-',72)
   end
/* identify the type of statement, if, say, call else it is a normal set statement */
   wrd1=upper(word(nstmt,1))
   lhs=''
   if wrd1='IF' | wrd1='SAY' | wrd1='CALL' then rhs=nstmt
   else do                      ## else it is a normal set statement
      stmts=qsplit(nstmt,"=")   ## split the set at = sign
      lhs=stmts.1               ## you get LHS and RHS
      rhs=stmts.2
   end
   if stemlog=1 then say 1 lino "'"stype[lino]"'" "'"line"'"
/* here we have a single RHS which is not a STEM */
   if words(rhs)=1 & isStemCandidate(rhs)=0 then lino=lhsStem(lhs,rhs,lino,line,stemlog)   ## moves single string or number to a stem
   else do               ## here we we do what need to be done */
     rhs=splitRHS(rhs,lino,stemlog)   ## Split RHS, if possible
     if stemlog=1 then say 2.1 line' LHS 'lhs' RHS 'rhs
     if rhs\=''  then do
        if oldlino=lino then call insert_source lino+1,1,'noexp',stemlog   ## insert new lines, if RHSsplit hasn't done it
        lhscan=isStemCandidate(lhs)
        if lhs='' | lhsCan=0 then lino=lhsnoStem(lhs,rhs,lino,line,stemlog)
        else if pos('.',lhs)>0 &  lhsCan>0 then lino=lhsisStem(lhs,rhs,lino,line,stemlog)
        else do
           if stemlog=1 then say 2.4 lhs rhs
           lino=insert_line(lino+1,lhs'='rhs)
        end
     end
     else if stemlog=1 then say "Should not happen"
   end
   if stemlog=1 then do
      do i=oldlino to lino
         if i=oldLino then say '9.'i' == "'stype[i]'"' source[i]
         else say '9.'i' ++ "'stype[i]'"' source[i]
      end
      do i=lino+1 to lino+3
         say '9.'i' >> "'stype[i]'"' source[i]
      end
   end
   if rxpp_lrc\='' then do
      say '\\\\\ Stem expansion warning: 'rxpp_lrc
      rc=  insert_source(lino+1,1,'noexp',stemlog)       ## insert new lines, shift buffer
      lino=insert_line(lino+1,'/* \\\\\ Stem expansion warning: 'rxpp_lrc' */') ## place identified stem in putstem function
   end
return
/* ------------------------------------------------------------------
 * lhsStem: Handles assignments where LHS may be a stem and RHS is a single non-stem token.
 * ------------------------------------------------------------------
 */
lhsStem: procedure=.int
   arg lhs=.string,rhs=.string,lino=.int,line=.string,stemlog=.int
   if stemlog=1 then say 1.1 line
   if isStem(lhs)=0 then source[lino]=line  ## LHS & RHS are not a stems, just keep the line
   else do                                  ## LHS is a stem / RHS not
      if stemlog=1 then say 1.2 line
      lhs=resolveStem(lhs,lino,stemlog)
      lhs=stemquote(lhs)                        ## represent LHS as "root."tail1"."tail2 ...
      if stemlog=1 then say 1.2.1 lhs
      rc= insert_source(lino+1,1,'noexp',stemlog)       ## insert new lines, shift buffer
      lino=insert_line(lino+1,'src=putstem('lhs','rhs')') ## place identified stem in putstem function
   end
return lino

/* ------------------------------------------------------------------
 * lhsnoStem: Handles assignments where LHS is not a stem and RHS may contain a stem or scalar value.
 * ------------------------------------------------------------------
 */
lhsnoStem: procedure=.int
   arg lhs=.string,rhs1=.string,lino=.int,line=.string,stemlog=.int
   if stemlog=1 then say 2.2 line' LHS 'lhs' RHS 'rhs
   if isStem(rhs1)=1 then do                  /* RHS is a stem */
      rhs2=resolveStem(rhs1,lino,stemlog)
      rhs3=stemquote(rhs2)
      if lhs='' then lino=insert_line(lino+1,rhs3)
      else lino=insert_line(lino+1,lhs'='rhs3)
      if stemlog=1 then do
         say 2.2.1 rhs1 lino
         say 2.2.2 rhs2 lino
         say 2.2.3 rhs3 lino "*** "lhs
      end
      return lino
   end
/* RHS is not a stem */
   if stemlog=1 then say 2.2.9 "'"lhs"'" lino "*** ""'"rhs1"'"
   if line=rhs1 then source[lino]=line
   else do
      call insert_source lino+1,1,'noexp',stemlog
      if lhs='' then lino=insert_line(lino+1,rhs1)
      else lino=insert_line(lino+1,lhs'='rhs1)
   end
return lino
/* ------------------------------------------------------------------
 * lhsisStem: Handles assignments where LHS is a stem and RHS is an expression/value.
 * ------------------------------------------------------------------
 */
lhsisStem: procedure=.int
  arg lhs=.string,rhs=.string,lino=.int,line=.string,stemlog=.int
  if stemlog=1 then say 2.3 lhs
  lhs=resolveStem(lhs,lino,stemlog)
  lhs=stemquote(lhs)
  lino=insert_line(lino+1,'src=putstem('lhs','rhs')')
return lino
/* ------------------------------------------------------------------
 * splitRHS: Splits RHS containing multiple stems into temporaries and returns the rewritten RHS.
 * ------------------------------------------------------------------
 */
splitRHS: procedure=.string
   arg rhs=.string,expose lino=.int,stemlog=.int
   position=.int[]
   start=1
   nstem=findstem(rhs,start,position)
   if nstem.0=0 then return rhs
   new=nstem.0+1        ## add line for each rhs + 1 for final replacement line
   call insert_source lino+1,new,'noexp',stemlog        ## insert new lines, shift buffer
   tempvar=.string[]
   foundstem=.string[]
   do i=1 to nstem.0
      foundstem.i=nstem.i
      nstem.i=resolveStem(nstem.i,lino,stemlog)
      temp=stemquote(nstem.i)
      lino=lino+1
      stem_scount=stem_scount+1
      tempvar[i]=stem_scount
      stem_tempvar[stem_tempvar[0]+1]="_tmp_rxpp_"tempvar[i]
      lino=insert_line(lino,"_tmp_rxpp_"tempvar[i]'=getstem('temp')')
      ## in case of debug results ##source[lino]=source[lino]  ##   source[lino]=source[lino]'; say "VX <"_tmp_rxpp_'temp[i]"'>'"
   end
   lino=lino+1
   do i = nstem.0 to 1 by -1
      rhs = replaceOnce(foundstem.i, rhs, "_tmp_rxpp_"tempvar[i], position.i)
   end
   /* lino refers at the last added line */
return rhs
/* ------------------------------------------------------------------
 * ResolveStem: Resolves computed stem tails (.()) by evaluating expressions first and substituting placeholders.
 * ------------------------------------------------------------------
 */
ResolveStem: procedure=.string
  arg stemstring=.string, expose lino=.int, stemlog=.int
/* lino current line number will be maintained here */
  expressions = qextractall_dotparen_flat(stemstring)

  if stemlog=1 then say "Resolve 1 "stemstring lino
  if expressions.0=0 then return stemstring

  if stemlog=1 then do
     say "Resolve 2, expressions found "expressions.0
     do j=1 to expressions.0
        say "Expression: "j expressions.j
     end
  end

  /* reserve lines: 1 per expression + 1 extra */
  call insert_source lino+1, expressions.0+1, 'noexp', stemlog

  anchor = lino     /* all inserts go AFTER anchor */

  do i = 1 to expressions.0
     stem_tempvar[stem_tempvar[0]+1] = "_expr_"i

     /* rewrite stems inside this expression FIRST,
        inserting _tmp_rxpp_* lines immediately after anchor */
     changed.i = rewriteStemsInExpr(expressions.i, anchor, stemlog)

     /* now insert _expr_i AFTER those tmp lines */
     anchor = anchor + 1
     anchor = insert_line(anchor, '_expr_'i'='changed.i'""')   /* ++ use brute force to ensure result is a string */

     /* replace original ( ... ) with _expr_i in the stemstring */
     stemstring = replaceOnce('('expressions.i')', stemstring, '_expr_'i)
  end

  /* move lino to last inserted line */
  lino = anchor

  if stemlog=1 then say "Resolve 3, current lino "lino stemstring
return stemstring

/* ------------------------------------------------------------------
 * rewriteStemsInExpr: Rewrites stems inside an expression into temporaries inserted after an anchor line.
 * Inserts required _tmp_rxpp_N lines immediately AFTER `anchor`,
 * and returns expr with stem tokens replaced by those tmp vars.
 * ------------------------------------------------------------------ */
rewriteStemsInExpr: procedure=.string
  arg expr=.string, expose anchor=.int, stemlog=.int

  position = .int[]
  stems = findStem(expr, 1, position)

  if stems.0 = 0 then return expr

  tempvar = .int[]
  found   = .string[]

  do i = 1 to stems.0
     found[i] = stems[i]
     temp = stemquote(stems[i])

     stem_scount = stem_scount + 1
     tempvar[i] = stem_scount
     stem_tempvar[stem_tempvar[0]+1] = "_tmp_rxpp_"tempvar[i]

     /* insert BEFORE _expr_i: right after anchor */
     anchor = anchor + 1
     anchor = insert_line(anchor, "_tmp_rxpp_"tempvar[i]'=getstem('temp')')

     if stemlog=1 then say 'ExprStem: 'stems[i]' -> _tmp_rxpp_'tempvar[i]
  end

  /* replace from right to left to keep positions stable */
  do i = stems.0 to 1 by -1
     expr = replaceOnce(found[i], expr, "_tmp_rxpp_"tempvar[i], position[i])
  end

return expr
/* ------------------------------------------------------------------
 * replaceOnce: Replaces one occurrence of a substring at a given position (or first occurrence).
 * ------------------------------------------------------------------
 */
replaceOnce: procedure=.string
  arg needle=.string,haystack=.string,newstring=.string,position=0
  if position=0 then position=pos(needle,haystack)
  if position<=0 then return haystack
return substr(haystack,1,position-1) || newstring || substr(haystack,position+length(needle))

/* ------------------------------------------------------------------
 * qextractall_dotparen_flat(text)
 * Collect ALL nested .( ... ) inner expressions, deepest-first.
 * Strategy:
 *   - find the RIGHTMOST unquoted ".(" in the current work string
 *   - match its ')' with qextractpair (nest/quote aware)
 *   - record its inner expression
 *   - replace that whole "(...)" with a hole and repeat
 * ------------------------------------------------------------------ */
qextractall_dotparen_flat: procedure=.string[]
  arg text=.string,delim='.('
  flat = .string[]
  work = text
  dlen=length(delim)

  do forever
     /* find rightmost ".(" outside quotes */
     p = qpos(delim, work, 1)
     if p = 0 then leave
     lastp = p
     do forever
        p2 = qpos(delim, work, lastp + dlen)
        if p2 = 0 then leave
        lastp = p2
     end
  /* lastp points to '.' of ".(" ; '(' is at lastp+1 */
     inner = qextractpair('(', ')', work, lastp+1, 'X')   /* exclusive: returns inside only */
     if inner = '' then leave   /* unmatched, should not happen if grammar is correct */
     indx=flat[0]+1
  /* add deepest-rightmost */
     flat[indx] = inner
  /* replace this "(inner...)" occurrence with hole */
     work = replaceOnce('('inner')', work, '_expr_'indx)
  end
return flat

/* ---------------------------------------------------------------
 * isstem: Validates whether a string is a syntactically valid stem.
 * Returns:
 *   1  -> string looks like a stem
 *   0  -> not a stem
 * ---------------------------------------------------------------
 */
isstem:procedure=.int
  arg s=.string
  irc=isStemX(s)
 ## say "ISSTEM '"s"' is RC="irc
return irc

/* ------------------------------------------------------------------
 * isStemx: Full stem grammar validation including computed segments and tail count policy.
 * ------------------------------------------------------------------
 */
isStemx: procedure=.int
  arg s=.string
  s = strip(s)
  if s = '' then return 0
  if left(s,1)='.' | right(s,1)='.' then return 0
 ## if pos(' ', s) > 0 then return 0
  if pos('..', s) > 0 then return 0
  len = length(s)

  /* ROOT first char: not numeric; must be A-Z/a-z/_/$ */
  c = substr(s,1,1)
  if datatype(c,'N') > 0 then return 0
  if \ (datatype(c,'M') > 0 | c='_' | c='$') then return 0
  /* consume ROOT name chars (no dots here) */
  i = 2
  do while i <= len
    ch = substr(s,i,1)
    if ch='.' then leave
    if isStemNameChar(ch)=0 then return 0
    i = i + 1
  end
  if i > len | substr(s,i,1) \= '.' then return 0
  tails = 0

  /* parse .SEGMENT repeating */
  do while i <= len
    if substr(s,i,1) \= '.' then return 0
    i = i + 1
    if i > len then return 0
    ch = substr(s,i,1)

    /* computed segment */
    if ch='(' then do
      close = matchParen(s, i)
      if close = 0 then return 0
      tails = tails + 1

      /* after ')': must be '.' or end (reject bare '(...)' following) */
      if close < len then do
        if substr(s, close+1, 1) \= '.' then return 0
      end
      i = close + 1
      iterate
    end
   /* name segment: one or more name chars (no '.') */
    if isStemNameChar(ch)=0 then return 0

    do while i <= len
      ch = substr(s,i,1)
      if ch='.' then leave
     if isStemNameChar(ch)=0 then return 0
      i = i + 1
    end

    tails = tails + 1
    /* loop continues; if i<=len, next must be '.' (it is, due to leave) */
  end
 /* based on definition stems may have 2 or at least 2 tails */
   if pos(' dotisstem',cflags) > 0 then do
     if tails < 1 then return 0
     else return 1
   end
   else if tails < 2 then return 0
return 1

/* ---------------------------------------------------------------
 * isStemCandidate: Heuristic filter: quickly decides if a string is worth attempting stem parsing.
 * Relaxed stem candidate detector for TOP-LEVEL routing.
 * Does NOT fully validate; just says "worth trying stemEndBySplit()".
 * --------------------------------------------------------------- */
isStemCandidate: procedure=.int
  arg s=.string
  s = strip(s)
  if s = '' then return 0

  /* fast dot presence */
  if qpos('.', s) = 0 then return 0
  if left(s,1)='.' then return 0

  /* root first char */
  c = substr(s,1,1)
  if datatype(c,'N') > 0 then return 0
  if \ (datatype(c,'M') > 0 | c='_' | c='$') then return 0

  /* DOT_IS_STEM gating: need >=1 or >=2 dots */
  dots = countstr('.', s)
  if pos(' dotisstem', cflags) > 0 then do
     if dots < 1 then return 0
  end
  else if dots < 2 then return 0

  /* allow anything else here (operators, parentheses, etc.) */
return 1

/* ------------------------------------------------------------------
 *isStemNameChar: Returns true if a character is valid within a stem name segment.
 *
 * Purpose (GRAMMAR / SEGMENT VALIDATION):
 *   Returns 1 if `c` is allowed inside a *name segment* (ROOT or .name).
 *
 *   IMPORTANT: does NOT include '.'.
 *   Dots are handled structurally by the stem grammar parser:
 *     ROOT '.' SEGMENT '.' SEGMENT ...
 *
 * Typical usage:
 *   - Validate ROOT characters (excluding the first-char special rules).
 *   - Consume/validate ".name" segments.
 *
 * NOT for:
 *   - Token boundary scanning (it would stop at '.' and break chains).
 * ------------------------------------------------------------------ */
isStemNameChar: procedure=.int
  arg c=.string
  if c='' then return 0
  if c='_' then return 1
  if datatype(c,'A') then return 1
  if datatype(c,'N') then return 1
return 0

/* ------------------------------------------------------------------
 * findStem: Finds stem tokens in an expression and returns stems + start positions.
 *
 * Strategy:
 *  1) Use qpos to locate '.' outside quotes and parentheses.
 *  2) Expand LEFT to find start of ROOT.
 *  3) Use stemEndBySplit() (QSPLITSAFE-based) to find token end.
 *  4) Validate with IsStem() and store stem + position.
 * ------------------------------------------------------------------ */
findStem: procedure=.string[]
  arg s=.string, start=1, expose position=.int[]
  len = length(s)
  rstem=.string[]
  stemdebug=0
  indx=0
  rxpp_lrc=''
  do forever
     p = qpos('.',s, start)
     if stemdebug=1 then say "stem Locate "indx s p start
     if p = 0 then leave

     /* dot must have plausible stem-root char to the left */
     if p=1 | \isStemNameChar(substr(s,p-1,1)) then do
        start = p + 1
        iterate
     end

     /* expand LEFT: find start of ROOT */
     l = p - 1
     do while l >= 1 & isStemNameChar(substr(s,l,1))
        l = l - 1
     end
     l = l + 1

     /* determine RIGHT end via QSPLITSAFE */
     r = stemEndBySplit(s, l,stemdebug)
     if stemdebug=1 then say "stemEndBySplit l="l" -> r="r 'MSG 'rxpp_lrc

     if r = 0 then do          /* not a stem chain from this root; continue search */
        start = p + 1
        iterate
     end

     stem = substr(s, l, r-l+1)
     if stemdebug=1 then say "stem candidate '"stem"'"

     if IsStem(stem) > 0 then do
        indx = indx + 1
        rstem[indx] = stem
        position[indx] = l
        if stemdebug=1 then say "New stem found "stem
     end
     start = r + 1
  end
return rstem

/* ------------------------------------------------------------------
 * stemEndBySplit: Determines the end of a stem token using safe split on '.' (quote/paren aware).
 * Determine the end position of a stem token starting at root position l.
 *
 * Uses QSPLITSAFE on the substring from l, splitting by '.' while ignoring
 * dots inside quotes and balanced ().
 *
 * Returns:
 *   absolute end position r (>=l) of stem token
 *   or 0 if it cannot form a valid stem chain shape.
 *
 * Behavior:
 * - Accepts computed segments if the segment (after strip) is exactly "(...)".
 * - Accepts name segments consisting of isStemNameChar().
 * - Stops the token at the first non-name char in a name segment (e.g. "\=''" etc).
 * - Requires at least ROOT + 2 tails (>= 3 split parts) to match your IsStem policy.
 * ------------------------------------------------------------------ */
stemEndBySplit: procedure=.int
  arg s=.string, l=.int,stemlog=.int
  rxpp_lrc = ''          /* clear */

  if stemlog=1 then say 'stemEndSplit 's l
  tail = substr(s, l)
  if tail = '' then do
     rxpp_lrc = rxpp_lrc' empty-tail: 's
     return 0
  end

  parts = qsplitsafe(tail, '.', 1, '()')

  if pos(' dotisstem',cflags) > 0 then mintails=1
  else mintails=2

  if parts.0 <= mintails then do
     rxpp_lrc = rxpp_lrc' too-few-tails: 's
     return 0
  end

  if isPlainStemName(parts[1]) = 0 then do
     rxpp_lrc = rxpp_lrc' bad-root: 's
     return 0
  end

  rrel = length(parts[1])

  do i = 2 to parts.0
     seg = parts[i]
     segs = strip(seg, 'L')

     if left(segs,1) = '(' then do
        block = qextractpair('(', ')', segs, 1, 'I')
        if block = '' then do
           rxpp_lrc = rxpp_lrc' unmatched-paren: 's
           return 0
        end

        rrel = rrel + 1 + (pos(block, segs) + length(block) - 1)

        afterPos = (pos(block, segs) + length(block))
        if afterPos <= length(segs) then do
           nextch = substr(segs, afterPos, 1)
           if nextch = '(' then do
              rxpp_lrc = rxpp_lrc' bare-parens-after-computed: 's
              return 0
           end
           if nextch \= '.' then leave
        end
        iterate
     end

     j = 1
     do while j <= length(seg) & isStemNameChar(substr(seg,j,1))
        j = j + 1
     end

     if j = 1 then do
        rxpp_lrc = rxpp_lrc' bad-name-segment: 's
        return 0
     end

     rrel = rrel + 1 + (j-1)
     if j <= length(seg) then leave
  end

return l + rrel - 1
/* ------------------------------------------------------------------
 * isPlainStemName: Validates that a segment contains only stem-name characters.
 * True if every character is a stem-name char (no dots, no spaces, etc.)
 * ------------------------------------------------------------------ */
isPlainStemName: procedure=.int
  arg t=.string
  if t = '' then return 0
  do i = 1 to length(t)
     if isStemNameChar(substr(t,i,1)) = 0 then return 0
  end
return 1

/* ------------------------------------------------------------------
 * matchParen: Finds the matching ')' for an opening '(' with nesting support.
 * Return position of matching ')' for s[openPos]='(' (balanced).
 * This allows nested parentheses inside computed expressions.
 * ------------------------------------------------------------------
 */
matchParen: procedure=.int
  arg s=.string, openPos=.int
  len = length(s)
  if openPos<1 | openPos>len | substr(s,openPos,1) \= '(' then return 0

  depth = 1
  i = openPos + 1
  do while i <= len
    ch = substr(s,i,1)
    if ch='(' then depth = depth + 1
    else if ch=')' then do
      depth = depth - 1
      if depth = 0 then return i
    end
    i = i + 1
  end
return 0

/* ------------------------------------------------------------------
 * insert_source: Inserts reserved lines into source[]/stype[] and shifts following lines down.
 * the line number is the first which is shifted,
 * !! it does not mean add empty line after provided at position !!
 * ------------------------------------------------------------------
 */
insert_source: Procedure=.int
  arg at=.int,new=.int,expand='',stemlog=0
  stemlog=0
  rc=insert_array(source,at,new)    ## insert new lines, shift buffer
  rc=insert_array(stype, at,new)
  do i=0 to new-1                   ## in some cases you want special
     source[at+i]='/* reserved 'at+i i' */'
     stype[at+i]=expand             ## treatment of the new entry for
  end                               ## example: no further inspection
  if stemlog=0 then return 0
  say 'Insert 'new' lines at 'at
  say '----------------------------------'
  if at>2 then say '<< 'right(at-2,4,'0') source[at-2]
  if at>1 then say '== 'right(at-1,4,'0') source[at-1]
  do i=at to at+new-1
     say '++ 'right(i,4,'0') source[i]
  end
  say '>> 'right(i,4,'0') source[i]
  say '>> 'right(i+1,4,'0') source[i+1]
return 0                            ## when line is inspected afterwards
/* ------------------------------------------------------------------
 * Insert_line: Writes a line into a reserved slot
 * inserts a reserved line first if needed.
 * ------------------------------------------------------------------
 */
insert_line: procedure=.int
  arg nlino=.int, newline=.string
  if pos('/* reserved',source[nlino])>0 then nop
  else do
     call insert_source nlino,1,'noexp',0
     if pos('/* reserved',source[nlino])=0 then do
        say '++ 'right(nlino,4,'0')' 'newline' overwrites'
        say '++ 'right(nlino,4,'0')' 'source[nlino]
     end
  end
  source[nlino]=newline

return nlino
/* ------------------------------------------------------------------
 * CMD_for: Expands a FOR pseudo-construct into an equivalent REXX looping sequence.
 * ------------------------------------------------------------------
 */
CMD_for: procedure
  arg lino=.int,line=.string
  line=subword(line,2)
  stype.lino= 'FOR'
  nlino=lino+1
  line=qstripcomment('/*','*/',line)
  parts=qsplit(line,';')
  from=parts.1
  condc=parts.2
  step=parts.3
  fromx=qsplit(from,',')
  start=''
  do i=1 to fromx.0
     start=start||fromx.i';'
  end
  stepx=qsplit(step,',')
  update=''
  do i=1 to stepx.0
     update=update||stepx.i';'
  end

  call insert_source nlino,6         ## insert new lines, shift buffer

  select_count=select_count+1
  source[lino+1]='__first_'select_count'=1'
  source[lino+2]=start
  source[lino+3]='do forever'
  source[lino+4]='   if __first_'select_count'=1 then __first_'select_count'=0'
  source[lino+5]='   else do; 'update'; end'
  source[lino+6]='   if 'condc' then nop; else leave'
  do i=1 to 6
     stype[nlino+i]=' '
  end
return
/* ------------------------------------------------------------------
 * CMD_when: Expands WHEN / OTHERWISE / CASE(DEFAULT) into corresponding REXX clauses.
 * This applies the REXX syntax
 * ------------------------------------------------------------------
 */
CMD_when: procedure
  arg lino=.int,line=.string,mode=.int
  stype.lino= 'WHEN'
  nlino=lino+1
  rc= insert_source(nlino,1)    ## insert new lines, shift buffer
  ifpart2=subword(line,2)
/* save some vars for CASE and CASEX */
  vvalue=word(line,2)
  action=subword(line,4)
/* check which type is CASE/WHEN */
  if mode=1      then source[nlino]='  else if 'ifpart2
  else if mode=2 then source[nlino]='  else 'ifpart2
  else if mode=3 then source[nlino]='  else if __select_'select_count'='vvalue' then 'action
  stype[nlino]=' '
return
/* ------------------------------------------------------------------
 * CMD_select: Expands SELECT / SWITCH constructs and initializes selector state.
 * This applies the C syntax
 * ------------------------------------------------------------------
 */
CMD_select: procedure
  arg lino=.int,line=.string, mode=.int
  stype.lino= 'SELECT'
  rc= insert_source(lino+1,1)    ## insert new lines, shift buffer
  if mode=2 then do         /* switch instruction */
     select_count=select_count+1
     variable=word(line,2)
     source[lino+1]='__select_'select_count'='variable'; do 1; if 1=0 then nop'
  end
  else do
     source[lino+1]='do 1; if 1=0 then nop'
  end
  stype[lino+1]=' '
return
/* ------------------------------------------------------------------
 * CMD_data: Implements ##DATA blocks: converts lines into array assignments until ##END.
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
 * writeline: Appends a line to outbuf[] after variable injection and OO translation.
 * splitting on backslashes (\)
 * ------------------------------------------------------------------
 */
writeline: procedure
  arg oline=.string
  oline=injectVariable(oline)
  ## if pos('~',oline) then oline=oo_translate(oline)
   if qpos('.',oline) then oline=ooTranslate(oline)
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
   if oline='' & outbuf.lino='' then return  ## not more than one blank line
   lino = lino + 1
   outbuf.lino = oline
  return
/* ------------------------------------------------------------------
 * oo_translate_tilde: Legacy translator for object~method(...) syntax
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

/* ------------------------------------------------------------------
 * ooTranslate: Translates object.method(...) calls into prefixed function calls.
 * ------------------------------------------------------------------
 */
ooTranslate: procedure=.string
arg oline=.string
line = strip(oline)
if upper(word(line,1))='ARG' then return oline
prf=substr(line,1,2)
if prf='##' | prf='/*'  then return oline
if stype[curlino]='cmt' | stype[curlino]='noexp' then return oline
result = ''
posStart = 1
oocall=0
do forever
    dotPos = qpos('.', line, posStart)
    if dotPos<=0 then leave
    ##Detect method name after the dot
    methodStart = dotPos + 1
    methodEnd = methodStart
    do while methodEnd <= length(line) & verify(substr(line, methodEnd, 1), 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_') = 0
        methodEnd = methodEnd + 1
    end
  ## If not followed by a '(', this is a stem access
    if substr(line, methodEnd, 1) \= '(' | substr(line, methodEnd-1, 2)='.(' then do
        stemAccess = substr(line, posStart, methodEnd - posStart)
        ##Preserve the stem text
        result = result || substr(line, posStart, methodEnd - posStart)
        posStart = methodEnd
        iterate
    end
    oocall=1
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
    if fpos(ooprefix,'stem',1)=1 then do
       if upperMethod='GET'| upperMethod='PUT' then do
          ppi=pos(',',args)
          if ppi=0 then args=changestr(".",args,"'.'")
          else do
             arg1=substr(args,1,ppi-1)
             arg1=changestr(".",arg1,"'.'")
             args=arg1','substr(args,ppi+1)
          end
       end
    end
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
 * expandRecursive: Expands a line repeatedly until no further macro substitutions occur.
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
 * expandLine: Expands all macro invocations within a single line (one expansion pass).
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
 * resolveMacro: Expands one macro occurrence by parsing arguments and substituting into the macro body.
 *     line is input line
 *     i    contains macro index to resolve line
 * ------------------------------------------------------------------
 */
resolveMacro: procedure=.string
   arg i=.int, line=.string,level=.int
   ppl=lastpos('##',line)       ## ending comment, if any
   if ppl>1 then line=substr(line,1,ppl-1)
   uline   = upper(line)
   callPos = 0
   /* load the macro header and definition */
   name    = macros_mname.i    ## macro name
   args    = macros_margs.i    ## macro arguments
   body    = macros_mbody.i    ## macros body {...}
   mspace  = macros_mspace.i   ## macro type, 1 is a sapce type, 0, normal, parms are separated by ","

   body=injectVariable(body)    ## inject pre-compiler variables, if there are any
   mexpanded=mexpanded+1
   ## test Macro is variadic
   if pos('...', args,1) > 0 then do
      isVariadic = 1
      args = strip(changestr('...', '', args))
   end
   else isVariadic = 0

   if mspace=1 then do   ## command macros have stricter rule, they must be the first word of the line and they can't be nested
      if word(uline,1) \=strip(name) then return line  ## not a valid command macro call
   end
   do forever
      callpos=pos(name, uline, callPos + 1)        ## search position of macro in input line
      if callpos=0 then leave                      ## nothing there, leave
    ## now we know macro is part of the line
       if printgen_flags='all' then call writeline printGen(strip(line),0)     ## some logging if wanted
       else if level=0 & printgen_flags='nnest' then call writeline printGen(strip(line),0)
       level=level+1     ## increase level in case a second instance of macro is found
    ## check which char is prior the macro call
       if callpos>1 then do
          status_before=verify(substr(uline,callpos-1,1), alphaN, 'N')
          if status_before=0 then iterate
       end
    ## move remaing line, which is the argument list of the macro
       remain  = substr(line, callPos + length(name))    ## set to parameter part, macro has format name( +length positions into it
    ## extract argument list
       argtext = fetchArguments(remain)
       if mspace=1 then argtext=ConcParms(argtext)
       callargcount = parseArgList(argtext)
       bodyExp = body
       if isVariadic then return variadic(bodyExp, callargcount, argtext)
    /* Here is the non variadic processing */
       bodyExp = replaceFixArg(bodyExp, name,args,callargcount)
       if substr(bodyexp,1,3)='+++' then return bodyexp  ## +++ error occurred in call
       callLen = length(name) + 1 + length(argtext)  /* inkl. len(name()+1 for ) */
       line=insertatc(bodyexp,line,callpos,callLen)
       if mspace= 1 then leave        /* nesting for command macros are not permitted */
       uline    = upper(line)         /* update uline for repetition */
       callPos  = callPos - 1         /* set next start point */
    end
return line
/* ------------------------------------------------------------------
 * injectVariable: Injects pre-compiler variables ({var}) into text lines.
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
 * replaceFixArg: Substitutes fixed macro parameters with supplied call arguments (positional + keyword).
 * ------------------------------------------------------------------
 */
replaceFixArg: Procedure=.string
  arg bodyexp=.string,macname=.string, xargs=.string,callargcount=.int
  iargs=xargs

  xargs   = translate(xargs, , ',')
  wrds=words(xargs)
  if wrds\=callargcount then do    ## callargs.0 is may be not set properly
     call writeline '## +++++ macro 'macname' call parameters do not match with template'
     call writeline '## +++++ template 'quote(xargs)
     say '+++++ macro 'macname' call parameters do not match with template'
     say '+++++ template 'quote(xargs)
  end
 ## First take the positional parameters
  keypositional=0
  do k=1 to wrds
     aname = word(xargs, k)
     if pos('=',aname)>0 then do
        if keypositional=0 then keypositional=k
        iterate
     end
     if keypositional>0 then do
        say 'CRX0960E+['time('l')'] positional parameters must not defined after keyword parameters: 'macname||iargs')'
        return '+++ SyntaxError: positional argument follows keyword argument 'macname||iargs')'
     end
     bodyExp = replaceArg(bodyExp, aname, callargs.k)   ## replace macro variable by value in callargs.k
   end
  if keypositional=0 then return bodyExp
## now that we know we have keyword paramters, we need to translate tso-style call back to key= syntax
  do i=1 to callargcount
       callargs.i=tso2func(callargs.i)
  end
## start matching macro definition parm list to calling parameters (keywords only)
  do k=keypositional to wrds        ## run through the macroc header definition
     aname = translate(word(xargs, k),,'=')         ## aname is the keyword definition in the macro header
     adefault=word(aname,2)                        ## 2. word is the default value
     if adefault='' then adefault="''"             ## if not there, use empty string
     aname=word(aname,1)                           ## now isolate parameter name
     do j=1 to wrds                  ## find the appropriate keyword, sequence is free
        if fpos(aname'=',callargs.j,1)>0 then leave    ### +++++++++++ modify  ## use fpos function to search case-insensitive
     end
     aval=callargs.j                       ## identifies the call of the macro
     if pos('=',aval)=0 then aval=adefault ## if keyword is not defined use the default in the macro def part
     else do                               ## if so check if the keyword call is also in the value clause
        aval=translate(aval,,'=')
        aval=subword(aval,2)
     end
     bodyExp = replaceArg(bodyExp, aname, aval)
  end
return bodyExp
/* ------------------------------------------------------------------
 * tso2func: Normalizes TSO-style keyword arguments to function-style key=value syntax.
 * ------------------------------------------------------------------
 */
tso2func: procedure=.string
  arg inparm=.string
  pp1=pos('(',inparm)
  if pp1=0 then return inparm
  pp2=lastpos(')',inparm)
  inparm=overlay('=',inparm,pp1)
  if pp2>1 then inparm=substr(inparm,1,pp2-1)
return strip(inparm)
/* ------------------------------------------------------------------
 * variadic: Expands variadic macros (macros with ... arguments) into repeated generated statements.
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
 * debugArgs: Debug helper to print macro call arguments (usually disabled).
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
 * fetchArguments: Extracts a macro argument list, handling nested parentheses.
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
 /* old Version
replaceArgx: procedure=.string
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
    p = pos(name, str, posn)
  end
return str
*/

/* ------------------------------------------------------------------
 * replaceArg: Replaces a parameter occurrence respecting identifier boundaries
 * also supports NAME## terminator.
 *  - Matches bare  name  only at identifier boundaries
 *  - Also matches   name##   unconditionally and consumes the '##'
 * ------------------------------------------------------------------ */
replaceArg: procedure=.string
  arg str=.string, name=.string, value=.string
  posn = 1
  p = fpos(name, str, posn)
  nlen=length(name)
  do while p > 0
     before = ''
     if p > 1 then before = substr(str, p - 1, 1)

     after1 = substr(str, p + nlen, 1)          /* next 1 char */
     after2 = substr(str, p + nlen, 2)          /* next 2 chars */
  /* ---- Fast path: explicit end-of-param "##" right after name ---- */
     if after2 = '##' then do
        /* replace NAME## with VALUE (consume the hashes) */
        str  = insertatc(value, str, p, nlen+2)
        posn = p + length(value)
        p    = fpos(name, str, posn)
        iterate
     end

     /* ---- Original boundary check: only replace full identifiers ---- */
     verbb = verify(before, alphaN, 'N')  /* 0 => before is [A-Za-z0-9_] */
     verba = verify(after1, alphaN, 'N')  /* 0 => after  is [A-Za-z0-9_] */
     if length(before) = 0 then verbb = 1
     if length(after1)  = 0 then verba = 1

     if verbb = 0 | verba = 0 then do   /* Not a standalone identifier (e.g., nameX or Yname); skip ahead */
        posn = p + 1
        p    = fpos(name, str, posn)   /* BUGFIX: was `p = posn` */
        iterate
     end
     /* ---- Valid standalone `name` ---- */
     str  = insertatc(value, str, p, nlen)   /* C-function */
     posn = p + length(value)
     p    = fpos(name, str, posn)
  end
return str
/* ------------------------------------------------------------------
 * InsertAt: Pure string insert/replace helper (legacy).
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
 * parseArgList: Splits a macro argument list into callargs[] and returns the argument count.
 * ------------------------------------------------------------------
 */
parseArgList: procedure=.int
  arg argstr=.string
  callargs=.string[]
  anum=splitargs(argstr,callargs)
  do j=1 to anum                 ## could also be callargs.0, but might be not set correctly due to higher previous macro call
     callargs.j=strip(callargs.j)
  end
return anum
/* ------------------- the following sequence is part of the C function splitargs
  callargs=.string[]
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
  if strip(token) <> '' then do
     callargs.i = strip(token)
  ##   call templist 'PUT',i,callargs.i      ### +++++++++++ remove
  end
return i
----------------- */
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
 * getvarindx: Returns the index of a pre-compiler variable in the variable table.
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
 * findvar: Checks whether a pre-compiler variable exists (returns index or 0).
 * ------------------------------------------------------------------
 */
findvar: procedure=.string
  arg varname=.string
  varname='{'lower(varname)'}'
  index=getvarindx(varname)
return index
/* ------------------------------------------------------------------
 * getvar: Returns the value of a pre-compiler variable.
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
 * setvar: Sets a pre-compiler variable (stored as {name}).
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
 * quote: Quotes a string value for code generation.
 * ------------------------------------------------------------------
 */
quote: procedure=.string
  arg line=.string, prefix=''
  if prefix \= '' then return prefix'={'line'}'
return "'"line"'"
/* ------------------------------------------------------------------------
 * parsevar: Implements ##PARSE by generating runtime parsing code and variable assignments.
 * ##PARSE command, re-parse tokens from template to receive Variable names
 *   PARSE VAR variable template
 *   PARSE VALUE 'quoted-value' [WITH] template
 *   PARSE VALUE variable [WITH] template
 * ------------------------------------------------------------------------
 */
parsevar: Procedure=.int
  arg lino=.int, parseLine=.string
 ## 1. strip off PARSE VAR variable template or PARSE VALUE 'string'/variable [WITH] template
 ##                1w  2w   3w     4w            1w    2w     3w                4w
 ## 2. strip off PARSE variable template or PARSE 'string'/variable [WITH] template
 ##                1w   2w         3w        1w      2w                         3w
  parseLine=subword(parseLine,2)    ## strip off PARSE command
  parsestmt=parseline               ## save this for adding it later as comment
  ww1=upper(word(parseLine,1))
  uplow=0
  if ww1='UPPER' | ww1='LOWER' then do
     if ww1='UPPER' then uplow=1
     else uplow=2
     parseLine=subword(parseLine,2)
  end
  ppi=pos('/*',parseLine)
  if ppi>1 then parseLine=substr(parseLine,1,ppi-1)
  ppi=pos('##',parseLine)
  if ppi>1 then parseLine=substr(parseLine,1,ppi-1)
  pmode=upper(word(parseLine,1))    ## check if we have a VALUE or VAR clause
  if pmode='VALUE' | pmode= 'VAR' then parseLine=subword(parseLine,2)   ## set behind VALUE/VAR clause

  sstr=substr(parseLine,1,1)
  if sstr="'" | sstr='"' then do
     lhs=Firstword(parseLine)
     parseLine=substr(parseLine,length(lhs)+1)
  end
  else do
     lhs=word(parseLine,1)
     ParseLine=subword(ParseLine,2)
  end
  if fpos('WITH',parseLine,1)=1 then template=subword(ParseLine,2)   ## yes, then drop it too!
  else template=ParseLine
  lhs=strip(lhs)

  template=preCleanTemplate(template)
  template=strip(template)
  template=preCleanTemplate(template)
  k       = 0
  buf     = ''                              /* pending unquoted text */
  WHITESPACE = ' '||'09'x||'0D'x||'0A'x||'0B'x||'0C'x||'A0'x

 /* flush pending unquoted text as a token */
  tokenhi=0
  token=.string[]
  L = length(template)
  i = 1
  do while i <= L
     ch = substr(template, i, 1)
     if pos(ch, WHITESPACE) > 0 then do
        buf=flush(buf,token,tokenhi)
        j = i + 1
        do while j <= L & pos(substr(template, j, 1), WHITESPACE) > 0
           j = j + 1
        end
        tokenhi=tokenhi+1
        token.tokenhi = substr(template, i, j - i)  /* exact run of blanks/tabs/etc */
        i = j
        iterate
     end

    /* quoted literal => single token, quotes included; supports doubled quotes */
    if ch = "'" | ch = '"' then do
       buf=flush(buf,token,tokenhi)
       q     = ch
       qtok  = q
       i     = i + 1
       closed = 0
       do while i <= L
          c = substr(template, i, 1)
          qtok = qtok || c
          if c = q then do
          /* doubled quote => literal quote inside string */
          if i < L & substr(template, i+1, 1) = q then do
             qtok = qtok || q
             i = i + 2
             iterate
          end
          closed = 1
          i = i + 1
          leave
        end
        i = i + 1
      end
      if \closed then do
        say 'Error: unmatched quote in template'
        return -1
      end
      tokenhi=tokenhi+ 1
      token.tokenhi = qtok
      iterate
    end
     /* otherwise accumulate into current unquoted token */
    buf = buf || ch
    i = i + 1
  end

  buf=flush(buf,token,tokenhi)
/* ----------------------------------------------------------------------
 * Classify tokens in token_types
 * ----------------------------------------------------------------------
 */
  j=0
  insert=.string[]
  do i=1 to tokenhi
     quotechr=substr(token.i,1,1)
     if quotechr = "'" | quotechr = '"' then do
        token.i=strip(token.i,,quotechr)       ## drop quotes
     end
     else if quotechr = " " then  token_type.i=2     ## blank seperator
     else if verify(token.i,'0123456789')=0 then nop
     else if (quotechr='+' | quotechr='-' ) & verify(substr(token.i,2),'0123456789+-')=0 then nop
     else if (quotechr='(') then nop    ## token_type.i=6
     else do
        if fpos('parse',token.i,1)>0 then iterate
        j=j+1
        if token.i='.' then iterate
        insert.j=token.i"=_pass_variable_content."j
     end
  end
  insert_count = j
  imax=insert_count*2+15        ## add 15 lines to handle all generated lines, maybe too much, but empty lines will be dropped anyway
  rc= insert_source(lino+1,imax)

   inew=inject2Source(lino+1,0,'/* PARSE 'parseSTMT' */')
   inew=inject2Source(inew+1,3,"_pass_variable.1='' ; _pass_variable_content.1='' /* init array for PARSE function */ ")
   inew=inject2Source(inew+1,3,'_string2Parse='lhs)
   inew=inject2Source(inew+1,3,'_parsetemplate='embed(template))
   if pos(' trimparse',cflags)>0 then inew=inject2Source(inew+1,3,'call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,1,'uplow)
   else inew=inject2Source(inew+1,3,'call parse _string2parse,_parsetemplate,_pass_variable,_pass_variable_content,0,'uplow)
  if pos(' parse',cflags)>0 then do
     inew=inject2Source(inew+1,3,'say ">> "copies("-",72)')
     if substr(lhs,1,1)='x' then lhs=' 'lhs
     upmode=''
     if uplow=1 then upmode='UPPER '
     else if uplow=2 then upmode='LOWER '
     inew=inject2Source(inew+1,3,'say ">> PARSE Statement: 'upmode'["'lhs'" -- VAR/VALUE WITH --> "' embed(template) '"]"')
     inew=inject2Source(inew+1,3,'say ">> PARSE STRING   : ["'lhs'"]"')                 ## value function doesn't help, if the parm is a variable
     inew=inject2Source(inew+1,3,'say ">> PARSE TEMPLATE : ["'embed(template)'"]"')
  end
  inew=inject2Source(inew+1,0,'## ---------- set parse variables ----------')
  do j=1 to insert_count
     inew=inject2Source(inew+1,3,insert.j)
     if pos(' parse',cflags)>0 then do
        inew=inject2Source(inew+1,3,'say ">> PARSE Result   : "_pass_variable.'j'"=["_pass_variable_content.'j'"]"')
     end
  end
  inew=inject2Source(inew+1,0,'## ---------- parse variables set ----------')
return token.0

/* ------------------------------------------------------------------
 * flush: Flushes buffered text into token[] as a single token.
 * ------------------------------------------------------------------
 */
flush: procedure=.string
  arg buf=.string, expose token=.string[], expose tokenhi=.int
  if buf = '' then return ''
  tokenhi=tokenhi+1
  token.tokenhi = buf
  buf = ''
return buf
/* ------------------------------------------------------------------
 * embed: Quotes a string using ' or \" depending on content.
 * ------------------------------------------------------------------
 */
embed: procedure=.string
  arg strx=.string
  if pos("'",strx,1)>0 then return '"'strx'"'
return "'"strx"'"

/* -------------------------------------------------
 * preCleanTemplate
 *  - Preserve inner content of quoted literals exactly
 *  - Remove whitespace only if it directly neighbors a quote
 *  - No suppression around numbers, +/-, punctuation, etc.
 *  - Supports doubled quotes inside literals ('' and "")
 * -------------------------------------------------
 */
preCleanTemplate: procedure=.string
  arg template=.string

  WHITESPACE = ' '||'09'x||'0D'x||'0A'x||'0B'x||'0C'x||'A0'x
  out = ''
  L = length(template)
  i = 1
  last_out_char = ''     /* track last emitted char for "prev is quote" check */
  do while i <= L
    ch = substr(template, i, 1)
    /* ---- quoted literal: copy EXACTLY as-is, including quotes ---- */
    if ch = "'" | ch = '"' then do
      q    = ch
      qtok = q
      i    = i + 1
      closed = 0
      do while i <= L
        c = substr(template, i, 1)
        qtok = qtok || c
        if c = q then do
          /* doubled quote -> keep both quotes and continue */
          if i < L & substr(template, i+1, 1) = q then do
            qtok = qtok || q
            i = i + 2
            iterate
          end
          closed = 1
          i = i + 1
          leave
        end
        i = i + 1
      end
      if \closed then return template  /* unmatched quote: leave template as-is */

      out = out || qtok
      last_out_char = q               /* closing quote is the last emitted char */
      iterate
    end
    /* ---- whitespace run ---- */
    if pos(ch, WHITESPACE) > 0 then do
      j = i + 1
      do while j <= L & pos(substr(template, j, 1), WHITESPACE) > 0
        j = j + 1
      end
      /* peek next non-WS char */
      nextch = ''
      if j <= L then nextch = substr(template, j, 1)

      /* suppress only if whitespace touches a QUOTE on either side */
      if last_out_char = "'" | last_out_char = '"' | nextch = "'" | nextch = '"' then do
        i = j
        iterate
      end

      /* otherwise keep the whitespace as-is */
      out = out || substr(template, i, j - i)
      i = j
      last_out_char = ' '             /* non-quote marker */
      iterate
    end
    /* ---- ordinary char ---- */
    out = out || ch
    i = i + 1
    last_out_char = ch
  end
  return out

/* ------------------------------------------------------------------
 * FirstWord: Extracts the first token from a string, respecting quoted string rules.
 * ------------------------------------------------------------------
 */
FirstWord: Procedure=.string
  arg line=string
  line = strip(line)
  i = 1
  quoted = ''
  llen=length(line)
  do while i <= llen
     qchar = substr(line,i,1)
     if qchar = '"' | qchar = "'" then do
        i = i + 1
        part = ''
        do while i <= llen
           c = substr(line,i,1)
           if c = qchar then do
              if substr(line,i+1,1) \= qchar then leave   /* search for quote */
              part = part || qchar
              i = i + 2
              iterate
           end
           part = part || c
           i = i + 1
        end
        quoted = quoted || part
        i = i + 1
     end
     else if qchar = ' ' then leave
     else leave
  end
return "'"quoted"'"

/* ------------------------------------------------------------------
 * inject2Source: Writes a prepared line into a reserved source slot and marks it as generated.
 * inject a new line into source code, the new lines must have been
 * prepared prior to the call
 * returns the line number used
 * ------------------------------------------------------------------
 */
inject2Source: procedure=.int
  arg lnr=.int,inc=.int,line=.string
  if source.lnr \='' & pos('/* reserved',source.lnr)=0 then say "++++++++++++ parse overflow at line "lnr' contains 'line
  if inc>0 then source.lnr='   'line
  else source.lnr=line
  stype.lnr='R'
return lnr
/* ------------------------------------------------------------------
 * linkerInfo: Writes an include file containing imported module names for linking.
 * ------------------------------------------------------------------
 */
linkerInfo: procedure
  arg outfile=.string, imported=.string
  userpath=translate(outfile,,'/\')
  wrds=words(userpath)
  member=word(userpath,wrds)
  fmember=translate(member,,'.')
  member=word(fmember,1)
  wlast=wordindex(userpath,wrds)
  userpath=substr(outfile,1,wlast-1)
  userpath=translate(userpath,'/','/\')
  linkfile=userpath||member'.inc'
  linker.1=substr(imported,2)
  call writeall linker,linkfile,-1
  if verbose then say 'CRX0510I ['time('l')'] Library import names passed to 'linkfile
return
/* ------------------------------------------------------------------
 * normalizePath: resolves ../, ./ and slashes in mixed paths to normalised UNIX-style
 * Example: normalizePath("C:\\Users\\Peter/../Docs/./file.txt") -> "C:/Users/Docs/file.txt"
 * ------------------------------------------------------------------
 */
normalisePath: procedure=.string
  arg rawpath=.string

  path = translate(rawpath, '/', '\')    /* Convert backslashes to forward slashes */
  /* Split path into components by '/' */
  components=.string[]
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
/* ---------------------------------------------------------------
 * CommentScan: Scans a line to update block-comment nesting and reports comment intersection.*
 * CommentScan(lino, depth)
 * Returns: "depth_out hit"
 * hit=1 if ANY portion of this line is in a /* */ block comment
 * --------------------------------------------------------------- */
CommentScan: procedure=.int
  arg lino=.int, depth=.int, expose hit=.int
  line = source[lino]
  pos  = 1
  hit = (depth > 0)          /* already inside a comment at BOL */
  do forever
     po = qpos('/*', line, pos)
     pc = qpos('*/', line, pos)
     if po = 0 & pc = 0 then leave
     /* choose earliest marker */
     if pc = 0 | (po > 0 & po < pc) then do  /* opening marker */
        hit = 1
        depth = depth + 1
        pos = po + 2
     end
     else do  /* closing marker: the region up to here is comment iff depth>0 */
        if depth > 0 then hit = 1
        if depth > 0 then depth = depth - 1
        pos = pc + 2
     end
  end
  return depth
/* ------------------------------------------------------------------
 * rxppinit: Initializes RXPP global state, arrays, counters, module name, system path, and default flags.
 * ------------------------------------------------------------------
 */
rxppinit: procedure=.string
  arg rexxname=.string,maclib=.string
  alphaN='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_'
  source=.string[]
  stype.1='R'
  macros_mname=.string[]
  macros_mspace=.int[]
  macros_varname=.string[]
  macros_varvalue=.string[]
  included_files=.string[]
  stem_tempvar=.string[]
  imported_funcs=""
  globaldef=''
  lino=0
  select_count=0
  stem_scount=0
  outbuf=.string[]
  mexpanded=0
  rxmodule=translate(rexxname,,'/\')
  wrds=words(rxmodule)
  rxmodule=word(rxmodule,wrds)
  syspath=translate(maclib,,'/\')
  wrds=words(syspath)
  wlast=wordindex(syspath,wrds)
  syspath=substr(maclib,1,wlast-1)
  call setvar 'rxpp_rexx',rxmodule
  rxmodule=word(translate(rxmodule,,'.'),1)
  cflags=' ndef nset svars siflink n1buf n2buf n3buf nvars nparse ndotisstem'
  call setvar 'cflags',cflags
  printgen_flags='all'
  call setvar 'printgen',printgen_flags
  call setvar 'rxpp_date',date()' 'time()
return strip(syspath)