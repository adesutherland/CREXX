# RXAS inlining architecture

**Status: this design is approved and native VM preparation is locally qualified.
Shared Level L adoption and inliner2 remain open under the
[single toolchain plan](../PLAN.md).** Delivery is VM changes and
QA, then shared Level L changes and QA, then a fresh inliner2. This document owns
semantics and mapping rules; the plan owns numbered outcomes, tasks and gates.
Inliner1 is not the implementation baseline for the new service.

## Outcome, placement and core boundary

One language-neutral B/L service transforms checked executable RXAS bodies.

| Context | Order and binding authority |
|---|---|
| Assembler | Exact in-module bodies/globals; inline before ordinary assembler optimization. |
| Linker | Exact selected module/body identities; cross-module inline after selection and before final relocation/sealing, then bounded cleanup of changed callers. |
| Standalone | Checked assembler-like or linker-selected context supplied to the same service; ordinary RXAS input/output, independent of compiler AST inline templates. |

The cloned body and the linked/executed body are the same selected version.
Runtime binding must remain that exact provider; context-wide export names or
arbitrary body files cannot prove this. Native, unresolved or dynamically loaded
bodies remain calls. A dynamic selector requires an exact body plus preserved
selection/failure behavior, not a guessed target. Linker-local child expansions
already performed by the assembler need not be repeated; remaining dependencies
are processed bottom up with affected facts rebuilt after each body change.

Separate body availability, semantic representability and profitability. Growth,
recursion, depth and work limits are policy/resource outcomes, not semantic
impossibility. A missing canonical descriptor is a normalization error; a
missing optional value fact reduces optimization. Report retained calls with
causal reason, layer and exact source/site owner. Malformed or exhausted work
never publishes a partial candidate or warnings.

The inliner preserves the **published VM surface**. Internal allocation, dirty
recycler contents, cache state and C helper branches do not define another ABI.
Register return is logically COPY-equivalent assignment followed by callee-owned
lifetime cleanup. Buffer transfer/copy elision may implement that contract when
equivalent. Diagnose runtime mismatches at their owner or explicitly accept a
bounded difference; do not reproduce them in the expander. Genuine MOVE retains
its independently published storage-relocation semantics.

There is no fake-call fallback: publish the direct clone or retain ordinary CALL.
INLENTER, LINKARGV and QGLOBBIND retire with reserved opcode numbers; INLENTER2
was never implemented. The previous guarded INL2 proposal is unrelated to the
new service named inliner2. No CALLVIEW, interpreted return plan, callbr/retinline,
logical frame allocation or new hot-dispatch condition is part of this design.
SIGRET retires under VMIP-01; SIGCALLBR remains supported.

## One call, return and signal-exit model

Canonical call descriptors express exact body/module, result capture, ordered
actual pointers/count/presence, entry/base/active/slot identities, initializer
ingress, preparation phases and normal/abrupt continuations. They are ordinary
shared target data, not serialized proof recipes or a new execution mechanism.

- Every bytecode call has private callee-owned a0 backing, initialized with the
  captured argument count. It is distinct from the caller's count register;
  later body operations may modify/link it. a1+ borrow actual pointers, with
  independently mutable callee slot views. Repeated actuals may alias values
  while their slots remain distinct.
- Fixed CALL1–CALL4 capture explicit result/actual pointers before initializer
  ingress; ordinary counted calls capture their count/result/window after it.
  Zero-actual CALL clears its result before ingress and resolves the result
  binding afterward. Fused status/clear/swap preparation retains canonical
  order. Do not substitute a fixed call solely because arity is small.
- Preserve producer by-value copies and status/disposable conventions. Basic
  inlining does not prove that a producer/callee follows its own convention
  or synthesize extra copies to repair noncompliant code. Generic alias-aware
  optimization may later remove redundant copies.
- RETURN assigns the published value to the captured result with logically
  COPY-equivalent behavior, then performs required logical exit work and
  continues. Immediate and void returns retain their own surface effects.
  Required value conversion/copy effects occur under the callee numeric view
  before enclosing context restoration.
- Internal catches and resuming handler calls keep the clone live. A normal
  return or an owner-directed outward signal ends only the crossed logical
  activations, inside out. Retained real children remain real frames.

