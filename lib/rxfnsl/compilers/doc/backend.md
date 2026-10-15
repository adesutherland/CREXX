# C/G IR to RXAS

The STEP04 backend completes the unoptimised executable path for the
[admitted C/G profile](poc-scope.md). `llrxc-c` and `llrxc-g` are separately
selected commands. The existing `rxc`, `rxc-c` and wrapper routes remain the
production routes. Shared IR optimisation is STEP05; full replacement and
INTERPRET retain their later gates in [PLAN.md](../PLAN.md).

## Ownership and public surface

```rexx
import llcompiler
import llir
request = .llcompiler..request('g', source, artifacts, 'example.crexx')
if request.status() <> 0 then say request.diagnostic()
else do
  output = request.output()
  say output.text()
end
```

`source` is binary, preserving exact UTF-8 source bytes. `artifacts` is one
explicit directory of target import images. The optional arguments after the
filename are semantic/source budget (1,000,000), nesting (512), emitted bytes
(67,108,864), and registers per function (65,536). These are protection limits,
not throughput promises. Exhaustion fails the request and exposes no partial
successful output. Register budgets below 16 are invalid.

`status`, `error`, `offset`, `diagnostic` and `output` evaluate the request once.
`graph` retains frontend inspection. The installed `llcompilers.rxbin` package exposes these B services; a consumer
links it with `rxfnsl.rxbin` and `library.rxbin`. The command I/O provider and
Classic target runtime are absent from that service package.

The request owns its frontend and backend;
the backend owns the frozen graph and the completed Level L output. A returned
output owns its chunks independently and survives the request. Requests share
no mutable compiler state. Repeated independent requests are tested; concurrent
service use and interpreted-code lifetime are later qualification work.

An advanced caller can pass a frozen graph to
`.llrxas..compilation(graph, level, artifacts, bytes, registers)`. It exposes
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
| Import cache | Package catalogs and checked callable signatures | B-only RXBIN007 reader, result/arity/type checks, no optional arguments in this ABI |
| Output | Level L writer sections and owned origin-tagged chunks | Per-section and total byte budgets, finish/stream error checks |

Selection uses two passes per function: allocate stable locations first, then
visit its blocks and operations in stored order. This refines the review's
proposed virtual target records: the source IR itself remains the virtual
representation, so there is no second set of mutable virtual-value records.
Selected records already contain checked physical operands. No register reuse,
liveness allocation, block rewriting or IR optimisation occurs here.

`r0` is reserved. Slots come first, then each operation's result. Two scratch
locations and a separate call window follow. Main's wrapper has 16 registers.
A local call targets an internal `llfN` label; a block targets `llbN`. Source
identifiers never become assembler labels. Recursive calls receive independent
VM frames. G local calls are checked against the callee's exact IR signature;
all Classic routines use argument/numeric packets and a result-presence packet.

Arguments are copied into the private window, their ABI flags set, and the call
writes a distinct result register. The window is cleared after return. Classic
pool arguments are explicit references: pool loads/stores use links, and adapter
calls borrow the same activation pool. Numeric and argument packets are value
snapshots. Callee NUMERIC updates do not modify its caller's state. Each Classic
PROCEDURE creates its own pool; CALL applies the returned presence packet to
RESULT and clears RESULT when no value is returned. Expression calls require a
value. Main forwards VM `-a` arguments as an ordered presence packet.

## Semantics and literals

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
runtime. The adapter installs the explicit DIGITS/FUZZ/FORM snapshot in its call
frame and uses the checked Classic expression/BIF helpers. Exact source literal
spelling, pool keys, unset reads, UTF-8 comparison policy, eager logical operands,
ARG uppercase/presence, numeric changes and Classic error codes remain distinct
from G. ARG preserves surrounding blanks. The target `rxfnsc` package still has
G prerequisites; that dependency does not enter the compiler host.

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

The IR has no real authored clause association yet. Source records therefore
use clause ID zero rather than pretending origin IDs are clause IDs. Full clause
grouping and TRACE-event fidelity remain open. The backend suppresses identical
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
[bounded design](CONDITION-PROVENANCE-REVIEW.md) and PLAN.md's CP criteria retain
full trapping, clause grouping, stack walking and INTERPRET lifetime as later
work. No new public condition object, IR format or VM API is introduced.

Compile-time failures distinguish source syntax/type/profile diagnostics from
IR lowering, import, output and limit failures. Runtime failures stay runtime
failures, including division in a taken branch; an untaken failing expression
is not evaluated by this unoptimised compiler.

## File boundary, build and delivery

`stage-product` selects `stage-l-libraries`, including `llcompilers`,
`llctarget`, `llrxc-c` and `llrxc-g` alongside `rxfnsl` and `levell_cli`.
All four compiler delivery products must exist before payload staging or
`cmake --install`; declaring a producer `ALL` does not include it in an
explicit stage selection. The `llcompiler_product_inventory` essential test
uses `stage-product` as its preparation target and checks the four files.

```sh
llrxc-g -i /path/to/bin -o demo.rxas demo-g.crexx
rxas -n demo.rxas
rxlink -o demo-linked demo.rxbin /path/to/bin/library.rxbin
rxbvm demo-linked.rxbin
```

For C, select `llrxc-c` and also link `llctarget.rxbin` and `rxfnsc.rxbin`.
The command first completes the owned service result. It then claims the sibling
`output.rxas.llrxc-stage` directory using exclusive directory creation, streams
Level L chunks into its own file, flushes/closes it and renames it over the
destination. Rename is the only destination mutation. Failure leaves the prior
file intact. A pre-existing stage directory causes a diagnostic and is left
untouched; cleanup removes only the staging files owned by that invocation.

All compiler logic and the target adapter are `.crexx` with `options levelb`;
the two grammars are Level L. Each metadata import root contains only declared
unlinked prerequisites, never neighboring linked test images. Native `rxc`
builds the host at bootstrap. Standard `rxcpack` generates the small VM launchers
for installed commands; those generated C launchers contain no compiler logic.
Source requests never invoke a native compiler, a compiler exit or a C backend.
The command uses the existing `rxfs` native platform I/O service.

The permanent [corpus](../tests/backend/cases.json),
[service regressions](../tests/backend-tests.crexx) and
[orchestration](../tests/check-backend.py) exercise both host modes, both VM
dispatches, both assembler modes, source/error fidelity and file failures.
The focused [provenance checks](../tests/check-provenance.py) additionally cover
all four fatal relay arms, recursive/deep helper errors, loops, skipped failures,
stripping, physical-root independence and straight-path metadata/context
deduplication. Python only orchestrates tests; it is not a compiler stage. The separate
[qualification receipt](../evidence/step04-20261015/README.md) identifies the
actually executed original local checks and their platform limits. The
[provenance receipt](../evidence/provenance-20261015/README.md) covers this
correction. Rebuild compiler tools and `llctarget.rxbin` together: the internal
adapter contract now returns a formatted message through `failuremessage`;
the emitted handler owns the origin-preserving raise.
