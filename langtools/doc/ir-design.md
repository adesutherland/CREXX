# Level L intermediate representation

The `llir` module supplies checked numeric storage, construction handles,
read-only views, exact saved forms and a text emitter. `llirservices` supplies
revision-bound facts, flow analysis, semantic-service dispatch and atomic
transformations. Both are Level B modules in `rxfnsl`.

## The model

An **intermediate representation**, or IR, describes operations and how their
results are used. A compiler builds it after recognising source syntax and
checking language rules. An AST describes the source constructs; IR makes their
evaluation order and control transfers explicit.

A **module** owns functions, constants and source origins. A **function** owns
local slots and blocks. A **block** is an ordered sequence of operations ending
in one control operation. A **value** is an immutable result of an operation.
A **slot** is mutable storage within one invocation. An **edge** connects a block
to a possible successor.

A temporary value may be used only after its definition in the same block.
Store a value in a typed slot and load it in another block when it must cross a
branch or loop. This representation uses no phi nodes: the special join values
of an SSA representation. Arguments and constants become local temporary values
through explicit operations.

```text
function main -> core.void
  entry:
    %1 = core.constant 2 : crexx.g.int64
    %2 = core.constant 3 : crexx.g.int64
    %3 = crexx.g.iadd %1 %2
    %4 = crexx.g.itext %3
    crexx.g.say %4
    core.return
```

The percent numbers identify operation results. They are neither source variable
names nor machine registers. `crexx.g.iadd` gets its arithmetic meaning from the
compiler's registered operation family. Level L supplies its storage and checks.

## Register a contract

A family has a name and semantic version. Types have codecs for exact constants;
operations declare named inputs, results, attributes and effects. Finish the
schema before constructing a module.

```rexx
schema = .llir..schema()
family = schema.family('demo',1)
integer = family.type('integer',.llir..intcodec() as .object)
add = family.operation('add')
call add.input('left',integer)
call add.input('right',integer)
call add.output('value',integer)
call add.state('none')
call add.failure('overflow')
call add.termination('returns')
call add.evaluator(my_evaluator as .object)
contract = schema.finish()
```

Names use ASCII letters, digits, underscore and dot. Source-language names and
source files can contain Unicode. Family registration qualifies a local type or
operation name, so `integer` above becomes `demo.integer`. Duplicate names,
foreign types and wrong service interfaces produce `LL_IR_SCHEMA` diagnostics.
A frozen contract cannot be edited. While registering signatures, `schema.type(name)`
looks up an already registered type, and `schema.boolean()`/`schema.voidtype()`
provide the core handles without prematurely freezing the schema. The finished
contract provides the corresponding read-only lookups.

The default state, failure and termination summaries are `unknown`.
`state('none')` means no access to mutable state or observable output.
`failure('none')` means no language failure; other named failure summaries retain
that possibility. `termination('returns')` means ordinary continuation, including
propagation of a declared failure out of the function. `diverges` and `transfer`
identify other outcomes; the initial flow analyser refuses those continuations.

State and failure names are compiler-owned contract identifiers. Level L does
not infer aliasing, effects or mathematical properties from a name. Complete
language semantics belong to the registering application. The normal control
contracts and the meaning of core slots cannot be redefined by an extension.

### Types and exact constants

A `typecodec` implements three methods:

| Method | Result |
|---|---|
| `parse(text)` | Exact `.binary` encoding of the external literal text. |
| `check(bits)` | Empty string for valid encoding, otherwise an explanation. |
| `format(bits)` | External text representing those exact bits. |

Built-in codecs are `intcodec` (signed 64-bit little endian), `boolcodec` (one
byte, zero or one) and `bytescodec` (arbitrary bytes, with uppercase hexadecimal
external text). `core.boolean` uses `boolcodec`. `core.void` denotes absence of
a function result and cannot type a temporary, slot or constant.

`constant(type,text,origin)` uses the codec to parse a literal.
`constantbytes(type,bits,origin)` checks an already encoded constant. Exact byte
identity is distinct from source-language numeric equality. Define an operation
when the language needs a comparison or conversion.

### Semantic services

Services are host-registered Level B objects. Loading an IR file never discovers,
loads or executes callback code from that file. A host must supply the matching
frozen contract, including its trusted services.

