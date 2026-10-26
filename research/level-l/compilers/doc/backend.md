# C/G IR to RXAS

The backend emits RXAS for the current [Classic C profile](levelc-conformance.md)
and the admitted typed G profile. `llrxc-c` and `llrxc-g` are separately selected
commands. The existing `rxc`, `rxc-c` and wrapper routes retain their product
roles. Optional shared IR optimisation precedes selection. STEP11 locally
delivers same-frontend C INTERPRET and Rexx source calls through a separate IR
execution route; it does not assemble runtime source. Full compiler,
delivery/platform acceptance and replacement retain their gates in
[PLAN.md](../PLAN.md). See the [C user guide](levelc-user-guide.md) for current
commands and the [dynamic architecture](levelc-architecture.md#dynamic-execution-strategy)
for the compiled/IR bridge. Earlier STEP04/05 examples retain their stated profiles.

## Ownership and public surface

```rexx
import llgcompiler
import llir
request = .llgcompiler..request(source, artifacts, 'example.crexx')
if request.status() <> 0 then say request.diagnostic()
else do
  output = request.output()
  say output.text()
end
```

`source` is binary, preserving exact UTF-8 source bytes. `artifacts` is one
explicit directory of target import images. The optional arguments after the
filename are semantic/source budget (1,000,000), nesting (512), emitted bytes
(67,108,864), registers per function (65,536), and `optimise` (1). Zero disables
shared IR optimisation without disabling construction/emitter cleanup. The
work and byte limits also bound the optimiser; phases have separate logical
budgets, not a combined memory/CPU cap. These are protection limits,
not throughput promises. Exhaustion fails the request and exposes no partial
successful output. Register budgets below 16 are invalid.

C additionally appends optional `providers:llcbindings.binding[]`,
`routine_export:string` and `host_export:string` after those eight arguments.
Bindings select prebuilt Classic activation-packet exports, admitted ordinary
scalar B/G/RXPA procedures or explicit executable routes. Routine output supplies
a signed Classic wrapper; host output supplies a typed
`RexxClassicHostRequest -> RexxClassicHostResult` wrapper. The two export modes
are mutually exclusive. Empty defaults avoid provider work. The request anchors
checked selections before
borrowed frontend/backend work, including a `graph()`-first request. See the
[static-provider contract](levelc-architecture.md#call3-static-provider-contract),
[Group 2 scalar/host contract](levelc-architecture.md#group2-calls-host-proposal) and
[CLI guide](levelc-user-guide.md#compiler-options).

`status`, `error`, `offset`, `diagnostic` and `output` evaluate the request once.
`graph` retains frontend inspection; `emissiongraph` evaluates the request and
returns the graph selected for emission. Query successful status before using
that graph. Its revision may differ from the frontend graph. The separate `llccompiler.rxbin` and `llgcompiler.rxbin` packages expose their
own B services; a consumer links the selected package with `rxfnsl.rxbin` and `library.rxbin`. The command I/O provider and
Classic target runtime are absent from that service package.

The request owns its frontend and backend;
the backend owns the frozen graph and the completed Level L output. A returned
output owns its chunks independently and survives the request. Requests share
no mutable compiler state. Repeated independent requests have maintained
controls; full concurrent-service qualification remains open. STEP11 supplies
bounded interpreted-source/IR/frame ownership for its locally qualified profile;
full dynamic and delivery/platform proof remains in PLAN.

An advanced caller can pass a frozen graph to
`.llcrxas..compilation(graph, artifacts, bytes, registers)` for C, or the corresponding `.lltrxas..compilation(...)` for the typed core. It exposes
`status`, `error`, `offset` and `output`. This target is deliberately closed to
unknown operation meanings: registering a generic Level L operation does not
implicitly grant it RXAS semantics.

## Structures and algorithm

| Structure | Contents and purpose | Validation |
|---|---|---|
| Source IR | Existing immutable Level L function/block/value/slot/origin records | Frozen module verification, entry/function ABI, language family, supported version, canonical extension shapes |
| Virtual cell maps | Integer arrays indexed by IR slot ID and value ID | Function-local identities; C entry-bound context aliases share a checked cell |
| Call window | Count followed by contiguous argument registers, outside every live slot/value | Maximum call arity measured before selection; result and arguments are disjoint |
| Selected instruction | Typed virtual operands, canonical opcode/effects/components/signals and origin ID | Optional `llrxas.program` admits records before assignment, including helper/window/cleanup and TRACE uses |
| Assignment | Whole cells, scalar intervals, unique owners and fixed/window constraints | Independent map/interference check, mapped frame budget, atomic rendering |
| Import cache | Package catalogs and checked callable signatures | B-only RXBIN007 reader, result/arity/type/optional checks and per-position presence masks |
| Classic DO facts | Function-local text keys, finite phase/owner stacks and hidden strong state registers | C-owned original/final protocol checks; no escaping state value or SDK lifetime rule |
| Output | Level L writer sections and owned origin-tagged chunks | Per-section and total byte budgets, finish/stream error checks |

Selection numbers virtual cells, runs the selector without output to determine
scratch/call needs, then records selected operations in stored block order.
The optional [Level L RXAS service](../../../../langtools/doc/rxas-design.md)
immediately admits typed operand fields and retains exact literal spans. It
assigns physical cells only after all selected helper work and observations are
visible. A request's register budget applies to that physical result; virtual
storage/work has separate finite bounds. This construction path also applies
when semantic IR optimization is off.

`r0` is reserved. Slots come first, then each operation's result. Scratch storage
is added only when selection uses it; Classic also retains its diagnostic
location. A separate call window includes the argument-count register and the
largest selected call's operands. The same selector is used for planning and
emission, including hidden conversions; Classic's cold failure-message call
requires two arguments. Main's wrapper uses two registers for G and sixteen for C.
The public minimum register-budget argument remains 16. Proved plain integer and
Boolean temporaries can reuse cells within one target block. Inclusive interval
endpoints keep simultaneous inputs/results distinct. Unknown signals/components,
addressed or owned storage, cross-label ranges, backedges and exceptional control
select conservative storage. A pinned cell retains unique identity but can move
as a whole before execution; ABI-fixed cells retain their index. Initial reuse
clears stale components without moving an object finalization point. No component
packing, speculative cross-block reuse or pressure spilling is implemented.
C supplies a combined target-work allowance of `output.bytes + 1000000`.
Record admission charges less than the selected-text byte bound already
enforced by C; the existing million-work allowance remains for finalization.
The addition is checked before constructing any target. Generic `llrxas`
defaults are unchanged, and dense allocation can still exhaust finite work;
the output/register limits and atomic refusal remain independent constraints.
Jumps to the next emitted block fall through, retaining its label
and resetting source/context state at that entry.
Classic counted/controlled DO adds one hidden owner per static key, outside
IR values and the call window. Owners initialize before fallible setup; helper
calls borrow them synchronously, and normal/transfer/fatal exits clear them in
the [approved lifetime order](levelc-architecture.md#approved-do-contract).
Grouping and state-free repetition add no DO owner.
A local call targets an internal `llfN` label; a block targets `llbN`. Source
identifiers never become assembler labels. Recursive calls receive independent
VM frames. G local calls are checked against the callee's exact IR signature;
Classic source units use required `(context, entry)` parameters and return a
fresh result-presence packet. Internal calls select positive checked entries in
one shared source body; see the [current C ABI](levelc-architecture.md#proposed-procedure-contract).

Supplied arguments are copied into the private window with flag 768; omitted
optional positions get flag 512 without constructing a placeholder value. The call
writes a distinct result register. The window is cleared after return. G callee reads test original presence
before selecting their fallback; sole-use literal defaults emit only on the
missing branch. Qualified G exports retain optional flags in `.meta`, and
provider-only files have no synthetic main. Classic's caller owns fresh
activation/bundle objects and copies borrowed pool/configuration references into
temporary call windows. The callee creates a private pool only when PROCEDURE
executes. Getters resolve live pool bindings through the bundle; receiver
aliases use the existing swap/restoration and unlink protocol. Argument/result
packets retain value semantics. Callee NUMERIC updates do not modify its caller's
activation or VM frame. Each admitted private PROCEDURE has its own pool; CALL
applies the returned presence packet to RESULT/.RESULT and drops both when no
value is returned. Authored bare RETURN checks function kind at the callee;
an expression receiving EOF absence fails44.1 at its invoking target.
Direct BIFs borrow metadata-checked raw argument vectors synchronously and use
the existing named entries. Generated callers inspect the shared context's
error flag and detail, release temporary aliases, then raise Classic SYNTAX at
the authored call site; successful calls retain condition delivery. Runtime
helpers may use the existing checked-error wrapper. The
[CALL contract](levelc-architecture.md#call-contract) owns the exact interfaces.
Main forwards VM `-a` arguments as an ordered presence packet.

The optional dynamic profile preserves the C body ABI and adds an
invocation-owned scope plus a finite inline dispatcher at resumable boundaries.
The shared dynamic service returns checked requests for compiled calls,
normal completion, owner RETURN/SIGNAL/EXIT or failure; only the selector maps
those requests to actual VM destinations. A suspended IR group resumes once
after its call completes. Runtime groups use the frontend-only `llcgraph` route
and neutral L executor, so the optional dynamic runtime contains no RXAS selector.
The [STEP11 contract](levelc-architecture.md#dynamic-execution-strategy) owns persistent
handler/private-pool state, source lookup, resource budgets and mixed-frame cleanup.

`rxfnsl-analysis` supplies the neutral indexed graph and structured effect/proof
services; the C-owned `llceffects` adapter retains C context identity and effect
premises. The existing once-bound entry-context check proves which slots/loads
denote argument 1. Those aliases share one cell, so redundant load/store copies
and result registers are absent even with semantic optimization off. Pool,
numeric, handler and callback mutation changes the context's contents without
rebinding its identity. It does not authorize pool-value caching or eliminate
activation/configuration, condition and cleanup work. CPROC/SIGNAL/DO/CLAUSE keep
their existing semantic proof ownership and ordering, including conditional R.

Both selectors consume per-function block/slot/operation indices rather than
repeated whole-module membership scans. C loop/command cleanup lists preserve
their prior order through indexed function membership. Shared runtime algorithms
and native C remain unchanged. Parsing/frontends depend only on core L; optional
analysis/RXAS packages enter at their first actual host consumer. Static C target
images contain the existing runtime without either package or a compiler.

## Semantics and literals

Source positions use request-owned `llsource.index` instances, one per source
in the graph. The Level L index also serves diagnostics. It owns line starts
and ends; lookup preserves byte offsets and Unicode scalar columns without
repeated whole-prefix scans. The compiler still owns RXAS `.srcstep` flags,
logical names and control-entry suppression rules; the generic writer only
transports the selected text and origin tags.

G selects integer, float, decimal, string and Boolean instructions directly.
Conversions use the matching VM conversion. String `||` uses `concat`, which
inserts no blank. Loose text comparisons use the VM REXX comparison family;
strict equality compares exact bytes. Binary64 constants are transported as eight exact bytes and
extracted with `bgetf64` using an offset register; they do not round-trip through
a decimal display string. A float negation multiplies by negative one. The VM's
ordinary textual formatting can display either zero sign as `0`.

A G decimal declaration retains the VM's absent decimal default, displayed as
`nan`. Its compiler-owned `crexx.g.decimal.default` operation emits `null`;
it is not manufactured as numeric zero. Other admitted defaults retain zero,
false or empty text. Existing ignored integer overflow policy stays unchanged:
an overflowing integer instruction leaves its destination unchanged. In this
backend a fresh destination begins at zero. No claim of checked arithmetic
exceptions is inferred from an ignored VM signal.

Classic operations call the B-written `llctarget` adapter and existing Classic
runtime. The executing C frame selects Classic numeric standard and installs
validated DIGITS/FUZZ/FORM at entry and after successful NUMERIC. Temporary
expression/BIF helper frames inherit those settings and borrow the context. Exact source literal
spelling, pool keys, unset reads, UTF-8 comparison policy, eager logical operands,
ARG uppercase/presence, numeric changes and Classic error codes remain distinct
from G. ARG preserves surrounding blanks. The static target links the standalone
B Unicode core, class library and Classic runtime; no G aggregate or compiler
dependency enters that target closure.

The backend independently recreates the supported semantic contract. It checks
extension field names/types/arity and rejects another language's operations or
unknown family versions. Constants use the backend's canonical codec, never a
foreign codec's formatter. A graph with a familiar type name cannot inject RXAS
through its formatting callback. All string/binary literal bytes go through one
quoting routine: printable safe ASCII is literal; quotes, backslashes, control
bytes and UTF-8 bytes use exactly two hexadecimal digits after `\x`.

Target import catalogs are read as data, not executed. The initial adapter
metadata uses existing runtime argument type spellings (`.rexxvalue`,
`.rexxvariablepool`, `.rexxactivationarguments`) and fully qualified result
spellings; the selector normalizes only these known argument names before
checking. Broader module/type alias resolution remains a later ABI increment.

## Source and errors

Each selected sequence retains its IR origin ID in Level L output chunks.
Source steps now use the complete single-line expression range, Unicode columns
and CRLF-aware lines. The backend translates exact, derived, composite and
synthetic origins into the existing RXAS flags. Multiline origins display the
first-line segment with composite provenance; the full byte range stays in IR.
Frontend-created routine setup and implicit returns carry derived origins.
Generated wrapper/handler regions explicitly clear the authored source anchor,
so they cannot inherit the preceding routine's location. The current VM printer
may display that empty source anchor as `:0:0`; it is not an authored location.
Improving that presentation is separate from this B-only repair.

General IR origins and the G frontend do not imply an authored clause identity;
their records retain clause ID zero. C's admitted clause-begin protocol supplies
actual statement-head identities. TRACE-capable C units retain these boundaries
as source metadata even when the clock proof erases executable begin work; TRACE
does not introduce DATE/TIME sampling. Retained scalar events name live produced
RexxValues, and the private handler reads the interrupted frame directly through
the shared B controller. The approved partial coverage and differences belong
to the [practical conformance profile](levelc-conformance.md#practical-trace-target).
The backend suppresses identical
straight-path anchors and location writes, but resets its emission state at
block entries because control-flow predecessors can leave different dynamic
location state. Different expressions on one line can still need separate
records. Debug metadata is address-associated, not an executable TRACE print.

The service preserves deliberate logical relative filenames and `<source>`;
absolute Unix/Windows/UNC physical names become basenames. The CLI always uses
the basename of the opened file, even for relative input paths. Hosts can supply
`examples/demo.crexx` through the existing filename argument to disambiguate
sources. Module plus local ID is the linked identity, not basename alone. This
policy concerns compiler source metadata; the assembler/VM can separately show
physical binary-module names in their diagnostics.

Source lookup uses request-owned source indexes. Function block/slot/operation
and cleanup-owner membership is indexed once; this does not claim every compiler
pass is linear or establish a performance verdict.

Keep debug metadata when linking (`rxlink -o ...`, without `-s`) for runtime
source reporting. G direct failures use the VM's original source metadata.
Classic's deeper runtime helpers can fail after the authored call-site frame
has unwound, so its existing function-level fatal handler still needs one
location field. The field names an authored expression, or explicitly describes
generated work/routine entry; it is not the original fault address.

The current [condition/observation profile](levelc-conformance.md) owns the
configured producers, typed payload/origin transport, authored SIGNAL and delayed
CALL handlers, clause checkpoints and cleanup. Fatal formatting retains the
original event and authored context. Default unhandled C events remain
nonresumable; full host/platform and LC-87 dynamic proof retain their named
qualification gates. Allocation preserves these contracts and changes no public
condition object, IR format or VM API.

Compile-time failures distinguish source syntax/type/profile diagnostics from
IR lowering, import, output and limit failures. Runtime failures stay runtime
failures, including division in a taken branch; an untaken failing expression
is not evaluated in either compiler mode.

## File boundary, build and delivery

Research delivery is explicitly selected. Follow [the build/install guide](../README.md).
`stage-product` owns SDK/tool products; `stage-research-compilers` owns the
selected compiler services, commands, Classic adapter and optional dynamic
runtime, with their required package dependencies. Research installation
uses `--component ResearchCompilers` after base installation. Old combined and
stale compiler outputs are excluded from the broad base-bin installation.
