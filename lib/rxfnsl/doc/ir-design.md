# Extensible Level L IR: review design

**Proposed API and representation for compiler STEP01. Nothing in this document
is an implemented public API yet.** The [compiler plan](../compilers/PLAN.md)
owns delivery. Existing AST/flow APIs remain as documented in the user guide.

## The basic model

An intermediate representation describes operations and how their results are
used. Level L supplies its structure and algorithms. An application registers
the operations, types and semantic services appropriate to its language.

A **module** owns functions, symbols, constants and source origins. A
**function** owns local slots and blocks. A **block** is an ordered sequence
of operations ending in one control operation. A **value** is an immutable
result of an operation. A **slot** is mutable storage within one invocation.
An **edge** connects one block to a possible successor.

The proposed first profile uses explicit slots and block-local temporary
values. A temporary is defined once and may be used only later in that block.
An expression result needed across a branch is stored in a typed slot and
loaded after the join. Constants and incoming arguments become block-local
values through explicit operations. This avoids phi nodes: special join values
used by an SSA representation. Full SSA may be a later checked profile; this
first profile does not pretend that arbitrary cross-block value uses are valid.

```text
function main -> void
  slot total : crexx.g.int64
  entry:
    %a = core.constant 2 : crexx.g.int64
    %b = core.constant 3 : crexx.g.int64
    %sum = crexx.g.iadd %a, %b
    core.store total, %sum
    core.return
```

This is proposed readable IR notation, not cREXX source. `%sum` identifies an
immutable result. `total` identifies a slot. Integer overflow belongs to the
`crexx.g.iadd` contract; the generic graph does not decide its arithmetic.

## Standard operations

| Operation | Contract in the first profile |
|---|---|
| `core.constant` | Materialise an exact typed constant from the module pool. The registered type defines its encoding and validates it. |
| `core.argument` | Read a typed argument of the current function. Source-language missing arguments require a separate explicit representation. |
| `core.load`, `core.store` | Read/write a named function slot of the exact declared type. No implicit casts or dynamic aliasing. |
| `core.jump` | End the block with one successor in the same function. |
| `core.branch` | Consume a Level L Boolean and end with explicit true/false successors. Source truth conversion happens before this operation. |
| `core.return` | End the function with values matching its IR signature. Source EXIT, Classic no-value return and resumable transfers are separate contracts. |

A Boolean is a distinct logical type with two values. The G mapper may use a
representation-compatible G Boolean after an explicit checked type agreement.
Classic converts its value with the appropriate IF/WHILE logical-validation
operation. Numeric truth is never assumed by `core.branch`.

Calls are registered operations with signatures, effect and control contracts;
there is no default claim that an unknown call is pure or always returns.
Exceptional termination is an explicit possible outcome, distinct from the
normal block successor. The first profile permits propagation out of a
function, not a hidden in-function handler. Later catch/resumption operations
must introduce their edges and activation contracts before analyses admit them.

## Schema and semantic extension API

Use an ordinary Level B API first. It fits existing cREXX classes and interfaces
and avoids introducing a second declaration language before the contracts are
tested. Convenience operations cover common cases; a named builder handles
unusual arity and attributes.

The proposed registration example is:

```rexx
schema = .llir..schema()
g = schema.family('crexx.g', 1)
int64 = g.type('int64', .gsemantics..int64type())
add = g.operation('iadd')
call add.input('left', int64)
call add.input('right', int64)
call add.output('value', int64)
call add.state('none')
call add.failure('overflow')
call add.termination('returns')
call add.evaluator(.gsemantics..integeradd())
contract = schema.finish()
```

`state('none')` says this operation does not access mutable program state. It
does not erase the declared overflow outcome. `termination('returns')` excludes
divergence and nonlocal transfers; declared failure can still propagate.
Absent declarations mean unknown, not the reassuring default. `finish()`
validates the schema and returns a frozen contract or diagnostics.

The corresponding small construction surface is:

```rexx
draft = .llir..builder(contract)
main = draft.function('main', contract.voidtype())
entry = main.block('entry')
left = entry.constant(int64, '2', origin)
right = entry.constant(int64, '3', origin)
sum = entry.binary(add, left, right, origin)
total = main.slot('total', int64)
call entry.store(total, sum, origin)
call entry.returnvoid(origin)
built = draft.finish()
```

These are proposed class/method spellings. `origin` is an already-created checked
source origin. Literal text is accepted at this construction boundary by the
type codec; the stored constant has an exact typed encoding. Hot passes use
constant handles and resolved descriptors, not repeated numeric text parsing.

`entry.binary` is shorthand for a checked operation with two inputs and one
output. A general operation builder exposes `input(name,value)`,
`attribute(name,typedValue)`, `successor(name,block)` and `finish()`, returning
named result handles. Registering an operation does not require every caller
to use that verbose form.

| Service | Proposed contract |
|---|---|
| Type codec | Validate exact constant encoding, decode/encode external text, and supply equality appropriate to that encoding. Float bit identity and numeric equality are separate operations. |
| Operation verifier | Read-only operation and schema view -> diagnostics. Check additional relationships beyond structural type/arity checks. |
| Effect summary | State domains read/written, I/O/trace, possible failures, divergence, transfer and alias/escape behaviour. Explicitly unknown components remain unknown. |
| Constant evaluator | Read-only constant operands and proved semantic context -> `constant`, `keep`, `invalid` or `limit`. No access to the running program's variable pool or mutable host numeric defaults. |
| Lowering service | Read-only operation -> checked replacement fragment in a destination builder, with result/origin mappings and control/effect obligations. |

