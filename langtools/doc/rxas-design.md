# Level L RXAS target package

`rxfnsl-rxas` is an optional Level L package for programs that select RXAS
instructions. It supplies `llrxascatalog` for the canonical instruction
description and `llrxas` for checked target records, whole-register assignment
and RXAS rendering. A compiler, code generator or another tooling program can
use it independently of the research Classic compiler.

The general IR, graph analysis and semantic proof services have their own
[IR guide](ir-design.md). Source-language evaluation rules belong to their
frontend. RXAS encoding, instruction execution and runtime algorithms retain
their existing owners. This package adds no opcode, load-file format, runtime
interpreter, source-language feature or assembler replacement.

## Build, import and link

Build the optional image explicitly:

```sh
cmake --build cmake-build-debug --target rxfnsl_rxas
```

The result is `cmake-build-debug/bin/rxfnsl-rxas.rxbin`. A Level B consumer
imports `llrxas`, and imports `llrxascatalog` when it needs instruction queries.
Its runtime link includes `rxfnsl-rxas.rxbin` and `library.rxbin`. The package
does not require a source compiler or the general analysis package at runtime.
It is separate from the core `rxfnsl.rxbin` image and grammar bootstrap. Normal
bytecode installation installs the separately named image; `stage-l-libraries`
prepares it alongside core, analysis and execution packages. This distribution
selection does not introduce those other packages into an RXAS consumer.

The build-time native extractor links the existing `opdata.c` and
`rxopmeta.c`. Its generated B module is a checked view of `rxops.h`,
`rxopeffects.h`, `rxopsignals.h` and `rxdefs.h`, including the native component
and signal algorithms. The source fingerprint covers those headers and the
two owning C files. Changing an owner regenerates the view. Compile requests
read this prepared image; they do not invoke native source-compiler services.

## Construct one function

This complete Level B program emits a small RXAS program. The virtual frame
has five cells. The two disjoint integer temporary lifetimes can share one
physical cell, allowing a four-cell output budget.

```rexx
options levelb
import llrxas

main: procedure = .int
  target = .llrxas..program(5,4)
  call target.record('main()','.locals=5')
  call target.candidate(1,'integer')
  call target.candidate(3,'integer')
  call target.record('load','r1,1')
  call target.record('icopy','r2,r1')
  call target.record('itos','r2')
  call target.record('say','r2')
  call target.record('load','r3,3')
  call target.record('icopy','r4,r3')
  call target.record('itos','r4')
  call target.record('say','r4')
  call target.record('ret','0')
  result = target.finish()
  if result.valid() = 0 then do
    say result.error()
    return 1
  end
  do i = 1 to result.records()
    say result.text(i)
  end
  return 0
```

Compile this producer with an import directory containing the prepared
`library.rxbin` and `rxfnsl-rxas.rxbin`; assemble it and link those two images.
Run the producer to save its stdout as `demo.rxas`. The ordinary `rxas demo`
and `rxvm demo` route then prints `1` and `3`. The resulting target program uses
only VM instructions; the producer's compiler/allocator package is not needed
by that static program.

`program(virtualLocals,registerBudget,workBudget=1000000)` owns one bounded
function. `virtualLocals` bounds input local identities; `registerBudget`
bounds the validated physical result. Allocation can therefore admit a virtual
frame larger than the physical budget. Work exhaustion is a separate atomic
failure, rather than evidence of a language error.

`record(mnemonic,operands='',origin=0)` admits each selected instruction before
allocation. `recordline(text,origin=0)` is the corresponding adapter for a
producer with one selected instruction per line. It strips surrounding
indentation and separates the mnemonic, then performs the same admission.
Neither method reparses completed output or delegates instruction selection
to the assembler. Operand kinds, virtual local cells, fixed argument/global
locations, labels, source metadata and numeric TRACE observations are recorded
at this boundary. Quoted bytes are preserved exactly.

`finish(reuse=1)` seals the program and returns an `allocation`. Check
`valid()` or `error()` before using `records()`, `text(i)`, `origin(i)`,
`locals()` and `assigned(virtualRegister)`. Record queries are one-based;
virtual and physical local registers are zero-based. The function header's
`.locals=` token is rewritten; other declaration metadata remains intact.
Inserted scalar resets retain the defining record's origin. Failure publishes
no text or assignment, and a finished program cannot be extended or finished
again. `finish(0)` validates and renders the exact physical identity profile.

