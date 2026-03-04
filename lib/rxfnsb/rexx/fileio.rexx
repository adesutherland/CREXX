/*
 * CREXX LEVEL B - FILE IO Functions
 */
options levelb
namespace rxfnsb expose lineout linein lines _execio readlines
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
  mode=upper(strip(mode))
  say 'EXECIO Debug mode='mode', file='fname', records='mmax
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
     assembler SETATTRS stem,0
     do while eof=0 & count<maxrec
        assembler freadline line,fileid
        assembler feof eof,fileid
        if eof > 0 & line = '' then leave
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
       if count<maxrec then assembler fwrite fileid,nl  /* do not write nl for last record, as a last empty record appears */
    end
  end
  rc=0
  assembler fclose rc,fileid
  if rc\=0 then return rc
return count

/* -------------------------------------------------------------------------------
 * readLines — Read text file records into a string array.
 *
 * Syntax:
 *     lines = readLines(fileName
 *                       [, fromLine = 1
 *                       [, maxRecords = 0]])
 *
 * Parameters:
 *     fileName    : .string
 *         Path to the input text file.
 *
 *     fromLine    : .int (optional, default = 1)
 *         1-based index of the first line to return.
 *         Must be >= 1.
 *
 *     maxRecords  : .int (optional, default = 0)
 *         Maximum number of records to return.
 *         If <= 0, all remaining lines from fromLine are returned.
 *
 * Returns:
 *     .string[] array
 *         A dense array containing the selected lines.
 *         Indexing starts at 1.
 *
 * Semantics:
 *     - Lines are returned without trailing CR/LF characters.
 *     - Supports ASCII and UTF-8 encoded text files.
 *     - Physical empty lines inside the file are preserved.
 *     - A trailing newline at physical EOF does NOT create
 *       an additional empty array entry.
 *
 * Error Handling:
 *     - Raises condition "error", code "40.27" if:
 *           * file cannot be opened
 *           * file cannot be closed
 *           * invalid parameter values are supplied
 *     - Read errors propagate as runtime exceptions from freadline.
 *
 * Notes:
 *     - This function loads the selected range fully into memory.
 *       Not suitable for extremely large files.
 *     - File handle is always closed before return.
 *
 * -------------------------------------------------------------------------------
 */
readlines: procedure = .string[]
  arg fname=.string, from=1, maxrec=0   /* maxrec=0 => all remaining */

  if from < 1   then from=1
  if maxrec < 0 then maxrec=0

  eof=0
  in_no=0
  out_no=0
  fileid=0
  llen=0
  rmode="r"
  lines=.string[]

  assembler fopen fileid,fname,rmode
  if fileid=0 then call raise "error", "40.27", "cannot open "fname

  do while eof = 0
     line = ""
     assembler freadline line, fileid
     assembler feof eof, fileid
     if eof > 0 & line = "" then leave    /* stop on the final read-at-EOF empty result */

     in_no = in_no + 1
     if in_no < from then iterate

     out_no = out_no + 1
     if maxrec > 0 & out_no > maxrec then leave

     lines[out_no] = line
  end

  rc=0
  assembler fclose rc,fileid
  if rc\=0 then call raise "error", "40.27", "cannot close "fname
return lines