# cREXX PipeStreaming

## Status

Experimental synchronous streaming prototype.

This implementation is separate from the collection-oriented Pipe class in
experiments/Pipe. The original implementation remains the behavioral
reference and is not modified by this experiment.

## Purpose

PipeStreaming changes the processing model from:

    complete collection -> stage -> complete collection

to:

    one record -> stage -> next stage

A record passes through the configured stages before the source produces the
next record. The current implementation is synchronous and single-threaded.
It does not create operating-system threads or processes.

## Source File

The implementation and executable samples are in:

    experiments/PipeStreaming/samples.crexx

The main class is:

    .StreamingPipe

## Building and Running a Pipeline

Stages are added in order:

    pipe = .StreamingPipe()
    call pipe.add("READ", input)
    call pipe.add("SELECT", "alpha")
    call pipe.add("UPPER", "")
    call pipe.add("WRITE", output)

The pipeline is executed with:

    result = pipe.run()

The result is a cREXX string array:

    result[0]  record count
    result[1]  first record
    result[2]  second record

The first stage must currently be READ.

## Processing Model

Each stage conceptually receives:

    onRecord(record)
    onEnd()

onRecord may emit zero, one, or multiple records. onEnd is propagated after
the source reaches EOF and allows stages to finish buffered or aggregated
work.

Immediate stages do not materialize intermediate collections. Buffered stages
retain only the state required by their semantics.

## Implemented Stages

### Source and sinks

READ filename

Reads one text record at a time using LINES and LINEIN. Line terminators are
not included.

WRITE filename

Writes each received record immediately and closes the output stream when EOF
reaches the WRITE stage.

SAY

Displays each received record immediately and does not forward it. Its
returned result is therefore empty.

TOARRAY

The run method returns the final records as a string array. A separate
TOARRAY stage is not required by the current prototype.

### Immediate record stages

    SELECT text
    DROP text
    UPPER
    LOWER
    TRIM
    REPLACE old new
    WORD n
    SUBSTR start [length]
    LENGTH
    WORDS
    SPACE
    EMPTY
    NONEMPTY
    STARTSWITH text
    ENDSWITH text
    PREFIX text
    SUFFIX text
    SPLIT separator

Filtering stages preserve order. SPLIT can emit multiple records for one
input record and preserves empty fields.

The current add method accepts one argument string. Multi-argument stages are
therefore encoded in the argument string:

    call pipe.add("REPLACE", "alpha A")
    call pipe.add("SUBSTR", "1 5")

### Stateful stages

    FIRST
    LAST
    TAKE n
    SKIP n
    UNIQUE
    COUNT

FIRST emits the first record and consumes later input. LAST retains the most
recent record and emits it at EOF. TAKE and SKIP maintain counters. UNIQUE
retains values already seen. COUNT emits one result at EOF.

### Buffered stages

    SORT
    REVERSE
    JOIN
    DISPLAY

SORT and REVERSE retain records until EOF. JOIN accumulates one joined value
and emits it at EOF. DISPLAY retains records, displays them in input order at
EOF, and then forwards them downstream.

## DISPLAY

Example:

    pipe = .StreamingPipe()
    call pipe.add("READ", input)
    call pipe.add("SELECT", "alpha")
    call pipe.add("DISPLAY", "")
    call pipe.add("WRITE", output)
    result = pipe.run()

DISPLAY waits for EOF before displaying its records. It preserves input
order. If a downstream stage exists, each displayed record is then forwarded
to that stage.

## File Handling

Named LINEIN and LINEOUT streams remain open between calls. Close a stream
explicitly before reopening it:

    rc = lineout(filename)

This closes the stream. It is different from:

    lineout(filename, "")

which writes an empty record.

The samples close their input and output files between tests.

## Samples

samples.crexx demonstrates:

    READ -> SELECT -> UPPER -> WRITE
    READ -> SPLIT -> WRITE
    READ -> SELECT -> COUNT -> WRITE
    READ -> SORT -> WRITE
    READ -> DISPLAY -> WRITE

It also exercises the immediate, stateful, and buffered stages individually.
Diagnostic logging can be enabled with:

    call pipe.logging(1)

and disabled with:

    call pipe.logging(0)

## Current Guarantees

The prototype provides:

- one input source,
- one synchronous execution path,
- ordered logical string records,
- explicit EOF propagation,
- zero, one, or many output records per input record,
- deterministic stage order, and
- no intermediate collection for immediate stages.

## Not Implemented

The following are intentionally outside the current prototype:

- operating-system process stages,
- concurrent execution,
- queues,
- back-pressure,
- multiple inputs,
- multiple outputs,
- named paths,
- fan-in,
- fan-out,
- loops,
- deadlock detection, and
- RexxScript stages.
