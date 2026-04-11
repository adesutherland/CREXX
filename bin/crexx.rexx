/*
 * crexx compiler driver - copyright RexxLA, rvjansen 2021-2025
 * crexx is an executable which controls the phases of the crexx
 * processor, compiles, assembles and executes rexx programs.
 * It optionally compiles the program to a native executable,
 * for which it needs an operable gcc or clang compiler.
 * For this, the rxcpacker phase generates a c source file.
 *
 * The options processor can receive files and options in any
 * sequence, where options need at least one dash at the left
 * of the option string. This determines if the strings fed to
 * it are options or filenames.
 *
 * if a filename has a suffix of .rxpp, the crexx precompiler
 * is invoked to perform its magic - it produces a .rexx
 * which then is compiled and executed, or compiled to an
 * object file which is linked to the interpreter to
 * form a native executable.
 *
 */           

 options levelb comments_dash
 import rxfnsb
 import sysinfo
 
 /* procedure main is what is run when crexx starts execution */
 main: procedure = .int
 arg fn = .string[]
 module = .string[]
 
 /* defaults for options for this program */
 native=0;version=0;help=0;compile=0;filename='';filenames='';verbose=0
 execute=1;linking=0;compile=1;optimize=1;nocolor=0;keep=0;decimal=1
 binfiles='';lastfile=0
 
 esc             = '1b'X
 ANSI_RESET      = '[0m'
 ANSI_GREEN      = '[32m'
 ANSI_BLUE       = '[34m'
 ANSI_RED        = '[31m'
 ANSI_YELLOW     = '[33m'
 ANSI_LINE_CLEAR = '[2K'
 ANSI_LINE_UP    = '[1A'
 
 /* when there are no commandline arguments found, display help */
 if fn[0]=0 then do
   ret = help(nocolor)
   exit
 end
 
 /* determine the path for libraries etc */
 rxpath=''
 env_wanted='CREXX_HOME'
 /* honour CREXX_HOME environment variable when set */
 assembler getenv rxpath,env_wanted
 
 /* if not set, then */
 rexx_version = ''
 assembler rxvers rexx_version
 dirsep = '/'
 platform = word(rexx_version,1)
 if  platform = 'windows' then dirsep = '\'
 if rxpath='' then do
   rxpath=getLoadPath()
   lastSegment=lastpos(dirsep,rxpath)
   rxpath=substr(rxpath,1,lastSegment-1)
 end
 else do
   if verbose>3 then say 'relpath set from environment CREXX_HOME'
 end
 
 rxpath=rxpath||dirsep /* to avoid having to start -l with a slash */

 libraries='bin'

 modulenumber=1
 /* loop through arguments and find options and program files */
 libs = ""
 do i=1 to fn.0
   if left(fn.i,1)<>'-' then
     do /* it is not a flag but a filename */
       filename=fn.i
       filenames=strip(filenames) strip(filename)
     end
     else
       do
	 /* allow for single- and double dash options */
       if left(fn.i,2) = '--'  then fn.i=substr(fn.i,2)
       if fn.i = '-help'       then ret = help(nocolor)
       if fn.i = '-noexec'     then execute=0
       if fn.i = '-native'     then native=1
       if fn.i = '-version'    then version=1
       if fn.i = '-verbose'    then verbose=1
       if fn.i = '-verbose0'   then verbose=0
       if fn.i = '-verbose1'   then verbose=1
       if fn.i = '-verbose2'   then verbose=2
       if fn.i = '-verbose3'   then verbose=3
       if fn.i = '-verbose4'   then verbose=4
       if fn.i = '-nocolor'    then nocolor=1
       if fn.i = '-nocolour'   then nocolor=1
       if fn.i = '-nooptimize' then optimize=0
       if fn.i = '-keep'       then keep=1
       if fn.i = '-nokeep'     then keep=0
       if fn.i = '-nodecimal'  then decimal=0
       if fn.i = '-decimal'    then decimal=1

       if left(fn.i,2)= '-l' then do
	 if left(fn.i,1)=' ' then do
	   fn.i=fn.i||fn.i+1
	   fn[i+1]=''
	 end
	 lastSlash = lastpos('/',fn.i)
	 libs = libs';'rxpath||'bin'dirsep||substr(fn.i,3) /* for rxvm execution */
	 module[modulenumber] = substr(fn.i, lastSlash + 1)
	 -- say '>' module[modulenumber]
	 libraries = libraries';'rxpath||substr(fn.i,3,lastSlash-2) /* for rxc compile */
	 modulenumber = modulenumber+1
       end
     end
   end

   declib = ''
   if \decimal then declib = '-p rxvm_db_decimal'
   if verbose>1 then call logo nocolor  

   decstat = rxpath'bin/rxvm_mc_decimal_manual.a'
   if \decimal then decstat =  rxpath'bin/rxvm_db_decimal_manual.a'
   
   if verbose>2 then     do
     call banner
     say 'INVOCATION OPTIONS'
     say 'Options in effect:'
     if \color  then say '  COLOR'
     else say 'NOCOLOR'
     if decimal  then say '  DECIMAL'
     else say 'NOCDECIMAL'
       
     if execute then say '  EXEC'
     else say 'NOEXEC'
     if native  then say '  NATIVE'
     else say 'NONATIVE'
     if \keep  then say '  KEEP'
     else say 'NOKEEP'
     if \optimize  then say 'NOOPTIMIZE'
     else say '  OPTIMIZE'

     say 'VERBOSE' verbose
     -- call formfeed
     say;say;say
     end
       