Start with fresh private cells/labels and generic typed remapping. All ordinary
body opcodes retain their database-described effects without inliner value
interpretation or a per-opcode permission list. A physical parent/register/raw
caller-address observation needs a concrete equivalent or a named retained-call
reason. Logical diagnostic labels alone cannot satisfy an operational read.

## Five exit duties

All five designs and M1–M3 are approved. Their machinery is conditional on actual
state duties; a policy-free/context-unchanged integer leaf needs no signal or
numeric entry/exit scaffolding.

### Duty 1 — signal policy and saved handlers

The supported response inventory is IGNORE (resume), HALT/SILENT_HALT (terminal),
BRANCH/BRANCH_VALUE (owner-directed unwind/jump), CALL (real handler then resume
interrupted continuation), CALL_BRANCH (unwind to owner, call handler, fixed
continuation), and CALL_ACTION (handler action protocol). Bytecode SIGCALLA's
`__rxsignal_skip` resumes; other results take its failure path. Its native
plugin signal-value relay is a separate public path. The valueless physical-
frame RETURN/SIGRET action is retiring; it is not replaced with a fake return.
CANCEL is a signal code with default silent halt, but installed policy can make
it nonterminal. KILL permits only terminal dispositions; OS SIGKILL bypasses VM.

SIGPUSH saves one exact named entry; SIGPOP restores the latest save for that
name even across other names. Real children inherit the handler table but start
with an empty save stack; policy writes are copy-on-write. Branch entries retain
the installing frame, code target and runtime stack marker, so an inherited
branch unwinds to its owner rather than treating its address as child code.
SIGNALORIGIN changes diagnostic provenance, not executable ownership.

| ID | Required rule and owner |
|---|---|
| **S1 — instruction self-failure** | VM: invalid signal instruction operands/handler/origin, missing POP save or allocation failure terminate the current VM run uncatchably with host cleanup/error result. Allocate/validate before policy publication. A valid SIGNAL's requested program event remains catchable. No silent failed PUSH or catchable secondary management error. |
| **S2 — policy and source-save facts** | Shared Level L: resolved tags/actions, source-save identities/ownership, path state and outstanding saves at exits. Analyze a source activation with its own empty save stack. A malformed authored POP must not succeed by consuming caller/protective saves. Retained real-child local policy is isolated. |
| **S3 — entry and normal exit** | Inliner: for each changed saveable tag reuse adequate authored protection or PUSH once at clone entry, before copied handler installation. Preserve source saves; internal catches/resuming calls retain protection. On return transfer result, drain known outstanding source saves and POP protective saves. No per-loop-write saves or setup for unchanged policy. |
| **S4 — outward branch** | VM uses the selected handler's existing dynamic marker to discard younger saves. Copy the selected entry before cleanup; unwind real children first. Executable region ancestry describes other crossed numeric/reference/binding cleanup inside out, before owner handler entry. A real child's saved callsite identifies its suspended inline parent. |
| **S5 — interrupted multi-tag cleanup** | Accepted SHRUG: ordinary successive POPs and existing dispatch remain. External interrupt timing/context may differ, including rare interrupted-run cleanup/terminal errors. Preserve ordinary ownership/values/control; no atomic group/return instruction or exact CALL/RET checkpoint proof. |
| **S6 — KILL** | Accepted SHRUG: HALT/SILENT_HALT terminal status/reporting and later KILL policy differences do not veto inlining. Copy authored setters; never synthesize unsupported KILL PUSH/POP. No KILL snapshot extension. |
| **S7 — producers/breakpoints** | Shared rules cover G blocks and C wrappers/relay. BPON with a procedure changes handler plus pending state; bare BPON/BPOFF affect pending state only. Preserve pending operations and protect changed handler entries. Physical TRACE/host observations retain their explicit M3 contract. |
| **S8 — native mode recovery** | VM retirement reapplies the surviving parent policy when discarding a private child table. Native mode transitions preserve the first captured host disposition for final restoration. Real-child normal/unwind and host readback controls qualify this; it is not an inliner admission rule. |

Native B/G protected blocks and Level L C use grouped PUSH/BR or BRV/reverse POP.
C adds Classic activation/condition and fatal-relay layering; native C handler
helpers can write unsaved policy. Both use the same protection model. Language
SIGNAL/TRACE services use CALL/CALL_ACTION and breakpoint control. Handwritten
CALL_BRANCH remains supported even though reviewed compilers rarely emit it.
Runtime frequency is unmeasured; assess practical use before adding special
complexity. G blocks must work, and C wrapper semantics must be understood.

