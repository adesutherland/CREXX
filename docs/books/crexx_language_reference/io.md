# File and Stream I/O

## Overview

For input and output Rexx traditionally[^streamio] provides a small set of built-in functions for reading and
writing streams. The same functions can be used with ordinary files and with
the standard input and output streams. They deliberately avoid exposing file
descriptors, buffers, and other operating-system details to the program.

cRexx Level B provides the familiar line-oriented functions:

* `LINEIN()` reads a line;
* `LINEOUT()` writes a line;
* `LINES()` reports whether another line is available;

and the corresponding character-oriented functions:

* `CHARIN()` reads one or more characters; and
* `CHAROUT()` writes text without adding a line terminator.

These functions form the portable Rexx-style interface described in this
chapter. The Level B library also provides `_EXECIO`, `READLINES()`,
`LOADTEXT()`, and `ERASEFILE()` for bulk or specialized file operations. Those
additional routines are documented separately later in the chapter because
they are not all part of the standard Rexx built-in-function set.

The implementation treats these facilities as UTF-8 text I/O. `LINEIN()` and
`LINEOUT()` operate on text lines, and `CHARIN()` reads Unicode code points
rather than arbitrary bytes. Programs that need to preserve arbitrary binary
data should use binary I/O facilities instead; the current Level B file-I/O library does not expose 
a corresponding binary I/O interface.

[^streamio]: Except for the mainframe implementations on CMS and TSO where the product cutoff date precluded delivery of the stream I/O functions, and the PL/S stream implementation was available as an add-on (PRPQ). VM employed the EXECIO program for I/O, which was reproduced for the TSO/E implementation. The z/OS USS (Unix System Services) implementation of Rexx does include all stream I/O functions.

## Streams and File Names

The first argument to each standard I/O function identifies a stream. For an
ordinary file, this is its file name or path:

```rexx
line = linein("customers.txt")
```

The special names:

```text
stdin
stdout
stderr
```

refer to the standard streams of a process. The standard functions use `stdin`
or `stdout` as appropriate when the file-name argument is omitted:

```rexx
line = linein()          -- read from standard input
call lineout , line      -- write to standard output
```

A blank file name is treated in the same way as an omitted one. For input it
selects `stdin`; for output it selects `stdout`:

```rexx
line = linein("")
call lineout "", "ready"
```

The library manages the underlying open streams. A program normally names the
same file on successive calls and lets the library retain its current
position. An output or input stream can be closed explicitly through the
output functions, as described under `LINEOUT()` and `CHAROUT()`.

## Line-Oriented Input

Line-oriented input is the natural choice for text files in which records are
separated by line terminators. The terminator identifies the end of the
record, but it is not included in the value returned to the program.

### LINEIN

`LINEIN()` reads the next line from a text stream.

```rexx
line = linein([fileName])
```

The Level B signature is:

```rexx
linein(fileName = "stdin") = .string
```

If `fileName` is omitted or blank, input is read from `stdin`. Otherwise, the
named file is opened for reading when necessary and its current stream
position is used.

For example:

```rexx
name = linein("names.txt")
say "First name:" name
```

Successive calls read successive lines:

```rexx
do while lines("names.txt") > 0
    say linein("names.txt")
end
```

The returned string does not contain the line terminator. An empty physical
line is therefore returned as an empty string. End of file can also yield an
empty string, so a program that must distinguish an empty line from the end
of the stream should use `LINES()` to test availability.

The cRexx Level B form implements sequential line input. Unlike some extended
Rexx stream interfaces, it does not accept a starting line number or a count;
each call reads one line at the current position.

If the file cannot be opened, `LINEIN()` writes an error diagnostic. Programs
that require explicit and structured open-error handling can instead use
`READLINES()` with its error option.

### LINES

`LINES()` reports whether a line is available from a text stream.

```rexx
available = lines([fileName])
```

The Level B signature is:

```rexx
lines(fileName = "stdin") = .int
```

It returns:

```text
 1   at least one line is available
 0   the stream is at end of file
-1   the stream could not be opened
```

`LINES()` is therefore an availability test, not a count of all remaining
lines. A positive result means that one subsequent `LINEIN()` call can obtain
a line; it does not promise that exactly one line remains.

A conventional file-reading loop is:

```rexx
file = "report.txt"

do while lines(file) > 0
    line = linein(file)
    say line
end

call lineout file
```

