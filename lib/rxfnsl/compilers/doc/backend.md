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

Each selected sequence retains its IR origin ID in Level L output chunks and
emits `.srcstep` with the original filename, line, Unicode column and source
line. CRLF is one newline. Synthetic origins remain valid. The simple source
lookup scans original bytes for each origin; register and block discovery also
use straightforward scans. Large-program indexing and resource tuning remain
later work, with no performance claim from this implementation.

Keep debug metadata when linking (`rxlink -o ...`, without `-s`) for runtime
source reporting. G failures use the VM's source metadata. Classic's adapter
can fail several runtime frames away from the authored operation, beyond the
VM's short panic backtrace. Each Classic function therefore tracks its current
source location in a separate register. A local fatal-signal handler attaches
that location and re-raises the same signal name. It covers CLASSIC_SYNTAX,
DIVISION_BY_ZERO, CONVERSION_ERROR and INVALID_ARGUMENTS; it does not enable
ignored overflow or lost-digits signals. Complete authored TRACE/condition
handler parity is outside this first profile.

Compile-time failures distinguish source syntax/type/profile diagnostics from
IR lowering, import, output and limit failures. Runtime failures stay runtime
failures, including division in a taken branch; an untaken failing expression
is not evaluated by this unoptimised compiler.

## File boundary, build and delivery

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
Python only orchestrates tests; it is not a compiler stage. The separate
[qualification receipt](../evidence/step04-20261015/README.md) identifies the
actually executed local checks and their platform limits.