The selected handler's own block saves remain for authored handler-entry POPs.
For nested A/B/C/D clones, escape to A restores saves newer than A's recorded
marker and retires D,C,B; a target in C retires only D. A returning handler keeps
its suspended region. There is no new signal-stack marker or atomic multi-tag
restore. Native support is locally qualified; shared L policy facts and generated clone emission remain open.

`SIGBRV` is an ordinary required use case, not an unused exotic form.
The native signal-block/frame emitter uses it when a handler binds an event;
the research C emitter uses it in ordinary procedure error wrappers and host/
child failure layers. Classic recording consumes the event for SIGL, SYNTAX RC,
origin, message and payload. Handler installation is common even when delivery
is exceptional. Installation reads no old destination value; successful delivery
writes an initialized `runtime_signal` on its handler edge. Shared facts must
separate that known definition/type from unknown fields and genuine payload
COPY effects. Do not reject whole procedures merely for containing SIGBRV.

### Duty 2 — numeric and provider context

Five independent fields are DIGITS, FUZZ, FORM, CASE and STANDARD. Decimal and
Unicode providers are worker-selected and borrowed by ordinary children; no
published per-clone provider switch or provider allocation is required.

| ID | Required rule and owner |
|---|---|
| **N1 — changed fields** | Level L consumes canonical separate numeric read/write masks and point/exit facts. A read alone needs no restoration. Real-child local settings do not become the enclosing clone's write set. |
| **N2 — normal restore** | Use existing GET/SET. Known enclosing values restore as literals; unknown values are saved once in fresh protected locals. Restore only affected fields, reuse adequate authored restoration and omit equal/unchanged work. Unknown caller settings alone do not reject inlining. Transfer result under callee view first. |
| **N3 — nonlocal restore** | Internal catches/resuming handlers retain current view. Cold region data restores crossed fields from constants or mapped saved registers before entering an ancestor handler, inside out. No numeric stack, entry allocation or new numeric opcode. |
| **N4 — surviving real frame** | VM reattaches shared decimal provider to the surviving live numeric context on every return/unwind, including void and handler returns. Rebind even when values match. Native return/unwind repair and component integration are locally qualified; L projection QA remains open. |
| **N5 — provider identity** | Preserve selected worker providers and authored operations. Defensive legacy loaded-here flags do not imply a frame-owned provider. Only a real published switch would add a new duty. |
| **N6 — errors/timing** | Ordinary invalid numeric setters retain catchable program errors. Only signal-instruction self-errors use S1. External snapshots during successive restores use the approved timing SHRUG; once outward delivery is selected, N3 still restores before owner handler entry. |

Shared `contextrestoration` reports literal-versus-save choices, ordinary
return sites and potential cold exits independently of CALL removal. Hooks
conservatively include their represented numeric fields; asynchronous and
terminal paths are not blanket refusals. Cold actions use the shared edge
projection. Incomplete numeric contracts remain explicit unknowns. There is
no inliner-specific numeric solver or numeric scope.

Decimal ABI 3's `value-only-nonreentrant.v1` synchronization contract is approved:
for a valid context, syncNumericContext is total, non-signalling, non-reentrant
and depends only on effective values, not pointer identity/call count/order.
It performs no allocation or user callback; other plugin operations restore
their temporary internal context. Omit redundant sync only when effective values
are unchanged, while still rebinding the pointer to live context. Registration
and dynamic v3 descriptors reject incompatible older plugins; layout itself
does not change. Ordinary decimal operations retain their own effects.

### Duty 3 — reference lifetimes

