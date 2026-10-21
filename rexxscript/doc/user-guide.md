# RexxScript User Guide

RexxScript is an interpreted, sandboxed Rexx-family scripting surface for small
rules, calculations, and generated-code execution inside CREXX applications.
RexxScript borrows familiar syntax from REXX but implements only the subset
described in this guide; it is not a full REXX interpreter.
It is a RexxScript product, not the Level C compiler path.

**Two explicit versions:** the command/API described first here is legacy
`rexxscript`, including the `REXXSCRIPT` compiler exit. The unreleased
[modern `rexxscript1` review implementation](#rexxscript-1-modern-level-l-review-implementation)
is selected separately and uses the generated Level L tree. Neither command
automatically tries the other dialect.

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

For array sharing, declare the host variable as `.string[]` before the call
and expose only its name:

```rexx
measurements = .string[]
REXXSCRIPT script EXPOSE measurements
```

The array is transferred as `measurements.0`, `measurements.1`, and so on.
Other host array types, such as `.int[]`, are rejected by the compiler exit
with a clear diagnostic that exposed arrays must be `.string[]`. A scalar
declaration such as `.string` remains a scalar expose.

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

`ELSE IF` is currently unsupported. This limitation may be lifted in the future.

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
Conditions support one test or comparison at a time; compound conditions joined
with `&` or `|` are not supported.

## Variables

RexxScript stores script variables internally as strings. Arithmetic and
comparisons perform conversion when needed. Values copied back through
`EXPOSE` then follow the receiving CREXX variable's type conversion rules.

Variable names are matched case-insensitively.

## Intrinsic Functions

RexxScript supports only the intrinsic functions listed here. The set includes
shared Classic-compatible functions and a small number of RexxScript-specific
intrinsics; it is not the full REXX BIF library.

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
QPOS
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
QWORD
WORDS
QWORDS
WORDPOS
VALUE
LINEIN
LINEOUT
LINES
ERASEFILE
```

Function names are matched case-insensitively. RexxScript owns the sandbox-safe
name controller. Classic-compatible intrinsics use the shared `rxfnsc`
`RexxClassicBifs` implementation; the quote-aware `QPOS`, `QWORD`, and
`QWORDS` intrinsics use the imported `rxfnsb` implementations.

`VALUE(name)` is a RexxScript-specific, read-only intrinsic. It evaluates its
argument to a variable name and returns that variable's value from the current
RexxScript variable pool. Names are case-insensitive. It is useful for reading
RXPP-generated scalar names such as `MEASUREMENTS_1`; dynamic assignment is not
supported.

```rexx
do i = 1 to MEASUREMENTS_COUNT
  say VALUE('MEASUREMENTS_' || i)
end
```

### One-dimensional string arrays

RexxScript supports script-local one-dimensional string arrays using dotted
names. The `.0` element conventionally stores the element count:

```rexx
words.0 = 3
words.1 = 'red'
words.2 = 'green'
words.3 = 'blue'

do i = 1 to words.0
  say words.i
end
```

The suffix may be a literal non-negative integer or the name of an integer
variable. Missing elements remain undefined variables. Host cREXX array
Host cREXX arrays can be exposed by declaring the host variable as `.string[]`
and using `EXPOSE name`; the compiler exit transfers the elements through this
dotted representation.

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

The quote-aware word and position functions treat quoted text as part of the
surrounding token. `QWORD` preserves quote delimiters, `QWORDS` counts quoted
words, and `QPOS` finds a delimiter only outside matching single or double
quotes. Doubled quote characters are accepted as escaped quote characters.

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

## RexxScript 1: modern Level L review implementation

For an explanation of how source becomes tokens and trees, how the IF rules
attach ELSE, and how the interpreter walks the result, read the
[Level L process overview](../../langtools/doc/user-guide.md#overview-from-a-language-description-to-a-useful-program).
It also distinguishes generated token stages from RexxScript's handwritten
layout and concatenation passes.

The modern implementation is selected explicitly:
`rexxscript1`, `rexxscript1.rxbin`, namespaces `rexxscript1` and `rexxscript1run`.
It uses a [Level L grammar](../grammar/rexxscript.levell), Level G
NUMERIC_COMMON expressions and typed RexxValue values. It is for review and has
not been released. The `rexxscript` command and APIs described above continue to
select the legacy evaluator; the implementation never guesses a dialect from
whether parsing succeeds.

```sh
cmake --build cmake-build-levell-product --target rexxscript1_cli
cmake-build-levell-product/bin/rexxscript1 rexxscript/examples/modern.rexx
```

The example prints `total: 46`, `café`, `ok` and returns a typed result of 46
through the reusable API. The CLI exit code is execution status, not the script
RETURN value. It prints captured SAY output and a located diagnostic on failure.

| Area | Modern review contract |
|---|---|
| Values/names | Scalar integer, float, explicit `d` decimal, string and shared-BIF RexxValue results. Assignment creates a case-insensitive local name. Undefined reads fail. |
| Expressions | Level G NUMERIC_COMMON precedence; right-associative power binds more tightly than prefix minus. Integer `/` divides as integer; floating operands select floating arithmetic. Comparisons, blank/abuttal/explicit concatenation, short-circuit AND/OR and named integer operators are implemented. Named operators are case-insensitive. |
| Literals | Decimal integers, ordinary dotted/exponent floats, decimal `d` suffix, `0x` integers and single/double quoted strings with doubled quote escaping. UTF-8 string contents are preserved. |
| Statements | Assignment, IF/THEN/ELSE, simple/count/WHILE/UNTIL/FOREVER/controlled DO, LEAVE, ITERATE, CALL of a BIF, RETURN, SAY, COLLECT, NOP. ELSE binds to the closest unmatched IF. Controlled DO supports `TO` and optional `BY`. |
| Layout | Newlines/semicolons terminate clauses. A comma before a physical newline continues a clause; commas in calls are argument separators. Parenthesized newlines and THEN/ELSE layout are normalized. A semicolon is never a continuation. Nested `/* */` comments preserve original byte offsets. |
| Functions | Existing Level C BIF bodies and RexxBifCallContext, including omitted-argument flags and the per-run RexxVariablePool/configuration. This is the existing rxfnsc connection; Level G expression rules do not change Classic BIF contracts. |
| Output | SAY collects printable lines; COLLECT appends typed values available through the API. RETURN terminates the run and stores a typed result. |
| Exclusions | Declarations, classes, collections/indexing, script-defined procedures, imports, SIGNAL, GOTO and INTERPRET. No host registration, exposed inputs/outputs or write-back contract is selected yet. |

This is a specified scalar subset, not complete Level G. Identifiers are ASCII
letters/underscore plus digits. Named integer operators are contiguous angle
bracket tokens. Type constructors/casts, binary/hex quoted literals, Unicode
operator aliases, and the wider compiler's identifier and contextual-keyword
rules are outside this first surface. A leading statement word can be assigned
(e.g. `say=7; say say`); THEN/ELSE and DO-header words have the particular finite
classification rules shown in the grammar, rather than a promise that every
keyword is soft in every position. The grammar and these examples are intended
for language review.

Numeric comparisons preserve the runtime operand types. In particular,
`0.1+0.2=0.3` is false for ordinary binary floats, while
`0.1d+0.2d=0.3d` is true for decimals. The integrated Level G compiler now
preserves the first result during constant folding. Optimized/unoptimized
compiler runs and RexxScript agree on the 28-case scalar overlap panel; the
earlier RS-F01 discrepancy and its repair remain in the review evidence.

The dispatch module lists the available Level C functions. File, stream,
activation/context and clock services such as LINEIN, STREAM, ARG, DATE and TIME
return `RS_CAPABILITY` until the host contract is selected. Unknown names return
`RS_FUNCTION`; BIF argument errors retain their existing Level C codes. RANDOM
and QUEUED retain their shared library behavior; no independent replacement or
host-wide isolation claim is made for those services.

A script is parsed and structurally validated before its first statement runs.
Syntax recovery never produces an executable program. Runtime failure preserves
already captured output in the result; it does not roll back BIF effects.
Diagnostics include a stable code, line/column, byte offset and source excerpt;
malformed UTF-8 uses a safe byte excerpt. Resource exhaustion is distinct from
invalid syntax (`RS_PARSE_LIMIT`, `RS_FUEL`, `RS_DEPTH`, `RS_VALUE_LIMIT`,
`RS_OUTPUT_LIMIT`); cancellation is `RS_CANCELLED`.

Default limits are 1 MiB source, 1,000,000 parser actions, nesting 128, 100,000
execution ticks, 1 MiB output/value extent and 10,000 output lines. Selected
expanding BIF requests are checked before allocation; this is not a hard bound
on every allocation or arbitrary shared/host library work. Cancellation is
currently a synchronous run flag, tested before running. An asynchronous
cross-worker cancellation/host API has not been selected or qualified.

### Application trees, analysis and folding

`rexxscript1..compile(source)` returns a reusable parsed program. Call
`program.ast()` to obtain an owned application tree with named fields. An
assignment has `name` and `value`; an IF has `condition`, `yes`, and an optional
branch represented by the `IfElse` kind's `no` field. The interpreter uses these
fields instead of parser child positions. `program.view()` retains the named
parse view for inspection.

The `rexxscriptanalysis` module uses Level L's children-first analysis service.
Its `analyse(reference tree)` function calculates constants and conservative
direct variable-read/effect information. Calls are marked as potential effects.
Its `fold(reference tree)` function returns a
new, checked tree with eligible constant expressions replaced by exact typed
literals. Keep the original tree alive while using its analysis results; a
copied or rewritten tree needs its own results.

Execution validates control placement and nesting before folding, then follows
only the selected branches. Analysis never invokes a BIF. Arithmetic that fails
or cannot be represented by an exact literal stays in the tree, so an unselected
`1/0` does not become a preparation error. Folding preserves numeric types,
floating-point bits and original diagnostic locations.

The constructor `execution(program, fuel, maximum, fold)` enables folding by
default. Set the fourth argument to `0` to inspect ordinary execution. Both
paths have the same language and function behavior. Folding can reduce the
number of execution ticks needed. The [worked analysis
example](../examples/ast-analysis.crexx) and [Level L analysis guide](../../langtools/doc/user-guide.md#calculate-facts-about-a-tree)
show the complete pipeline and the common result-table API.
