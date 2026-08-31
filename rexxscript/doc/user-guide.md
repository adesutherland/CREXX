# RexxScript User Guide

RexxScript is an interpreted, sandboxed Rexx-family scripting surface for small
rules, calculations, and generated-code execution inside CREXX applications.
It is a RexxScript product, not the Level C compiler path.

RexxScript is delivered as:

- `bin/rexxscript.rxbin`: runtime image used by CREXX programs.
- `bin/rexxscript`: standalone runner for RexxScript source files.

## Standalone Runner

Run a RexxScript file directly:

```sh
rexxscript rules.rxs
rexxscript rules.rxs --debug
```

The runner evaluates the file with an isolated evaluator instance, prints
captured `SAY` output, and exits non-zero when evaluation does not return `OK`.

Example `rules.rxs`:

```rexx
say 'starting'
score = 7
score = score + 3
if score >= 10 then say 'approve'
```

## CREXX Command Integration

CREXX programs use the `REXXSCRIPT` command:

```rexx
options levelb
import rexxscript

score = "7"
decision = ""
out = .string[]
raw = .string[]

rule = "score = score + 3; if score >= 10 then decision = 'APPROVE'; say decision"

REXXSCRIPT rule EXPOSE score, decision OUTPUT out RESULT raw

if script_status <> "OK" then do
  say "RexxScript failed:" script_status
end
```

`EXPOSE` copies named host variables into the RexxScript sandbox before
execution and copies them back afterwards. Variables not listed in `EXPOSE`
remain local to the script.

`OUTPUT` receives a `.string[]` array containing captured `SAY` output.

`RESULT` receives the raw evaluator result array. `raw[1]` is the status.

## Direct Runtime API

Use the direct API when a program needs multiple independent RexxScript
instances in the same process:

```rexx
options levelb
import rexxscript

runner = .rexxscript..rexxscriptevaluator()
result = runner.evaluate("value = 'A'; say value", 0)
out = runner.output()

say result[1]
say out[1]
say runner.value("value")
```

Each evaluator instance owns its own variable pool, output buffer, labels, and
parser state. Reusing the same instance for another `evaluate()` resets the
script state for that instance.

The procedural helpers `rexxscript_evaluate`,
`rexxscript_evaluate_exposed`, `rexxscript_output`, and
`rexxscript_value(name)` remain for compatibility and simple call sites. They
report the last procedural or facade evaluation, so prefer explicit evaluator
instances when multiple scripts are active.

## Sandbox Model

RexxScript runs in a sandbox variable pool.

- Intrinsic BIF helpers receive the RexxScript sandbox pool as caller context.
- `LINEIN()` and `LINEOUT()` use streams owned by the evaluator instance.
- `LINES()` reports whether another line is available without consuming it.
- Multiple filenames may be open at once; repeated `LINEIN()` calls on one
  filename continue from that filename's retained position.
- `LINEOUT(filename)` without a line closes that evaluator-owned stream.
- `LINEOUT("stdout", text)` appends to the evaluator's captured output.
- `ERASEFILE(filename)` truncates or creates a file and returns `0` on success
  or `-1` when it cannot perform the operation.
- `DATE()` and `TIME()` use the shared Level B date/time implementations,
  including their documented optional format arguments.
- `COLLECT expression` appends one scalar application value without affecting
  `SAY` output; embedded callers receive these values through `VALUES`.
- The host CREXX variable pool is not exposed directly.
- Host variables cross the boundary only through `EXPOSE`.
- RexxScript does not provide `ADDRESS`, shell command execution, external
  function dispatch, or host environment access in the current slice.

## Language Surface

Statements supported in the current slice:

```text
assignment
SAY
IF / THEN
IF / THEN / ELSE
DO / END
DO WHILE
DO UNTIL
DO variable = start TO limit [BY step]
LEAVE
ITERATE
label:
SIGNAL label
GOTO label
COLLECT expression
RETURN
CALL intrinsic(...)
```

`GOTO label` is accepted as an alias for `SIGNAL label`.

Expressions support:

```text
integer literals
string literals
variables
intrinsic function calls
string concatenation with ||
implicit string concatenation between adjacent terms
integer arithmetic: + - * /
comparisons in conditions
```

Expression evaluation is intentionally simple and left-to-right.

## Variables

RexxScript stores script variables internally as strings. Arithmetic and
comparisons perform conversion when needed. Values copied back through
`EXPOSE` then follow the receiving CREXX variable's type conversion rules.

Variable names are matched case-insensitively.

## Intrinsic Functions

RexxScript currently supports this shared Classic-compatible intrinsic set:

```text
ABBREV
ABS
CHANGESTR
COUNTSTR
COPIES
DATE
DATATYPE
DELSTR
DELWORD
LENGTH
LEFT
INSERT
LASTPOS
LOWER
MAX
MIN
OVERLAY
POS
RIGHT
SIGN
SPACE
STRIP
SUBSTR
TIME
TRANSLATE
UPPER
VERIFY
WORD
WORDS
WORDPOS
LINEIN
LINEOUT
LINES
ERASEFILE
```

Function names are matched case-insensitively. RexxScript owns the sandbox-safe
name controller, which routes each allowed intrinsic to its specific shared
`rxfnsc` `RexxClassicBifs` implementation.

The embedded `REXXSCRIPT` statement supports `VALUES array` for collecting
values produced by `COLLECT expression`. The target is a `.string[]`, and values
are stored in execution order. `SAY` remains available through `OUTPUT`; the
two streams are independent. The raw `RESULT` vector keeps its existing
layout for scripts that do not execute `COLLECT`.

Examples:

```rexx
x = left(name, 10)
y = substr(code, 2, 4, ".")
z = words("Now is the time")
```

Nested intrinsic calls are supported:

```rexx
x = length(substr("abcdef", 2, 3))
```

## Errors

Successful execution returns status `OK`.

For the `REXXSCRIPT` command, the status is also available in
`script_status`.

For direct API calls, check `result[1]`:

```rexx
result = runner.evaluate(rule, 0)
if result[1] <> "OK" then say "RexxScript failed:" result[1]
```

The exact diagnostic wording is not part of the stable API yet. Tests should
prefer checking for the stable condition or code when one is available.

## Current Limits

The current RexxScript slice is intentionally small:

- no general function dispatch beyond the documented intrinsic set;
- no `CALL`;
- no `ADDRESS`;
- no object model inside RexxScript;
- no arrays or stems in the script language surface;
- no `INTERPRET`;
- no direct host variable-pool access;
- no source mapping beyond current runtime status strings.

These limits keep the sandbox and runtime behavior predictable while the
Release 1 beta work establishes the shared Rexx value, variable-pool, and BIF
foundation.
