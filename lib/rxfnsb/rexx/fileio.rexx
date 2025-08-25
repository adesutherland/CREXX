/*
 * CREXX LEVEL B - FILE IO Functions
 */
options levelb
namespace rxfnsb expose lineout linein lines _execio
import _rxsysb

/* REXX Lineout BIF */
lineout: procedure = .int
    arg fname = "stdout", line = ""
    if fname = "" then fname = "stdout" /* In case of blank argument */

    if ?line then do
        /* line specified - open file */
        nl = '0a'x
        fileid = _open(fname, "w")
        if fileid = 0 then say "ERROR lineout() file" fname "could not open"
        assembler fwrite fileid,line
        assembler fwrite fileid,nl
    end
    
    else do
        /* line not specified - close file */
        call _close fname
    end
    return 0

/* REXX Linein BIF */
linein: procedure = .string
    arg fname = "stdin"
    if fname = "" then fname = "stdin" /* In case of blank argument */

    line = ""
    fileid = _open(fname, "r")
    if fileid = 0 then say "ERROR linein() file" fname "could not open"
    assembler freadline line,fileid
    return line

/* REXX Lines BIF */
lines: procedure = .int
    arg fname = "stdin"
    if fname = "" then fname = "stdin" /* In case of blank argument */

    line = ""
    fileid = _open(fname, "r")
    if fileid = 0 then return 0

    eof = 0
    assembler feof eof,fileid

    if eof = 0 then return 1
    return 0

    /* REXX charin BIF */
charin: procedure = .string
    arg fname = "stdin"
    if fname = "" then fname = "stdin" /* In case of blank argument */

    line = ""
    fileid = _open(fname, "r")
    if fileid = 0 then say "ERROR linein() file" fname "could not open"
    assembler freadline line,fileid
    return line

/* REXX charout BIF */
charout: procedure = .int
    arg fname = "stdout", line = ""
    if fname = "" then fname = "stdout" /* In case of blank argument */

    if ?line then do
        /* line specified - open file */
        nl = '0a'x
        fileid = _open(fname, "w")
        if fileid = 0 then say "ERROR lineout() file" fname "could not open"
        assembler fwrite fileid,line
        assembler fwrite fileid,nl
    end
    
    else do
        /* line not specified - close file */
        call _close fname
    end
    return 0

/* REXX EXECIO BIF */
_execio: procedure=.int
arg mmax=.string,mode='R',fname=.string, expose stem=.string[]
  maxrec=0
  mode=upper(mode)
  count=0
  if mode='DISKR'       then omode='r'
  else if mode='READ'   then omode='r'
  else if mode='DISKW'  then omode='w'
  else if mode='WRITE'  then omode='w'
  else if mode='DISKA'  then omode='a'
  else if mode='APPEND' then omode='a'
  fileid=0

  assembler fopen fileid,fname,omode

  if fileid=0 then return -12
  if omode='r' then do
     if mmax='*' then maxrec=9999999
     else maxrec=mmax
     line=''
     eof=0
     stem.1=""
     do while eof=0 & count<maxrec
        assembler freadline line,fileid
        assembler feof eof,fileid
        count=count+1
        stem.count=line
     end
  end
  else do
    nl = '0a'x
    if mmax='*' then maxrec=stem.0
    else maxrec=min(mmax,stem.0)

    do count=1 to maxrec
       assembler fwrite fileid,stem.count
       assembler fwrite fileid,nl
    end
  end
  rc=0
  assembler fclose rc,fileid
  if rc\=0 then return rc
return count