/**
 * crexx compiler driver - copyright RexxLA, rvjansen 2021-2026
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
 import system
 
 /* procedure main is what is run when crexx starts execution */
 main: procedure = .int
 arg fn = .string[]
 module = .string[]
 
 /* defaults for options for this program */
native=0;version=0;help=0;compile=0;filename='';filenames='';verbose=0
execute=1;linking=0;compile=1;optimize=1;nocolor=0;keep=1;decimal=1
binfiles='';lastfile=0;sourceRoots='';binaryRoots='';importRxas=0
linkStripSource=1;linkPreserveInline=0;linkMap='';linkOptionsUsed=0;cleanupFiles=''
 
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
platformUpper = translate(platform)
if  platformUpper = 'WINDOWS' then dirsep = '\'
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
       if fn.i = '-exec'       then execute=1
       if fn.i = '-noexec'     then execute=0
       if fn.i = '-native'     then native=1
       if fn.i = '-nonative'   then native=0
       if fn.i = '-compile'    then compile=1
       if fn.i = '-nocompile'  then compile=0
       if fn.i = '-version'    then version=1
       if fn.i = '-verbose'    then verbose=1
       if fn.i = '-verbose0'   then verbose=0
       if fn.i = '-verbose1'   then verbose=1
       if fn.i = '-verbose2'   then verbose=2
       if fn.i = '-verbose3'   then verbose=3
       if fn.i = '-verbose4'   then verbose=4
       if fn.i = '-color'      then nocolor=0
       if fn.i = '-colour'     then nocolor=0
       if fn.i = '-nocolor'    then nocolor=1
       if fn.i = '-nocolour'   then nocolor=1
       if fn.i = '-optimize'   then optimize=1
       if fn.i = '-nooptimize' then optimize=0
       if fn.i = '-keep'       then keep=1
       if fn.i = '-nokeep'     then keep=0
       if fn.i = '-nodecimal'  then decimal=0
       if fn.i = '-decimal'    then decimal=1
       if fn.i = '-import-rxas' then importRxas=1
       if fn.i = '-link-keep-source' then do
         linkStripSource = 0
         linkOptionsUsed = 1
       end
       if fn.i = '-link-strip-source' then do
         linkStripSource = 1
         linkOptionsUsed = 1
       end
       if fn.i = '-link-keep-inline' then do
         linkPreserveInline = 1
         linkOptionsUsed = 1
       end
       if fn.i = '-link-strip-inline' then do
         linkPreserveInline = 0
         linkOptionsUsed = 1
       end

       optarg = ''
       if fn.i = '-source' | fn.i = '-s' then do
         i = i + 1
         if i > fn.0 then do
           say 'missing source root after' fn.i
           return 2
         end
         optarg = fn.i
         sourceRoots = appendSemicolonValue(sourceRoots, optarg)
       end
       else if left(fn.i,2) = '-s' & fn.i <> '-source' then do
         optarg = substr(fn.i,3)
         if optarg = '' then do
           say 'missing source root after' fn.i
           return 2
         end
         sourceRoots = appendSemicolonValue(sourceRoots, optarg)
       end

       if fn.i = '-i' then do
         i = i + 1
         if i > fn.0 then do
           say 'missing binary import root after -i'
           return 2
         end
         optarg = fn.i
         binaryRoots = appendSemicolonValue(binaryRoots, optarg)
       end
       else if left(fn.i,2) = '-i' & fn.i <> '-import-rxas' then do
         optarg = substr(fn.i,3)
         if optarg = '' then do
           say 'missing binary import root after' fn.i
           return 2
         end
         binaryRoots = appendSemicolonValue(binaryRoots, optarg)
       end

       if fn.i = '-l' then do
         i = i + 1
         if i > fn.0 then do
           say 'missing library path after -l'
           return 2
         end
         optarg = fn.i
         fullLibrary = rxpath||'bin'dirsep||optarg
         lastSlash = lastpos('/',fullLibrary)
         if lastSlash = 0 then lastSlash = lastpos('\',fullLibrary)
         libs = libs';'fullLibrary
         if lastSlash > 0 then libraries = libraries';'left(fullLibrary,lastSlash-1)
         else libraries = libraries';'fullLibrary
         modulenumber = modulenumber+1
       end
       else if left(fn.i,2)= '-l' & fn.i <> '-l' & left(fn.i,5) <> '-link' then do
         optarg = substr(fn.i,3)
         if optarg = '' then do
           say 'missing library path after' fn.i
           return 2
         end
         fullLibrary = rxpath||'bin'dirsep||optarg
         lastSlash = lastpos('/',fullLibrary)
         if lastSlash = 0 then lastSlash = lastpos('\',fullLibrary)
         libs = libs';'fullLibrary
         if lastSlash > 0 then libraries = libraries';'left(fullLibrary,lastSlash-1)
         else libraries = libraries';'fullLibrary
         modulenumber = modulenumber+1
       end

       if fn.i = '-linkmap' then do
         i = i + 1
         if i > fn.0 then do
           say 'missing map path after -linkmap'
           return 2
         end
         linkMap = fn.i
         linkOptionsUsed = 1
       end
       else if left(fn.i,8) = '-linkmap' & fn.i <> '-linkmap' then do
         optarg = substr(fn.i,9)
         if optarg = '' then do
           say 'missing map path after' fn.i
           return 2
         end
         linkMap = optarg
         linkOptionsUsed = 1
       end
     end
   end

   -- standard libs plugin & classlibs included
   libs = libs';'rxpath||'bin'dirsep'classlib'
   libs = libs';'rxpath||'bin'dirsep'rx_treemap'
   libs = libs';'rxpath||'bin'dirsep'rx_system'
   libs = libs';'rxpath||'bin'dirsep'rx_llist'
   libs = libs';'rxpath||'bin'dirsep'rx_keyaccess'
   libs = libs';'rxpath||'bin'dirsep'rx_id'
   
   declib = ''
   if \decimal then declib = '-p rxvm_db_decimal'
   if verbose>1 then call logo nocolor  

   decstat = rxpath'bin/rxvm_mc_decimal_manual.a'
   if \decimal then decstat =  rxpath'bin/rxvm_db_decimal_manual.a'
   
   if verbose>2 then     do
     call banner
     say 'INVOCATION OPTIONS'
     say 'Options in effect:'
    if \nocolor then say '  COLOR'
     else say 'NOCOLOR'
     if decimal  then say '  DECIMAL'
     else say 'NOCDECIMAL'
       
     if execute then say '  EXEC'
     else say 'NOEXEC'
     if native  then say '  NATIVE'
     else say 'NONATIVE'
     if keep  then say '  KEEP'
     else say 'NOKEEP'
     if \optimize  then say 'NOOPTIMIZE'
     else say '  OPTIMIZE'
     if importRxas then say '  IMPORT-RXAS'
     else say 'NOIMPORT-RXAS'

     say 'VERBOSE' verbose
     say 'SOURCE ROOTS' sourceRoots
     say 'BINARY ROOTS' binaryRoots
     -- call formfeed
     say;say;say
     end
       
if version then call logo nocolor

    
  if verbose>1 then say esc||ANSI_GREEN'using relpath   :'esc||ANSI_RESET rxpath

lpath = libraries

if verbose>1 then say esc||ANSI_GREEN'using lpath     :'esc||ANSI_RESET lpath
if verbose>1 then say esc||ANSI_GREEN'using s roots   :'esc||ANSI_RESET sourceRoots
if verbose>1 then say esc||ANSI_GREEN'using i roots   :'esc||ANSI_RESET binaryRoots

if linkOptionsUsed & \native then do
  say 'link options require -native'
  return 2
end

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
    ''rxpath'/'lpath'/rxpp -i' filename'.rxpp -o' filename'.rexx -m 'rxpath'bin/maclib.rexx -verbose 'verbose
    if verbose then do
      if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
      else res = esc||ANSI_RED||RC||esc||ANSI_RESET
	if compile then say '[ 'res' ] rxpp     - Preprocessed   ' esc||ANSI_BLUE||filename'.rxpp'||esc||ANSI_RESET
      end
      if RC>0 then exit RC
    end
  end
  /* if all is well, we now have a .rexx ready for compilation    */  
    if lower(right(filename,5))='.rexx' then filename=substr(filename,1,length(filename)-5)   /* if file has .rexx extension, chop it off */
  /* print the file when verbose ibm style output is requested */
    if verbose>3 then do
      call printFileToSTDout filename'.rexx'
    end
  compat_flags = ''
  if \hasSourceHeader(filename'.rexx') then do
    compat_flags = ' --level levelb --import rxfnsb'
    if verbose>1 then say esc||ANSI_GREEN'simple script defaults:'esc||ANSI_RESET compat_flags
  end
  optiflag=''; if optimize=0 then optiflag= '-n'
  rxc_flags = ''
  if sourceRoots <> '' then rxc_flags = rxc_flags' -s 'sourceRoots
  binaryPath = rxpath||lpath
  if binaryRoots <> '' then binaryPath = binaryPath';'binaryRoots
  if binaryPath <> '' then rxc_flags = rxc_flags' -i 'binaryPath
  if importRxas then rxc_flags = rxc_flags' --import-rxas'
  rxcmd = rxpath'bin'dirsep'rxc' optiflag compat_flags rxc_flags filename
  if verbose>1 then
    do
      if compile then say esc||ANSI_GREEN'rxc command     :' rxcmd
	if compile=0 then
	  do
	  say 'rxc does not compile due to --nocompile option'
	end
    end
  rxcOut = .string[]
  rxcErr = .string[]
  if compile then address cmd rxcmd output rxcOut error rxcErr /* Note that cmd has no meaning currently */
  else RC = 0
  do j = 1 to rxcOut.0
    say '> rxc output:' rxcOut.j
  end
  do j = 1 to rxcErr.0
    say '> rxc error: ' rxcErr.j
  end
  if verbose then do
    if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
    else res = esc||ANSI_RED||RC||esc||ANSI_RESET
    if compile then say '[ 'res' ] rxc      - Compiled       ' esc||ANSI_BLUE||filename||esc||ANSI_RESET
    end
  if RC>0 then exit RC


  binfile = chop_suffix(filename)
  binfiles = binfiles binfile

  if compile then do
/* print the file when verbose ibm style output is requested */
    if verbose>3 then do
      call printFileToSTDout filename'.rxas'
    end

    cleanupFiles = appendWordUnique(cleanupFiles, binfile'.rxas')
    cleanupFiles = appendWordUnique(cleanupFiles, binfile'.rxbin')
    asmcmd = rxpath'bin'dirsep'rxas' optiflag '-o' binfile binfile
    address cmd asmcmd output out error err
    if verbose then do
      if verbose>1 then say esc||ANSI_GREEN||'rxas command    :'   asmcmd
      if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
      else res = esc||ANSI_RED||RC||esc||ANSI_RESET
      say '[ 'res' ] rxas     - Assembled      ' esc||ANSI_BLUE||binfile'.rxas'||esc||ANSI_RESET
    end
    if RC>0 then exit RC
  end
  else do
    existingRxbin = binfile'.rxbin'
    if \fileExists(existingRxbin) then do
      say 'missing existing bytecode for -nocompile:' existingRxbin
      exit 2
    end
    if verbose>1 then say esc||ANSI_GREEN'using existing rxbin:'esc||ANSI_RESET existingRxbin
  end

  modules = translate(libs,' ',';')
  
  if verbose>2 then call banner

  if native then do
    linkedOutput = binfile'_linked'
    outputStem = binfile
    staticFlags = ''
    linkInputs = ''
    linkInputs = appendWordUnique(linkInputs, binfile)
    linkInputs = appendWordUnique(linkInputs, rxpath || 'bin' || dirsep || 'library')
    loop f=1 to words(modules)
      modulePath = word(modules,f)
      staticModule = findStaticModule(modulePath)
      if staticModule <> '' then do
        if isMacPlatform(platformUpper) then
          staticFlags = staticFlags ' -Wl,-force_load,'staticModule
        else
          staticFlags = staticFlags ' -Wl,--whole-archive 'staticModule' -Wl,--no-whole-archive'
      end
      else
        linkInputs = appendWordUnique(linkInputs, modulePath)
    end

    link_cmd = rxpath || 'bin' || dirsep || 'rxlink'
    if linkStripSource then link_cmd = link_cmd || ' -s'
    if linkPreserveInline then link_cmd = link_cmd || ' -i'
    if linkMap <> '' then link_cmd = link_cmd || ' -m ' || linkMap
    link_cmd = link_cmd || ' -o ' || linkedOutput
    loop f=1 to words(linkInputs)
      link_cmd = link_cmd || ' ' || word(linkInputs,f)
    end
    if verbose>1 then say esc||ANSI_GREEN'rxlink command  :'esc||ANSI_RESET link_cmd
    address cmd link_cmd output out error err
    if verbose then do
      if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
      else res = esc||ANSI_RED||RC||esc||ANSI_RESET
      say '[ 'res' ] rxlink   - Linked         ' esc||ANSI_BLUE||linkedOutput'.rxbin'||esc||ANSI_RESET
    end
    if RC>0 then exit RC

    pack_cmd = rxpath || 'bin' || dirsep || 'rxcpack -o ' || outputStem || ' ' || linkedOutput
    if verbose>1 then
    do
      say esc||ANSI_GREEN'rxcpack command :'esc||ANSI_RESET pack_cmd
      end
    address cmd pack_cmd output out error err
    if verbose then do
      if RC = 0 then res=esc||ANSI_GREEN||'OK'esc||ANSI_RESET
      else res = esc||ANSI_RED||RC||esc||ANSI_RESET
      say '[ 'res' ] rxcpack  - C-Packed ' esc||ANSI_BLUE||outputStem||esc||ANSI_RESET
    end
    if RC>0 then exit RC

    cc_command = 'gcc -O3 -DNDEBUG'
    socketLibs = ''
    if isWindowsPlatform(platformUpper) then socketLibs = ' -lws2_32'
    nativeRuntimeLibs = readNativeRuntimeLinkLibs(rxpath, dirsep)
    if nativeRuntimeLibs <> '' then socketLibs = socketLibs || ' ' || nativeRuntimeLibs
    if isMacPlatform(platformUpper) then
      cc_command = cc_command ' -Wl,-search_paths_first -Wl,-headerpad_max_install_names'
    cc_command = cc_command ' -o 'outputStem
    cc_command = cc_command ' 'outputStem'.c'
    cc_command = cc_command ' -L'rxpath'bin'
    if isMacPlatform(platformUpper) then do
      cc_command = cc_command ' -lrxvml -lrxpashim -lrxvmplugin -lplatform'
      cc_command = cc_command ' 'decstat
      cc_command = cc_command ' -ldecnumber -lavl_tree -lrxpa -lm' || socketLibs
      if staticFlags <> '' then cc_command = cc_command staticFlags
    end
    else do
      cc_command = cc_command ' -Wl,--start-group'
      if staticFlags <> '' then cc_command = cc_command staticFlags
      cc_command = cc_command ' -lrxvml -lrxpashim -lrxvmplugin -lplatform'
      cc_command = cc_command ' 'decstat
      cc_command = cc_command ' -ldecnumber -lavl_tree -lrxpa -lm' || socketLibs
      cc_command = cc_command ' -Wl,--end-group'
    end

    if verbose>1 then
      do
	say 'cc compile command:'
	say cc_command
	end
    address system cc_command
    
    if verbose then
      do
	if RC = 0 then res='OK'esc||ANSI_RESET
	else res = esc||ANSI_RED||RC||esc||ANSI_RESET
	say '[ 'res' ] gcc       - C-Compiled' esc||ANSI_BLUE||outputStem||esc||ANSI_RESET
	end
    if RC>0 then exit RC  
    if \keep then do
      nativeCleanupFiles = linkedOutput'.rxbin' outputStem'.c'
      if compile then nativeCleanupFiles = binfile'.rxas' binfile'.rxbin' nativeCleanupFiles
      delrc = deleteFiles(nativeCleanupFiles,verbose)
    end
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

  /* now determine if the compile/link intermediates need to be deleted */
  if \keep & \native=0 then delrc = deleteFiles(cleanupFiles,verbose)
  
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
say '-exec            -- execute the compiled rxbin (default)'
say '-noexec          -- compile only; do not execute with rxvme'
say '-compile         -- enable compilation to rxbin (default)'
say '-nocompile       -- skip rxc/rxas and reuse an existing .rxbin'
say '-native          -- package a native executable from the compiled program'
say '-nonative        -- disable native packaging (default)'
say '-verbose[0-4]    -- report on progress; default verbose0'
say '-[no]colo[u]r    -- enable or disable colo[u]r output'
say '-[no]optimize    -- enable or disable optimization'
say '-keep            -- keep compile/link intermediates (default)'
say '-nokeep          -- delete compile/link intermediates after the run'
say '-decimal         -- use decimal arithmetic'
say '-l[library path] -- packaged binary/runtime library relative to CREXX_HOME/bin'
say '-s[path]         -- additional source import root for the rxc phase'
say '-i[path]         -- additional raw binary import root for the rxc phase'
say '--import-rxas    -- allow the rxc phase to auto-import .rxas from binary roots'
say '--linkmap path   -- write an rxlink map file when using -native'
say '--link-keep-source -- keep source/file metadata in the linked native image'
say '--link-keep-inline -- keep inline-body metadata in the linked native image'
say
say 'Headerless top-level scripts are compiled with --level levelb --import rxfnsb.'
say 'Options -s, -i and --import-rxas affect compilation only; runtime/native loading still uses -l.'
say 'Native packaging uses rxlink before rxcpack; plugin/static libraries are linked natively with platform-appropriate linker flags.'
say
say 'all options can also be prefixed with --'
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
hasSourceHeader: procedure = .int
arg sourceFile = .string
line = .string
text = .string
firstWord = .string
inBlockComment = 0

do while lines(sourceFile) > 0
  line = linein(sourceFile)
  text = strip(line)

  do while 1
    if inBlockComment then do
      endPos = pos('*/', text)
      if endPos > 0 then do
        text = strip(substr(text, endPos + 2))
        inBlockComment = 0
        iterate
      end
      else do
        text = ''
        leave
      end
    end

    if text = '' then leave
    if left(text, 2) = '/*' then do
      endPos = pos('*/', text, 3)
      if endPos > 0 then do
        text = strip(substr(text, endPos + 2))
        iterate
      end
      inBlockComment = 1
      text = ''
      leave
    end
    leave
  end

  if text = '' then iterate
  if left(text, 1) = '#' then iterate

  firstWord = word(translate(text), 1)
  if firstWord = 'OPTIONS' | firstWord = 'IMPORT' | firstWord = 'NAMESPACE' then return 1
  return 0
end

return 0

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

/**
 * delete the files (when crexx option = --nokeep)
 * and log this when verbosity > 2
 * @parm filenames .string containing file paths
 * @return .int
 */
deleteFiles: procedure = .int
arg filenames = .string, verbosity = .int
loop i=1 to words(filenames)
  filename = word(filenames,i)
  if verbosity > 2 then say 'deleting' filename
  rc = deletefile(filename)
end
return 0

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

appendSemicolonValue: procedure = .string
arg current = .string, value = .string
if value = '' then return current
if current = '' then return value
return current';'value

appendWordUnique: procedure = .string
arg current = .string, value = .string
if value = '' then return current
if current = '' then return value
if wordpos(value, current) > 0 then return current
return current value

findStaticModule: procedure = .string
arg modulePath = .string
staticModule = modulePath || '_static.a'
if fileExists(staticModule) then return staticModule
staticModule = modulePath || '_static.lib'
if fileExists(staticModule) then return staticModule
return ''

isMacPlatform: procedure = .int
arg platformName = .string
platformUpper = translate(platformName)
if platformUpper = 'MACOS' | platformUpper = 'MACOSX' then return 1
return 0

isWindowsPlatform: procedure = .int
arg platformName = .string
platformUpper = translate(platformName)
if platformUpper = 'WINDOWS' | platformUpper = 'WIN32' | platformUpper = 'WIN64' then return 1
return 0

readNativeRuntimeLinkLibs: procedure = .string
arg rxpath = .string, dirsep = .string
configFile = rxpath || 'bin' || dirsep || 'crexx_native_libs'
if \fileExists(configFile) then return ''
line = linein(configFile)
call lineout configFile
return strip(line)

fileExists: procedure = .int
arg filePath = .string
if testfile(filePath) = 0 then return 1
return 0
  