## Storage and allocation constraints

Every local cell starts with a unique storage identity. A producer may declare
`candidate(register,'integer')` or `'boolean'` for plain scalar storage whose
language contract permits reuse. This declaration does not override machine
constraints. `pin(register)` retains a unique cell lifetime while permitting
whole-cell relocation before execution. `fixed(register)` also retains its
exact physical number. Register zero is fixed by default.

`window(base,count)` reserves a contiguous group including its count register.
For example, a count cell plus four argument cells has `count=5`. The current
conservative implementation reserves the complete suffix beginning at `base`;
implicit CALL windows are recognized from the canonical instruction contract.
Exact argument-presence flags, result separation, receiver swaps and cleanup
remain real instructions in the target stream. Their operands participate
before allocation.

The allocator uses deterministic first-fit assignment over inclusive record
intervals. An input and result used by the same instruction interfere. It
reuses only admitted scalar cells with a known non-reading initial definition
and a lifetime contained in one basic block.
A later occupant receives a `null` reset before that definition, clearing stale
payload/status components. Managed text, objects, references, addressed cells,
flag observations and failure-visible results retain distinct lifetimes;
their required releases are not advanced. All hidden helper and cleanup
instructions must be submitted before calling `finish`.

Intervals crossing a label retain unique cells: textual first-definition order
does not prove dominance of every entry. Backward-edge extents additionally pin
overlapping candidates. Signal-policy
or asynchronous/indirect control and numeric indirect local addressing use an
identity profile when a stronger whole-function proof is unavailable. Unknown
instruction semantics cannot authorize reuse. In particular, the native
catalogue deliberately marks several arithmetic signal contracts UNKNOWN;
their operands remain conservative. No component packing, speculative alias
coalescing or invented spilling is performed.

The assignment checker independently reconstructs intervals from the original
records and checks frame bounds, fixed locations, unique pinned cells,
interfering shared cells and contiguous windows. It does not trust the colour
heuristic or its cached endpoints. `program.validate(reference mapping,locals)`
also exposes this diagnostic check: `mapping[virtual+1]` is the assigned physical
number. A nonempty result rejects the assignment. This method does not publish
a program or authorize a caller-supplied map as executable output.

Record construction and checks have bounded work. First-use buckets avoid a
whole-frame scan for each instruction; the checker compares members sharing a
physical cell. Finalization indexes exact label spellings once, preserving the
first target for repeated spellings and every label's storage boundary. Label
index construction and branch lookup charge bounded key work rather than
rescanning every label per branch. Dense interference can still require
quadratic comparisons and may exhaust the request budget. Admission and
finalization work failures retain their resource-limit identity; only a genuinely
absent target has an unresolved-label record error. A rejected request can leave
a producer's previously published output intact.

## Typed assignment API

`llrxas.assignment` is the same allocation kernel used by the text `program`
adapter, exposed for consumers that already have checked typed references and
access facts. It receives numeric rows, not target/module handles, and performs
no parsing, instruction selection, reference remapping or rendering. The
consumer owns those operations and the semantic proof behind each input row.

Construct `assignment(virtualLocals,recordIds,registerBudget,work=1000000,
bytes=67108864)`, passing `recordIds` by reference as an `.int[]`. The vector
contains unique positive stable record IDs in source order; their numeric
values need not increase. The constructor validates and retains its own copy.
Zero virtual cells, zero register budget and an empty record vector are valid
when the other inputs permit them; work and byte budgets must be positive.
`error()` reports construction or later admission failure, and `remaining()`
reports remaining work.

Supply every declared row exactly once with
`row(ordinal,refs,reads,writes,kills,observations,origin=0)`. Pass all five
`.int[]` vectors by reference; admission checks and retains owned copies.
Row ordinals and vector positions are one-based; virtual and physical cells
are zero-based. Every vector has unique in-range members. Reads, writes and
observations must be subsets of `refs`, and definite kills must be a subset
of writes. An empty row explicitly records no local-cell references. Origins
are opaque numeric identities preserved for the consuming emitter.

The rows must include executable, implicit-window, metadata, TRACE and hidden
cleanup references relevant to allocation. Reads and observations require the
incoming allocation-domain state. A definite kill describes a checked
definition in that domain; it does not assert that a scalar VM instruction
clears the whole register value. Candidate admission additionally requires the
consumer to prove successful scalar initialization, equivalent zero entry and
absence of hidden status, payload, identity, lifetime or release obligations.
Missing references or an unproved premise cannot authorize reuse or deletion.

