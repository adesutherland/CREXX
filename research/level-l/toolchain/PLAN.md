# Level L target-toolchain research

**Status: phase 1 VM preparation is implemented and locally qualified; the
shared Level L baseline and new inliner2 implementation/qualification remain open.** This is the single
authoritative plan. The [inlining architecture](doc/inlining.md) owns the detailed
mapping and exit contracts; this file owns outcomes, steps and completion gates.

## Vision and approved delivery order

Provide language-neutral RXAS inlining over the exact executable bodies selected
by the assembler or linker. Ordinary instructions are cloned/remapped through
the canonical opcode database and shared Level L target services. Inlining must
produce useful code, with the simple integer-loop example reduced to the loop
and result work rather than a renamed call. Existing native AST inlining and
product defaults are unchanged by this research initiative.

Adrian has reset implementation around three sequential, coherent phases:

1. **VM preparation:** complete the approved runtime, native catalogue and
   supporting assembler/codec/loader/linker changes together; review, QA and
   commit that baseline before beginning the Level L phase.
2. **Level L foundation:** complete and document all generic target changes
   needed by this architecture, including the resulting VM contracts; review,
   QA and commit them before starting the new inliner.
3. **Inliner2:** build a fresh service and consumer path, reusing only reviewed
   infrastructure from inliner1. First demonstrate one integer argument, loop
   calculation and integer return with no call scaffolding; QA and commit it.

Each phase is a named integration gate for its component, not full initiative
acceptance, production replacement, publication or release qualification. No
later phase may hide a failed earlier gate with an inliner workaround. Native
QA preparation/execution remains centrally owned; independent workers handle
design, implementation and source review under AGENTS.md allocation.

The initial IN2 proof is deliberately small. The wider standalone inlining
outcome remains open under INL-AC01–09 below; the complete Level L assembler,
RXBIN reassembler and linker remain a separate future TGT proposal. The new
service must support assembler-like and linker-selected contexts without two
algorithms, but the first integer-loop milestone does not claim all call forms,
signals or cross-module integration.

## Governing requirements

- **VM surface first.** The canonical database describes published behavior.
  Internal recycling, caching and copy/buffer-move heuristics are implementation
  details. An implementation mismatch is repaired at its owner or explicitly
  accepted as a bounded difference; it is not simulated by the inliner.
- **Return is logically COPY-equivalent.** Preserve value assignment, descriptor
  targets, destination behavior and required copy effects, then end callee-owned
  lifetimes. Do not impose universal use of a particular C helper or alter the
  independent semantics of genuine MOVE. A returned descriptor does not extend
  a callee-owned target's lifetime.
- **Inline or call.** No fake-call fallback, allocating inline scope, guarded
  clone/real-call scheme, call-view or interpreted return plan. Retire INLENTER
  and its dependent LINKARGV/QGLOBBIND mnemonics with numeric identities reserved.
  INLENTER2 was never an implemented opcode. SIGRET retirement is selected below.
- **One database and one shared analysis.** Normal/signal/unwind flow, effects,
  reference domains and facts belong to the canonical catalogue and Level L,
  serving future RXAS tools too. No inliner opcode permission list, second
  instruction evaluator, callback veto list or proof of producer convention
  compliance. Preserve ordinary body operations; optional optimization needs
  its own facts, while basic cloning does not require understanding each value
  algorithm.
- **B/L runtime ownership.** Target requests use B/L algorithms without native
  assembler/linker encoding backends, native source-compiler services, subprocess
  fallbacks or provider bridges to those backends. Native B bootstrap, prepared
  canonical-schema extraction, native VM execution and independent native
  comparison oracles remain allowed.
- **Simple construction before optimization.** Fresh locals/labels and register
  growth are valid. Current local extent is not a refusal reason. Preserve
  active/base/entry/slot identities and whole call windows; compact later with
  generic constrained register assignment. Failed optional cleanup leaves the
  valid larger expansion.
- **Bottom up and reanalyse.** Expand children before parents; after a changed
  body rebuild affected shared facts before considering remaining sites. Old
  summaries are not evidence about new code. Recursive groups obey explicit
  depth/growth/work limits; budget exhaustion is a supported unknown result.
- **Dirty is not defined-unknown.** New owned locals start dirty. Reading an
  unwritten component is a source error, not a promise to reproduce recycled
  bytes/cache state. A standard shared early normalization diagnoses a proved
  dirty read and safely inserts an ordinary clear with a warning. Unknown
  analysis alone never licenses clearing; borrowed arguments/globals are not
  cleared indiscriminately. Preserve the same contract for inline locals.
- **Metadata is a public contract.** Typed, handwritable signatures, startup,
  linking, source and required cold-exit data are appropriate. No serialized
  inliner proof/certificate or hidden compiler side channel.
- **Independent expectations and atomicity.** Existing native output/tests are
  evidence, not automatic correctness oracles. Distinguish unavailable body,
  semantic/missing fact, policy/resource refusal and normalization error.
  Malformed input or failed transformation publishes no partial output or
  warnings; retain exact owner identities and last-good publication.