| ID | Required rule and owner |
|---|---|
| **R1 — owned roots** | Level L distinguishes owned local/a0/nested backing from borrowed a1+/globals/private suffix. Ownership is independent of active links, register number and slot swaps. Publish finite owned roots per logical clone. |
| **R2 — normal exit** | Existing ENDLIFE invalidates target identities and releases descriptor retains throughout owned backing, including inactive unlinked attributes. Address the base via proved mapping/UNLINK, never borrowed active storage. After result assignment end remaining owned lifetimes before repeated activation reuse. No ordinary value clear/native finalizer is implied. |
| **R3 — returned values** | Logical COPY-equivalent return governs value, destination assignment and descriptor targets, then callee-owned expiry. A returned descriptor still targets original storage; returning it does not extend a local target's lifetime. Internal copy/move choices are not an inliner contract or per-site compliance proof. |
| **R4 — outward exit** | Cold cleanup uses the same owned-base map with exit-specific presence facts, retiring crossed regions inside out. A root emptied by a later normal return may still require cleanup on an earlier signal. Internal/resuming handlers retain locals; preserve authored lexical ENDLIFE independently. |
| **R5 — conservative cleanup** | Unknown presence with known owned root permits ENDLIFE, not call rejection. Omit only if identities AND descriptor payloads are absent across the owned tree, including inactive capacity and child MKREF through borrowed clone storage. ENDLIFE has a canonical NONE signal row and complete no-callback inventory. |
| **R6 — VM conformance** | Ordinary register RETURN uses COPY assignment and marks surviving owners when reference state may be installed. Controls cover source/destination identities, linked attributes, returned descriptors, borrowed sources and native-copy effects. Result-installed identity/descriptor retains must mark the surviving owner for retirement. No universal COPY helper requirement or blanket reference SHRUG. |

ENDLIFE is opcode 514, one register; its helper neither allocates, invokes Rexx
callbacks nor synchronously signals. It follows owned unlinked attribute backing,
not borrowed active links, and does not run native finalizers. Deterministic
REFVALID after a completed logical exit reflects the actual owner lifetime.
General MOVE's identity-following relocation remains useful but does not redefine
RETURN's surface.

### Duty 4 — native payload finalization

| ID | Required rule and owner |
|---|---|
| **F1 — eventual physical retirement** | VM scans owned local/a0/all owned attribute backing, including inactive capacity, before recycling on normal/unwind exit. Finalize each outstanding ownership obligation once; exclude borrowed actuals/globals/active links/private suffix. Other dirty value contents remain. The scan and real-frame normal/unwind/terminal/reuse controls are locally qualified. |
| **F2 — clone exit** | Accepted SHRUG: payload left in clone backing remains owned by enclosing physical frame until its retirement; explicit destruction/overwrite may release earlier. No per-clone finalizer instruction, logical-exit scan or cold finalizer action. Timing and active Rexx frame/numeric/signal context are unspecified. Reference expiry is separate. |
| **F3 — copies/transfer/return** | Preserve COPY/share and genuine MOVE ownership, logically COPY-equivalent return and required native-copy effects. Faster buffer movement must remain equivalent or have a specifically accepted difference. Never lose/double ownership or consume borrowed storage without a valid contract. |
| **F4 — body operations** | Preserve explicit clear/overwrite/close/dispose and native calls through canonical effects. Possible finalization does not exclude a procedure. Optional deletion/reordering needs ordinary shared effect/ownership facts; uncertainty keeps the operation. |
| **F5 — provider convention** | Finalizers release resources/private bookkeeping without Rexx reentry, active-context dependence or catchable/resumable Rexx signals. Cleanup failure may log/continue or fail terminally. Trust this public convention; violations are provider defects, not per-site inliner proofs. Shared effects retain real COPY hooks/explicit calls but no Rexx callback edge solely for conforming cleanup. |

The false-positive/no-false-negative frame flag to skip scans is separate deferred
work VM-FINALIZER-SCAN-01 in performance/ROADMAP.md. Correct simple retirement
scanning and ownership qualification suffice now; no optimization prerequisite.

### Duty 5 — temporary pointer bindings