Declare constraints before `finish()`:

| Method | Contract |
|---|---|
| `candidate(virtual,kind='integer')` | Mark a proved plain scalar candidate; only `integer` and `boolean` kinds are accepted. |
| `pin(virtual)` | Preserve distinct storage while allowing relocation. |
| `fixed(virtual)` | Preserve distinct storage and its authored physical index. The typed API does not fix cell zero implicitly. |
| `retain(virtual,required=1)` | Retain an otherwise unused cell by default. Setting zero permits omission only when references and other constraints do not require it. |
| `window(base,count)` | Preserve the exact ordered contiguous group; `count` includes every member. Overlapping windows form one group, and a fixed member fixes that group. |
| `boundary(ordinal)` | Prevent reuse of a lifetime crossing this source-order row boundary. |
| `protect(first,last)` | Prevent reuse of a lifetime overlapping this inclusive row interval. |

`finish(mode='reuse')` seals the request once and returns an owned
`assignmentresult`. `identity` retains the entire virtual frame at its original
indices. `dense` places required cells distinctly, allowing unused cells to
disappear and movable cells to change index. `reuse` additionally shares cells
between eligible nonoverlapping inclusive intervals. An eligible lifetime
starts with a definite definition that neither reads nor observes its incoming
state, and crosses no declared boundary or protected interval. Pinned, fixed
and window cells remain distinct. These source-order intervals alone provide
no general CFG interference or every-path initialization proof; a graph-aware
consumer must impose adequate constraints or use distinct placement.

Check `valid()` or `error()` before reading the result. `locals()` gives the
physical frame extent, and `assigned(virtual)` gives the physical cell or `-1`
for an omitted cell. `resets()` counts ordered reset requests;
`reset(ordinal)` returns a `typedreset` with `virtual()`, `physical()`,
`recordid()` and `origin()`. A reset requests whole-cell `NULL` immediately
before the later occupant's first source record. The consumer emits those
resets with their origins and applies the validated mapping to its records;
allocation does not insert instructions itself. Identity and dense placement
need no resets.

`validate(reference mapping,locals,reference resets)` independently rebuilds
intervals and checks the proposed map against the declared rows, constraints
and current assignment mode. The mapping is an `.int[]` indexed by
`virtual+1`; resets are an `.object[]` of `typedreset` objects. Validation checks
required and omitted cells, frame bounds, fixed/identity positions, distinct
storage, inclusive interference, ordered windows, and the exact required reset
count, order, arrival record and origin. It returns an empty string on success
or a diagnostic on failure, without publishing output. It checks the supplied
premises rather than proving the consumer's machine semantics. `finish()` runs
this checker before publishing its result.

Construction, placement and validation share bounded work and logical-byte
accounting; validation also consumes those budgets. A failed `finish()` exposes
no partial mapping or reset list. A sealed request cannot accept more rows or
constraints or be finished again. The earlier text API retains its
positive-frame/budget contract and its own parsing, conservative window and
rendering behavior.

## Query the canonical instruction view

```rexx
import llrxascatalog
catalog = .llrxascatalog..catalog()
opcode = catalog.find('icopy','RR')
if opcode >= 0 then do
  say catalog.reads(opcode,2)
  say catalog.writecomponents(opcode,1)
end
```

`find(mnemonic,signature)` returns the canonical opcode, or `-1` for an unknown
or non-source instruction. Signature characters are the owning RXAS operand
kinds: `R` register, `I` integer, `F` floating, `D` decimal, `S` string, `C`
character, `B` binary, `L` label and `P` procedure. An empty signature describes
a no-operand instruction. Do not duplicate opcode numbers in consumers.

The named query surface is:

