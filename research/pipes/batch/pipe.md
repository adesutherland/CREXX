# cREXX Pipe

## Overview

The `.Pipe` class provides record-oriented pipeline processing for cREXX.

A pipe contains an ordered collection of string records. Records are loaded
into a pipe and then processed by a sequence of stages.

Each stage may:

- select or remove records,
- transform record contents,
- change record order,
- change the number of records,
- aggregate records, or
- move records to the output area.

Example:

```rexx
source = .string[]

source[1] = "  src parser.c  "
source[2] = "  src scanner.c "
source[3] = "  docs pipe.md  "
source[4] = "  src runtime.c "

p = .Pipe()

call p.fromArray(source)

rc = p.pipe("SELECT", "src ")
rc = p.pipe("TRIM")
rc = p.pipe("REPLACE", "src ", "SOURCE ")
rc = p.pipe("SAY")
```

The stages are executed sequentially against the current contents of the
pipe.

---

## Creating a Pipe

Create a pipe instance with:

```rexx
p = .Pipe()
```

Each `.Pipe` instance maintains its own:

- records,
- active state,
- output records, and
- error information.

---

# Array Methods

## `fromArray`

```rexx
call p.fromArray(source)
```

Replaces the current pipeline records with a copy of a `.string[]`.

Example:

```rexx
source = .string[]

source[1] = "src parser.c"
source[2] = "src scanner.c"
source[3] = "src runtime.c"

call p.fromArray(source)
```

After loading:

```text
src parser.c
src scanner.c
src runtime.c
```

The pipe becomes active.

An empty array produces an active pipe containing zero records.

Existing pipeline records are replaced.

---

## `toArray`

```rexx
target = p.toArray()
```

Returns a copy of every current pipeline record as a new `.string[]`,
preserving record order.

The pipeline is not consumed or modified.

On success:

- the pipe remains active,
- `recordCount()` is unchanged,
- record contents and order are unchanged, and
- the returned array is an independent snapshot.

Empty records are preserved as empty array elements.

An active pipeline containing zero records returns an empty array while
remaining active.

If the pipeline is inactive, an empty array is returned and
`pipeline_inactive` error information is set.

A saved array may later be restored with `fromArray()`:

```rexx
saved = p.toArray()

rc = p.pipe("TAKE", "1")

call p.fromArray(saved)
```

This provides a simple snapshot and restore mechanism without adding a
separate pipeline history or stack.

---

# File Stages

## READ

```rexx
rc = p.pipe("READ", filename)
```

Reads a text file into the pipeline.

Each input line becomes one record and line terminators are not included.
Physical empty lines are preserved as empty records.

On success:

- the current pipeline records are replaced,
- the pipe becomes active,
- `recordCount()` is the number of lines read, and
- the return code is `1`.

An empty file produces an active pipeline containing zero records.

On failure, `READ` returns `0` and sets error information.

---

## WRITE

```rexx
rc = p.pipe("WRITE", filename)
```

Writes every current pipeline record to a text file.

Each record is written as one output line.

`WRITE` does not consume or deactivate the pipeline.

On success:

- record contents are unchanged,
- record count is unchanged,
- the pipe remains active, and
- the return code is `1`.

The current implementation does not explicitly truncate an existing file
before writing. Existing file contents may therefore remain or new records
may be appended according to the underlying file I/O behaviour.

On failure, `WRITE` returns `0` and sets error information.

---

# Executing Stages

Stages are executed with:

```rexx
rc = p.pipe("COMMAND")
```

or, for stages requiring arguments:

```rexx
rc = p.pipe("COMMAND", "argument")
```

For example:

```rexx
rc = p.pipe("SELECT", "src ")
rc = p.pipe("REPLACE", "src ", "SOURCE ")
rc = p.pipe("SUBSTR", "5", "6")
```

Return codes are:

| Return code | Meaning |
|---:|---|
| `1` | Stage completed successfully |
| `0` | Stage failed |

Error information can be obtained from the pipe instance.

---

# Stage Reference

## Selection Stages

### SELECT

```rexx
p.pipe("SELECT", text)
```

Keeps records containing `text`.

Matching is case-sensitive.

Example:

```text
Input:

src parser.c
docs pipe.md
src runtime.c
test parser_test.c
```