| ID | Required rule and owner |
|---|---|
| **P1 — fresh layout** | Start with distinct fresh storage and affine relocation of the original packed vector: `fresh(D) || old-r || g || a`. Original rN becomes r(N+D); g/a semantic owners remain. Grow within actual encoding/resource limits, then compact later. Current local extent is not a refusal. Simple scalar output still has no call/link/swap scaffolding. |
| **P2 — independent views** | LINK changes active binding; UNLINK restores slot base. SWAP/SWAPN exchange pointers AND slot identities in source operand order; bases remain fixed. Track entry separately. Aliasing values do not merge independently rebindable slots. Formal UNLINK returns to captured actual, not unused fresh-r backing; normalize or retain fresh entry anchors as needed. |
| **P3 — retained windows** | Fixed CALL1–4 name arbitrary actual pointers without a contiguous window and do not target native functions. Finite scattered counted windows get fresh contiguous linked blocks preserving aliases, without argument-value copies. Affine whole-vector relocation preserves original contiguous windows with unknown-length tails/cross-bank spans. Only genuinely unbounded scattered views lacking a representation retain a call, with that exact reason. Preserve source capture/ingress phases. |
| **P4 — restore actual deltas** | On normal/outward exit undo only temporary binding/permutation/window changes introduced or deliberately retained by expansion, using exit-specific state. Never undo authored value writes or arbitrary intentional earlier swaps. Existing real-child cold window restoration uses unique slot identities, not pointer equality. Internal/resuming handlers keep mappings. A direct mapping has an empty duty. |
| **P5 — assign after expansion** | Reanalyse changed bodies bottom up, then generic constrained assignment/coalescing reduces cells and swaps using component liveness, aliases, base/entry identity and ordered windows. Unknown/limited optional compaction leaves the valid larger mapping. No new opcode or pointer-specific SHRUG. |

Fresh private numbering does not initialize dirty components. Component liveness
and logical activation boundaries are analysis marks, not extra instructions.
Runtime binding anchors are introduced only where the source slot semantics
actually require them, never as a disguised universal call frame.

## Shared Level L ownership and facts

One canonical instruction database owns identities, typed operands/references,
ordered reads/possible writes/definite kills, component/status/cursor effects,
binding actions, numeric reads/writes, signal names/phases, call/return control,
physical observations and relocation. Missing facts are explicit unknowns.
Ordinary instructions are cloned by typed remapping, not reinterpreted by the
inliner. Canonical database defects are fixed there; flow/algorithm defects in L.

The shared machine CFG includes normal, table, handler/resume, unwind, terminal
and unknown alternatives. Named signal policy/save correlation is path/edge
state, not a mnemonic recognizer. Track storage versus value identity and every
published subvalue/individual flag. Conversions carry dependencies: an int-derived
string is current only while source identity and conversion conditions remain.
Possible writes do not become definite kills on resumed/error paths. Lexical
metadata is address state, not conditionally executed state.

Procedure facts distinguish direct/transitive and internal/caller-visible
effects; may/must/unknown and normal/resumed/abrupt exits remain separate. Known
return/context facts retain entry premises and actual normal-return existence,
not vacuous guarantees. Exact child dependencies are processed bottom up and
recursive components use bounded fixed points. Rewrites/provider changes
invalidate affected summaries. Summaries do not replace point proofs for copy,
swap, flag or lifetime removal. Unknown is not evidence of absent effects.

Owned locals start dirty until defined. The shared early normalization warns and
safely clears only proved entry-dependent reads with source/component identity.
Do not clear an already-defined component on another path; place earlier or use
a component operation where needed. Do not clear borrowed arguments/globals.
Unknown defined values and incomplete demand coverage are not dirty-read proofs.
Publication of clears, remaps and warnings is atomic. This generic capability
serves assembler, linker and standalone inputs without old receipt-driven
universal clone-permission machinery.

Checked typed remapping relocates explicit/implicit banks/windows, labels,
constants, procedures, exact module/global owners and metadata. Assignment uses
component/flag/address/reference liveness with pin/fixed/window/lifetime
constraints and independent validation. Fresh valid placement is the fallback.
Enduring APIs live in [RXAS design](../../../../langtools/doc/rxas-design.md) and
[IR design](../../../../langtools/doc/ir-design.md); the plan's phase 2 qualifies
them with independent non-inliner consumers before adoption.

Current RXAS adapters still reject raw TRACE backslash without lexical proof
and nonnegative TRACE K pool offsets without relocatable binding; .const aliases
currently support string/binary, not float/decimal. These are named representation
gaps to reconcile in phase 2 against its required profile, with wider coverage
open under TGT. They are not permanent inliner admission rules or evidence that
the complete future RXAS profile is implemented.

## Typed metadata and entry contracts

These are approved handwritable contracts, not compiler proof payloads. Current
implementation does not by itself establish complete integration QA.

### Static inline diagnostic frames

