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
arg fn = .string[]

/* when there are no commandline arguments found, display help */
if fn[0]=0 then do
  call help
  exit
end

/* defaults for options */
native=0;version=0;help=0;compile=0;filename='';filenames='';verbose=0
execute=1;linking=0;libraries=''

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
      if fn.i = '--noexec' then execute=0
      if left(fn.i,2)= '-l' then libraries = libraries';'substr(fn.i,3)
    end
end -- do i

/* figure out the path for libraries etc */
rxpath=''
env_wanted='CREXX_HOME'
assembler getenv rxpath,env_wanted

if version then call logo
if verbose>1 then say 'using CREXX_HOME:' rxpath

/*
 * here we are using the rxvm instead of the rxvme
 * and have to specify the functions and libraries
 * in the responsefile
 */
lpath = '/lib/rxfnsb'libraries

if verbose>1 then say 'crexx Library path:' rxpath||lpath

/* Output Arrays for command output */
out = .string[]
err = .string[]

do i=1 to words(filenames)
  filename=word(filenames,i)
  rxcmd = 'rxc -i' rxpath||lpath filename
  if verbose>1 then
    do
      say 'rxc command:' rxcmd
      end
  address cmd rxcmd output out error err /* Note that cmd has no meaning currently */
  do j = 1 to out.0
    say '> rxc output:' out.j
  end
  do j = 1 to err.0
    say '> rxc error: ' err.j
  end
  if verbose then do
    if RC = 0 then res='OK'
    else res = RC
    say '[ 'res' ] rxc     - Compiled' filename
    end
  if RC>0 then exit
  'rxas' filename
  if verbose then do
    if RC = 0 then res='OK'
    else res = RC
    say '[ 'res' ] rxas    - Assembled' filename
  end
  if RC>0 then exit
  
  modules = translate(libraries,' ',';')
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
      say 'crexx executes:' ex_command
      end
    ex_command
    end
  end   -- do i
    
help: procedure 
call logo
say
say 'Arguments are: in_file_specification... [--option]...'
say
say 'The following options are available:'
say
say '-help           -- display (this) help info'
say '-version        -- display the version number'
say '-exec           -- execute (default)'
say '-native         -- build native executable; implies noexec; default nonative'
say '-verbose[0-3]   -- report on progress; default verbose0'
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


/* for decimal: gcc -O3 -DNDEBUG -o we -L/Users/rvjansen/apps/crexx_release/interpreter -L/Users/rvjansen/apps/crexx_release/interpreter/rxvmplugin -L/Users/rvjansen/apps/crexx_release/interpreter/rxvmplugin/rxvmplugins/db_decimal -L/Users/rvjansen/apps/crexx_release/interpreter/rxvmplugin/rxvmplugins/mc_decimal -L/Users/rvjansen/apps/crexx_release/machine -L/Users/rvjansen/apps/crexx_release/avl_tree -L/Users/rvjansen/apps/crexx_release/rxpa -L/Users/rvjansen/apps/crexx_release/platform -lrxvml -lrxpa -lmachine -lavl_tree -lplatform -lm -lrxvmplugin -L/Users/rvjansen/apps/crexx_release/interpreter/rxvmplugin/rxvmplugins/mc_decimal -ldecnumber /Users/rvjansen/apps/crexx_release/interpreter/rxvmplugin/rxvmplugins/mc_decimal/rxvm_mc_decimal_manual.a */