**Accepted differences (`SHRUG`).** Exact asynchronous interrupt placement and
pre/post-return numeric/policy/binding snapshots are unspecified. Ordinary
successive signal POPs are allowed, including rare interrupted-run cleanup or
terminal differences. KILL terminal status/reporting differences, including a
later KILL after a cloned setter, are accepted; do not synthesize invalid KILL
saves. Removed frame-allocation failure, recycler timing and profiling counts
need not be reproduced. Required initializer, body and cleanup failures retain
their contracts. Native finalizer incidental timing and active
Rexx context are unspecified. These are not refusal reasons and require no
atomic return/group opcode or new dispatch-loop condition. They do not waive
ordinary values/control, synchronous outcomes, reference lifetime, once-only
ownership or memory safety. No blanket reference-specific SHRUG is approved.

## Approved exit and mapping design

All five duty designs are **approved**, including duty 5 and the VM-surface
return clarification. Detailed S1–S8, N1–N6, R1–R6, F1–F5 and P1–P5 rules
are in the [architecture](doc/inlining.md#five-exit-duties).

| Duty | Required behavior |
|---|---|
| Signal policy and saved handlers | Protect changed saveable tags once before clone handler installation; preserve authored save ownership; normal exits POP known source/protective saves. Outward handlers use their existing dynamic save marker. Internal/resuming handlers keep clone state. |
| Numeric/provider context | Restore changed fields from known literals or ordinary saved GET/SET values. Outward exits restore the enclosing view before its handler. Shared provider attachment must name live surviving context. |
| Reference lifetimes | End owned local/a0/owned-attribute lifetimes at logical exit using ENDLIFE or its cold equivalent; borrowed actuals/globals stay live. Unknown presence with known ownership permits conservative cleanup. |
| Native payload finalization | Preserve each ownership obligation until once-only eventual physical retirement; clone exit adds no finalizer instruction/scan. Explicit COPY/close effects remain. |
| Temporary pointer bindings | Undo only actual introduced/retained binding/permutation/window deltas. Direct fresh mapping can make this empty; do not undo value writes or arbitrary authored swaps. |

Three independent mapping/control requirements remain:

- **M1:** preserve argument/result capture phase and order, aliases, counts,
  presence/status, calling-convention copies, private callee a0 count backing,
  borrowed independently rebindable a1+ views and logically COPY-equivalent
  returns. Fixed, counted and fused calls retain their own ingress phases.
- **M2:** preserve exact selected body/module/global ownership, worker-local
  private globals and initializer dependency/READY/INITIALIZING/FAILED state
  at the original reached boundary. Untaken clones do not initialize early.
- **M3:** preserve retained real children, synchronous signal continuations,
  logical source/diagnostics and concrete operational parent/frame/address
  observations. Normalize a known logical access or retain its call with a
  specific semantic reason; diagnostic names alone cannot satisfy an executable
  physical-register read.

## Phase 1 — complete VM preparation

Native execution and format work belongs here, including supporting assembler,
disassembler, binutils, loader and linker changes needed to exercise it without
inliner2. Shared Level L algorithms and clone emission wait for phases 2 and 3.

### VM inline preparation worklist

| ID | Complete native capability and closure | State |
|---|---|---|
| **VMIP-01** | Retire SIGRET / signal RETURN; reserve opcode 502; reject source/stale binaries; remove root-SIGRETURN and active catalogue/handler contracts. Preserve SIGCALLBR. Complete INLENTER/LINKARGV/QGLOBBIND retirement and reserved identities without reviving INLENTER2. | done — reserved source/binary identities and active contracts retired |
| **VMIP-02** | Implement checked executable inline-region/continuation metadata and cold crossed-region cleanup. Copy selected handler first; unwind real children, restore its existing save marker, then retire crossed logical regions inside out before owner handler entry. Internal/resuming paths stay active. Include numeric, owned-reference and introduced-binding actions; no per-clone finalizer action or entry allocation. | done — typed format, native integration and handwritten boundary controls |
| **VMIP-03** | Make signal-instruction self-failures uncatchable current-VM-run errors with embedding cleanup. Invalid tag/target/origin, unmatched POP and allocation failure cannot silently continue, publish partial policy or raise catchable secondary signals. A valid SIGNAL's requested event remains catchable. | done — terminal self-errors, atomic allocation failures and reusable embedding |
| **VMIP-04** | Complete native support for the approved S1–S8 policy design, verify existing named save/marker semantics and real-child behavior. Shared named-save facts and inliner PUSH/POP emission are explicit later-phase dependencies, not additional VM scope operations. | native support/QA done; shared L facts and clone emission remain phases 2/3 |
| **VMIP-05** | Resolve ordinary return conformance against logical COPY-equivalent assignment and reference owner bookkeeping. Cover source/destination identity, linked/inactive attributes, borrowed sources, returned descriptors and required native-copy effects. Fix the shared runtime/catalogue surface; do not prescribe universal COPY or copy internal MOVE branches into inliner2. | done — COPY-equivalent runtime and surviving-owner reference controls |
| **VMIP-06** | Complete once-only native-payload retirement in owned local/a0/inactive attribute backing on every physical normal/unwind exit; exclude borrowed storage; preserve other dirty values. Reconcile native retirement inventory and audit first-party finalizers against the no-Rexx-reentry/no-catchable-signal convention. | done — physical normal/unwind/terminal retirement and provider controls |
| **VMIP-07** | Correct ENDLIFE's canonical synchronous-signal row to its nonallocating, non-callback, non-signalling contract. Reconcile result/retirement/provider contracts in the canonical inventory once with the native fixes. | done — non-signalling complete ENDLIFE and reconciled native inventory |
| **VMIP-08** | Reproduce suspected native-mode leakage after an unsaved real-child SIGIGNORE; repair at VM owner if confirmed, with an independent real-child regression. | done — parent native-mode recovery and original host disposition preserved |
| **VMIP-09** | Preserve the numeric provider reattachment repair across void/value returns, returning signal handlers and owner-directed unwind. Rebind even when numeric values match; reuse unaffected completed checks. | done — provider attachment and affected return/unwind controls |

VMIP-05/06 implementation uses COPY assignment for the ordinary register result,
then expires the retiring frame's owned reference backing and scans that same
owned local/a0 backing, including inactive attributes, for native finalizers.
Result copies that may carry reference state mark surviving ancestors for their
own later retirement; ordinary scalar returns need no ancestor walk. Genuine
MOVE remains storage relocation. Native-policy restoration after discarding a
child's private table belongs to the shared frame-retirement path.

The native callback inventory separates resource release from Rexx callbacks:
conforming finalizers, retained-capacity cleanup, result preclear and ABI-3
numeric synchronization add no callback edge. Their value/ownership/context
effects remain. Source COPY hooks, explicit host/provider/selection calls and
physical-root external argument copyback retain their real callback routes;
incomplete intrinsic inventories remain unknown.

VMIP-03 covers SIGIGNORE, SIGHALT, SIGSHALT, SIGBR, SIGBRV, SIGCALL, SIGCALLA,
SIGCALLBR, SIGPUSH, SIGPOP, handler-installing BPON, SIGNALORIGIN and invalid
use/internal failure of SIGNAL variants. Allocate/validate before publication.
Bare BPON/BPOFF pending-bit behavior remains distinct. Ordinary invalid numeric
settings and other program errors retain their own catchable semantics. A
terminal VM error is not a host-process abort.

The native catalogue records self-errors with
`RXOP_SIGNAL_PROP_STATIC_NAMES_TERMINAL` (bit 64): `static_names` identifies
uncatchable validation/allocation errors. Pure policy setters have normal and
terminal continuations; SIGNAL keeps its requested literal/register event's
ordinary catchable continuations separately from those terminal alternatives.
This extends the existing canonical row rather than adding a second database
or instruction evaluator.
Reserved opcode 502 uses the same reserved-effect/unknown-signal sentinel as
the retired inline opcodes: it grants no executable or analysis authority.

VMIP-02 uses RXBIN 007 feature 12 and three typed records: version-1 REGION
defines a module-local ID, parent and owning PROC; RANGE associates a half-open
instruction range with that region; EXIT associates an ordered numeric,
owned-base ENDLIFE or introduced UNLINK/SWAP/LINK action with a source-PC phase.
RXAS writes `.inlineregion id,parent`, `.inlinerange begin,end,id` and
`.inlineexit id,begin,end,order,"action",operands`. Fragmented regions are
allowed; each child range is contained in a parent range, and unrelated regions
cannot overlap. Numeric sources are checked literals or saved owned local base
registers; ENDLIFE addresses owned local bases. Binding operands name typed
r/g/a slots, with static local/global bounds and checked active argument bounds.
Action phases begin only after their saved values/binding deltas exist and end
when ordinary cleanup discharges them; they are not per-instruction state maps.
The codec validates identities, ownership, ancestry, phases, ordering and
instruction boundaries before publication. Relocation preserves executable
records through stripping and updates PROC/code/register references together;
native optimization conservatively protects affected storage/ranges where its
facts do not yet describe cold consumers. This is executable metadata, not an
inliner proof payload or normal-return interpreter. Diagnostic
inline maps alone grant no executable cleanup authority. Determine source
ownership from actual interrupted/dispatch PC; retained children need a checked
saved callsite because their return PC can equal a region endpoint. Destinations
are the active region or ancestors, never inactive siblings. Keep selection and
cleanup cold; add no normal instruction-loop condition, fake frame, signal-stack
mark, grouped POP, callbr/retinline or interpreted normal-return plan. A material
change to the approved semantics remains a user decision; specifying the approved
representation is an implementation task, not a new blanket permission gate.

Preserve and finish affected native integration for already-approved diagnostic
inline maps, .inlinedprivate/QGR1, exact QPR1 calls, typed program entry and
initializer readiness. Their contracts are in the architecture. Existing source
is not proof that their complete QA is done; include affected negative/relocation
controls rather than recommissioning a new design for each interface.

| Phase step | Gate |
|---|---|
| **VM-STEP01 — freeze coherent native contracts** | **done.** Review complete source inventory and independently expected handwritten fixtures against VMIP-01–09 and approved metadata. Set the concrete cold-exit representation and affected QA panel before implementation. |
| **VM-STEP02 — implement together** | **done.** Complete native changes and canonical rows; review early representative RXAS and ownership/control paths; batch material repairs. No tactical inliner exception. Depends on STEP01. |
| **VM-STEP03 — QA and commit** | **done.** Freeze inputs, run the panel below, repair/replay affected failures, update owning guides and commit only reviewed native/supporting paths. Depends on STEP02; phase 2 starts after this gate. |

**Phase 1 QA gate (done for the selected local component scope).** Normal Debug
uses rxbvm with a small rxtvm contrast; focused maintained Apple ASan covers
changed native ownership. Apple LSan is unavailable. This does not establish
other platforms, full initiative acceptance or release/SAN closure. The gate is:
Cover retired source/binary rejection; terminal self-errors despite installed
handlers and allocation-failure atomicity; named saves/markers and native-mode
recovery; handwritten nested region exits to different ancestors, retained-child
origin, internal/resuming handlers and malformed/stale/sibling metadata; numeric,
reference and introduced-binding cleanup; return COPY-equivalence and surviving
owner bookkeeping; real normal/unwind payload retirement, repeated reuse/teardown
and borrowed exclusions; terminal-run cleanup; affected codec, relocation,
roundtrip and catalogue controls. No emitted inliner2 is needed to qualify these
native mechanisms.
A small rxtvm contrast is justified for changed terminal/cold dispatch paths.
After focused normal checks, use focused maintained sanitizer checks for changed
native ownership; this is not a full instrumented functional matrix. Existing
actual SAN findings retain all repository closure requirements. Report exact
input/configuration/coverage and limits in conversation, not a run archive.

## Phase 2 — complete the shared Level L foundation

Phase 1 supplies the VM contract. Level L owns generic algorithms, typed RXAS
projection and all facts needed by the five duties/M1–M3, not an inliner-specific
solver. Existing generic capabilities remain valuable; review them against this
architecture and remove legacy restrictions rather than copy them forward.

| Shared capability | Required implementation and controls |
|---|---|
| Catalogue and machine CFG | Canonical typed operands/references and ordered effects, normal/table/call/return/signal/resume/unwind/terminal/unknown paths, exact owner validation, signal tags/actions/source-save ownership. Model terminal signal-instruction misuse separately from the requested catchable event; remove retired scope/SIGRET effects. |
| Identity and component facts | Distinguish register, active/base/entry/slot, storage and value identities. Track each published int/float/string/binary/decimal/reference/object/attribute/cursor component and individual flag, conversion dependencies, possible writes versus definite kills, aliases, ownership and path/exit-specific lifetime. VM-private caches are not extra semantics. |
| Context and procedure facts | Independent numeric read/write fields, known/literal versus saved GET/SET restoration, named policy stacks/correlation, owned-root cleanup and introduced-binding deltas. Direct/transitive summaries distinguish internal versus caller-visible effects, may/must/unknown, normal versus resumed/abrupt exits and actual normal-return existence. |
| Generic normalization | Shared early dirty-read clear/warning with exact component/source proof, safe path placement and atomic publication. No clear on mere uncertainty. Ordinary body cloning is independent of optional constant proofs; normalization errors are reported as such, not hidden opcode admission. |
| Remapping and assignment | Checked typed bank/slot/label/procedure/constant/module/global/source/metadata relocation. Preserve whole packed windows, lexical metadata address state, truthful contributors and inline ancestry. Typed assignment/reset validation respects pin/fixed/ordered-window/lifetime constraints; component liveness drives reuse. |
| Calls and bindings | Exact actual/result capture phase, count distinct from presence, independently rebindable repeated aliases, private a0, source by-value copies, finite scattered block construction and affine preservation of original dynamic windows. Unknown count is not an invented absent tail or automatic rejection. |
| Bottom-up and lifecycle | Exact dependency invalidation after rewriting; bounded monotone recursive summaries with explicit unknown/resource reason; owned immutable results, stale/foreign evidence rejection, cumulative work/bytes and atomic last-good publication. No repeated whole-caller charge for each small query. |

The generic facts are available to independent target clients before adoption
by inliner2. In particular, no old scopefree/directequivalence/bodyrelation,
source/candidate receipt or hidden C-helper theorem is a mandatory part of this
architecture. Audit any useful underlying generic algorithm independently.
Actual COPY hooks, native calls and physical observations retain their described
effects; cleanup-only conforming finalizers do not acquire Rexx callback edges.
Unknown facts reduce optional optimization or leave a specific semantic duty
unproved; they do not authorize absent effects or broad callback vetoes.

| Baseline criterion | Observable completion condition | State |
|---|---|---|
| **LLF-AC01 — target and flow contracts** | Documented catalogue, CFG, transfer, point/edge and signal policy APIs agree with independent constructed and ordinary RXAS controls, including all phase-1 changes. Invalid/stale/incomplete inputs yield explicit failure/unknown. | open |
| **LLF-AC02 — identity and summaries** | Links/swaps/remaps preserve storage/component identities and exact module owners. Direct/transitive summaries propagate bottom-up, terminate in recursive groups and preserve uncertainty. Work/byte refusal is atomic. | open |
| **LLF-AC03 — baseline qualification** | The complete selected one-VM target-foundation panel is green, independently covering positive/negative/state/resource cases. Owning API/consumer guides and exact limits are current, warning-free inputs reviewed, qualified foundation committed. | open |
| **LLF-AC04 — native payload retirement** | Phase 1 proves once-only normal/nonlocal physical retirement, inactive owned backing, borrowed exclusions and repeated reuse/teardown; phase 2 faithfully projects the resulting entry/resource/ownership contract without declaring current-execution payloads absent. Native implementation stays wholly in phase 1. | native phase qualified; L projection open |

| Baseline step | State and dependency |
|---|---|
| **LLF-STEP01 — complete audit** | todo: reconcile documented APIs, canonical inventory and every included foundation control with this architecture, separating algorithm/database defects, stale tests and legacy inliner policy; after VM-STEP03. |
| **LLF-STEP02 — coherent repair** | todo: repair the complete shared capability table, update projections and independent tests, review representative flow/remap/assignment output; after STEP01. |
| **LLF-STEP03 — QA and commit** | todo: prepare changed owners once, freeze and run the complete selected foundation panel, repair/replay affected failures, publish current guides/limits and commit qualified L paths; after STEP02. Phase 3 starts only after this gate. |

**Phase 2 QA gate (todo).** Use one normal configuration/VM and independent
constructed plus ordinary RXAS consumers. Cover normal and signal CFGs; separate
subvalues/flags and conversion validity; links/swaps/repeated aliases and active
versus base cleanup; known/unknown numeric and signal-save state; logical returns,
references/finalizer ownership; exact globals/entry/initializers; lexical/source
metadata; finite/dynamic windows and constrained assignment; dirty-read warning/
clear without borrowed corruption; recursive/transitive invalidation; stale,
foreign, repeated/interleaved and exhausted requests. Include the updated numeric
site-facets controls not yet runtime-qualified. Retain useful structural assertions
and one independent non-inliner client. The guide must map APIs to maintained
controls and state remaining unknown boundaries before “Level L ready” is claimed.

Shared lexer/parser preparation is part of the foundation check, not a new
performance initiative. Preserve exact DFA/order/UTF-8/source behavior and public
input ownership. Generic generation stays serial by default; the maintained
RXAS build uses four workers with bounded packets and serial/threaded exact-image
parity. Preserve fingerprints, unique locked staging, success stamps, unchanged-
output avoidance and missing-byproduct recovery. Reuse unchanged valid generated
inputs; no unnecessary rebuild or new production speed claim. Enduring details:
[RXAS APIs](../../../langtools/doc/rxas-design.md),
[IR algorithms](../../../langtools/doc/ir-design.md),
[generation](../../../langtools/doc/generation-design.md).

## Selected standalone inliner outcome

### Phase 3 — a fresh inliner2

Start a new service and explicit consumer path after LLF-STEP03. The first
profile is one exact private local CALL1 with one integer actual and one integer
result. The callee computes through a loop and returns normally; the selected
fixture changes no signal/numeric context and has no extra reference/payload
exit duty. It is a first useful proof, not the full generic-profile verdict.

Mechanically capture/map the published argument/result ABI, assign fresh locals
and labels, clone ordinary typed instructions, lower return to result assignment
and continuation, relocate typed source/inline diagnostics, and remove dead
private helper code and unneeded records. The final caller contains the loop and
result calculation, without CALL, LINK, SWAP, scope/frame or call-setup overhead.
The service must not wait for an optional optimizer to hide construction defects.

| Inliner2 criterion | Observable completion condition | State |
|---|---|---|
| **IN2-AC01 — direct scalar loop** | Standalone RXAS read/rewrite/write expands one integer-argument/result loop and executes independently expected zero, one and several iterations in one VM. | open |
| **IN2-AC02 — emitted shape and metadata** | Complete RXAS shows the loop in the caller and no call scaffolding; locals/labels, source and typed diagnostic frames refer to the clone; dead private helper/unneeded records are removed. | open |
| **IN2-AC03 — bounded refusal and delivery** | Nonmatching calls remain with exact reasons; malformed/resource/I/O failure publishes no partial result and preserves last-good output. Source/RXAS review, small normal integration panel, docs and local commit complete. | open |

| Step | State and dependency |
|---|---|
| **IN2-STEP01 — fixture and reuse review** | todo: freeze independent CALL1/RET loop oracle, expected RXAS and explicit reused API list. Check phase-1/2 qualified inputs; depends on LLF-STEP03. |
| **IN2-STEP02 — new service** | todo: implement generic typed clone/remap and return lowering with a new entry path, no legacy policy import; inspect first representative output early. Depends on STEP01. |
| **IN2-STEP03 — QA and commit** | todo: run focused reader/writer→assemble/link/VM controls and structural assertions; fix shared defects at their owner, freeze, update guide and commit. Depends on STEP02. |

**Reuse boundary.** File/package reuse never approves all of its policy.

| Eligible after source review | Excluded from automatic reuse |
|---|---|
| llir/llirgraph, source, budgets, diagnostics, atomic file output | New private solver/arena/transaction framework |
| lltarget owned typed records and module/body/source identity | lltargetrecipe fake-scope and old INL1 recipes |
| RXAS grammar, llrxasinput/output reader/renderer | Retired recipe syntax and unverified profile assumptions |
| lltargetremap ordinary typed register/reference/metadata relocation | Correspondence/identity receipts as inherited clone-permission machinery |
| Qualified shared CFG/flow/transfer/summary/assignment services from phase 2 | scopefree/directequivalence/bodyrelation, callback/dirty-recycle admission theorems |
| Native exact provider selection and QGR1/QPR1 codec/rebase primitives | Wholesale old rxlink_inline_service/stage/proof handshake |
| Harnesses and independent semantic/structural oracles | Old slice-specific expected rejections as normative behavior |

llinline, lltargetfacts, lltargetrecipe, old receipt-driven CLI/stage and llcleanup
imports are not whole-file reuse approvals. Extract an obvious generic utility
only with its exact responsibility and tests; do not copy old admission logic
and rename it. Later generic cleanup can reuse independently reviewed techniques.

**Phase 3 QA gate (todo).** Author independent expected loop results before
comparison, execute original and expanded inputs, inspect complete RXAS and
assert absent scaffolding/dead helper plus truthful metadata. Check retained
nonmatching call, disabled mode, exact rejection explanation and atomic malformed/
resource/output failure. Correct expansion is tested with optional cleanup and
assembler optimization disabled. Reuse prepared tools; this source-only phase
does not itself require a native sanitizer rebuild or broad compiler matrix.

### Wider standalone outcome retained open

The former inliner1 implementation/capability steps are retired. These stable
outcome IDs preserve the wider requested capability without treating old code,
old per-shape guards or partial tests as completion. They are **all open** and
will be served by reviewed growth of inliner2 after the initial loop proof.

| ID | Wider acceptance condition |
|---|---|
| **INL-AC01 — shared service and adapters** | One callable B/L target inliner serves RXAS CLI, assembler-local, exact-selected linker and an independent handwritten/DSL producer without AST payloads, copied frameworks or native source compiler fallback. |
| **INL-AC02 — exact call/storage mapping** | Direct/counted/fixed/fused forms preserve captures, counts/presence/defaults, active/base/entry/slot identity, aliases/status/convention copies, child windows and logically COPY-equivalent returns, including normal and abrupt exits. |
| **INL-AC03 — full representable profile** | Shared inventory/facts support rebinding, varargs, multiple returns, reference/aggregate lifetimes, context/cleanup and exact-bound imports; optional value precision is not structural admission. All unimplemented representable categories keep this open; each retained call has a causal reason. |
| **INL-AC04 — module and observation integrity** | Private/public globals retain module ownership without exposure/proof metadata; entry/readiness/poison/cycle, five duties/M1–M3, logical source/TRACE/diagnostics and actual physical observations meet the approved contract and SHRUGs. |
| **INL-AC05 — bounded useful cleanup** | Expansion is correct with cleanup off. Generic flag/presence/constants/branch/dead-write rules, alias-aware copy/transport removal and constrained allocation remove redundant caller setup/reverse swaps while retaining required body/child swaps and copies. Reanalyse already-optimized changed callers. |
| **INL-AC06 — deterministic policy/resources** | Separate availability, legality and profitability; explicit recursion/depth/code/register/metadata/work limits, force-legal/disabled controls and causal diagnostics terminate predictably with ordinary calls on refusal and no partial publication. |
| **INL-AC07 — metadata and lifecycle** | Relocate executable/observation references together; preserve source/module/provider/task/runtime graphs and initializers; prune only under reachability/consumer rules. Repeated/interleaved/invalid/resource/cancellation/I/O requests preserve owners and last-good output. |
| **INL-AC08 — reviewed research delivery** | Independent source/emitted-code review and a compact whole-outcome integration panel cover expanded/retained cases, generated/handwritten consumers, both placements and cleanup off/on. Current guides/limits; no implied production-speed, broader platform, release or publication claim. |
| **INL-AC09 — shared target foundation** | Reusable checked transfer/CFG/remap, component/flag identity/liveness/lifetime, clear/warning and direct/transitive facts serve independent clients with explicit unknown/stale/resource outcomes; database completeness is independently checked. |

Future growth must be coherent by capability and common normalization, not a
new per-opcode or fixture-by-fixture admission ladder. Assess actual G/C and
handwritten use of difficult signal shapes before adding special complexity;
G protected blocks are required and C wrapper/relay layering must be understood.
Known finite register shortages are repaired by fresh layout, not refusal. A
genuinely unrepresentable unbounded scattered window or physical observer keeps
its call with a precise reason. Dynamic/unavailable code is not made inlineable
by a metadata assertion.

Post-link cleanup is a bounded shared panel: rebuild correctness facts and
references; propagate known actual/presence/flag facts; remove proven branches
and dead status writes; remove redundant copies and whole-region caller setup/
reverse swaps; recompute component liveness, constrained assignment, unreachable
code/metadata and reseal references. Optional failure preserves the validated
expansion. In-module expansion precedes ordinary assembler optimization.

## Preserved interfaces and deferred work

The [architecture](doc/inlining.md#typed-metadata-and-entry-contracts) retains
exact approved diagnostic inline frames/maps (feature 9), borrowed private-global
suffix declarations (feature 10), typed program-default entry (feature 11),
initializer default context/readiness and the decimal ABI-3 synchronization
contract. Preserve them through parsing, stripping, relinking and load validation.
Their existence does not close unverified integration criteria.

The optional false-positive frame “may have finalizers” scan-avoidance flag
remains deferred as VM-FINALIZER-SCAN-01 in performance/ROADMAP.md. It is not a
prerequisite for these phases. No new threading project, production optimization
verdict, native B/C frontend replacement, full new assembler/linker implementation
or publication is implicitly commissioned by this plan cleanup.

## Future complete Level L target-toolchain profile

RXAS means the full current UTF-8 source-visible language: declarations,
imports/exports, register banks, labels/fixups, all current instructions and
operand widths, constants/aliases, jump tables, metadata and source/TRACE
directives. Runtime-only opcodes are not admitted as source instructions.
Nonoptimizing assembly resolves required symbols and constructs valid graph
data, but performs no peepholes, instruction fusion, register reuse or inlining.
External legacy text encodings and future syntax are separate scope decisions.

RXBIN means current 007, not a new format: the fixed-width little-endian
container and all six sections (modules, instructions, constants, metadata,
graph facts and graph indexes), standalone/multimodule images and concatenated
container libraries. Read raw and current LZSS-compressed sections. The first
writer may emit valid raw sections; identical compressed bytes are unnecessary.
Decode to the owned model, validate, then encode; copying the input file cannot
satisfy reassembly. Header/feature/schema/alignment and reserved-field rules,
packed instruction integers, signed extrema and unsigned sentinels stay exact.

All current record kinds in the canonical RXBIN inventory are included,
including inline diagnostic frames/maps, private-global bindings and entry-state
records. The profile follows the canonical inventory rather than a historical
fixed record count. Preserve unused records, ordered chains, raw literal
bytes and IEEE float bits, including signed zero/NaN payloads. Keep known inline
payloads opaque when not interpreting them. Preserve origins, procedure and
metadata addresses, module globals, initializer order, required/optional
providers, autoload hints, descriptions/RexxDoc and sealed task bindings with
their use-site records.

Semantic roundtrip preserves every supported payload and reference relationship,
not incidental pool IDs, packing, compression or serialized index ordering.
One relocation owner maps logical instructions, pool/module/graph IDs and
expanded code-cell addresses; it regenerates affected indexes and task seals.
Unknown structural record/section kinds, versions, schemas, unsupported feature
bits, invalid references and malformed or over-budget inputs fail explicitly.
An opaque known payload is distinct from an unknown layout that cannot be
safely relocated; never discard the latter or claim future-format roundtrip.

The linker implements current selection/control semantics: OMIT, INCLUDE, ROOT,
main/first-input defaults, import closure, conservative dynamic-member and
interface-factory retention, selector forms, duplicate/signature diagnostics,
maps, provider requirements and documented source/inline stripping controls.
Unresolved external/provider imports follow current link/runtime policy; required
closure failures remain errors rather than a blanket ban on external imports.
Preserve module identity/global counts, exposed alias links, stable module order
and module-instance initializer dependency/cycle/poison semantics; initializers
are neither exports nor selection roots. Merge compatible leaf
constants, relocate structured records and graph-bearing operands, refine only
unresolved import signature fields, and retain strict known type/flag contracts.
Rebuild the image graph, declaration origins, interfaces, dispatch/factory/
provider indexes and task-binding digests/placeholders. Autoload hints do not
discover link inputs. Reuse the committed mandatory-core provider policy and
generated canonical declarations, with checking implemented in B/L: retain
all provider/map metadata and omit only validated core bindings from external
archive requirements. The baseline linker mode changes references/encoding
where necessary and does not otherwise rewrite instruction sequences. Its
selected module/body set is also the input to the optional cross-module
inliner and bounded cleanup before final relocation/sealing; this uses the
same service as assembler-local inlining, not a second linker algorithm.

Naive expansion is one language-neutral target service, independent of source
compiler AST inline transport. Begin with proven direct local scalar calls;
then extend the reviewed profile to defaults/presence, nested calls, multiple
returns, imported helpers and a handwritten DSL using the same service.
Conservatively retain ordinary calls where equivalence is unproved, while
repairing representable supported shapes rather than broadly disabling them.
Each supported expansion preserves the approved VM surface and the five exit
duties/M1–M3 in the [inlining architecture](doc/inlining.md). Active, base,
entry and slot identities survive LINK/UNLINK/SWAP and nested call windows.
Register RETURN is logically COPY-equivalent; internal data moves are not an
additional contract. Exact asynchronous snapshots and incidental finalizer
timing use the approved SHRUGs. Required initialization, reference lifetime,
synchronous control and ownership remain obligations. Diagnose missing proof
precisely, repair representable supported shapes at their owner, and establish
correct expansion with optional optimization disabled and fresh registers.

## Stable acceptance criteria

Every criterion is **open**; passing an early subset cannot close the profile.

1. **TGT-AC01 — owned model and schema.** One checked, bounded module/image
   model represents the entire profile, typed reference domains and provenance.
   Canonical catalog/schema alignment and independent constructed-model tests
   prove operand identities, graph contracts and rejection of stale references.
   Reusable procedure summaries and point proofs expose explicit unknowns and
   reject stale body, catalog or exact-binding revisions.
2. **TGT-AC02 — L grammar and diagnostics.** Full current UTF-8 RXAS parses
   through L with correct byte/source spans, literal bytes, precedence and EOF.
   Independent positive/negative fixtures cover directives and grammar errors;
   diagnostics distinguish syntax, binding, format and resource failures.
3. **TGT-AC03 — lossless semantic codec.** All six sections/current canonical record kinds,
   raw/LZSS reads, libraries and linked images survive decode/encode/redecode.
   Independent fixed-width/record/graph oracles inspect complete meaning,
   extrema, float bits and opaque payloads; native interoperability adds proof.
4. **TGT-AC04 — complete nonoptimizing assembler.** Every profile construct
   assembles with exact symbols, literals, register banks, fixups, tables and
   metadata. Independently expected VM results and structural instruction/
   graph assertions pass; native differences are diagnosed, not copied blindly.
5. **TGT-AC05 — complete module linker.** Selection, signatures, globals,
   initializers, providers, interfaces, stripping, graph/task resealing and all
   relocation domains pass independent multi-input and negative fixtures.
   With inlining disabled, instruction sequences remain unchanged apart from
   required reference remaps; enabled inlining uses the new IN2 service and is qualified against the
   wider INL-AC02–07 outcome and TGT-AC06/07, beyond the initial IN2 loop proof.
6. **TGT-AC06 — shared naive expansion.** The staged profile above uses one
   service for generated and handwritten callers. Structural expansion/call-
   retention assertions plus once-only, alias/state, omission, nested and
   multiple-return controls prove behavior and each admitted ABI obligation.
7. **TGT-AC07 — observable execution and provenance.** Output/control/state,
   TRACE/diagnostic source, condition/exception/context, module initialization
   and ownership/lifetime observations survive assembly, reassembly, link and
   admitted expansion. Physical-frame-sensitive cases retain calls unless proved.
8. **TGT-AC08 — resources and atomic lifecycle.** Checked input/output/work,
   decompression, graph/record and expansion limits fail without partial results.
   Malformed/truncated/unknown data, repeated small-after-large and interleaved
   owned requests, I/O failure and cancellation preserve last-good publication
   and clean temporary state; rejected input cannot leak partial model mutation.
9. **TGT-AC09 — independent consumers.** A non-C/G grammar client, constructed
   record client and real record-transform client exercise generic L services;
   a handwritten DSL emits, links and expands through the same target services.
   A representative compiler/library client supplements these independent tests.
10. **TGT-AC10 — bootstrap and delivery.** Explicit seed/producer dependencies,
    optional build/install closure and relocated CLI plus B service consumers
    work on the agreed platform scope. Runtime requests contain no forbidden
    native backend. Product defaults and core L packages gain no research edge.
11. **TGT-AC11 — final-stage L validation/debugging.** Exercise grammar/source/
    diagnostics, owned records/modules, transformation, validation and atomic
    output together, including failed transformations and independent consumers.
    Repair exposed defects at the generic L or target owner and retain focused
    regressions; document remaining profile limits without universal-L claims.
12. **TGT-AC12 — reviewed and qualified delivery.** Detailed scope/API/schema
    approval precedes implementation; independent expected behavior and early
    coherent source/RXAS/model review precede freeze. Selected affected checks,
    owning guides, limits and open criteria are current; scratch is deleted.
    Local increments, final initiative QA and any publication remain distinct.

## Stable implementation steps and decision gates

All TGT steps are **todo**. This is preserved future scope, separate from the
approved VM → Level L → initial inliner2 delivery below. Detailed full-toolchain
profile, packaging/platform scope and implementation approval remain open.

1. **TGT-STEP01 — profile and contract approval (AC01–12).** Freeze current
   grammar/wire/ABI crosswalk, ownership, service APIs, CLI names, bounded limits,
   platform/package scope and independent oracles. Resolve expansion/optimizer
   boundary without implementing an optimizer; obtain detailed user approval.
2. **TGT-STEP02 — owned model and primitives (AC01/08/09/11/12; after 01).**
   Reuse L-owned structures/validation, factor checked binary/schema primitives
   with existing owners and expose constructed-model/transform clients first.
3. **TGT-STEP03 — complete codec (AC01/03/07–12; after 02).** Implement raw/
   LZSS decoding, full validation and raw writing; qualify semantic no-op and
   a real record transform, malformed inputs, graph/task integrity and atomicity.
4. **TGT-STEP04 — complete L RXAS assembler (AC01–04/07–12; after 03).**
   Generate the grammar, bind every profile construct and emit through the same
   model/codec. Inspect first representative instructions early; complete the
   source/binding/error panel before claiming nonoptimizing-assembler coverage.
5. **TGT-STEP05 — module linker (AC01/03/05/07–12; after 04).** Implement
   selection, shared pools and relocation/graph/task closure without instruction
   optimization. Qualify independent linked-module lifecycle and failure cases.
6. **TGT-STEP06 — integrated scalar expansion (AC01/06–09/11/12; after 05).**
   Integrate the new shared inliner2 service developed through IN2-STEP01–03;
   do not revive inliner1 or implement another algorithm. Prove local direct scalar cases and unchanged
   unsafe calls through the wider toolchain. Standalone development need not
   wait for TGT-STEP05.
7. **TGT-STEP07 — reviewed expansion growth (AC06–12; after 06).** Add defaults/
   presence, nested/multiple-return and imported-helper cases plus the handwritten
   DSL only with each ownership/frame/context obligation proved. Preserve open
   unsupported shapes explicitly; do not label the broader profile complete.
8. **TGT-STEP08 — final agreed integration (AC01–12; after 03–07).** Review the
   complete services and representative emitted code, qualify independent and
   compiler/library consumers plus agreed installed/relocated/platform checks,
   resolve final-stage L findings at their owners and update durable guides.
   Select broader QA for actual integration risks; keep replacement/performance
   decisions and unrelated develop publication outside this milestone.

GPT-6 Sol Extra High coordinates work and owns centrally scheduled build/test
execution and diagnostics. GPT-6.1 Sol Extra High workers own design,
implementation and source review with disjoint service/test owners, handing
execution to that coordinator. Development normally uses one normal configuration
and one VM, independent expectations and early coherent record/RXAS review; batch
material repairs and reuse unchanged prepared inputs. At STEP08 qualify both
concrete VMs and the declared delivery scope. Source-only B/L work does not
trigger per-increment sanitizer matrices; actual native changes/findings retain
repository rules. Do not retain run reports, logs or timing archives in Git.