RXAS declares `.inlineframe id parent "module" "procedure" callsite-step` and
`.inlinemap begin-label,end-label,id`. META_INLINE_FRAME/META_INLINE_MAP use
RXBIN007 feature bit 9. Nonzero IDs are module-local; parent zero denotes the
containing physical procedure. Names are diagnostic only. A nonzero callsite
step names an unambiguous output source anchor; identical repeated .srcstep
records with the same ID/source payload are allowed, not duplicate frame
identities. The frame definition sits at the removed call's clone-entry anchor.

Maps are half-open instruction-cell ranges selecting the deepest frame;
disjoint ranges can share a frame. Validate names, unique identities, parent
existence/acyclicity, source references, instruction boundaries, depth/range
consistency and one containing physical procedure for definitions/maps/ancestors.
No map crosses procedure boundaries. Preserve frames/maps through source/compiler
inline stripping; source stripping sets callsite-step zero. Relocate ranges,
source and ancestry through optimization/linking; drop a map only if its whole
range becomes empty, retaining required definitions/endpoints/ancestors. This
uses ordinary request work/byte limits.

The cold diagnostic walker expands the static chain from actual fault/resume
and retained-child callsite coordinates. No per-entry instruction/allocation.
Diagnostic maps alone grant neither executable identity nor cleanup authority.

### Executable cold-exit boundaries

RXBIN 007 feature 12 carries version-1 REGION, RANGE and EXIT records. REGION
defines a module-local identity, parent and owning PROC at the clone-entry
coordinate; `.inlineregion id,parent` writes it. `.inlinerange begin,end,id`
maps a half-open instruction range. Several disjoint ranges may name one
region, every child range is contained in a parent range, and unrelated ranges
cannot overlap. Diagnostic inline IDs confer no executable region authority.

`.inlineexit id,begin,end,order,"action",operands` records a finite cold action
for a half-open source-PC phase within one range. Active orders within a region
are unique. Numeric field names select literals or saved owned r bases;
ENDLIFE names owned r bases; UNLINK/SWAP/LINK use typed r/g/a slots, including
captured entry anchors. Static local/global/owned-prefix bounds are checked at
decode and argument-window bounds at cold use. Unused fields are zero. Phases
begin after a required save or binding change exists, and finish when ordinary
cleanup discharges that duty. They describe duty changes, not every instruction.
The codec owns portable encoding and prepublication validation. Native linking
relocates PROC references while preserving module-local code and bank indices;
executable records survive stripping. The native optimizer protects affected
ranges and named storage until it models cold consumers, leaving unrelated
code available to ordinary optimization. No inliner refusal follows from that
conservative native cost; phase 2 owns shared target relocation and facts.

Only a selected cold departure evaluates these actions. Numeric saved values
obey the individual RXAS field bounds; no source-language FUZZ-versus-DIGITS
restriction is imposed. Sources and bounds are validated before cleanup; fields
restore inside out and the provider reattaches to the final surviving view.
Actual PC, not SIGNALORIGIN, determines source owner. Validate retained-child
callsite ownership independently of return PC, active/ancestor destination,
action storage ownership and bounds. Inactive sibling/stale targets fail safely.
Copy the selected handler before cleanup; unwind real children, restore its
signal marker and retire crossed regions inside out before entering the handler.
Internal/resuming responses keep the activation. No clone-entry allocation,
normal-path plan interpreter or new hot-loop condition is needed.
IGNORE/resuming responses retain live regions; HALT/SILENT_HALT and terminal
instruction self-errors perform required run and physical-owner cleanup. This
adds no logical finalizer scan or exact finalizer-timing requirement.

### Borrowed private globals and exact calls

Procedure-entry `.inlinedprivate rK,"exact-provider",gN` uses
META_INLINED_PRIVATE / RXBIN007 bit 10. Declarations follow the header before
the first instruction at the procedure start coordinate. `.locals` is total
extent; one declaration per slot covers a complete contiguous borrowed suffix,
with owned prefix inferred from its first slot. The record stores owning PROC,
typed QGR1 provider selector, local/global indices and owned-prefix length.
Repeated target globals are allowed as distinct slots.