The final `LINEOUT()` call closes the named stream. Although this looks like
an output operation, the omitted second argument gives it the traditional
stream-closing meaning.

To determine whether data is available without losing it, the implementation
may read one line ahead. That line is retained internally and returned by the
next `LINEIN()` for the same file. Repeated calls to `LINES()` therefore do
not skip records:

```rexx
if lines(file) > 0 then do
    if lines(file) > 0 then
        say linein(file)
end
```

Only one availability value is needed because the prefetched line remains
pending until `LINEIN()` consumes it or the stream is closed.

## Line-Oriented Output

### LINEOUT

`LINEOUT()` writes one line of text and appends a line terminator.

```rexx
result = lineout([fileName], line)
```

The Level B signature is:

```rexx
lineout(fileName = "stdout", line = "") = .int
```

The presence of the second argument is significant. If `line` is supplied,
its value is written and an LF line terminator, byte `0A` hexadecimal, is
appended:

```rexx
call lineout "messages.txt", "first message"
call lineout "messages.txt", "second message"
```

An explicitly supplied empty string writes an empty line:

```rexx
call lineout "messages.txt", ""
```

If the `line` argument is omitted, nothing is written. Instead, the named
stream is closed:

```rexx
call lineout "messages.txt"
```

This distinction between an omitted argument and an empty argument is
important:

```rexx
call lineout file, ""     -- write an empty line
call lineout file         -- close the stream
```

When the file name is omitted or blank, `stdout` is used:

```rexx
call lineout , "Processing complete"
```

`LINEOUT()` flushes the stream after writing a line and checks both the flush
status and the stream error status. It returns zero after a successful write
or close. If the file cannot be opened, it writes a diagnostic and returns
`1`. A non-zero error reported while flushing or checking the stream is
returned to the caller.

A complete writing sequence can therefore test each operation:

```rexx
file = "summary.txt"

rc = lineout(file, "Summary")
if rc = 0 then rc = lineout(file, "-------")
if rc = 0 then rc = lineout(file)

if rc <> 0 then
    say "Unable to write" file", return code" rc
```

## Character-Oriented Input

### CHARIN

`CHARIN()` reads text from the current position in a stream without treating
line terminators as record boundaries.

```rexx
text = charin([fileName] [, count])
```

The Level B signature is:

```rexx
charin(fileName = "stdin", count = 1) = .string
```

`count` specifies the maximum number of Unicode code points to read. Its
default is one:

```rexx
character = charin("input.txt")
```

Several characters can be read in one call:

```rexx
prefix = charin("input.txt", 12)
```

The result may contain fewer code points than requested if the end of the
stream is reached. A count less than or equal to zero returns an empty string
without reading the stream:

```rexx
text = charin("input.txt", 0)       -- returns ""
```

`CHARIN()` is character-oriented rather than byte-oriented. A UTF-8 character
encoded with more than one byte still counts as one code point for the purpose
of `count`. This makes the operation suitable for Unicode text but unsuitable
for reading a precise number of bytes from a binary file.

The Level B form reads sequentially from the current stream position. It does
not provide the optional absolute starting position found in some other Rexx
implementations.

If the file cannot be opened, `CHARIN()` writes an error diagnostic and returns
an empty string.

## Character-Oriented Output

### CHAROUT

`CHAROUT()` writes text without appending a line terminator.

```rexx
result = charout([fileName], text)
```

The Level B signature is:

```rexx
charout(fileName = "stdout", text = "") = .int
```

This function is useful when a line is to be assembled in several operations,
or when the program needs to write a prompt and leave the cursor on the same
line:

```rexx
call charout , "Name: "
name = linein()
```

Text from successive calls is written contiguously:

```rexx
call charout "result.txt", "part one"
call charout "result.txt", " and part two"
```

No separator or newline is inserted between the calls. A line terminator can
be supplied explicitly as part of the text when needed, or the final part can
be written with `LINEOUT()`.

As with `LINEOUT()`, omission of the second argument closes the stream:

```rexx
call charout "result.txt"
```

An explicit empty second argument is different. It performs a zero-length
write rather than requesting a close:

```rexx
call charout "result.txt", ""
```

`CHAROUT()` returns zero after a successful write or close. If the file cannot
be opened, it writes a diagnostic and returns `1`.

## Closing Streams

The standard Level B interface does not expose a separate `CLOSE()` built-in
function. A stream is closed by calling `LINEOUT()` or `CHAROUT()` with its
data argument omitted:

```rexx
call lineout file
```

or:

```rexx
call charout file
```

Closing an input stream through `LINEOUT()` also discards any line that
`LINES()` has read ahead for that file. A later input operation starts with a
newly opened stream.

It is good practice to close a file when a program has finished using it,
especially when:

* output must be made visible to another process;
* the same file will later be reopened in another mode;
* the program uses many different files; or
* an availability test may have retained a pending input line.

The standard streams can be named and closed in the same way, although a
program normally leaves their lifetime to the process environment.

## Mixing Line and Character Operations

Line-oriented and character-oriented functions use the same underlying file
position, but they apply different interpretations to the data. Mixing them on
one stream can therefore be difficult to reason about.

There is an additional consideration in the Level B implementation:
`LINES()` may read a complete line ahead and retain it for `LINEIN()`.
`CHARIN()` does not consume that pending line. If a program calls `CHARIN()`
after a successful `LINES()` but before the corresponding `LINEIN()`, the
character read begins after the internally retained line. A later `LINEIN()`
then returns the earlier pending line.

For predictable sequential processing, use either:

```text
LINES() with LINEIN()
```

or:

```text
CHARIN()
```

for a particular stream at a particular stage of processing. Close and reopen
the stream before changing models if its position must be reset.

The same general advice applies to `LINEOUT()` and `CHAROUT()`: combining them
is valid when their different newline behaviour is intentional, but the
program must account for exactly which operation supplies each line
terminator.

## Standard I/O Examples

### Copying a text file

The following example copies a text file one line at a time:

```rexx
source = "input.txt"
target = "output.txt"
rc = 0

do while lines(source) > 0
    rc = lineout(target, linein(source))
    if rc <> 0 then leave
end

call lineout source
close_rc = lineout(target)

if rc = 0 then rc = close_rc
if rc <> 0 then
    say "Copy failed, return code" rc
```

Because `LINEOUT()` supplies a new LF terminator for every input line, this is
a text copy rather than a byte-for-byte copy. It normalizes the output line
terminators to the Level B convention.

### Reading a prompt

The standard streams allow terminal interaction without special console
functions:

```rexx
call charout , "Enter a file name: "
file = linein()

if lines(file) < 0 then
    say "Cannot open" file
else
    say "The first line is:" linein(file)

call lineout file
```

`CHAROUT()` writes the prompt without a newline, while `LINEIN()` reads the
reply as a complete line.

## Record I/O with EXECIO

`EXECIO`, included in the IBM Classic Rexx implementations for VM/CMS and TSO/E, offers an IBM-mainframe style record-oriented
syntax for transferring lines between a text file and a string array.

`EXECIO` is especially convenient when a program reads or writes an entire
group of records.

### Syntax

```rexx
EXECIO record-count mode file-name (STEM stem-name FINIS
```

The operands are:

* `record-count` specifies the maximum number of records to transfer. A
  positive whole number requests that number of records. An asterisk requests
  all applicable records.
* `mode` specifies whether records are read, written, or appended.
* `file-name` identifies the text file.
* `STEM stem-name` identifies the string array that supplies or receives the
  records.
* `FINIS` requests completion of the operation and closure of the file.

The supported modes are:

```text
DISKR    Read records from a file
DISKW    Write records, replacing the file
DISKA    Append records to the file
```

The longer names `READ`, `WRITE`, and `APPEND` are also accepted by the
underlying Level B function.

### Writing Records

With `DISKW`, the array supplies the records written to the file:

```rexx
out_stem = .string[]
out_stem[1] = "Line 1: Hello from Execio"
out_stem[2] = "Line 2: Testing the exit"
out_stem[3] = "Line 3: Goodbye"

EXECIO 3 DISKW 'test_execio_temp.txt' (STEM out_stem FINIS
```

This writes the first three elements of `out_stem`. Existing contents of the
file are replaced.

Records are separated by LF characters. The implementation does not append an
additional line terminator after the final record, thereby avoiding the
appearance of an extra empty record when the file is subsequently read.

When the record count is `*`, the number of records is obtained from the
array's item count:

```rexx
EXECIO * DISKW 'output.txt' (STEM out_stem FINIS
```

A numeric record count cannot cause more records to be written than the array
contains.

To add records without replacing the existing file, use `DISKA`:

```rexx
EXECIO * DISKA 'output.txt' (STEM additional_lines FINIS
```

### Reading Records

With `DISKR`, records are read from the file into the named string array:

```rexx
EXECIO * DISKR 'test_execio_temp.txt' (STEM mystem FINIS
```

An asterisk reads all records through the end of the file. A numeric count
limits the number of records:

```rexx
EXECIO 10 DISKR 'input.txt' (STEM first_ten FINIS
```

The first record is placed in element `1`, the second in element `2`, and so
forth:

```rexx
say mystem[1]
say mystem[2]
say mystem[3]
```

Line terminators are not included in the array elements. Empty physical lines
inside the file are preserved, while a final line terminator does not create
an additional empty element.

Before reading begins, the existing item count of the receiving array is reset.
The resulting array contains the records obtained by the current operation.

### Automatic Stem Declaration

The `EXECIO` compiler exit recognizes the name in the `STEM` clause. If the
receiving stem has not previously been declared, the compiler exit hoists an
appropriate declaration into the procedure:

```rexx
EXECIO * DISKR 'input.txt' (STEM records FINIS
```

It is therefore not necessary to precede the statement with:

```rexx
records = .string[]
```

The generated declaration gives `records` the `.string[]` type required by the
underlying `_execio()` function. This is particularly useful for input, where
the array exists primarily to receive records.

An explicit declaration remains valid and may make the role of an output array
clear:

```rexx
records = .string[]
records[1] = "first record"
records[2] = "second record"

EXECIO * DISKW 'output.txt' (STEM records FINIS
```

### Complete Example

```rexx
options levelb
import rxfnsb

main: procedure

out_stem = .string[]
out_stem[1] = "Line 1: Hello from Execio"
out_stem[2] = "Line 2: Testing the exit"
out_stem[3] = "Line 3: Goodbye"

EXECIO 3 DISKW 'test_execio_temp.txt' (STEM out_stem FINIS

/* mystem is declared automatically by the compiler exit. */
EXECIO * DISKR 'test_execio_temp.txt' (STEM mystem FINIS

say "Read back from file:"
say "1:" mystem[1]
say "2:" mystem[2]
say "3:" mystem[3]
say "SUCCESS"

return
```

## Relationship to `_execio()`

`EXECIO` is source-level convenience syntax. The actual transfer is performed
by the `_execio()` Level B library function. The compiler exit:

* parses the record count, mode, file name, and `STEM` clause;
* supplies a `.string[]` declaration when the stem is undeclared;
* constructs the corresponding `_execio()` call; and
* lets the resulting cRexx code be compiled normally.

The statement consequently retains the familiar appearance of IBM `EXECIO`
while using typed cRexx arrays and the Level B file-I/O implementation
underneath. It should be regarded as an IBM-style cRexx facility rather than
part of the portable ANSI Rexx built-in-function set.


### The _EXECIO Built-in function

`_EXECIO` provides an IBM-style record interface for transferring a group of
text lines between a file and a stem. It is used in the implementation of the above mentioned `EXECIO`
facility, but can also be used as a built-in function by itself.

Its signature is:

```rexx
_execio(maxRecords, mode, fileName, stem) = .int
```

The arguments are:

* `maxRecords`, a record count or `*`;
* `mode`, selecting input, replacement output, or append output;
* `fileName`, the file to process; and
* `stem`, the string array used to receive or supply records.

The accepted modes are:

```text
DISKR    READ
DISKW    WRITE
DISKA    APPEND
```

Mode names are stripped and converted to uppercase. `DISKR` and `READ` open
the file for reading. `DISKW` and `WRITE` open it for replacement writing.
`DISKA` and `APPEND` open it for append writing.

### Reading records

For input, `*` requests all remaining records. Otherwise, `maxRecords` gives
the maximum number of records to read:

```rexx
records = .string[]
count = _execio("*", "DISKR", "input.txt", records)
```

The records are placed in:

```text
records[1]
records[2]
...
```

and the function returns the number read. The array's item count is reset
before input begins. Line terminators are not stored in the array, physical
empty lines are retained, and a final newline does not create an extra empty
record.

### Writing records

For output, `*` writes the number of records indicated by `stem.0`. A numeric
maximum writes no more than the smaller of that value and `stem.0`:

```rexx
records = .string[]
records[1] = "first"
records[2] = "second"

count = _execio("*", "DISKW", "output.txt", records)
```

