/* rexx
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
 */           

options levelb dashcomments
import rxfnsb
import sysinfo

main: procedure = .int
arg fn = .string[]
module = .string[]

/* when there are no commandline arguments found, display help */
if fn[0]=0 then do
  call help
  exit
end


/* figure out the path for libraries etc */
rxpath=''
env_wanted='CREXX_HOME'
assembler getenv rxpath,env_wanted
if rxpath='' then do
  rxpath=getLoadPath()
  lastSegment=lastpos('/',rxpath)
  rxpath=substr(rxpath,1,lastSegment-1)
end
rxpath=rxpath'/' /* to avoid having to start -l with a slash */

/* defaults for options */
native=0;version=0;help=0;compile=0;filename='';filenames='';verbose=0
execute=1;linking=0;compile=1;optimize=1

libraries='/lib/rxfnsb'

modulenumber=1
/* loop through arguments and find options and program files */
do i=1 to fn.0
  if left(fn.i,1)<>'-' then
    do /* it is not a flag but a filename */
      filename=fn.i
      filenames=strip(filenames) strip(filename)
    end
  else
    do
      if left(fn.i,2) = '--' then fn.i=substr(fn.i,2)
      if fn.i = '-help' then call help
      if fn.i = '-noexec' then execute=0
      if fn.i = '-native' then native=1
      if fn.i = '-version' then version=1
      if fn.i = '-verbose' then verbose=1
      if fn.i = '-verbose0' then verbose=0
      if fn.i = '-verbose1' then verbose=1
      if fn.i = '-verbose2' then verbose=2
      if fn.i = '-verbose3' then verbose=3
      if fn.i = '-noexec' then execute=0
      if fn.i = '-nooptimize' then optimize=0
      if fn.i = '-nocompile' then compile=0
      if left(fn.i,2)= '-l' then do
	if left(fn.i,1)=' ' then do
	  fn.i=fn.i||fn.i+1
	  fn.i+1=''
	end
	lastSlash = lastpos('/',fn.i)
	libs = libs';'rxpath||substr(fn.i,3) /* for rxvm execution */
	module[modulenumber] = substr(fn.i, lastSlash + 1)
	say '---->' module[modulenumber]
	libraries = libraries';'rxpath||substr(fn.i,3,lastSlash-2) /* for rxc compile */
	modulenumber = modulenumber+1
      end
    end
end -- do i


if version then call logo
if verbose>1 then say 'using rxpath:' rxpath

lpath = libraries

if verbose>1 then say 'using lpath :' lpath

/* Output Arrays for command output */
out = .string[]
err = .string[]

do i=1 to words(filenames)
  filename=word(filenames,i)
  /* see if there is a .rxpp file to run the preprocessor against */
  
  
  rxcmd = 'rxc -i' rxpath||lpath filename
  if verbose>1 then
    do
      if compile then say 'rxc command:' rxcmd
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
    if RC = 0 then res='OK'
    else res = RC
    if compile then say '[ 'res' ] rxc     - Compiled' filename
    end
  if RC>0 then exit
  'rxas' filename
  if verbose then do
    if RC = 0 then res='OK'
    else res = RC
    if compile then say '[ 'res' ] rxas    - Assembled' filename
  end
  if RC>0 then exit
  
  modules = translate(libs,' ',';')
  if native then do
    pack_cmd = 'rxcpack' filename rxpath'/lib/rxfnsb/library' modules
    if verbose>1 then
    do
      say 'rxcpack command:' pack_cmd
      end
    pack_cmd
    if verbose then do
      if RC = 0 then res='OK'
      else res = RC
      say '[ 'res' ] rxcpack - C-Packed' filename
    end

    cc_command = 'gcc -O3 -DNDEBUG -o' filename ,
      '-L'rxpath'/interpreter',
      '-L'rxpath'/interpreter/rxvmplugin',
      '-L'rxpath'/interpreter/rxvmplugin/rxvmplugins/db_decimal',
      '-L'rxpath'/interpreter/rxvmplugin/rxvmplugins/mc_decimal',
      '-L'rxpath'/machine',
      '-L'rxpath'/avl_tree',
      '-L'rxpath'/rxpa',
      '-L'rxpath'/platform',
	'-lrxvml',
	'-lrxpa',
	'-lmachine',
	'-lavl_tree',
	'-lplatform',
        '-lm -lrxvmplugin',
	  rxpath'/interpreter/rxvmplugin/rxvmplugins/mc_decimal/rxvm_mc_decimal_manual.a ',
	    rxpath'/interpreter/rxvmplugin/rxvmplugins/mc_decimal/libdecnumber.a ',
	 '-Wl,-force_load,"'rxpath'""',
	  filename'.c'
    cc_command
    if verbose>1 then
      do
	say 'cc compile command:'
	say cc_command
	end
    
    if verbose then
      do
	if RC = 0 then res='OK'
	else res = RC
	say '[ 'res' ] gcc     - C-Compiled' filename
	end
    if RC>0 then exit  
    end
  else do
    ex_command = 'rxvme' filename modules

    if verbose>1 then do
      if execute then say 'crexx executes:' ex_command
      if execute=0 then say 'crexx does not execute because of --noexec'
    end
    if execute then ex_command
  end
end   -- do i

  return 0
  
help: procedure 
call logo
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
say '-verbose[0-3]    -- report on progress; default verbose0'
say '-l[library path] -- use import library'
say
say 'all options can also be prefixed with --'
return

logo: procedure
rversion = ''
/* note that for brevity we use an assembler directive to get to the version */
assembler rxvers rversion
say 'cRexx compiler driver' rversion
say 'Copyright (c) Adrian Sutherland 2021,'left(date('j'),4)'. All rights reserved.'
say 'Copyright (c) RexxLA 2021,'left(date('j'),4)'. All rights reserved.'
return

/*
  /usr/bin/cc -O3 -DNDEBUG -arch arm64 -Wl,-search_paths_first -Wl,-headerpad_max_install_names  bin/CMakeFiles/crexx.dir/crexx.c.o -o bin/crexx  -Wl,-force_load,"/Users/rvjansen/apps/crexx_release/lib/plugins/sysinfo/rx_sysinfo_static.a"  interpreter/librxvml.a  rxpa/librxpa.a  machine/libmachine.a  avl_tree/libavl_tree.a  platform/libplatform.a  -lm  interpreter/rxvmplugin/librxvmplugin.a  interpreter/rxvmplugin/rxvmplugins/mc_decimal/rxvm_mc_decimal_manual.a  interpreter/rxvmplugin/rxvmplugins/mc_decimal/libdecnumber.a
  */