Exact linker selection seals provider identity and storage dependency without
public exposure or changing caller globals. Resolve worker-local pointers without
running initializers. Activation/reuse reinstalls suffix active/base pointers
and unique slot identities; allocate backing only for owned prefix/a0. All
implicit cleanup excludes suffix storage. LINK/SWAP never manufacture ownership;
return does not consume borrowed globals. Preserve initializer/poison/cycle and
provider lifetime. Reject malformed/unsealed/unresolved/conflicting bindings;
optimizers preserve suffix/borrowed carrier constraints. No per-call bind op.
Every declaration, including an unused slot, survives optimization. Pin the
borrowed suffix; temporary growth cannot cross it without a separately validated
layout and ownership remap.

A retained private CALL/SIGCALL may use typed QPR1 `@M::P()` with exact linked
image/provider procedure ordinals. Loader validation/sealing is required; a
standalone unsealed operand cannot manufacture authority. It is not an export
or a fake call. Preserve old-image identity rebasing on relink.

### Program-default entry and initializers

`main() .locals=N .entry=program-default` (optional `.expose=name` precedes
`.entry`) uses META_ENTRY_STATE, version 1/kind 1, RXBIN007 feature bit 11. One
local main owner per module; reject malformed/duplicate/non-main ownership and
conflict with META_INITIALIZER. Preserve through stripping/disassembly/linking.
Ordinary unmarked main remains ordinary.

Checked entry presents a logical root with no program parent, handler-save
stack or active handler, canonical default signals and numeric DIGITS 18/FUZZ 0/
SCIENTIFIC/LOWER/COMMON. The canonical inventory is VM-owned rxvmentry.h, including
its mixed per-signal responses; L consumes that projection. Ordinary argument/
result/module rules remain; recycled locals do not become clean. This certifies
initial state, not callback absence, future policy or initializer success.

VM/loader/debugger may establish or emulate that view. Supported reentry
suspends/restores outer state; pending events are preserved and delivered under
the declared initial policy at the defined boundary. rxvm_run/rxvml_run use the
checked one-shot authorization at exact worker entry depth; ordinary rxvm_call
does not grant it. Forbidden child/signal/task/dynamic entry rejects before
initializer/preparation/capture/body effects. Static forbidden references fail
before publication; dynamic child rejection uses the original continuation if
handled. It raises INVALID_ARGUMENTS with an entry-contract diagnostic at the
invoking instruction, before preparation/capture. Unsupported paths fail
explicitly rather than violating the root view.
This is not a closed-world ban on debuggers, other modules or dynamic loading.

Existing META_INITIALIZER grants the same canonical observable entry view with
no second flag. Initialization order and once-only state are per worker/thread
module instance. READY is required for ordinary entry; FAILED remains poisoned;
INITIALIZING allows only checked initializer-lineage helpers. External/reentrant
ingress to that still-initializing instance rejects. Preserve dependency/cycle,
outer-state restoration and original call-capture timing. Structural absence of
an initializer is not arbitrary runtime READY evidence. Untaken cloned paths
do not initialize a provider early.
Disassembly preserves initializers at their canonical metadata-chain position,
including unexposed local owners; do not reorder or duplicate by symbol name.

## Post-inline cleanup and qualification boundary

Expansion is correct with cleanup and assembler optimization disabled. In-module
cloning precedes ordinary assembler optimization; linker cloning reanalyses
changed callers even when input modules were already optimized. The minimum
shared cleanup panel rebuilds/validates references and CFG; propagates actual/
presence/individual-flag facts; removes proved branches and dead status writes;
removes redundant copies plus caller setup AND reverse swaps by whole-region
alias/permutation proof; then recomputes liveness/constrained assignment and
prunes unreachable code/metadata under consumer rules. Preserve necessary body
swaps, child windows and value snapshots. Optional cleanup failure retains the
validated uncompressed layout, never an invalid candidate.

Known-on TRACE can disable optimization/inlining. Dynamic TRACE still requires
truthful source/logical maps and any concrete operational observation contract,
without exact asynchronous checkpoints or a special clone-entry scope. Final
address/source/task/graph relocation and sealing are transactional.

The first inliner2 milestone is only the integer CALL1/loop/result proof with
no scaffolding. Wider fixed/counted/fused/vararg/nested/alias/global/context/
signal/reference cases remain open against the plan's stable criteria. Native
VM mechanisms are first qualified with handwritten fixtures; shared L facts
with independent clients; generated clone behavior only in phase 3 and later
reviewed capability growth. No code or full-coverage claim follows from this
approved architecture document.
