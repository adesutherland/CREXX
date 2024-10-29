file=arg(1)
/* file="C:\temp\CREXX\fpool.txt" */
/* -------------------------------------------------------------------------------------
 * Create a PLUGIN Skeleton based on a plugin definition
 * -------------------------------------------------------------------------------------
 */
func=0
oldfunc=''
lci=0
do while lines(file)>0
   line=strip(linein(file))
   lci=lci+1
   line.lci=line
   ppi=pos('//',line)
   if ppi>0 then line=substr(line,1,ppi-1) /* clear comment from line */
   if line='' then iterate                 /* drop empty lines */
   if translate(substr(line,1,8))='FUNCTION' then do
	  if func>0 then call poolgen    /* generate the previous function definition */
      func=func+1
	  oldfunc=line                   /* save function name */
   end
   interpret line        /* now the line should contain just variable settings, execute it */
end

call PoolGen             /* create outstanding (last) function */
call pluginCreate        /* create the PLUGIN Skeleton         */
exit
/* -------------------------------------------------------------------------------------
 * Create the PLUGIN based on its definition
 * -------------------------------------------------------------------------------------
 */
pluginCreate:
  call cmtblock 'PLUGIN: 'pool
  call genline 0,'#include <stdio.h> '
  call genline 0,'#include <stdlib.h>'
  call genline 0,'#include <unistd.h>     // For POSIX systems (Linux/macOS)'
  call genline 0,'#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file'

  do ai=1 to addprc.0
     call cmtblock 'FUNCTION: 'addfunc.ai'('substr(addpassing.ai,2)')'
     call genline 0,'PROCEDURE('addfunc.ai') {'
     pnum=words(addparms.ai)/2
     call genline 2,'if( NUM_ARGS != 'pnum') RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "'addfunc.ai' requires 'pnum'  parameters")'
     call fetchparms ai,addparms.ai,addfunc.ai
     call genline 0,'ENDPROC'
     call genline 0,'}'
  end
  call cmtblock 'ADDPROC definitions'
  call genline 0,'LOADFUNCS // function   pool.function                   return type       parameters variable=.type ...'
  do ai=1 to addprc.0
     call genline 2,addprc.ai
  end
  call genline 0,'ENDLOADFUNCS'
  call cmtblock 'Generated from the following definition on 'date()' at 'time(),'noCMTEND'
  do ai=1 to lci
     call genline 3,line.ai
  end
   call genline 0,' */'
return
/* -------------------------------------------------------------------------------------
 * Sets up Variables from its definition
 * -------------------------------------------------------------------------------------
 */
poolgen:
  parse var function 1 fname'('ftype')'
  fname=lower(fname)
  ftype=lower(ftype)
  if datatype(addprc.0) <> 'NUM' then addprc.0=0
  addi=addprc.0+1
  if substr(ftype,1,1)='.' then nop
  else ftype='.'ftype
  fpool='"'left(pool'.'fname'",',25)
  addproc='ADDPROC('left(fname,12)','fpool'"b",  'left('"'ftype'",',16)
  xparms=''
  cparms=''
  addpassing.addi=''
  do until passing=''
     parse var passing 1 pparm'('ptype')' passing
	 pparm=lower(pparm)
	 ptype=lower(ptype)
     if substr(ptype,1,1)='.' then ptype=substr(ptype,2)
     xparms=xparms','pparm'=.'ptype
     cparms=cparms' 'pparm' 'ptype
	 addpassing.addi=addpassing.addi','pparm
  end
  addprc.0=addi
  addprc.addi=addproc'"'substr(xparms,2)'");'
  addfunc.addi=fname
  addftype.addi=translate(substr(ftype,2))
  addparms.addi=cparms
return
/* -------------------------------------------------------------------------------------
 * Sets up C variable definition of a Procedure
 * -------------------------------------------------------------------------------------
 */
fetchparms:
  parse arg axi,inparms,xname
  ak=0
  isstring=0
  do aj=1 to words(inparms) by 2
     intype=translate(word(inparms,aj+1))
	 if substr(intype,1,1)='.' then intype=substr(intype,2)
     if intype='STRING'     then call genline 2,'char *'word(inparms,aj)'=GETSTRING(ARG('ak'));'
	 else if intype='INT'   then call genline 2,'int   'word(inparms,aj)'=GETINT(ARG('ak'));'
     else if intype='FLOAT' then call genline 2,'float 'word(inparms,aj)'=GETFLOAT(ARG('ak'));'
	 if intype='STRING'     then isstring=1
 	ak=ak+1
  end
  if addftype.axi='STRING'     then call genline 2,'char  preturn[255];  // change return size accordingly'
  else if addftype.axi='INT'   then call genline 2,'int   preturn;'
  else if addftype.axi='FLOAT' then call genline 2,'float preturn;'
  call genline 8,'// -------------------------------------------------'
  call genline 8,'// add processing code for the above variables here '
  if isstring=1 then do
     call genline 8,'// useful functions: '
	 call genline 8,'//     strncpy(dest,source+,variable(+offset),length) requires \0 termination'
     call genline 8,'//     strcpy(dest,source+,variable(+offset)) does not require \0 termination'
	 call genline 8,'//     strcmp(source,"constant"/variable)==0  compare, 0 if equal, <0 lt, >0 gt'
	 call genline 8,'//     fpos= (long) strstr(haystack[+offset],needle)  find offset '
     call genline 8,'// ...'
  end
  else do
     call genline 8,'// ...'
     call genline 8,'// ...'
  end
  call genline 8,'// -------------------------------------------------'
  if addftype.axi='STRING'     then call genline 2,'RETURNSTR(preturn);'
  else if addftype.axi='INT'   then call genline 2,'RETURNINT(preturn);'
  else if addftype.axi='FLOAT' then call genline 2,'RETURNFLOAT(preturn);'
return
/* -------------------------------------------------------------------------------------
 * Write a comment block
 * -------------------------------------------------------------------------------------
 */
cmtblock:
  call genline 0,'/* 'copies('-',70)
  call genline 0,' * 'arg(1)
  call genline 0,' * 'copies('-',70)
  if arg(2)='' then call genline 0,' */'
return
/* -------------------------------------------------------------------------------------
 * Output line
 * -------------------------------------------------------------------------------------
 */
genline:
  say copies(' ',arg(1))arg(2)
return