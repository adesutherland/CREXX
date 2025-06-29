/*
 * CREXX LEVEL B - FILE IO Functions
 */
options levelb
namespace rxfnsb expose lineout linein lines
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