if version then call logo nocolor

    
  if verbose>1 then say esc||ANSI_GREEN'using relpath   :'esc||ANSI_RESET rxpath

lpath = libraries

if verbose>1 then say esc||ANSI_GREEN'using lpath     :'esc||ANSI_RESET lpath

/* Output Arrays for command output */
out = .string[]
err = .string[]

/* see if there is a .rxpp file to run the preprocessor against        */
/* while we have no exists(file) function, specify the .rxpp extension */
do i=1 to words(filenames)
  filename=word(filenames,i)
  if i=words(filenames) then lastfile=1
    dotpos = pos('.',filename)
  if dotpos > 0 then do
    if right(filename,4) = 'rxpp'
    then do /* run the preprocessor */
      filename = left(filename,dotpos-1)
      /* print the file when verbose ibm style output is requested */
    if verbose>3 then do
      call printFileToSTDout filename'.rxpp'
    end
    
    'rxpp -i' filename'.rxpp -o' filename'.rexx -m 'rxpath'bin/maclib.rexx -verbose0'
    if verbose then do
      if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
	else res = esc||ANSI_RED||RC||esc||ANSI_RESET
	  if compile then say '[ 'res' ] rxpp     - Preprocessed   ' esc||ANSI_BLUE||filename'.rxpp'||esc||ANSI_RESET
    end
  if RC>0 then exit

    end
  end
  /* if all is well, we now have a .rexx ready for compilation    */  

  /* print the file when verbose ibm style output is requested */
    if verbose>3 then do
      call printFileToSTDout filename'.rexx'
    end
  optiflag=''; if optimize=0 then optiflag= '-n'
  rxcmd = rxpath'bin'dirsep'rxc' optiflag '-i' rxpath||lpath filename
  if verbose>1 then
    do
      if compile then say esc||ANSI_GREEN'rxc command     :' rxcmd
	if compile=0 then
	  do
	  say 'rxc does not compile due to --nocompile option'
	  RC = 0
	end
    end
  if compile then address cmd rxcmd output out error err /* Note that cmd has no meaning currently */
  do j = 1 to out.0
    say '> rxc output:' out.j
  end
  do j = 1 to err.0
    say '> rxc error: ' err.j
  end
  if verbose then do
    if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
    else res = esc||ANSI_RED||RC||esc||ANSI_RESET
    if compile then say '[ 'res' ] rxc      - Compiled       ' esc||ANSI_BLUE||filename'.rexx'||esc||ANSI_RESET
    end
  if RC>0 then exit