`constant` includes all exact result values and consumed context facts. `keep`
means no justified fold; possible runtime overflow/division errors use this
outcome rather than turning an unreachable expression into a compile error.
`invalid` means malformed IR or a broken evaluator contract. `limit` preserves
the input and identifies the exhausted budget. Optional optimisation may then
retain the original operation; mandatory lowering must report failure.

The folding coordinator verifies result type/arity, facts and effect eligibility
before replacement. An evaluator's answer alone does not authorise deleting
trace events, state reads or errors. The compiler must supply independently
tested or proved semantic contracts. Structural checking cannot prove that an
arbitrary user-supplied evaluator computes the right answer.

Registration is explicit and in-process, using cREXX service objects. A saved
IR image contains schema/family identities, never code to load or run. Numeric
codes are resolved once from namespaced, versioned families. A module needing
an unknown family cannot compile; an inspection tool may retain opaque records
only when the envelope is structurally valid and its limitations are explicit.

## Effects and facts

State domains describe what may change: for example a Classic variable pool,
numeric context, argument frame, output stream or an unknown external resource.
Domain identity and possible aliasing matter. Two different pool handles are
not automatically disjoint. Initially, unknown calls invalidate every fact
about mutable state they may access; there is no speculative motion across them.

Facts belong to a module revision, semantic-profile version and analysis run.
They can record constant values, reachable blocks and definitely assigned slots.
Sparse absence means unknown. An explicit false fact remains distinguishable
from absence, following the existing `has...()`/`get...()` property convention.
Source AST properties are not automatically trusted optimisation facts.

The first implementation retains operation order. It folds eligible constants,
propagates block-local values, simplifies proved branches and removes operations
proved discardable. A Boolean fact can remove an impossible edge without
removing the computation that produced the Boolean.

## Storage proposal

The baseline candidate is fixed-width little-endian records in Level B `.binary`
buffers, addressed through numeric IDs. Access uses the existing binary-memory
intrinsics; no native compiler helper is added. Compare this layout with packed
parallel columns during STEP02 before selecting the permanent working layout.

| Record | Candidate fields |
|---|---|
| Module | Schema families, functions, symbols, constants, source files, origins, properties and profile. |
| Function | Name/signature, entry block, block/slot lists, source origin and ABI extension attributes. |
| Block | Owning function, first/last operation, flags. |
| Operation | Opcode, block, next operation, operand start/count, result start/count, successor start/count, attribute-set ID, origin ID, flags. |
| Value | Defining operation, result ordinal, type ID, flags. |
| Slot | Owning function, type, name, flags; initialisation is explicit executable IR. |
| Origin | Exact source identity/ranges, provenance kind and optional contributing-origin IDs. |

With twelve 32-bit fields an operation header occupies 48 bytes; that is only
the candidate header cost. Operand/value/edge tables, constants, names, indexes,
sources and sparse facts all contribute to total memory. Record this full
storage formula and measure it. Numeric IDs are not machine pointers and do not
imply zero allocation or automatic packing of ordinary `.int[]` arrays.

ID zero is absent only in fields whose schema permits absence. Counts and
offset arithmetic are checked before allocation/access. A representational
ceiling produces a clear diagnostic; resource policy and representation are
reported separately. Opcode/record widths are encoding choices, not limits
silently imposed on a user's source grammar.

Only the owning builder mutates buffers. Handles carry owner identity, revision
and ID through explicit Level B references. Appending/growing storage preserves
IDs. A transform publishes a new frozen owner with a mapping where needed;
compaction may change IDs only with that mapping. No handle stores a byte
pointer that would survive a moved buffer accidentally. No record recycling
can make a stale handle refer to another operation.

Use dense records for common fields and sparse side tables for optional facts.
Intern immutable repeated names/types where equality permits it. Store exact
constants separately: decimal text/coefficient data, float bits, arbitrary
bytes and Classic source spelling are not coerced into one integer field.
Strings and byte sequences use explicit lengths, including embedded zero.

## Verifier and transformations

The first verifier checks schema/profile, unique ownership, valid ranges/IDs,
types and arity, operation order, same-block earlier definitions, signature
agreement and valid terminators/targets. Slot analysis separately establishes
which slots are initialised on every path reaching a load. Unreachable blocks
still undergo structural and type checks. Incomplete control summaries prevent
a claimed complete flow analysis.

An operation sequence can produce a runtime failure and terminate abnormally;
this is not a malformed graph. The semantic profile defines those outcomes.
Recovered-source damage, in contrast, prevents executable publication.

Analyses and rewrite inputs are read-only. A pass stages a candidate, checks
the result and commits it atomically. Failed validation or a budget stop leaves
the original owner usable and provides operation/source diagnostics. An
unchanged pass may return the original owner. Measure full-copy versus safe
sharing costs before promising persistent or copy-on-write storage.

## Readable and portable forms

Provide deterministic dumps with names, types, origins and optional facts;
filtered function/block/source-span views; and a structured programmatic view.
The complete text form round-trips a semantic module. Filtered views are not
presented as reloadable modules. Passes access handles directly without text.

A saved binary image declares magic/version, schema families, byte order,
record widths, counts, section ranges and constant encodings. Validate lengths,
overflow, references, type contracts and resource limits before publishing a
loaded owner. File-local IDs are resolved within that owner. Native pointers,
borrowed handles and arbitrary registered callback code never enter the image.

Text/binary round trips compare module semantics, exact observable constants
and source origins. They need not preserve temporary numeric IDs after an
explicit renumbering. Unknown versions, truncation and impossible counts are
tested refusals. Layout/codec decisions and measurements remain review items;
no speed claim follows merely from choosing a binary format.