| Queries | Meaning |
|---|---|
| `count()`, `version()`, `fingerprint()` | Complete native slot count, SDK view profile and owning-source identity. |
| `mnemonic(op)`, `signature(op)`, `sourcevisible(op)` | Source spelling, operand kinds and source availability. |
| `flow(op)`, `classification(op)`, `implicit(op)`, `semantics(op)`, `barrier(op)` | Native flow/effect classification, implicit register rule and semantic flags. |
| `reads(op,i)`, `writes(op,i)`, `kills(op,i)`, `branch(op,i)` | Conservative operand effects, proven value kills and label operands. Operand positions here are one-based. |
| `readcomponents(op,i)`, `writecomponents(op,i)` | Native component masks; a scalar payload write is not a complete register-cell reset. |
| `signalstate(op)`, `signalphase(op)`, `signalsource(op)`, `signalsourceoperand(op)`, `signalnames(op)` | Completeness, failure timing and signal identity source. Native source-operand indices are zero-based, with `-1` when absent. |
| `failurewrites(op,i)`, `failurecomponents(op)`, `failurecontext(op)` | Writes potentially visible on a failure continuation. |
| `signaldependencies(op)`, `continuations(op)`, `signalproperties(op)` | Numeric/plugin/policy dependencies and admitted failure outcomes/observations. Properties distinguish structural `POLICY_WRITE` (8), `PENDING_WRITE` (32), and static signal-name validation failure `STATIC_NAMES_TERMINAL` (64); an explicitly requested valid signal remains catchable. |
| `policyeffect(op)`, `policysource(op)`, `policysourceoperand(op)`, `policyname(op)` | Typed structural-policy or pending-delivery transfer, selected by `signalproperties`. `BPOFF` clears a pending BREAKPOINT bit and retains its typed name/effect; it does not remove an installed handler or establish IGNORE. |
| `unbounded(op)` | Native indirect-write/branch or asynchronous-entry flags require conservative frame handling. |

Numeric enums and masks are those in `binutils/include/rxdefs.h`; this package
does not assign another numbering. A proven value kill, a component write and
a failure-visible write answer different questions. Missing or UNKNOWN signal
knowledge must not become a non-failing assumption. `field(op,column)` exposes
the versioned low-level catalogue row for tooling; prefer the named methods.

## Shared typed target analysis

`rxfnsl-target` is a separate optional research image, built explicitly with
`cmake --build cmake-build-debug --target rxfnsl_target`. It contains `lltarget`,
`lltargettransfer`, `lltargetcfg`, `lltargetremap`, `lltargetflow`,
`lltargetbodyview`, `lltargetsummary` and `lltargetnormalize`. Its import/runtime
closure includes the core, analysis and RXAS images plus `library.rxbin`.
Enable `CREXX_BUILD_TARGET_IO=ON` and build `rxfnsl_target_io` for the prepared
RXAS grammar and `llrxasinput`/`llrxasoutput` adapters. Parsing requests use the
prepared grammar and never invoke a native assembler or source compiler.

This shared layer is independent of an inliner. It owns module records and
providers, typed relocation, graph/policy/exit analysis, storage and subvalue
facts, bounded procedure summaries and ordinary dirty-read normalization.
The [target-analysis consumer map](../../docs/ai-context/RXAS_ASSEMBLER.md#level-l-target-analysis-consumer-map)
names APIs, maintained controls and uncertainty boundaries. The
[authoritative plan](../../research/level-l/toolchain/PLAN.md#phase-2--complete-the-shared-level-l-foundation)
owns qualification; completion of this foundation does not qualify inliner2
or the future Level L assembler/linker.

Use component/edge liveness from this layer when constructing assignment rows
and constraints. Include cold saved-base observations, owned reference roots,
borrowed private suffixes and whole packed windows. The allocation kernel above
still checks the supplied rows; it does not invent a semantic no-alias or
initialization proof. Fresh affine placement preserves a dynamic window without
requiring its runtime length to be constant. Distinct placement is valid when
optional lifetime sharing is unproved.

## Maintained checks and current boundary

`langtools/tests/library/ts_llrxas.crexx` supplies independent target examples,
live-frame reduction, inclusive interference, resets/origins, windows, TRACE
locations, unknown signals, loops, corrupted assignments and atomic refusal.
The native `tests/test_rxop_metadata.c` remains the owner of metadata alignment,
component and signal checks. The maintained native flow fixtures, including
`storage_identity_flow.rxas`, `signal_contract_runtime.rxas`,
`successful_guard_flow.rxas` and `swap_round_trip_flow.rxas`, retain their native
ownership and are the crosswalk for shared contracts.

The initial allocator deliberately retains conservative execution profiles for
complex ownership and exceptional control. This is a reusable target facility,
not a claim that all C/B/G compiler paths have optimal frames or that full
compiler, platform, delivery or performance qualification is complete.