/* print the file when verbose ibm style output is requested */
    if verbose>3 then do
      call printFileToSTDout filename'.rxas'
    end

    binfile = chop_suffix(filename)
    binfiles = binfiles binfile
    asmcmd = rxpath'bin'dirsep'rxas' optiflag '-o' binfile binfile
    address cmd asmcmd output out error err
  if verbose then do
    if verbose>1 then say esc||ANSI_GREEN||'rxas command    :'   asmcmd
    if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
    else res = esc||ANSI_RED||RC||esc||ANSI_RESET
    if compile then say '[ 'res' ] rxas     - Assembled      ' esc||ANSI_BLUE||filename'.rxas'||esc||ANSI_RESET
  end
  if RC>0 then exit

  modules = translate(libs,' ',';')
  
  if verbose>2 then call banner

  if native then do
    forces = ''
    loop f=1 to words(modules)
      forces = forces '-Wl,-force_load,'word(modules,f)'_static.a'
    end
    pack_cmd = rxpath'bin'dirsep'rxcpack' filename rxpath'bin/library'
    if verbose>1 then
    do
      say esc||ANSI_GREEN'rxcpack command :'esc||ANSI_RESET pack_cmd
      end
    address system pack_cmd
    if verbose then do
      if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
      else res = esc||ANSI_RED||RC||esc||ANSI_RESET
      say '[ 'res' ] rxcpack  - C-Packed ' esc||ANSI_BLUE||filename||esc||ANSI_RESET
    end

    cc_command = ,
    'gcc -O3 -DNDEBUG -Wl,-search_paths_first -o' filename ,
    '-Wl,-headerpad_max_install_names ',
    '-L'rxpath'bin' ,
    '-lrxvml',
    '-lrxpashim',
    '-lrxvmplugin',
    '-lplatform',
    '-ldecnumber',
    '-lavl_tree',
    '-lrxpa',
    '-lm',
    decstat ,
    forces,
    filename'.c'

    address system cc_command
    if verbose>1 then
      do
	say 'cc compile command:'
	say cc_command
	end
    
    if verbose then
      do
	if RC = 0 then res='OK'esc||ANSI_RESET
	else res = esc||ANSI_RED||RC||esc||ANSI_RESET
	say '[ 'res' ] gcc       - C-Compiled' esc||ANSI_BLUE||filename||esc||ANSI_RESET
	end
    if RC>0 then exit  
    end
  else do
    ex_command = rxpath'bin'dirsep'rxvme' declib binfiles modules

    if verbose>1 then do
      if execute then say 'crexx executes:' ex_command
      if execute=0 then say 'crexx does not execute because of --noexec'
    end
    if lastfile=1 then if execute then address system ex_command
  end
end   -- do i

  return 0

/*----------------------------------------------------------------------*/
help: procedure = .string
arg nocolor = .string  
call logo nocolor 
say
say 'Arguments are: in_file_specification... [--option]...'
say
say 'The following options are available:'
say
say '-help            -- display (this) help info'
say '-version         -- display the version number'
say '-exec            -- execute (default)'
say '-compile         -- compile to rxbin (default)'
say '-native          -- compile to native executable; implies noexec; default nonative'
say '-verbose[0-4]    -- report on progress; default verbose0'
say '-colo[u]r        -- use colo[u]r (default)'
say '-keep            -- keep .rxas source (default nokeep)'
say '-decimal         -- use decimal arithmetic'
say '-l[library path] -- use import library'
say
say 'all options can also be prefixed with --'
say 'all options can be prefixed with NO for the inverse value'
return 'help done'

/*----------------------------------------------------------------------*/
logo: procedure
arg nocolor = .string
rversion = ''
/* note that for brevity we use an assembler directive to get to the version */
assembler rxvers rversion
rvers = word(rversion,1) word(rversion,2) word(rversion,4)
esc = '1b'x
if nocolor then do
  say 'CREXX compiler driver' rvers
end
else do
  say esc'[33mCREXX compiler driver' esc'[34m'rvers esc'[0m'
end
say 'Copyright (c) Adrian Sutherland 2021,'left(date('j'),4)'. All rights reserved.'
say 'Copyright (c) RexxLA 2021,'left(date('j'),4)'. All rights reserved.'
return

/*----------------------------------------------------------------------*/
printarray: procedure = .string
arg toprint = .string[]
loop i=1 to toprint.0
  say toprint.i
end
return 'printed'

  
banner: procedure
esc = '1b'x
say esc'[34m'||'======> RexxLA cREXX compiler release 0.01                          'date() time() esc'[0m'
say;say

formfeed: procedure
say x2c('0C')

printFileToSTDout: procedure
arg toread = .string
call banner
say '  LINENUM' copies('----+',15)
i=1
do while lines(toread)
  say '   'right(i,6,'0') linein(toread)
  i=i+1
end
call lineout toread

deleteFiles: procedure = .string
arg filename = .string

-- when linux or macos
'rm' filename'.rxas'

-- when windows

-- cmd.exe

-- terminal/powershell

return 'done'

chop_suffix: procedure = .string
arg fn = .string
-- we want to use parse when it is available
lp=lastpos('.',fn)
if lp >0 then
  do
    return left(fn,lp-1)
  end
  else do
    return fn
  end
  return 'error in chop_suffix'
  
