# One program through both frontends

**Proposed trees and IR for STEP01 review.** The source programs and their
outputs are checked with the existing tools; the IR below is explanatory design
notation. It is not generated output from the new compiler.

## Source and intended result

The [C program](../examples/step01/pipeline-c.crexx) begins:

```rexx
options levelc
total = 2 + 3
if total > 4 then say twice(total)
else say 0
```

The [G program](../examples/step01/pipeline-g.crexx) adds an explicit declaration:

```rexx
options levelg
import rxfnsb
total = .int
total = 2 + 3
if total > 4 then say twice(total)
else say 0
```

Each full program then counts `total` down to zero, prints it, calls LENGTH on
`'cat'`, and exits. `twice` is a local routine with an argument and RETURN.
Both produce `10`, `0`, `3`, each on its own line. The shared results do not
make their variable or numeric models identical.

## Tokens, tree and checks

For `total = 2 + 3`, scanning identifies a name, equals sign, numeric spelling,
plus sign and numeric spelling, retaining byte ranges and the clause boundary.
The source tree in either schema contains:

```text
Assign(name="total")
  value: Add
    left:  Literal(spelling="2", form="integer")
    right: Literal(spelling="3", form="integer")
```

The G binding/type tables resolve `total` to its declared int64 slot and
determine typed addition. The C tables retain the canonical pool name `TOTAL`;
addition uses Classic numeric rules and the active context. Its spelling-aware
literal values are not replaced with G integer nodes.

## Before optimisation

The selected G entry block might contain:

```text
slot total : crexx.g.int64
entry:
  %zero = core.constant 0 : crexx.g.int64
  core.store total, %zero                    ; declaration default
  %two = core.constant 2 : crexx.g.int64
  %three = core.constant 3 : crexx.g.int64
  %sum = crexx.g.iadd %two, %three
  core.store total, %sum
  %current = core.load total
  %four = core.constant 4 : crexx.g.int64
  %test = crexx.g.igt %current, %four
  core.branch %test true yes false no
```

The corresponding C entry uses its activation and numeric context:

```text
entry:
  %activation = core.argument 0 : crexx.c.activation
  %two = core.constant spelling("2") : crexx.c.value
  %three = core.constant spelling("3") : crexx.c.value
  %numeric = crexx.c.numeric_snapshot %activation
  %sum = crexx.c.add %two, %three, %numeric
  crexx.c.pool_write %activation, name("TOTAL"), %sum
  %current = crexx.c.pool_read %activation, name("TOTAL")
  %four = core.constant spelling("4") : crexx.c.value
  %comparison = crexx.c.greater %current, %four, %numeric
  %test = crexx.c.if_boolean %comparison
  core.branch %test true yes false no
```

`name(...)` denotes a schema-checked symbol attribute, not a value operand.
The numeric snapshot is an immutable value representing the applicable context;
the operation that obtains it declares its state read. The example's context
is unchanged between arithmetic and comparison. A context-changing call would
require a fresh snapshot. Other blocks obtain their own argument values;
temporary IDs cannot be used across blocks in this first IR profile.

## Shared optimisation, different arithmetic services

The common folding pass calls the registered G integer evaluator for `%sum`.
It proves the sum is 5 with no overflow and replaces only that operation with
an exact constant. The same pass can fold the C addition when the context facts
establish the entry numeric profile and no observable event is lost. If the
context is unknown it keeps the C operation.

The initial propagation pass handles block-local results and proven local-slot
facts. It can replace the G load after its store and simplify the comparison.
It does not assume that a Classic pool read is a typed local load. Pool alias
and condition semantics require their own proof before such propagation.

When a branch condition is proved true, the pass replaces the branch with a
jump to `yes`. Any required computation, failure or trace event that precedes
the branch remains. Dead code removal uses normal and admitted abnormal
control outcomes, not source indentation.

## Loops and short-circuiting

The G loop uses the same slot across blocks:

```text
header:
  %n = core.load total
  %zero = core.constant 0 : crexx.g.int64
  %test = crexx.g.igt %n, %zero
  core.branch %test true body false after
body:
  %n = core.load total
  %one = core.constant 1 : crexx.g.int64
  %next = crexx.g.isub %n, %one
  core.store total, %next
  core.jump header
```

Names such as `%n` are local display names here. Their numeric value IDs remain
unique in the module. ITERATE jumps to `header`; LEAVE jumps to `after`.
Classic uses the same block topology but explicit pool operations and Classic
WHILE Boolean validation.

For G `left & right`, evaluate `left`, branch, and evaluate `right` only on the
true edge. A hidden Boolean slot holds the expression result at the join.
For C, evaluate both operands in order, preserving the left value across the
right's effects, then call the Classic logical operation. The
[evaluation pair](poc-scope.md#runnable-source-examples) demonstrates why those
lowerings differ.

## RXAS and source explanation

Target lowering resolves operation families into RXAS instructions and runtime
calls. G scalar addition can map to typed integer instructions; Classic pool
and value operations map to the existing runtime's explicit ABI. Register
assignment keeps live results out of call argument windows and preserves
cleanup obligations. Origins map instructions back to the authored operator
or statement even when an operation was folded or generated.

The existing-compiler baseline emitted a real `rxfnsb.length()` call and its
signature metadata. Its register choices are comparison evidence, not the
proposed backend design. STEP04 will add actual new-compiler RXAS and source
maps to this walkthrough. STEP05 will add generated before/after IR dumps and
the corresponding pass facts.

For a missing operand, the parser reports the source position before an AST
can become executable. For a bad IR extension input, diagnostics name the
operation, expected type, supplied type and its source origin. For example:

```text
LL_IR_OPERAND_TYPE at rules.crexx:4:11
crexx.g.iadd input 'right' requires crexx.g.int64; received crexx.g.string.
```

This is proposed wording; it explains the contract without exposing raw opcode
numbers as the only useful information.
