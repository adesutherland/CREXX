/*
 * CREXX LEVEL B - FILE IO Functions
 */
options levelb
namespace rxfnsb expose lineout linein lines
import _rxsysb

/* REXX Lineout BIF */
lineout: procedure = .int
    arg fname = .string, line = .string

    if ?line then do
        /* line specified - open file */
        nl = '0a'x

      /* if length(fname)=0 then do */
      /* 	/\* no file specified, use stdout *\/ */
      /* 	assembler sayx line */
      /* 	return 0 */
      /* end */
      
      fileid = _open(fname, "w")
      if fileid = 0 then say "ERROR lineout() file" fname "could not open"
      else do
        assembler fwrite fileid,line
        assembler fwrite fileid,nl
      end
    end
    
    else do
        /* line not specified - close file */
        call _close fname
    end

    return 0

/* REXX Linein BIF */
linein: procedure = .string
    arg fname = .string

    line = ""

    fileid = _open(fname, "r")
    if fileid = 0 then say "ERROR linein() file" fname "could not open"
    else do
        assembler freadline line,fileid
    end

    return line

/* REXX Linein BIF */
lines: procedure = .int
    arg fname = .string

    line = ""

    fileid = _open(fname, "r")
    if fileid = 0 then return 0

    eof = 0
    assembler feof eof,fileid

    if eof = 0 then return 1
    return 0