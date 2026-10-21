# C/G IR to RXAS

The STEP04 backend completes the unoptimised executable path for the
[admitted C/G profile](poc-scope.md). `llrxc-c` and `llrxc-g` are separately
selected commands. The existing `rxc`, `rxc-c` and wrapper routes remain the
production routes. STEP05 supplies shared IR optimisation before this backend; full replacement and
INTERPRET retain their later gates in [PLAN.md](../PLAN.md).

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

C additionally appends optional `providers:llcbindings.binding[]` and
`routine_export:string` after those eight arguments. Bindings select prebuilt
ordinary void/frame exports; routine output supplies one such wrapper. Empty
defaults avoid provider work. The request anchors checked selections before
borrowed frontend/backend work, including a `graph()`-first request. See the
[static-provider contract](architecture.md#call3-static-provider-contract) and
[CLI guide](../../../../langtools/doc/user-guide.md#compiling-cg-source-to-rxas).

`status`, `error`, `offset`, `diagnostic` and `output` evaluate the request once.
`graph` retains frontend inspection; `emissiongraph` evaluates the request and
returns the graph selected for emission. Query successful status before using
that graph. Its revision may differ from the frontend graph. The separate `llccompiler.rxbin` and `llgcompiler.rxbin` packages expose their
own B services; a consumer links the selected package with `rxfnsl.rxbin` and `library.rxbin`. The command I/O provider and
Classic target runtime are absent from that service package.

The request owns its frontend and backend;
the backend owns the frozen graph and the completed Level L output. A returned
output owns its chunks independently and survives the request. Requests share
no mutable compiler state. Repeated independent requests are tested; concurrent
service use and interpreted-code lifetime are later qualification work.

An advanced caller can pass a frozen graph to
`.llcrxas..compilation(graph, artifacts, bytes, registers)` for C, or the corresponding `.lltrxas..compilation(...)` for the typed core. It exposes
`status`, `error`, `offset` and `output`. This target is deliberately closed to
unknown operation meanings: registering a generic Level L operation does not
implicitly grant it RXAS semantics.

## Structures and algorithm

| Structure | Contents and purpose | Validation |
|---|---|---|
| Source IR | Existing immutable Level L function/block/value/slot/origin records | Frozen module verification, entry/function ABI, language family, supported version, canonical extension shapes |
| Register maps | Integer arrays indexed by IR slot ID and value ID | Every function gets a fresh frame; all emitted register operands checked against its bound |
| Call window | Count followed by contiguous argument registers, outside every live slot/value | Maximum call arity measured before selection; result and arguments are disjoint |
| Selected instruction | Mnemonic, final ordered operand text and origin ID | Constructed only by fixed selectors, checked registers/labels/call names and exact literal quoting |
| Import cache | Package catalogs and checked callable signatures | B-only RXBIN007 reader, result/arity/type/optional checks and per-position presence masks |
| Classic DO facts | Function-local text keys, finite phase/owner stacks and hidden strong state registers | C-owned original/final protocol checks; no escaping state value or SDK lifetime rule |
| Output | Level L writer sections and owned origin-tagged chunks | Per-section and total byte budgets, finish/stream error checks |

Selection allocates stable value locations, runs the selector without output
to determine scratch/call needs, then emits blocks in stored order. This refines the review's
proposed virtual target records: the source IR itself remains the virtual
representation, so there is no second set of mutable virtual-value records.
Selected records already contain checked physical operands. No register reuse,
liveness allocation, block rewriting or IR optimisation occurs here.

`r0` is reserved. Slots come first, then each operation's result. Scratch storage
is added only when selection uses it; Classic also retains its diagnostic
location. A separate call window includes the argument-count register and the
largest selected call's operands. The same selector is used for planning and
emission, including hidden conversions; Classic's cold failure-message call
requires two arguments. Main's wrapper uses two registers for G and sixteen for C.
The public minimum register-budget argument remains 16. No liveness reuse is
introduced. Jumps to the next emitted block fall through, retaining its label
and resetting source/context state at that entry.
Classic counted/controlled DO adds one hidden owner per static key, outside
IR values and the call window. Owners initialize before fallible setup; helper
calls borrow them synchronously, and normal/transfer/fatal exits clear them in
the [approved lifetime order](architecture.md#approved-do-contract).
Grouping and state-free repetition add no DO owner.
A local call targets an internal `llfN` label; a block targets `llbN`. Source
identifiers never become assembler labels. Recursive calls receive independent
VM frames. G local calls are checked against the callee's exact IR signature;
Classic source units use required `(context, entry)` parameters and return a
fresh result-presence packet. Internal calls select positive checked entries in
one shared source body; see the [current C ABI](architecture.md#proposed-procedure-contract).

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
the existing named entries and checked-error builder; the
[CALL contract](architecture.md#call-contract) owns the exact interfaces.
Main forwards VM `-a` arguments as an ordered presence packet.

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

Source lookup scans original bytes for each origin; register/block discovery
also uses straightforward scans. Large-program indexing remains later work,
with no performance claim from this correction.

Keep debug metadata when linking (`rxlink -o ...`, without `-s`) for runtime
source reporting. G direct failures use the VM's original source metadata.
Classic's deeper runtime helpers can fail after the authored call-site frame
has unwound, so its existing function-level fatal handler still needs one
location field. The field names an authored expression, or explicitly describes
generated work/routine entry; it is not the original fault address.

The handler catches only CLASSIC_SYNTAX, DIVISION_BY_ZERO, CONVERSION_ERROR and
INVALID_ARGUMENTS. Small per-name entry labels select a literal relay; one
shared handler disables those catches and calls B `llctarget.failuremessage`
to format the retained context. That helper only returns text. The generated
handler uses `signalorigin` with the original VM-bound event, preserving the
original fault module/address, signal name and nonzero status. It does not
re-raise an ordinary signal at the formatting helper. Original error text is
retained after the authored-context prefix. Ignored overflow/lost-digits policy
is unchanged.

This is the existing standalone string-message fatal profile, not general
object-payload propagation or complete authored condition trapping. The
[current plan](../PLAN.md) and PLAN.md's CP criteria retain
full trapping, clause grouping, stack walking and INTERPRET lifetime as later
work. No new public condition object, IR format or VM API is introduced.

Compile-time failures distinguish source syntax/type/profile diagnostics from
IR lowering, import, output and limit failures. Runtime failures stay runtime
failures, including division in a taken branch; an untaken failing expression
is not evaluated in either compiler mode.

## File boundary, build and delivery

Research delivery is explicitly selected. Follow [the build/install guide](../README.md).
`stage-product` owns SDK/tool products; `stage-research-compilers` owns the
selected compiler services, commands and Classic adapter. Research installation
uses `--component ResearchCompilers` after base installation. Old combined and
stale compiler outputs are excluded from the broad base-bin installation.