```rexx
rc = p.pipe("SELECT", "src ")
```

produces:

```text
src parser.c
src runtime.c
```

Cardinality: `n -> 0..n`

---

### DROP

```rexx
p.pipe("DROP", text)
```

Removes records containing `text`.

Matching is case-sensitive.

Example:

```text
Input:

src parser.c
docs pipe.md
src runtime.c
```

```rexx
rc = p.pipe("DROP", "src ")
```

produces:

```text
docs pipe.md
```

Cardinality: `n -> 0..n`

---

### EMPTY

```rexx
p.pipe("EMPTY")
```

Keeps only zero-length records.

Whitespace-only records are not empty.

```text
""      -> kept
" "     -> removed
"   "   -> removed
```

To treat whitespace-only records as empty:

```rexx
rc = p.pipe("TRIM")
rc = p.pipe("EMPTY")
```

Cardinality: `n -> 0..n`

---

### NONEMPTY

```rexx
p.pipe("NONEMPTY")
```

Keeps records whose length is greater than zero.

Whitespace-only records are retained.

To remove empty and whitespace-only records:

```rexx
rc = p.pipe("TRIM")
rc = p.pipe("NONEMPTY")
```

Cardinality: `n -> 0..n`

---

### STARTSWITH

```rexx
p.pipe("STARTSWITH", text)
```

Keeps records beginning with `text`.

Matching is case-sensitive and leading whitespace is significant.

Example:

```text
src parser.c
src scanner.c
docs pipe.md
Src runtime.c
```

```rexx
rc = p.pipe("STARTSWITH", "src ")
```

produces:

```text
src parser.c
src scanner.c
```

Cardinality: `n -> 0..n`

---

### ENDSWITH

```rexx
p.pipe("ENDSWITH", text)
```

Keeps records ending with `text`.

Matching is case-sensitive and trailing whitespace is significant.

Example:

```text
src parser.c
src scanner.crexx
src runtime.c
src parser.c 
```

```rexx
rc = p.pipe("ENDSWITH", ".c")
```

produces:

```text
src parser.c
src runtime.c
```

Cardinality: `n -> 0..n`

---

## Positional Stages

### TAKE

```rexx
p.pipe("TAKE", count)
```

Keeps the first `count` records.

`count` must be a non-negative whole number.

If `count` is greater than or equal to the current number of records, all
records are retained.

A count of zero produces an active empty pipe.

Cardinality: `n -> 0..n`

---

### SKIP

```rexx
p.pipe("SKIP", count)
```

Removes the first `count` records.

`count` must be a non-negative whole number.

If `count` is greater than or equal to the current number of records, the
result is an active empty pipe.

Cardinality: `n -> 0..n`

---

### FIRST

```rexx
p.pipe("FIRST")
```

Keeps only the first record.

An active empty pipe remains active and empty.

Cardinality: `n -> min(n, 1)`

---

### LAST

```rexx
p.pipe("LAST")
```

Keeps only the last record.

An active empty pipe remains active and empty.

Cardinality: `n -> min(n, 1)`

---

## Ordering and Set Stages

### SORT

```rexx
p.pipe("SORT")
```

Sorts records into ascending lexical order.

Record count is unchanged.

Example:

```text
src scanner.c
src parser.c
src runtime.c
```

becomes:

```text
src parser.c
src runtime.c
src scanner.c
```

Cardinality: `n -> n`

---

### REVERSE

```rexx
p.pipe("REVERSE")
```

Reverses record order.

Example:

```text
src parser.c
src scanner.c
src runtime.c
```

becomes:

```text
src runtime.c
src scanner.c
src parser.c
```

Cardinality: `n -> n`

---

### UNIQUE

```rexx
p.pipe("UNIQUE")
```

Removes duplicate records while preserving the first occurrence and the
original order of retained records.

Example:

```text
src scanner.c
src parser.c
src scanner.c
src runtime.c
src parser.c
```

becomes:

```text
src scanner.c
src parser.c
src runtime.c
```

Cardinality: `n -> 0..n`

---

## Text Transformation Stages

### UPPER

```rexx
p.pipe("UPPER")
```

Converts every record to upper case.

Record count and order are unchanged.

Cardinality: `n -> n`

---

### LOWER

```rexx
p.pipe("LOWER")
```