| Interface | Method and purpose |
|---|---|
| `llir.verification` | `check(opview) -> string`. Additional semantic checks after all structural checks; empty string means success. |
| `llir.evaluator` | `evaluate(request) -> evaluation`. Compute exact constant results from literal operands and explicit facts. |
| `llir.lowering` | `lower(lowerrequest) -> fragment`. Construct replacement operations in the supplied destination block. |

Register them with `operation.verifier`, `.evaluator` and `.lowerer`, passing an
explicit `as .object` value. The structural signature generates the basic type
and arity checks; a verifier adds relationships specific to that operation.

An evaluator request offers `opcode()`, `operation()`, `size()`, `at(ordinal)`,
`has(key)`, `get(key)` and `budget()`. `at` returns exact operand bytes.
`operation()` provides read-only access to typed inputs, results and attributes.
The readable dump includes extension attributes by name (for example,
`target="TWICE"`), with the registered type codec formatting their values.
The versioned text/binary codecs remain the lossless interchange formats.
A context fact belongs to the input operation in the current module revision.

An evaluator returns `.llir..evaluation(status,message)`:

| Status | Meaning |
|---|---|
| `constant` | Append every exact result with `add(bits)`, in declared order. Record each consumed context fact with `use(key)`. |
| `keep` | A justified constant is unavailable. Leave possible runtime arithmetic errors as runtime operations. |
| `invalid` | Malformed IR or a broken semantic contract. |
| `limit` | The service's cooperative work budget was exhausted. |

The coordinator rejects wrong result counts, invalid encodings, missing consumed
facts, unknown outcomes and service signals. It requires no mutable state effect,
a known failure summary and ordinary continuation before accepting a constant.
An evaluator must establish that its particular inputs return those results
without a language failure. This permits a checked addition to fold `2+3` while
retaining an overflowing expression. A correct result encoding alone is not a
proof of the evaluator's arithmetic: compiler tests and proofs establish that
contract.

A callback is trusted Level B code, not a sandbox. The request budget is
cooperative; it cannot interrupt a callback that ignores it. Destination storage
budgets and coordinator work budgets are checked separately.

## Build and inspect a module

```rexx
draft = .llir..builder(contract)
source = draft.source('example.txt','say 2+3' as .binary)
at = source.origin(0,7)
main = draft.function('main',contract.voidtype())
entry = main.block('entry')
left = entry.constant(integer,'2',at)
right = entry.constant(integer,'3',at)
sum = entry.binary(add,left,right,at)
total = main.slot('total',integer)
call entry.store(total,sum,at)
call entry.returnvoid(at)
graph = draft.finish()
if graph.error() <> '' then say graph.error()
```

Ranges are zero-based, half-open byte ranges. The example's range covers all
seven bytes. Use `draft.synthetic(description)` for generated material without
source text. `origin.derive()` retains a contributing origin. Origin methods
include `first`, `last`, `filename`, `sourcebytes`, `kind`, `parents` and `parent`.
Kinds are `exact`, `composite`, `derived`, `synthetic` and `recovered`. Any
recovered origin prevents successful executable-module publication.

`finish()` freezes and verifies the complete draft, including unreachable
blocks. On failure the result reports an error and exposes no successful graph
or encoded output. Existing published modules remain usable. A published module
supports `functions`, `function(id)`, `blocks`, `operations`, `operation(id)`, `block(id)`,
`dump`, `text`, `binary`, `bytes`, `capacity` and `identity`.

### Construction reference

| Owner | Methods |
|---|---|
| Builder | `function(name,resulttype)`, `source(name,bytes)`, `synthetic(description)`, `finish()`, `error()`. |
| Function | `argument(type,optional=0)` before creating blocks; `exportas(name)`; `block(name)`; `slot(name,type)`. The first block is the entry. |
| Block | `constant`, `constantbytes`, `readarg(ordinal,origin,fallback)`, `hasarg(ordinal,origin)`, `load(slot,origin)`, `store(slot,value,origin)`. |
| Block | `binary(descriptor,left,right,origin)` for two inputs, one result and no attributes; `operation(descriptor,origin)` for a named draft. |
| Block | `jump(block,origin)`, `branch(boolean,yes,no,origin)`, `returnvoid(origin)`, `returnvalue(value,origin)`. |
| Operation draft | `input(name,value)`, `attribute(name,bits)`, `finish()`. Every declared field must be supplied once. |
| Operation view | `name`, `inputs`, `input`, `results`, `result`, `namedresult`, `attribute`, `origin`, `next`, `blockid`, `successors`, `successor`, `schema`. |
| Value handle | `type`, `isconstant`, `bytes` for literal constants. |

