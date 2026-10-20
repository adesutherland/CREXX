# STEP07A: scalar call contracts and tree design

**Approved and implemented bounded scalar-call contract.** The API and source
forms below are implemented; the [active plan](../PLAN.md) now tracks complete C.
The [acceptance checks](#acceptance-and-next-implementation-checks) below define
this slice, not wider B/G or complete Classic coverage.

## Intended result and ownership

An optional argument has two independent facts: its original presence and the
value used by the routine. Supplying a default or assigning the formal changes
the value, not whether the caller supplied it. Each activation owns those
facts, including recursive calls. An empty string is a supplied value.

Use the existing typed RXAS calling convention. A caller passes positions and
presence; a callee evaluates its own default. Exported metadata declares
`value=.int,?factor=.int`, without copying the literal `2` into an importing
compiler. Changing a provider's default must not require rebuilding its caller.
No new RXBIN metadata record, native primitive or G host dependency is needed.

Level L owns structural argument absence, checked typed values, exports and
their saved numeric records. The compiler owns default expressions, conversions,
C argument packets, metadata syntax, symbol resolution and the RXAS ABI.

## Inventory and admitted scope

| Form | Before STEP07A | STEP07A disposition |
|---|---|---|
| Fixed required scalar G arguments | Implemented | Retain |
| `factor = 2`, signed numeric and quoted-string literal defaults | Refused by G grammar | Admit; infer the existing scalar type |
| `?factor = .int` and other admitted primitive scalar type defaults | Refused | Admit the primitive default; no class/factory default |
| `?factor` expression | Refused lexically | Admit formal presence only; reject unknown/non-formal names |
| Missing middle/trailing optional actual; supplied empty value | G requires exact count/all positions | Admit omission only at optional positions; apply ordinary conversion to supplied values |
| Optional formal before a required formal | Refused | Admit; requiredness is positional, not a minimum-prefix count |
| Scalar `.int`, `.float`, `.decimal`, `.string`, `.boolean` and `.void` results | Implemented subset | Retain; `.void` is never an argument type |
| C local argument packets, ARG presence/default logic | Implemented | Retain; strengthen contracts and regression coverage |
| C LENGTH, ARG count/existence, DIGITS target routes | Implemented, selected metadata checked | Retain this explicit allowlist; also reject optional/required mismatches |
| G `import rxfnsb` / LENGTH | Implemented special case | Preserve through the checked import path |
| G `namespace scalelib expose scale` and `import scalelib` | Refused except rxfnsb import | Admit a declaration-only scalar provider and explicit qualified consumer calls |
| G `scalelib..scale(...)` | Refused | Admit existing qualification syntax; selected symbol must be exported |
| Arbitrary defaults, arrays, classes, references, variadic tails | Refused | Keep located refusals; later increments |
| Provider initialisers, source imports, arbitrary native providers | Refused | Later coverage; no executable library initialisation in this slice |
| General C external loading, pool exposure, INTERPRET | Refused | Existing later gates remain open |

Literal admission must consume the whole literal, not evaluate arbitrary
expressions. The source lexer/grammar owns quoting and signs. Historical
authored RXAS literal-default metadata is interpreted only within the admitted
scalar subset of the existing [metadata contract](../../../../compiler/docs/metadata_conformance.md).
Unadmitted forms get a located import/profile error, never a different signature.

## Small public Level L additions

Keep the current builder and block vocabulary. Callable spellings:

| API | Contract |
|---|---|
| `function.argument(type, optional=0)` | Existing calls remain required. `optional` is exactly 0 or 1. Declare before blocks; checked against the owning frozen schema. |
| `block.hasarg(ordinal, origin)` | A `core.boolean` value for original presence in this invocation. Required arguments are always present under the checked call contract. No value read or default evaluation. |
| `block.readarg(ordinal, origin, fallback)` | The existing two-argument form remains for required arguments. An optional argument requires a same-type fallback value from this block. Returns the supplied value or fallback without altering presence. |
| `function.exportas(name)` | Give a defined function one external symbol. Duplicate/empty exports are rejected. Omission retains private linkage. Language-specific identifier rules stay in the compiler. |
| `module.function(id)` | A checked borrowed function view, matching existing block/operation lookup. Frozen owners still reject mutation. |
| Function views: `name()`, `resulttype()`, `arguments()`, `argumenttype(n)`, `optional(n)`, `exportname()` | Read the same numeric records used by verification and codecs; avoid a separate signature table in each consumer. Invalid/stale/foreign access follows current handle rules. |

Level B construction:

```rexx
scale = draft.function('scale',integer)
call scale.exportas('scalelib.scale')
valuearg = scale.argument(integer)
factorarg = scale.argument(integer,1)
entry = scale.block('entry')
value = entry.readarg(valuearg,origin)
supplied = entry.hasarg(factorarg,origin)
two = entry.constant(integer,'2',origin)
factor = entry.readarg(factorarg,origin,two)
```

`fallback` is an already evaluated IR value. The generic operation does not
promise lazy evaluation of an arbitrary expression. STEP07A frontends only
admit pure literal/primitive defaults. Later effectful defaults must lower
explicit guarded control flow under a separately reviewed contract. A missing
fallback for an optional argument, a fallback on a required read, a wrong type,
foreign handle or out-of-range ordinal is a verification error.

No new generic CALL opcode or user-facing call-builder hierarchy is added.
Existing schema-declared operation inputs, effects and compiler verifiers
continue to describe calls. C activation packets remain compiler operations.

## Numeric representation and compatibility

IR format version 2 keeps the existing thirteen tables and extension
opcode offset. Argument records gain one unsigned field, `optional` (0/1).
Function field 6, reserved as zero in version 1, becomes the external-symbol
pool reference (zero means private). Names/default literal bytes remain in the
existing exact pool; pass loops use numeric IDs.

Reserve core opcode 8 for presence and 9 for read-with-fallback. Both use the
existing auxiliary ordinal. Presence has no inputs and one Boolean result;
read-with-fallback has one input and one result of the formal's type. Existing
core opcode 2 remains required-argument read. Verification includes both
reachable and unreachable blocks.

The binary/text writers emit version 2. Readers also accept version 1, validate
its original reserved fields, and import every old argument as required with
private linkage. They reject unknown future versions. Round trips, budget
accounting, clone/rewrite and optimiser reconstruction must preserve every
flag/export. Version-1 files are not silently reinterpreted with new semantics.
The on-disk RXBIN007 format and callable metadata remain unchanged.

## Source trees and processing

G parameter trees become explicit alternatives, preserving source spans:

```text
GProcedure(name="scale", result=Type(".int"), body=Sequence(...))
  GArg(items=[
    Parameter(name="value", type=Type(".int")),
    DefaultParameter(name="factor", value=Number("2"))
  ])
  Return(Multiply(Name("value"), Name("factor")))

Presence(spelling="?factor")                 -- source ?factor
OptionalParameter(name="count", type=Type(".int"))
GNamespace(name="scalelib", exports=[Export(name="scale")])
GImport(name="scalelib")
Call(name="scalelib..scale", arguments=[Argument(Number("7"))])
```

The arithmetic line abbreviates the existing Chain nodes; it is an explanatory
view, not a second AST representation. `Arguments` continues to retain an
`Argument` node without a value for each explicit hole. No tree rewrite fills
holes with defaults. C keeps its CProcedure/ARG/presence-call nodes and its
explicit default assignment/IF.

1. Inventory local declarations and permitted imports before schema freeze.
   Resolve scalar types, optional positions, defaults, exports and qualified
   target identities. Reject duplicates and missing/ambiguous selected exports.
2. Evaluate supplied actuals once, left to right; conversions stay at their
   language-defined call boundary. Check every required position, not only count.
3. Register a fixed call schema for each observed target/presence pattern. Its
   inputs contain only supplied positions, in ascending formal order; the
   compiler-owned presence attribute uses one checked 0/1 byte per formal.
   Registration is deterministic and subject to existing entry/work budgets.
   It is bounded by observed source calls, not all possible optional subsets.
4. Lower G formal reads/defaults/presence through the above Level L operations.
   The entry's resolved values initialise normal private slots. Later assignment
   changes those slots, not the invocation's original presence.
5. Validate imported and local call operations against complete result/formal
   contracts and presence masks. Unknown external state/failure remains
   conservative; no cross-call constant propagation or inlining is added.
6. Reconstruct/verify the selected graph, then emit with Level L writer services.
   Backend validation rechecks selected runtime/import contracts before output.
   All failures retain existing atomic output and source diagnostic behaviour.

The imported catalogue and signature classes retain their existing entry points.
Extend selected-signature validation, including duplicate names and required/
optional flags; add `paramname(ordinal)` only to expose already decoded names.
Metadata parsing, path resolution and C ABI details remain compiler-owned.

## Separate modules and emitted code

Provider:

```rexx
options levelg
namespace scalelib expose scale
scale: procedure = .int
  arg value = .int, factor = 2
  return value * factor
```

Consumer:

```rexx
options levelg
import scalelib
say scalelib..scale(7)
say scalelib..scale(7,3)
exit 0
```

Expected output is `14`, then `21`. Use the existing `-i` directory for admitted
runtime images and `scalelib.rxbin`; no new search policy or CLI switch. This
slice restricts module names to the existing simple identifier form and emits
no source import or native-provider fallback. Provider-only files have no
synthetic executable main; executable consumers retain the current main entry.
Exported routines use existing `.expose` and `.meta`; defaults execute in the
provider. Explicit linking remains the qualification route.

At a missing second actual the caller reserves the necessary ABI position and
clears its supplied flag. It does not construct a default or an empty string.
`invoke()` sets each position directly from the selected presence mask. Supplied positions retain the existing by-value/ownership flags and call
window cleanup. Internal/private calls use the same convention.

The callee tests presence before reading an optional argument. Its local value
uses the literal only on the missing path. Presence queries continue to observe
the original call even after assignments. Retain source origin on the default,
read and call. Required-only calls need no optional-presence machinery. Remove
unused presence results through shared demand analysis; never remove mandatory
conversion failures or caller/callee ownership operations.

These are code-shape obligations, not a demand to copy the native compiler's
incidental TRACE counts, registers, inline payload or instruction choices.
Native controls independently confirm the existing 256 supplied-value flag,
optional `.meta` spelling and provider-owned default body. The emitted shape is retained with the executed examples.

## Acceptance and next implementation checks

F7A-AC01 was closed by the explicit API approval on 16 October 2026.
The maintained checks for this implemented bounded contract are:

1. **F7A-STEP02 / AC02/04:** generic non-C/G optional-input/export/codec/rewrite
   tests, with malformed flags/ordinals/types, stale ownership, v1 compatibility
   and budgets; C/G omitted, empty, middle-hole, required-after-optional,
   assigned-formal presence, conversion error, recursion and evaluation order.
2. **F7A-STEP03 / AC03/05:** new G provider and consumer, new/native interop in
   both directions, metadata read/write, no main in libraries, selected export
   and signature failures, C runtime-context and optional-flag tampering. A
   provider default change with unchanged caller proves callee ownership.
3. **F7A-STEP04 / AC01–06:** inspect complete RXAS, run focused end-to-end modes
   and installed CLI/service requests with old compilers unavailable, refresh
   only invalidated evidence, and update architecture/trees/algorithms/guide.
   Follow [AGENTS.md](../../../../AGENTS.md) for isolated aggregate measurement,
   scheduling and sanitizer scope; no formal performance campaign or automatic CI rerun.

Full replacement, arbitrary defaults, references, conditions/TRACE, proper B
self-hosting and INTERPRET retain their named later gates.

The [current plan](../PLAN.md) tracks instruction status and remaining
delivery/platform work. Bounded scalar-call proof does not establish
complete C, B/G replacement or hosted qualification.