Converts ASCII upper-case characters in every record to lower case.

Record count and order are unchanged.

Cardinality: `n -> n`

---

### TRIM

```rexx
p.pipe("TRIM")
```

Removes leading and trailing whitespace from every record.

Internal whitespace is unchanged.

Example:

```text
"  src parser.c  "
```

becomes:

```text
"src parser.c"
```

Cardinality: `n -> n`

---

### SPACE

```rexx
p.pipe("SPACE")
```

Normalizes whitespace in every record.

Leading and trailing whitespace is removed and sequences of whitespace
between words are reduced according to the cREXX `SPACE` semantics.

Example:

```text
"   src      parser.c   "
```

becomes:

```text
"src parser.c"
```

Cardinality: `n -> n`

---

### REPLACE

```rexx
p.pipe("REPLACE", old, new)
```

Replaces all occurrences of `old` with `new` in every record.

An empty search string is invalid.

Example:

```rexx
rc = p.pipe("REPLACE", "src ", "SOURCE ")
```

converts:

```text
src parser.c
```

to:

```text
SOURCE parser.c
```

Cardinality: `n -> n`

---

### PREFIX

```rexx
p.pipe("PREFIX", text)
```

Prepends `text` to every record.

Example:

```text
parser.c
runtime.c
```

with:

```rexx
rc = p.pipe("PREFIX", "src/")
```

becomes:

```text
src/parser.c
src/runtime.c
```

Cardinality: `n -> n`

---

### SUFFIX

```rexx
p.pipe("SUFFIX", text)
```

Appends `text` to every record.

Example:

```text
parser
runtime
```

with:

```rexx
rc = p.pipe("SUFFIX", ".c")
```

becomes:

```text
parser.c
runtime.c
```

Cardinality: `n -> n`

---

### WORD

```rexx
p.pipe("WORD", n)
```

Replaces every record with its `n`th whitespace-delimited word.

Word numbering starts at `1`.

Example:

```text
src parser.c
src scanner.c
docs pipe.md
```

```rexx
rc = p.pipe("WORD", "2")
```

produces:

```text
parser.c
scanner.c
pipe.md
```

If the requested word does not exist, the resulting record is empty.

Cardinality: `n -> n`

---

### WORDS

```rexx
p.pipe("WORDS")
```

Replaces each record with its number of whitespace-delimited words.

Example:

```text
"src parser.c"       -> "2"
""                   -> "0"
"docs     pipe.md"   -> "2"
```

Cardinality: `n -> n`

---

### SUBSTR

```rexx
p.pipe("SUBSTR", start)
p.pipe("SUBSTR", start, length)
```

Replaces each record with a substring.

Positions are 1-based.

Example:

```text
src parser.c
```

```rexx
rc = p.pipe("SUBSTR", "5")
```

produces:

```text
parser.c
```

while:

```rexx
rc = p.pipe("SUBSTR", "5", "6")
```

produces:

```text
parser
```

`start` must be a positive whole number.

`length`, when supplied, must be a non-negative whole number.

Cardinality: `n -> n`

---

### LENGTH

```rexx
p.pipe("LENGTH")
```

Replaces each record with its character length represented as a string.

Example:

```text
"parser.c"  -> "8"
"runtime.c" -> "9"
""          -> "0"
```

Whitespace contributes to the length.

Cardinality: `n -> n`

---

## Record Shape Stages

### SPLIT

```rexx
p.pipe("SPLIT", separator)
```

Splits each record using `separator`.

Each resulting field becomes a separate record.

Example:

```text
src|include|test
```

```rexx
rc = p.pipe("SPLIT", "|")
```

produces:

```text
src
include
test
```

Empty fields are preserved:

```text
src||test
```

becomes:

```text
src
""
test
```

Leading and trailing separators also create empty records.

The separator must not be empty.

Multi-character separators are supported.

Record order is preserved.

Cardinality: may increase.

---

### JOIN

```rexx
p.pipe("JOIN", separator)
```

Combines all records into one record, inserting `separator` between adjacent
records.

Example:

```text
src
include
test
```

```rexx
rc = p.pipe("JOIN", ";")
```

produces:

```text
src;include;test
```

Empty records participate normally:

```text
src
""
test
```

becomes:

```text
src;;test
```

An active empty pipe remains active and empty.