The function inserts LF separators between records but does not append a
newline after the last record. This prevents the reader from interpreting a
final separator as an additional empty record.

The return value is the number of records processed. Failure to open the file
returns `-12`; a non-zero close error is returned instead of the count.

## Reading Several Lines with READLINES

`READLINES()` reads a selected range of a text file into a dense string array.
It is convenient when the complete selection is to be processed in memory
rather than one line at a time.

```rexx
lines = readlines(fileName [, fromLine [, maxRecords [, errorAction]]])
```

The effective signature is:

```rexx
readlines(fileName = .string,
          fromLine = 1,
          maxRecords = 0,
          errorAction = "raise") = .string[]
```

`fromLine` is one-based. Values smaller than one are adjusted to one.
`maxRecords` limits the number of returned lines; zero means all remaining
lines. Negative values are adjusted to zero.

For example, this reads at most 25 lines beginning with physical line 101:

```rexx
part = readlines("large.txt", 101, 25)
```

The result is a dense array indexed from one:

```text
part[1]    physical line 101
part[2]    physical line 102
...
```

Line terminators are removed. Empty physical lines inside the selected range
are preserved, but a trailing newline at physical end of file does not create
an additional empty array item.

By default, failure to open or close the file raises condition `error` with
code `40.27`. If `errorAction` is not `RAISE`, an open failure instead returns
an array whose first item is:

```text
-8 OPEN ERROR
```

Read errors raised by the underlying line operation propagate to the caller.
The file is closed before a successful return.

`READLINES()` holds the selected records in memory. Sequential `LINEIN()` is
preferable for very large files when the program does not need the entire
selection at once.

## Loading Text with LOADTEXT

`LOADTEXT()` reads a complete non-binary file and combines its lines into one
string.

```rexx
text = loadtext(fileName [, delimiter [, errorAction]])
```

Its effective signature is:

```rexx
loadtext(fileName = .string,
         delimiter = '0D0A'x,
         errorAction = "raise") = .string
```

Each physical line is returned without its original terminator and is followed
by `delimiter` in the resulting string. The default delimiter is CRLF:

```rexx
text = loadtext("chapter.txt")
```

A caller can request another separator, including LF:

```rexx
text = loadtext("chapter.txt", '0A'x)
```

or combine the lines without any separator:

```rexx
text = loadtext("chapter.txt", "")
```

Because the delimiter is appended after every line read, the returned string
also ends with the selected delimiter when the file contains at least one
line.

By default, failure to open or close the file raises condition `error` with
code `40.27`. If `errorAction` is not `RAISE`, an open failure returns:

```text
-8 OPEN ERROR
```

The routine loads the complete text into memory and should therefore be used
only when the expected file size is appropriate.

## Emptying a File with ERASEFILE

Despite its name, `ERASEFILE()` does not delete a file. It truncates the file
to zero length while retaining the directory entry:

```rexx
result = erasefile(fileName)
```

Its signature is:

```rexx
erasefile(fileName = .string) = .int
```

The function first closes any cached instance of the file. It then opens the
file in replacement-write mode, which truncates an existing file, and closes
it immediately.

The return values are:

```text
 0   success
-1   the file could not be opened or created
```

If the file does not exist, a new empty file is created. The operation is
similar to `OPEN WRITE REPLACE` in classic Rexx stream implementations.

For example:

```rexx
if erasefile("trace.log") <> 0 then
    say "Unable to empty trace.log"
```

Use an operating-system or file-system deletion facility when the file itself
must be removed.

## Selecting an I/O Facility

The appropriate operation depends on the structure of the data and the way it
will be consumed:

* use `LINEIN()` with `LINES()` for sequential text records;
* use `LINEOUT()` to write complete lines;
* use `CHARIN()` for sequential Unicode code-point input;
* use `CHAROUT()` for text that must not receive an automatic newline;
* use `_EXECIO` to transfer multiple records between a file and a stem;
* use `READLINES()` to load a selected line range into an array;
* use `LOADTEXT()` to obtain a complete text file as one string; and
* use `ERASEFILE()` to retain a file while reducing it to zero length.

The line and character functions keep memory use small and are appropriate for
streaming large inputs. The bulk routines are more convenient when random
access or repeated processing justifies holding the data in memory.

All the functions described here operate on text. Their treatment of line
terminators, Unicode characters, and record boundaries is intentional and
should not be relied upon for a byte-exact copy of binary data.