An operation draft produces a view; use `namedresult('value')` to obtain a result
by its declared name. Read operations in a block with `block.first()` and
`op.next()` until `op.valid()` is false. Function results currently have one
value or `core.void`; extension operations may have multiple results.

The nine core operations are constant, argument, load, store, jump, branch,
return, hasarg and optionalargument. A branch consumes exactly `core.boolean`, with true then false
successors. A return matches its owning function's result. Edges stay within one
function. A block has one final jump, branch or return. Extension operations in
this profile are straight-line operations; graph control uses the core forms.

Optionality is exactly 0 or 1 per formal, including optional positions before
required ones. `readarg(ordinal,origin)` reads a required formal. An optional
formal requires a fallback handle of the same type, defined earlier in the same
block. The fallback is an already evaluated value: arbitrary lazy defaults
require explicit control flow. `hasarg` returns `core.boolean` for original
invocation presence, independently of value reads and assignments. The caller's
language/target contract remains responsible for passing valid required values.

`module.function(id)` returns a borrowed function view with `name`, `resulttype`,
`arguments`, `argumenttype(n)`, `optional(n)` and `exportname`. Exports must be
nonempty and unique; consumer-specific symbol syntax is outside Level L.
Mutation of a frozen owner fails without changing its saved image. Unknown,
foreign and expired handles are rejected. The display dump includes signature
and export lines; it is still a view, not the lossless interchange format.

Handles borrow their owner through checked Level B references. Appending records
does not invalidate them. Destroying or replacing the owner does. A handle from
one module cannot be used in another. After `finish`, obtain fresh read handles
from the returned graph; draft handles still refer to the frozen draft owner.
An attempted edit to a published module raises `LL_IR_LIFECYCLE` and preserves it.

The `arena`, `table`, schema storage and reader classes are implementation
services exposed for Level B linkage and the framework's own algorithms.
Applications use the checked handles above, not raw table mutation.

## Analyse flow and record facts

```rexx
report = llirservices..analyse(reference graph)
if report.status() = 'ok' then do
  do i = 1 to report.issues()
    say report.diagnostic(i)
  end
  facts = report.facts()
end
```

`ok` means the analysis completed. `issues()` counts loads that may read an
unassigned slot. A compiler must inspect those issues before emitting code that
requires definite assignment. Structural verification alone does not establish
that every load is initialised.

Reachability follows both branch edges. Definite assignment intersects the
incoming sets at a join and iterates loops until no set changes. Function entry
starts with no assigned slots. The analysis checks each reachable load against
the stores preceding it. An unreachable block is still structurally checked.
Unknown state effects or incomplete continuation contracts return `unsupported`.
Other outcomes are `invalid` and `limit`; neither publishes analysis facts.

Facts use `has(domain,id,key)` and `get(domain,id,key)`. `get` returns exact bytes;
it raises `LL_IR_FACT` for absence. `reachable` facts belong to blocks and
`definitely.assigned` facts to load operations. Both use byte `00` or `01`.

For application analyses, construct `.llirservices..facts(reference graph)` and
use `put(domain,id,key,bits)`. Domains are `function`, `block`, `operation`,
`value` and `slot`. Explicit empty or false values differ from absent facts.
Recording a fact states the caller's conclusion; it does not prove that conclusion.
The store is bound to the exact owner and revision, and has entry/byte budgets.

## Evaluate, rewrite and lower

```rexx
facts = .llirservices..facts(reference graph)
answer = llirservices..evaluate(reference graph,operation_id,facts)
change = llirservices..rewrite(reference graph,operation_id,facts)
if change.status() = 'changed' then graph = change.result()
```

`evaluate` reports a checked result without modifying the module. `rewrite`
repeats the eligibility checks and builds a new module with the selected operation
replaced by constants. It remaps every use and gives replacement constants derived
origins. This is a one-operation transformation primitive, not an automatic
optimisation pipeline. Create fresh facts after accepting the new graph.

A change reports `changed`, `keep`, `invalid` or `limit`. Its `result()` is the
verified candidate on `changed`, otherwise the original graph. `operation(oldid)`
and `value(oldid)` map IDs into that result. `block(oldid)` maps retained blocks
and returns zero for removed ones. Zero denotes a removed operation/value with
no retained replacement. A substituted load has no replacement operation but
its used result maps to the earlier value. A multi-result fold maps its operation
to the first retained constant and each used result separately. Failed or kept
changes have identity mappings. Failed candidates are never published.