Cardinality:

```text
0 -> 0
1 -> 1
n -> 1, for n > 1
```

---

## Aggregation

### COUNT

```rexx
p.pipe("COUNT")
```

Replaces the current pipeline with one record containing the number of
records.

Example:

```text
src parser.c
src scanner.c
src runtime.c
```

becomes:

```text
3
```

An active empty pipeline therefore becomes one record containing:

```text
0
```

Cardinality: `n -> 1`

---

## Output

### SAY

```rexx
p.pipe("SAY")
```

Copies the current records to the pipe output area and then consumes the
active pipeline.

Records are appended to any records already present in the output area.

After `SAY`:

- the active pipeline contains zero records,
- the pipeline becomes inactive, and
- output records can be retrieved with `outputCount()` and `outputLine()`.

Example:

```rexx
rc = p.pipe("SAY")

do i = 1 to p.outputCount()
  say p.outputLine(i)
end
```

Cardinality: `n -> 0`

---

# Pipe State and Accessors

## `active`

```rexx
p.active()
```

Returns whether the pipeline is active.

Return values are:

```text
1   active
0   inactive
```

An active pipeline may contain zero records.

---

## `recordCount`

```rexx
p.recordCount()
```

Returns the number of records currently in the pipeline.

---

## `record`

```rexx
p.record(index)
```

Returns the requested pipeline record.

Indexes start at `1`.

An out-of-range index returns an empty string.

---

## `outputCount`

```rexx
p.outputCount()
```

Returns the number of records currently held in the output area.

---

## `outputLine`

```rexx
p.outputLine(index)
```

Returns an output record.

Indexes start at `1`.

An out-of-range index returns an empty string.

---

## `reset`

```rexx
call p.reset()
```

Resets the logical Pipe state.

After `reset()`:

- the pipeline is inactive,
- `recordCount()` is zero,
- `outputCount()` is zero, and
- error information is cleared.

---

# Error Handling

A stage returns `0` when an error occurs.

The following methods provide information about the most recent error:

```rexx
p.errorCode()
p.errorMessage()
p.errorStage()
p.errorDetail()
```

Typical error codes currently include:

| Error | Meaning |
|---|---|
| `missing_argument` | Required argument was not supplied |
| `too_many_arguments` | Too many stage arguments were supplied |
| `invalid_argument` | Argument value is invalid |
| `invalid_stem` | Invalid input array information |
| `pipeline_inactive` | An operation requires an active pipe |
| `unknown_command` | Unknown PIPE stage |
| `file_error` | File could not be opened, read, written, or closed |

Each call to `pipe()` clears previous error information before executing the
requested stage.

`fromArray()` and `toArray()` also clear previous error information before
performing their operation.

---

# Stage Summary

| Stage | Purpose | Cardinality |
|---|---|---:|
| `SELECT text` | Keep records containing text | `n -> 0..n` |
| `DROP text` | Remove records containing text | `n -> 0..n` |
| `EMPTY` | Keep empty records | `n -> 0..n` |
| `NONEMPTY` | Keep non-empty records | `n -> 0..n` |
| `STARTSWITH text` | Keep records starting with text | `n -> 0..n` |
| `ENDSWITH text` | Keep records ending with text | `n -> 0..n` |
| `TAKE n` | Keep first n records | `n -> 0..n` |
| `SKIP n` | Remove first n records | `n -> 0..n` |
| `FIRST` | Keep first record | `n -> min(n,1)` |
| `LAST` | Keep last record | `n -> min(n,1)` |
| `SORT` | Sort ascending | `n -> n` |
| `REVERSE` | Reverse record order | `n -> n` |
| `UNIQUE` | Remove duplicates | `n -> 0..n` |
| `UPPER` | Convert to upper case | `n -> n` |
| `LOWER` | Convert ASCII upper case to lower case | `n -> n` |
| `TRIM` | Remove surrounding whitespace | `n -> n` |
| `SPACE` | Normalize whitespace | `n -> n` |
| `REPLACE old new` | Replace text | `n -> n` |
| `PREFIX text` | Prefix records | `n -> n` |
| `SUFFIX text` | Suffix records | `n -> n` |
| `WORD n` | Extract nth word | `n -> n` |
| `WORDS` | Count words per record | `n -> n` |
| `SUBSTR start [length]` | Extract substring | `n -> n` |
| `LENGTH` | Character length per record | `n -> n` |
| `SPLIT separator` | Split records | may increase |
| `JOIN separator` | Join records | `0 -> 0`, `1 -> 1`, `n > 1 -> 1` |
| `COUNT` | Count records | `n -> 1` |
| `SAY` | Copy records to output and consume pipeline | `n -> 0` |
| `READ filename` | Read file lines into records | file lines -> `n` |
| `WRITE filename` | Write records to file without consuming pipe | `n -> n` |

