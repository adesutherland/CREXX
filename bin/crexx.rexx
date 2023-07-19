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

if fn[0]=0 then do
  call help
  assembler exit
end


/* defaults for options */
native=0;version=0;help=0;compile=0;filename='';filenames='';verbose=0
execute=1

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
      if fn.i = '--native' then native=1
      if fn.i = '--version' then version=1
      if fn.i = '--verbose' then verbose=1
    end
end -- do i

/* figure out the path for libraries etc */
crexxhome=.string[]
address cmd 'printenv CREXX_HOME' output crexxhome
rxpath=crexxhome[1]

if verbose then call logo

do i=1 to words(filenames)
  filename=word(filenames,i)
  if verbose then assembler sayx 'Compiling '
  'rxc -i' rxpath'/lib/rxfns' filename
  if verbose then say filename ' result =' RC
  if verbose then assembler sayx 'Assembling '
  'rxas' filename
  if verbose then say filename 'result =' RC
  
  if native then do
    if verbose then say 'Packing native executable' filename
    'rxcpack' filename rxpath'/lib/rxfns/library'
    if verbose then say 'Compiling native executable' filename
    'gcc -o' filename '-lrxvml -lmachine -lavl_tree -lplatform -lm -L',
    rxpath'/interpreter -L',
    rxpath'/machine -L',
    rxpath'/avl_tree -L',
    rxpath'/platform',
    filename'.c'
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
say '--help           -- display (this) help info'
say '--version        -- display the version number'
say '--exec           -- execute (default)'
say '--native         -- build native executable; implies noexec; default nonative'
return

logo: procedure
rversion = ''
assembler rxvers rversion
say 'cRexx compiler driver' rversion
say 'Copyright (c) Adrian Sutherland 2021,2023. All rights reserved.'
say 'Copyright (c) RexxLA 2021,2023. All rights reserved.'
return