`llirservices..lower(reference graph,target_contract)` rebuilds a graph against
a target contract. Shared types must have matching names and family versions;
retained operations must have matching signatures and effects. For operations
absent from the target it invokes their registered lowering service.

A lower request supplies `source()`, `destination()`, mapped `input(ordinal)`,
derived `origin()` and `budget()`. Return `.llir..fragment('ok')` and `add(value)`
for each mapped result. `keep`, `invalid` and `limit` retain the original module.
The first lowering profile supports pure, normally returning straight-line
fragments without language failure. New operations must satisfy the same effect
contract; state operations, terminators, foreign results, type mismatches and
cross-block results are rejected. This is the foundation for registering richer
checked lowerings as compiler backends are developed.

## Optimise a frozen module

```rexx
change = llirservices..optimise(reference graph)
select change.status()
  when 'changed' then graph = change.result()
  when 'keep' then nop
  otherwise say change.message()
end
```

The optional arguments are `budget=1000000`, `entries=1000000` and
`bytes=67108864`. One private indexed batch performs same-block private-slot
propagation, exact registered constant evaluation, Boolean branch selection,
entry-rooted reachability and safe unused-operation removal. It rebuilds once;
it does not clone the graph for each fold. `keep` is a normal result when
nothing changes or a continuation contract is unsupported.

Core slots belong to one invocation and have no address/escape operation.
A load may reuse the most recent same-block stored value; an extension whose
state contract is not `none` invalidates those facts. Unknown/control-transfer
continuations keep the original graph. All function entries remain roots,
including uncalled functions. Unknown branches keep both edges; a known branch
does not grant permission to remove observable condition evaluation.

Only constants, arguments and extensions with state `none`, failure `none` and
termination `returns` are intrinsically discardable. A successful checked
evaluation additionally proves that exact instance replaceable. Reachable
stores, unsubstituted loads, output, unknown effects, possible failures and
divergence remain. Pure unused chains are removed in reverse definition order.
The profile forbids cross-block values, so this needs no repeated fixed point.

Evaluator `request.at(i)` contains the proven operand bytes, including values
propagated or evaluated earlier in the block. `request.operation()` still views
the original operation, for its attributes and origins. The evaluator must use
the request's operand bytes when evaluating; original input views need not be
literal constants. The same checked callback boundary is used by `evaluate`.
Missing services and `keep` retain runtime operations. Invalid result counts,
codecs, consumed facts or signals reject the entire candidate. Callback
equivalence and compliance with its budget remain the host provider's duty;
the work limit is not a callback pre-emption clock.

Numeric arrays hold aliases, exact-value flags/bytes, folds, selected edges,
live blocks, demanded values and previous-operation IDs. Index entry accounting
is `5*operations + 4*values + 2*blocks + 2*slots`; its logical byte charge is
eight bytes per entry plus remembered constant bytes. Destination arena limits
apply separately during reconstruction. These are per-phase logical bounds,
not a combined peak-RSS cap; VM objects, codec/callback overhead and immutable
source storage are excluded. Work counts planning visits/operands/results/edges
and reconstruction records. Existing module verification remains mandatory.

Successful reconstruction retains the source/type/slot/constant tables and
byte pool, including unused pool entries. It compacts live blocks, operations
and values. Thus fewer executable records does not imply compacted saved pools.
Replacement constants/branches have derived origins; retained operations keep
their attributes and origins. The source graph stays immutable, and analyses
must be recreated for the returned revision. No externally supplied sparse fact
is treated as a proof by this pipeline.

## Saved forms and numeric storage

`graph.dump()` is a readable named view. Optional function and block IDs filter
it. `graph.text()` and `graph.binary()` are complete lossless forms.
`llir..loadtext(contract,text)` and `llir..loadbinary(contract,bits)` require the
same registered schema manifest and return a verified frozen module or an error.
The manifest includes ordered family versions, type names, operation names,
effects and field signatures. Changing registration order changes the manifest.
The host is responsible for versioning changes to codecs or semantic services.

