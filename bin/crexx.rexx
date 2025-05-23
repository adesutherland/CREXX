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
execute=1;linking=0

/* loop through arguments and find options and program files */
do i=1 to fn.0
  if left(fn.i,1)<>'-' then
    do /* it is not a flag but a filename */
      filename=fn.i
      filenames=strip(filenames) strip(filename)
    end
  else
    do
      if fn.i = '-noexec' then execute=0
      if fn.i = '-native' then native=1
      if fn.i = '-version' then version=1
      if fn.i = '-verbose' then verbose=1
      if fn.i = '--noexec' then execute=0
      if fn.i = '--native' then native=1
      if fn.i = '--version' then version=1
      if fn.i = '--verbose' then verbose=1
      if fn.i = '--linking' then linking=1
    end
end -- do i

/* figure out the path for libraries etc */
rxpath=''
env_wanted='CREXX_HOME'
assembler getenv rxpath,env_wanted

if verbose then call logo
if verbose then say 'using CREXX_HOME:' rxpath


/*
 * here we are using the rxvm instead of the rxvme
 * and have to specify the functions and libraries
 * in the responsefile
 */
if linking then do
  say 'linking!'
  exit
end
lpath = '/lib/rxfnsb'
do i=1 to words(filenames)
  filename=word(filenames,i)
  'rxc -i' rxpath||lpath filename
  if verbose then do
    if RC = 0 then res='OK'
    else res = RC
    say '[ 'res' ] rxc     - Compiled' filename
  end
  'rxas' filename
  if verbose then do
    if RC = 0 then res='OK'
    else res = RC
    say '[ 'res' ] rxas    - Assembled' filename
  end
    
  if native then do
    'rxcpack' filename rxpath'/lib/rxfnsb/library'
    if verbose then do
      if RC = 0 then res='OK'
      else res = RC
      say '[ 'res' ] rxcpack - C-Packed' filename
    end
    
    'gcc -o' filename '-lrxvml -lmachine -lavl_tree -lplatform -lrxpa -lrxvmplugin -ldecnumber -lm -L',
      rxpath'/interpreter -L',
      rxpath'/interpreter/rxvmplugin -L',
      rxpath'/interpreter/rxvmplugin/rxvmplugins/db_decimal -L',
      rxpath'/interpreter/rxvmplugin/rxvmplugins/mc_decimal -L',
      rxpath'/machine -L',
      rxpath'/avl_tree -L',
      rxpath'/rxpa -L',
      rxpath'/platform',
		  filename'.c'
    if verbose then do
    if RC = 0 then res='OK'
    else res = RC
    say '[ 'res' ] gcc     - C-Compiled' filename
  end

  end
  else do
    'rxvme' filename
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
return

logo: procedure
rversion = ''
/* note that for brevity we use an assembler directive to get to the versions */
assembler rxvers rversion
say 'cRexx compiler driver' rversion
say 'Copyright (c) Adrian Sutherland 2021,'left(date('j'),4)'. All rights reserved.'
say 'Copyright (c) RexxLA 2021,'left(date('j'),4)'. All rights reserved.'
return