---

# Pipe Method Summary

| Method | Purpose | Pipeline effect |
|---|---|---|
| `fromArray(source)` | Load records from a string array | Replaces records and activates pipe |
| `toArray()` | Copy current records to a string array | No change |
| `reset()` | Reset logical pipe state | Deactivates pipe and clears counts |
| `active()` | Return active state | No change |
| `recordCount()` | Return current record count | No change |
| `record(index)` | Return one current record | No change |
| `outputCount()` | Return output record count | No change |
| `outputLine(index)` | Return one output record | No change |
| `errorCode()` | Return last error code | No change |
| `errorMessage()` | Return last error message | No change |
| `errorStage()` | Return stage associated with last error | No change |
| `errorDetail()` | Return additional error detail | No change |

---

# Snapshot and Restore

`toArray()` and `fromArray()` may be used together to save and restore a
pipeline state.

Example:

```rexx
source = .string[]

source[1] = "src parser.c"
source[2] = "src scanner.c"
source[3] = "src runtime.c"

p = .Pipe()

call p.fromArray(source)

saved = p.toArray()

rc = p.pipe("TAKE", "1")

call p.fromArray(saved)
```

After `toArray()`, the original pipeline remains unchanged.

The returned array is independent of later pipeline transformations.

This provides explicit snapshot semantics without maintaining an internal
pipeline history.

---

# File Round Trip

A pipeline may be written to a text file and later read back.

Example:

```rexx
source = .string[]

source[1] = "src parser.c"
source[2] = ""
source[3] = "src runtime.c"

p = .Pipe()

call p.fromArray(source)

filename = "pipe_io_test.txt"

rc = p.pipe("WRITE", filename)

call p.reset()

rc = p.pipe("READ", filename)
```

The empty middle record is represented by a physical empty line in the file
and is restored as an empty record by `READ`.

---

# Example Pipeline

```rexx
source = .string[]

source[1] = "src scanner.c"
source[2] = "src parser.c"
source[3] = "src runtime.c"
source[4] = "src parser.c"
source[5] = "docs pipe.md"

p = .Pipe()

call p.fromArray(source)

rc = p.pipe("STARTSWITH", "src ")
rc = p.pipe("SORT")
rc = p.pipe("UNIQUE")
rc = p.pipe("WORD", "2")
rc = p.pipe("PREFIX", "[")
rc = p.pipe("SUFFIX", "]")
rc = p.pipe("JOIN", ",")
rc = p.pipe("SAY")

do i = 1 to p.outputCount()
  say p.outputLine(i)
end
```

Result:

```text
[parser.c],[runtime.c],[scanner.c]
```

---

# Current Scope

`.Pipe` is a string-record pipeline with array and optional text-file input
and output.

The current implementation provides sequential stage execution against one
ordered record stream.

The pipeline has one current working record collection:

```text
              .Pipe object
        +-----------------------+
input ->| records_              |
        | current working       |
        | record stream         |
        |                       |
        | stages manipulate it  |
        +-----------+-----------+
                    |
                   SAY
                    |
                    v
              outputLines_
```

`outputLines_` is an output sink. It is not a second processing pipeline.

The class does not maintain an internal history, stack, checkpoint list, or
branching graph.

Independent snapshots may be created explicitly with:

```rexx
saved = p.toArray()
```

and restored with:

```rexx
call p.fromArray(saved)
```

The following are not implied by the current implementation:

- asynchronous execution,
- concurrent stages,
- coroutine semantics,
- external process piping,
- automatic parallelism,
- typed record schemas,
- internal pipeline history,
- automatic save/restore stacks, or
- branching pipeline graphs.

These may be considered separately if required; they are not part of the
current `.Pipe` contract.