Binary version 2 begins with magic bytes `4C4C4952`, then u32 version 2 and a
length-prefixed UTF-8 manifest. Thirteen tables follow, each with u32 count and
width, then little-endian u32 fields in row order. A length-prefixed byte pool
follows. Each byte-pool item has its own u32 length. Text version 2 uses the same
record structure, decimal fields and hexadecimal byte strings. It preserves
embedded zeros, non-UTF-8 constants and exact source bytes. Both loaders reject
truncation, incompatible schemas, unknown versions, invalid ranges and trailing
data before publishing a graph.

| Table | Fields, in order |
|---|---|
| Functions (6) | name pool ID, result type, entry block, argument start/count, export-name pool ID (zero for private) |
| Blocks (4) | function, first/last operation, name pool ID |
| Operations (12) | opcode, block, next, operand start/count, result start/count, edge start/count, attribute start, origin, auxiliary ID |
| Values (4) | defining operation, result ordinal, type, reserved zero |
| Slots (4) | function, type, name pool ID, reserved zero |
| Operands / edges (1 each) | value ID / block ID |
| Constants (2) | type, byte-pool ID |
| Origins (6) | source, first/last byte, kind, contributor start/count |
| Sources (2) | name pool ID, source-bytes pool ID |
| Attributes (4) | operation, ordinal, type, constant ID |
| Arguments (3) | function, type, optional flag (0 or 1) |
| Contributors (1) | earlier origin ID |

Core opcode IDs are 1–9 in the order listed above. Extension IDs are 16 plus
registration ordinal. Numeric IDs are not pointers, persisted callbacks or VM
registers. Zero represents absence only in the fields that permit it.

Writers emit version 2. Readers also accept version 1 with argument rows of
width 2, all required, and private functions. They validate v1's reserved zero
export field and reject opcodes 8/9 in a v1 image. Neither old reserved fields
nor unknown future versions silently acquire new semantics. Clone/rewrite and
optimisation retain signature flags and exports, subject to normal budgets.

Working tables use fixed-width binary records, growing geometrically. An
operation header occupies 48 bytes. `bytes()` counts used record bytes and
byte-pool payloads; `capacity()` additionally counts allocated table slack.
These figures exclude VM objects, array indexes, schema/service objects and
allocator overhead. They are storage accounting, not process peak memory.

Builders and loaders default to 1,000,000 records/pool entries and 64 MiB of
logical record/payload bytes. Hosts can supply different positive budgets.
Loaders also apply their byte budget to the complete input image. Facts have
separate entry/payload budgets; analyses and transformations have work budgets.
The u32 representation ceiling is reported separately from resource policy.

## Emit text without repeated whole-output copying

```rexx
out = .llir..writer()
call out.line('main:')
call out.indent()
call out.line('say "hello"',origin_id)
call out.dedent()
result = out.finish()
if result.error() = '' then say result.text()
```

`write(text,origin_id)` appends exact text. `line` adds current indentation and
LF. `indent` and `dedent` change indentation. `section()` creates another writer;
finish it and append its output with `append(reference section_output)`.
`finish` rejects unbalanced indentation and publishes no partial successful
output after an error.

Output is a list of byte chunks with origin IDs. `text()` allocates the final
buffer once and copies each chunk once. `stream(sink)` calls a registered
`llir.sink.write(binary)` for each chunk and reports a returned error or signal.
A sink may already have written preceding chunks when it fails; file publication
should use a temporary file and rename after success. The writer's default byte
budget is 64 MiB, configurable in its factory. It does not choose target
instructions or machine registers.

## Worked examples and compiler families

The [SAY example](../../research/level-l/compilers/examples/ir-demo.crexx) constructs `2+3`, converts it to
text, folds the arithmetic and conversion, and prints `5` from the retained SAY
operation. It shows before/after IR. It does not invoke a new source compiler or
an RXAS backend.

The separate [compiler family module](../../research/level-l/compilers/typed/lltir.crexx)
registers `crexx.g.int64`, `crexx.g.text`, checked `iadd`, `itext`, and `say`;
Classic `value`, explicit `numeric` context, `add` and `say`. Classic constants
preserve literal bytes. Classic contextual arithmetic has no evaluator until
its exact semantic implementation is supplied. Both SAY operations declare an
observable output effect and are ineligible for constant replacement.

The independent [semantic-service fixture](../tests/library/ts_llirservices.crexx)
defines a small non-C/G arithmetic family, a target lowering, and flow examples.
It demonstrates that the generic library has no compiler-family dependency.
