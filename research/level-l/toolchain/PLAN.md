# Level L target-toolchain research

**Status: shared Level L foundation committed; approved one-sweep inliner POC in progress.**
Adrian has requested a standalone, effective RXAS inliner proof of concept and,
approved this plan for implementation. The [inliner contract](doc/inlining.md)
and INL criteria/steps below own that selected outcome within this single plan.
The wider assembler/reassembler/linker proposal remains proposed; all TGT
acceptance criteria remain open. Implementation and reviewed local commits for
the INL outcome are authorized; native replacement and publication are not.

The remaining INL implementation is one coherent database-driven expansion
and shared flow/cleanup increment, followed by focused qualification. In-module
calls are expanded during assembly before optimization; cross-module calls
are expanded during linking after exact provider selection, followed by bounded
cleanup. The standalone POC supplies both contexts to one service. Native or
dynamically loaded bodies without exact link-time selection remain calls.
Cross-module expansion also needs a checked runtime binding that cannot silently
resolve to a different provider through the context-wide export table.
Link-time private globals remain an in-scope ownership outcome without public
exposure or inliner proof metadata. The concrete borrowed-slot RXAS/RXBIN
binding for direct expansion is [approved](#approved-borrowed-register-private-global-binding-inl-ac04);
implementation and focused qualification remain open.
The current standalone CLI work default can exhaust before the final theorem
on a two-arm direct clone. A larger explicit allowance previously reached the
prior-a0 finalizer refusal without publishing a clone; that incidental-timing
gate is now removed. The same high-allowance endpoint next retains a possible
body callback with unproved physical-activation observation; the clone is
still unpublished. Reassess the default only after this independent semantic
issue is resolved; budget size grants no proof permission.

**Approved effective-inlining direction (31 October 2026; INL-STEP04/05).**
The normal inliner emits a direct remapped body with bounded ordinary
instructions for any required entry, return and context restoration, or
retains the original CALL with an exact reason. Failure of a scope-free proof
must not automatically select `INLENTER`: that existing operation allocates a
scope on every execution, clears owned backing and saves/restores broad state,
while an ordinary CALL usually reuses a frame. Keep its RXBIN/VM contract and
handwritten compatibility checks for now; it is not the default effective-
inlining path. A separately requested correctness experiment may exercise it,
without an effectiveness claim. Defer the proposed INL2 opcode/recipe/feature;
no INL2 product change is authorized by this direction.

**Approved static inline diagnostic metadata (INL-STEP04/05).** Direct
expansion carries a typed static inline-frame tree and physical code-range
map, with no entry instruction or per-call allocation. RXAS declares
`.inlineframe id parent "module" "procedure" callsite-step` and
`.inlinemap begin-label,end-label,id`. Nonzero frame IDs are module-local;
parent zero denotes the containing physical procedure. Names are diagnostic
strings only and grant no executable binding or proof authority. A nonzero
callsite step names an unambiguous source anchor in the output module;
repeated records with the same ID and identical source payload are allowed. Maps cover
disjoint half-open instruction-cell ranges and select the deepest frame;
several disjoint ranges may share one frame. Append `META_INLINE_FRAME` and
`META_INLINE_MAP` under `RXBIN007_FEATURE_INLINE_DIAGNOSTICS` bit 9. Validate
names, unique IDs, parent existence/acyclicity, source references, range bounds
and instruction boundaries. A frame definition sits at the removed call's
physical clone-entry anchor; definitions/maps/ancestors share one containing
physical procedure, and maps cannot cross procedure boundaries. Preserve the tree/maps when source or compiler
inline metadata is stripped; source stripping sets callsite-step to zero.
Assembler optimization and linking preserve/remap the address and source
identities. Level L cleanup drops an individual map only when its complete
instruction range becomes empty; frame definitions, ancestor links and endpoint
labels remain available, while retained or replacement instructions keep their
original attribution. Empty-range pruning uses the target model's checked
sorted boundaries and participates in the same work/byte limits. The existing VM diagnostic walker expands the static chain only
when capturing/printing diagnostics, using actual fault/resume and retained
child call-site addresses. Physical register/frame observers and restoration
remain independent semantic obligations. No new opcode or proof side channel
is introduced. Focused codec/parser/roundtrip/link/diagnostic controls and
corrupt/overlap/cycle/feature-negative cases qualify this contract.

Level L owns a per-site semantic certificate and bounded restoration plan over
canonical database effects, point facts and every normal, signal, cancellation
and unwind exit. It must distinguish no context change, a proved known-value
restore, a dynamic save/restore using existing GET/SET operations, an unproved
exit, and an actual nonrepresentable observation. Numeric fields are tracked
individually; a callee's write alone does not force a runtime scope. Unknown
caller DIGITS is not inherently impossible to restore. Signal policy, plugin
sync, callbacks, finalizer order, reference lifetime and physical-frame/code
observation remain explicit obligations. The inliner consumes the certificate
and emits only licensed operations; an incomplete opcode/reference mapping is
a normalization error, not an opcode admission rule. Availability, semantic
proof, resource limits and profitability have distinct reasons. The initial
default profitability policy admits no *introduced per-call heap allocation*;
allocations already in the copied body are irrelevant to that rule. A missing
initializer readiness/cycle/failure proof retains CALL until an approved cheap
equivalent ingress is available.

### Asynchronous signal delivery during inlining (1 November 2026)

An asynchronous signal handler must not depend on delivery occurring before or
after a call's numeric settings, signal policy, register bindings or other
program state are set or restored. It may see work in progress at any ordinary
instruction boundary. Exact CALL/RET pending-signal timing is therefore not an
inlining equivalence requirement. Synchronous fault behavior, internally valid
VM state, pending-event preservation and required cleanup on normal and
nonlocal exits remain requirements. If a short VM-internal transition genuinely
needs deferral, the taken cold interrupt route may park the event until that
transition ends; ordinary dispatch must not gain a per-instruction check.

The previously approved `callbr`/`retinline` and typed return-plan design is
withdrawn from implementation and its partial code was rolled back. It
recreated much of the call protocol solely to enforce exact asynchronous
checkpoints, without an established performance benefit. The direct inliner
must be assessed against ordinary CALL cost using the existing RXAS/VM
contracts; any new ISA/API proposal requires a separate demonstrated need and
approval. This rollback does not waive result, ownership, context, signal-flow,
source/TRACE or cleanup proofs.

The proposed `intdefer`/`intresume` VM opcode pair is outside this POC. Adrian
stopped that addition when review showed that asynchronous requests and
synchronous faults share the pending word, so a correct cold-path deferral also
needs provenance, nested-call/unwind ownership and race handling. The current
signal-handler timing contract stands without new interrupt-control opcodes.

Retained-call diagnostics preserve every per-site reason and the exact
`LL_INLINE_KEEP: <reason>` prefix. The CLI appends its display category/proof
layer, exact frozen input-owner/call ID, caller module/procedure, typed target
selector and RXAS origin's zero-based half-open byte span. Module-wide
normalization refusal is explicitly module-scoped. Escape and bound displayed
context strings without capping the number of rejected sites or shortening
the authoritative reason. This is diagnostic presentation only: no new
analysis permission, proof metadata or semantic classification follows from
the display labels. A focused CLI fixture checks early availability/selection/
SCC and ordinary proof refusals, complete site reporting and escaped names;
source review checks the pre-escape display bound and absence of a site cap.
Source-only changes await the coordinator's narrow CLI qualification.

Process exact selected call-graph SCCs from leaves towards callers, with
bounded recursive SCC policy. Rebuild affected Level L facts and exact owner
bindings after each changed body; an outer site must clone the newly selected
inner result rather than a stale original. Fixed CALL1–CALL4 have explicit
actual pointers and no contiguous-window placement veto, though nested
physical-frame observers still need proof. A finite counted child can use a
fresh contiguous count/argument pointer window with alias-preserving links and
proved normal/abrupt restoration, subject to frame/code/resource limits. An
unknown-length scattered tail without a finite bound is not guaranteed a
static register block. Focused QA must cover direct versus retained output,
numeric known/unknown restores and exceptional exits, initializer ingress,
fixed and counted nested calls, bottom-up A→B→C, and cost-policy controls.
Assembler-local inlining runs first; the linker consumes those already
transformed modules, so local child calls do not need a second link-stage wave.
Link-stage bottom-up is needed only for a selected cross-module chain that is
itself expanded at link time. Current foreign-provider direct permission is
unproved, and native selection capabilities are bound to frozen owner epochs;
fresh selected lineage requires an owner-bound one-to-many clone provenance
and trusted re-enrollment before link-stage waves can safely run. Keep those
calls while that shared interface remains open; it does not gate local direct
inlining or authorize a proof side channel in RXBIN metadata.
The approved architecture review is source-only; implementation and checks
remain open under INL-STEP04/05 and INL-STEP06.

**CALL cause projection and direct diagnostic certificate (INL-STEP04/05).**
Project the canonical typed CALL cause rows independently at each exact
owner-bound invocation. A checked same-module bytecode target discharges only
target resolution, target kind and initializer ingress; a finite validated
layout discharges only its own layout/window condition. Keep allocation
FAILURE as a named producer and retain the aggregate wildcard whenever any
provider, frame hook or preparation cause is unproved. In particular native
`frame_f` calls the inherited decimal provider's `syncNumericContext` during
entry. Resource-clean backing, local target selection and the existing
default-HALT/static-exact profiles do not prove this callback absent or
equivalent. Direct removal needs an exact provider contract or an explicitly
approved ABI/semantic decision; no such permission is inferred here. Genuine
host/finalizer/reentry/policy and initializer paths remain conservative.

For explicitly selected closed-call profiles, prove structural handler-table
preservation from the exact local provider body's complete canonical inventory
and bounded same-owner call closure. This policy-only property does not require
every caller operation or unrelated later host callback to satisfy whole-program
call membership. Keep the separately checked root's declared inherited
default-HALT premise when descendant membership is incomplete; do not confer
that premise on an unproved descendant. This legacy explicit profile remains
distinct from the VM's catalogued mixed program-entry defaults. The CFG still
propagates earlier reachable caller callbacks/policy changes to each site and
retains every resource-hook, ambient/event and exceptional alternative. A host
callback, policy/pending write, unknown effect or unbound provider inside the
provider closure refuses the policy proof. No hook absence, resource cleanliness,
initializer readiness or inliner permission follows from this property.

**Reviewed normalization and clone admission (INL-STEP04/05; implementation todo).**
Absolute zero-demand normalization is not a universal caller or provider
precondition for structural cloning. Keep `cfg.normalizationcomplete(function)`,
ordinary stamps, `flow.entrycoverageat(callid)` and
`lltargetnormalize.sitecertificate` with their existing complete, zero-dirty
meaning; do not relabel unknown availability as defined. A separate exact
no-clear identity receipt proves that normalization changed no operation,
without claiming universal entry cleanliness or callback absence. Actual
inserted clears still require the source dirty witness, safe placement,
candidate entry coverage, callback preservation and atomic warning/map/result
transaction. A proved remaining owned entry read is a normalization defect;
unknown contents alone neither license a clear nor veto an unchanged clone.

Replace the universal admission gates with one final-candidate source-to-clone
relational theorem at the shared target owner. Canonical typed effects and
references must be complete. Checked one-to-many operation, storage and logical
activation correspondence binds the exact provider, captured result/actual
pointers, aliases, count/presence, preparation/capture phases, globals and every
departure. Ordinary remapped actions retain their database semantics and may
operate on corresponding symbolic unknown values. CALL/RET rewrites separately
account for typed causes, initializer ingress, synchronization, result ownership,
signals, inherited SIGRET, cancellation and normal/abrupt restoration. No
mnemonic admission list or second opcode evaluator is introduced.

Each owned provider local starts logically dirty on every inline activation;
fresh numbering proves neither empty payload nor recycled-resource absence.
Borrowed arguments retain their captured storage's availability. Preserve the
source normalizer's licensed clears and warnings, and prove no new owned dirty
observation. Match callback possibility, order, phase, reachable storage and
permitted observations under the activation map, retaining value/binding havoc.
Unknown physical-frame/code observations and changed resource/finalizer effects
need their own equivalence proof. Verify the provider clone and retained caller
together, including added private cells, child windows and diagnostics. A
callback may copy dirty state: merging old state with an arbitrarily defined
foreign value is not a substitute for this relation.

The current `directplan` remains a draft/restoration plan and the existing
`directequivalence` theorem remains bounded to its zero-argument, ignored-result,
zero-local/no-state-delta domain until this replacement is implemented and
qualified. An identity normalization receipt alone grants no activation, target,
ABI or observer permission. Do not bypass these gates individually to admit a
fixture. Bind every receipt/theorem to frozen source/result, profile, CFG,
catalogue and provider epochs; invalidate it after rewriting or owner replacement.

A complete ingress cone remains necessary when consuming point values for
specialization, clear placement or restoration: include normal, handler/resumed,
asynchronous and backward paths. Unknown later destinations are not absent
edges, and changed-entry taint needs a complete kill or exact preservation
across a frontier. A parametric clone theorem need not prove unchanged borrowed
values defined, but must cover every applicable ingress and departure.

Focused controls include `io-branch` direct output with cleanup disabled and
both branch arms, slot swap/mutation and whole return (output 9, 11, 99);
symbolically unknown borrowed input; repeated/path-sensitive dirty-local
warning/clear; matched callback havoc; and hostile callback copying dirty state,
changed callback phase, physical-parent/code observation, wrong alias/presence,
initializer failure and abrupt exits. Keep stale/cross-owner correspondence,
incomplete reference inventory and low-budget atomic-refusal negatives. The
existing absolute site/ingress controls retain their stronger theorem. CLI
no-call acceptance, INL-STEP06 and INL-AC01–09 remain open until qualification.

**Relational replacement implementation API (INL-STEP04/05).**
`lltargetsummary.analysis.expansionplan(callid)` returns a frozen
owner/profile/CFG-bound `expansionplanfact`: `owner`, `operation`, `valid`,
`status`, `reason`, `draftallowed`, `layout`, `target`, `invocation`,
`restoration` and `returnmode`. `allowed`/`semanticallowed` remain false: the
plan permits unpublished construction after exact target/capture/layout,
reference and lowering inventory. It requires neither defined actual payloads
nor absolute zero-demand normalization or absence of ordinary copied callbacks.

The cycle-free remap locator is
`expansioninstance(callid,providerRef,procedureid,frameid,sourceops,candidateops,counts,continuationLabel)`.
`bind(original,replacement,role)` records original private, argument, proxy,
global and count slots; `added(candidate,role)` records save, return-temp,
base-proxy and window cells. `bindbase(original,replacement)` / `base(original)`
record the separate original base coordinate for proxy-bound formals/globals.
Getters are `callid`, `provider`, `procedure`,
`frame`, `rows`, `source(i)`, `candidate(i)`, `count(i)`, `continuation`,
`mappings`, `original(i)`, `replacement(i)`, `mappingrole(i)`, `additions`,
`addition(i)` and `additionrole(i)`. Source operation zero locates generated
entry/preparation rows. Each lowered RET row includes transfer, cleanup/context
restoration and its continuation branch. Locators are untrusted claims until
checked against the original and final models.

`bindrecord(sourceRecord,candidateRecord)` separately locates copied labels and
runtime metadata; source zero means a generated metadata-scope close.
`metadata(scope,steps,clauses,frames)` records their existing canonical rename
maps. The structural checker verifies payload/register renaming, fresh identity
separation, scope closure and exact next-instruction/continuation boundaries;
static frame/source diagnostics keep their final dedicated checker as well.
The structural owner enforces source-bank roles, fresh caller-r suffix bounds
and injective private/count/proxy/base/temp coordinates across the whole batch.
Each proxy base has exactly one fresh base-proxy addition. Explicit direct
argument coordinates alone may share captured caller slots; the final theorem
must still prove each argument capture and alias relation. Internal copied
label operands must select their corresponding renamed source label, not merely
some valid destination in the same procedure.

`lltargetremap.expansioncorrespondence(sourceRef,candidateRef,callerOperations,instances,work,bytes)`
freezes all batch instances and checks owner/catalogue epochs, exact caller
record/control/source cuts, provider instruction coverage and typed relocation.
It exposes `sourceowner`, `candidateowner`, `operation`, `instances`,
`instance(i)`, `instanceat(callid)`, `status`, `reason`, `valid`, `usedwork` and
`usedbytes`. This proves structural correspondence, not activation permission.
`analysis.expansionequivalence(plan,correspondence,work,bytes)` checks every
batch instance and grants the new final publication permission. The existing
typed `directequivalence(callid,diagnosticclone)` path remains intact until the
replacement and maintained controls are coherent.
Every new expansion-correspondence factory stamps a fresh overflow-checked,
worker-local `identity()`. Copies of that same immutable correspondence retain
the stamp. Receipts bind it together with source/candidate epochs and instance
call/procedure coordinates; reassigning an external map variable to another
valid same-owner correspondence invalidates the original receipt. Published
module copies match the live sealed arena identity, not copied wrapper-variable
addresses; proof owner/profile references remain exact.

Default publication runs the final expansion theorem once for the whole batch;
that theorem already checks every instance. Optional preflight clients may use
the separate `expansionlimits(source,candidateBound,instances,slots,rows,identities,work,bytes)`
structural envelope, not another whole semantic/diagnostic envelope. Its charged source
scan measures provenance/operand bytes and active source-record size; checked
linear record/control scans and quadratic slot/identity checks supply bounded
work/retained-byte allowances with overflow guards. The structural checker
continues to enforce its actual counters. A limit or final refusal preserves
the entire original and reports a nonempty shared or site-specific reason for
every selected call; no partly accepted batch is published.
Unexpected transformation signals keep that atomic invalid/limit result and
include the current construction/proof phase plus bounded VM origin data;
authored filenames are hex-encoded in this error-only diagnostic.
The CLI's outer unexpected-error handler also identifies its internal phase,
including the return/publication boundary, without emitting successful-path
diagnostic traffic or changing the service's proof status.
Structural budget refusals distinguish work from bytes and report the bounded
stage, instance and consumed/attempted/cap counters. The adapter adds its
request, preconstruction remainder and construction debit, so exhaustion can
be assigned to its owner without increasing a fixture's budget or suppressing
the structural proof.
Default relational expansion uses sequential checked stage debits. Construct
the candidate under the remaining request cap, subtract the owning draft's
construction/validation debit and newly incurred shared-fact costs, then run
structural correspondence and one final batch theorem under the exact
remainder. The gross pre-draft diagnostic estimate and fixed byte split do not
govern this path; the explicit legacy route retains its previous envelopes.
Completed stages consume their reported debit; the one remaining request quota
passes forward once, while no live owner's allowance is lent twice. The final theorem
charges only correspondence work first performed within that stage; a frozen
map already debited by the caller is not charged again. Shared owner deltas
already included in the final aggregate are counted once, never added twice.
The final fact reports source-query work/bytes already included in its total;
the summary wrapper excludes those costs from its additional ledger debit.
Completed dependency/helper costs remain charged on refusal, including a
dependency that exceeds the remaining final allowance. Such failure revokes
the same fact to `limit` without replacing its counters with zero; checked
counter saturation reports a limit rather than wrapping or granting credit.

`lltarget.draft.usedwork()`/`usedbytes()` are available after a successful
freeze and checked target construction. Work combines consumed adapter fuel,
an explicitly conservative target-schema IR-verification fee from actual table,
payload and duplicate-name dimensions charged before freeze, and actual target
module/inline-index/CFG-check validation work. Internal
`module.validationwork()` and `cfgcheck.usedwork()` report their own successful
validation debit. Bytes include arena allocation/capacity and an owner-checked
validation/index/scratch envelope derived before those allocations; this is
logical budget accounting, not RSS. Failed or unfinished construction grants
no unused-credit claim. All formulas check overflow and remaining allowances.
For the fixed target schema, the conservative pre-freeze work fee is 32 units
per actual IR table row, four per arena payload/accounted byte, four per named
row inspected, and `count² * (2 * maximum_name_bytes + 8)` for function/export,
slot and block duplicate-name dimensions. The checked byte envelope is arena
capacity plus 4096 fixed bytes, 512 per operation, 256 per block, 128 per
function and 16 per accounted arena byte for verification/codec/index scratch.
These are explicit conservative logical fees, not a second validator or
allocator measurements; the existing IR/target validators still decide validity.
The shared target owner applies this truthful accounting to all draft writers;
legacy inlining retains its previous reservation policy but receives the same
checked construction charges. A representative existing scoped control covers
that resource-behavior change.
Provider records are indexed once from their frozen module and reused, rather
than repeatedly rescanning its graph for each body ordinal. Resource limits
retain the complete original with nonempty per-site diagnostics and no partial
warnings/output.
An emitted manifest receives its provider reference from the live batch owner.
Manifest construction and validity distinguish a live reference from an
initialized, valid exact module owner. Replacing that module with an
uninitialized value invalidates the manifest without invoking module methods.
Per-instance dereferenced provider/IR views detach before the next loop
iteration reinitializes their local slots; lexical-scope cleanup at loop exit
does not license assigning a typed default through an active owner alias.
The relation charges subsequent source-flow, candidate-flow and summary-ledger
deltas once, including refused candidates and frozen binding-recipe queries.
Its final ledger is sealed before outer accounting; summary costs already
charged while making plans/coverage are subtracted from outer composition,
never spent twice. Exhaustion retains the exact original source atomically.

The existing validated decoder is exposed as
`lltargettransfer.profile.bindingprogram(opid,mode='binding',work,bytes) -> bindingprogram` with
`status/reason/steps/kind/arity/operand/usedwork/usedbytes`. It reads only an exact source operation
of that live profile and the owning native binding recipe, uses its existing
checked decoder and a caller-charged work/byte envelope, and grants no callback,
phase or lowering permission. Frozen profile state is not mutated by decoding.
Mode `preparation` selects the same native CALL prefix recipe and permits its
empty sequence; unknown modes fail without granting a lowering.
No second text parser is introduced. The direct-inline worker owns this narrow
export/wrapper; final event/phase equivalence belongs to the relational proof.

The shared resource projection is
`lltargetflow.analysis.resourceat(operation,bank,index,edgeid=0,anchor='active') -> abiresource`.
It reads the immutable before-operation state when edgeid is zero; otherwise
the exact frozen edge must have that operation as its source. Anchors are
active/base/entry. Missing, unreachable, mismatched or unknown state supplies
no live/absence/capacity facts. Existing `abiresourcepoint` owns interpretation;
the query charges the owning flow budget, binds its owner/epoch and never
evaluates a second resource algorithm. The relational-proof worker owns this
one focused flow addition.

The native bytecode CALL frame-preparation projection distinguishes fresh
`value_init` backing from recycler cleanup. Its getters are
`callframeclearrole='a0-original-backing'`,
`callframeclearextent='active-tree'`, `callframeclearwhen='reused-frame'` and
`callframelocalmode='relink-original'`: reused entry clears the original owned
a0 backing, then sets integer arity; ordinary owned locals only relink while
SAFE recycled-frame clearing is disabled. It grants no borrowed argument/global
or observer premise. The reused-a0 finalizer is incidental recycler cleanup
under the timing contract below; a current-body footprint cannot describe its
prior payload and need not do so for per-call equivalence. Ordinary copied body callbacks remain
independent paired events. The
coordinator owns this native catalogue addition.

`flow.privatebackinghistory(functionid,bank,index) -> backinghistoryfact`
supplies `status`, `reason`, `valid`, `resourcefree`, `nonescape`, `points`,
`function`, `bank` and `index`. The compatible API names now describe the
current invocation's published original-owned r resource/escape footprint;
they simulate no prior dirty scalar/cache contents. NULL/resource-clear masks
are not resource creation. Unproved imports/creation, pointer export,
attribute/native/address selectors and unknown hooks retain exact refusals.
Borrowed/globals and `.inlinedprivate` backings receive no owned seed, and no
scalar definedness follows. A0's current-invocation resource and escape
footprint remains subject to the ordinary ownership proof; no prior recycler
payload fact is inferred from the current body.

Under the public opaque-value-handle contract, a supplied-handle resource
callback and a separate nested public root cannot write an unescaped original
backing outside their supplied handles. The checked bytes-snapshot route may
preserve this footprint. Reentry, physical code/frame observation, host,
context, pending-signal and endpoint callback effects remain independent;
removed callbacks still need exact absence or equivalence.

The conservative pre-solver inventory supplies reference-record entry seeds
only. It establishes no native payload, retained capacity or active-attribute
absence and suppresses no first-overwrite finalizer. Unresolved footprints may
refine after solving through the same executor's frozen per-action
active/base/entry StorageIds, exact CFG prefixes and native resource predicates.
This introduces no binding interpreter or second resource evaluator. An
inactive original proxy backing remains distinct from borrowed active payload;
unknown identity/phase, resourceful revival or pointer export refuses. After a
possible callback the executor may publish binding-only points where the
existing native structural contract preserves the pointer tables. Component
cells/captures remain unknown/unmaterialized; unknown pointers remain zero.

**Retained-child original-backing projection (INL-STEP04/05; in progress).**
The shared flow owner refines `privatebackinghistory` over its same frozen
native action paths. Each action uses its own before-action active/base/entry
StorageIds and component domain; a slot selected at one compound phase does
not select that slot at another phase. Resource imports add exact original
owned backing dependencies to the existing least fixed point; foreign or
unknown imports retain a reason. An external action with a checked complete
intrinsic value footprint supplies no additional hidden resource write; its
`none` or `bytes-snapshot` callback value surface still preserves all separate
host/context/callback/observer obligations. Incomplete footprints and unknown
surfaces refuse. This changes no scalar availability or observer theorem and
adds no resource interpreter. A typed `context` action whose checked domain
is `context` updates ambient state, not a Rexx backing resource. The child
resource projection retains its unknown context value and all restoration,
synchronization and observer obligations; it grants no numeric-state precision.
A missing/non-context domain remains an explicit refusal.

The resource-only projection may replace an aggregate `failure` inventory
refusal caused solely by unknown native signal names/contract with the exact
canonical CFG's complete wildcard and typed cause departures. It requires
complete resource effects/references/components/ownership/bindings/order/control,
at least one exceptional producer, complete scope/path edges and no unproved
returning handler reach. Every normal and exceptional action prefix is still
checked. Retained child allocation/layout/initializer/provider and callback
failures remain native CALL behavior; none is discarded using the removed-call
allocation opportunity. Unknown handler destinations or incomplete phase/binding
points remain refusals. This refines only resource reach/creation, never scalar
failure state, signal names, delivery, observer equivalence or CALL removal.

An exact retained same-owner bytecode child is projected by substituting its
formal entry StorageIds with the parent's captured actual StorageIds, and its
physical result with the captured result StorageId. Child original locals and
a0 remain separate identities. This is the current invocation's published
resource/escape footprint, not a simulation of prior dirty private scalar or
cache state. The projection traverses the
same native actions and all complete normal/exceptional/asynchronous exits,
including return-source resource dependencies and pointer export. It uses the
canonical CALL cause, initializer-ingress and frame-preparation inventory;
resource-only opaque-handle frame callbacks cannot mutate the checked
unescaped parent backing, but remain real events with separate observation,
reentry, phase and lifecycle obligations. Unknown/delegated/native/reflective
routes, incomplete phase/binding inventories, recursive/deep child graphs and
unproved selector/presence remain explicit refusals. No fresh-root, private-name
or empty-exposure premise is used. The reused a0 `value_zero` route can call
the published native payload finalizer, but its incidental recycler timing is
not a per-call correspondence obligation. Reference-record retirement alone
runs no such hook. The same published finalizer may run on the first explicit
r overwrite; a positive current-body footprint therefore seeds no native or
capacity/attribute absence and suppresses no endpoint hook. Exact frozen
captures and actual writes remain the only such resource endpoint proof. The
finalization timing contract below governs only incidental cleanup. All work, substitutions
and action-path materialization consume the flow budget and frozen owner epochs.

A declared unreferenced owned r original may use a stable virtual history
cache key, just as unreferenced a0 does. It receives no solver register or value
fact; complete inventory must still exclude implicit/reflective access and
escaping callbacks. The focused `target-backing-child` consumer covers scalar
child writes/returns, aliasing/rebinding, unrelated compound endpoints, resource
imports/creation, pointer export, unresolved/dynamic targets, stale owners and
bounded refusal. Sol 6 owns compilation and focused execution; source review
alone leaves this implementation and INL-STEP06 unqualified.

Provider frame control remains a separate current-invocation obligation.
Native `release-frame-plugins` retains its conditional free-callback semantics;
the published-interface plugin nonownership invariant below proves that
private `loaded_here` ownership is never installed. No historical simulation
of dirty private local/cache contents is required. Current invocation, scope,
handler, private-policy and reference owners still require exact entry/body
and normal/abrupt/asynchronous exit facts. Value-backing absence/nonescape and
matching row schemas supply none of that control proof. The final relation
consumes its owner-bound `framecontrolhistoryfact`; unresolved control effects
retain their precise correspondence reason. Native retirement and eventual
execution-finish teardown stay separate. Explicit current-invocation payload
effects and eventual ownership remain resource/cause obligations. No prior-a0
recycler timing gate is required.

The native catalogue owns `callframecontrolstate(opcode)` and
`callframecontrolentry(opcode,field)` under the exact ordinary bytecode child
guard (nonnull parent, no invocation/interrupt/root/program-entry owner).
Known entry fields are plugins=`parent-borrowed`, pluginowners=`none`,
invocation/scope/handlers=`empty`, policy=`parent-borrowed-cow`,
references=`unmarked`, interrupt=`parent-inherited`, action=`off`;
callerwindow is `none` for explicit fixed arguments and `mapped-window` for
counted/fused mapped calls. `framecontrolstate(opcode)`,
`framecontrolmask(opcode)` and a named bit getter expose the one reviewed
direct inventory for policy, handler-stack, inline-scope,
reference-ancestors, child-entry and retire. Callback, delegated and async
obligations remain separate; an unknown name returning zero proves no absence.
The existing eight-column ABI gains `frame-retire`, with ordered operations
`leave-frame-scopes`, `clear-frame-handlers`, `release-frame-policy`,
`release-frame-references` (original owned prefix then original a0, then reset
the marker), `abandon-frame-invocation`, `unpin-frame-invocation` (then null the
owner), and `recycle-frame`. This expands ordinary RETIRE_FRAME, without
clearing retained values or freeing plugins. Final `clear_frame` and
execution-finish retain their distinct ordered value/plugin/control cleanup.
Level L's owner-bound frame-control fact checks typed direct/transitive
effects, current CFG exits and native control retirement; value-backing
footprints supply none of that ownership proof. This is an internal catalogue/ABI projection, with
no new ISA, RXBIN or public API.

`catalog.framepluginownership()` returns
`none-published-interface.v1` only under the reviewed native invariant that
decimal/Unicode private-frame `loaded_here` ownership is never installed by
supported instructions or public RXPA/RXVML callback/reentry routes. Those
flags are initialized/reset to zero; internal raw-struct mutation is outside
the published interface. This says nothing about plugin instance/context
immutability, value finalizers, callback or observer absence. An unknown
contract supplies no plugin-ownership proof.

Level L records native `framecontrol` atoms in the existing direct/transitive
summary closure. Its internal checked-phase receipt is created only after the
same expansion relation has checked entry, body and result/exit correspondence
for that exact call, with source/candidate owner epochs and the final mapping.
`analysis.framecontrolhistory(receipt,work,bytes)` returns an owner-bound
`framecontrolhistoryfact` with status/reason/valid/preserved, operation and
work/byte accounting. It accepts no caller-supplied callback-pairing boolean.
The receipt snapshots external summary/mapping owners, frozen call/provider
and module/catalogue identities, and copied checked no-hook event IDs. It
retains no reference to a prepared relation returned by value or to its flow
fields. Snapshot copy bytes/work and each no-hook lookup scan are charged.
The fact consumes native child-entry/control masks, exact CFG exits and the
ordered `frame-retire`/execution-finish control rows. Native published-interface
plugin nonownership is distinct from value backing absence; the current
invocation's policy/handlers/scopes/reference/invocation effects remain checked
independently. Private dirty caches require no historical simulation. The
reused-a0 recycler finalizer is not inferred absent, but its incidental timing
does not gate inlining. This fact grants no
final dispatch, observer or publication permission.

Approved finalizer timing contract (1 November 2026; clarified after a0
review): native reused-a0 `value_zero` may call the documented native-payload
finalizer, but an incidental dirty-private cleanup has no specified timing
relative to calls or instructions. It may happen on reuse, frame retirement or
context teardown. `a0` still contains the new argument count at entry; `a1+`
are borrowed caller bindings, not callee-owned recycler backings. Explicit
value operations and ownership transfers retain their documented effects and
ordering. Every payload actually installed must remain owned and receive
exactly one eventual finalization. The Level L prior-a0 timing refusal is
removed from per-call equivalence. Current-execution payload ownership still
needs proof across normal and nonlocal exits; do not infer
absence from a single current invocation or merely from no textual `a0`
write. The per-procedure recycler also permits a stronger inductive absence
proof if all aliases, attributes, callbacks and escape paths are covered.
Classify the existing native call-frame recycler-cleanup cause once at the
shared Level L equivalence boundary. Keep its raw callback possibility and
effect havoc in VM flow; only its incidental call-relative timing is excluded
from inliner correspondence. Do not add per-opcode or per-call-shape a0
exceptions, and do not weaken current-invocation payload operations.

`lltargetflow.analysis.relational(ownerRef,profileRef,cfgRef,work,bytes)` uses
the same solver and resource predicates, precomputing the current original-owned
footprint before solving and seeding only checked r reference-record absence.
It seeds no scalar/native/capacity/attribute absence, borrowed/global or
`.inlinedprivate` cells, and reinterprets no source execution profile. The
factory and cache ordering retain the same owners; frozen current-invocation
exits and published finalizer obligations remain independent.

`catalog.callbackvaluesurface(opcode)` is `none`, `bytes-snapshot` or `unknown`.
The reviewed SAY/SAYX byte route passes no suspended value handle; only a
checked nonescaped private backing may use that surface fact. HOST/reentry,
context, pending signals, physical observers and async effects remain explicit.
This internal catalogue projection changes no ISA/ABI.

`catalog.observationmask(name)` projects existing native activation-observation
bits for `frame`, `module`, `code`, `debug` and `unknown`; an unrecognized name
returns zero without proving absence. This adds neither opcode data nor ISA.
The final relation may distinguish truthful actual-selected-image CODE from
physical FRAME/DEBUG/UNKNOWN; module ownership still requires its separate map.

Optimized artifacts need not retain identical instruction bytes. Code readers
must truthfully expose the actual selected image, retained TRACE events must
remain safe and ordered, and static inline source/fault diagnostics must remain
truthful. Known-on TRACE may disable optimization at assembly/link selection;
runtime TRACE still observes the selected optimized image. Physical parent,
register and activation observation is a separate equivalence obligation.

The native owner extends the existing `rxop_abi_contract(opcode,mode)` table,
not a second cleanup algorithm, with `root-return`, `root-sigreturn`,
`root-unwind` and `execution-finish` modes. Ordered typed rows distinguish
result transfer when a receiver exists, deactivation, external argument
copyback/free, root retirement and recycler draining. Normal RET/SIGRET use
the live original a0 count; abnormal unwind uses saved `frame.number_args`.
The contract identifies checked `rxvm_run`, `rxvml_run` and descriptor-entry
variants. These rows grant no raw-root resource/observer absence: the final
relational theorem pairs original endpoints and proves any added-cell absence.
The coordinator/native owner implements and validates the rows; focused
controls cover live-versus-saved counts, receiver/no-receiver, normal/signal/
abrupt exit, entry variants and malformed/unknown mode refusal.

Empty `.expose` is not a closed provider-ingress proof: METALOADEDPROCS can
enumerate every procedure, DCALL accepts held handles, and public RXVML
META_FUNC fallback can discover metadata-described members. All-invocation
backing history must not rely on a private name. Context-bound closure needs
checked frozen module, entry, reflection/held-handle and late-load boundaries.

The final relation pairs source virtual CALL/RET micro-events from native
contracts with emitted preparation, body and exit events. It tracks active,
base and slot identities separately, aliases/presence, all published components
and flags, lifelines, globals, numeric/signal context and source/static
diagnostics. Unknown values remain corresponding unknowns; a matched callback
retains havoc without a definedness, resource-absence or no-observer theorem.
Removed hooks, initializer, physical observers, cleanup and signal departures
need their own absence/equivalent lowering proof; only the approved allocation
pressure opportunity may be discarded as infrastructure.

Implementation is one coordinated source batch: summary draft/final facts;
checked remap instance/correspondence; generic preparation/capture, proxy or
evolving-binding, return/context/signal emission; then adapter publication and
focused controls. Caller no-clear identity is separate from absolute site
normalization. Binding-mutated formals use private pointer coordinates rather
than changing caller slots; base/UNLINK semantics require saved base coordinates.
Fixed and finite counted/fused shapes use database recipes without INLENTER
fallback. Unbounded layouts or unmatched observable events retain concrete
semantic obligations. Direct-inline worker owns remap/inliner/this API section;
relational-proof worker owns summary facts and theorem controls; coordinator
owns native contracts and all builds/QA.

Owned normalization uses one deterministic bounded ledger, retaining the
existing maximum three-quarter outer reservation: one quarter for receipt and
proof operations and one half for owned preparation. The explicit normalizer
keeps its conservative bounded envelope while it lacks an actual-cost report;
sealed transfer/CFG construction returns unused quota through its existing
used counters. Persistent flow/summary owners retain their whole granted
envelopes for later queries, so reclaimed construction quota is never lent
twice. Source flow receives half the then-remaining preparation allowance;
summary is reserved only for conditional proof, and a distinct candidate flow
uses the remaining disjoint allowance. An unchanged exact frozen result graph
reuses its source analysis stack. The outer reported reservation remains
exactly three quarters, including held future proof/query quotas. Tiny work/byte exhaustion still refuses the
whole transaction before output/warnings. `policy=never` retains standard RXAS
normalization; it is a call-selection policy, not an unchecked-input mode.

**Native callback structural-policy preservation (INL-STEP04/05).** The checked
default-HALT profile supplies an entry policy, not a promise that arbitrary
callbacks preserve it. Add `rxop_callback_preserves_signal_policy()` and its
generated `catalog.callbackpreservessignalpolicy` projection for complete,
reviewed callback routes. Resource-only callbacks and the public SAY/SAYX
bytes-output route preserve the suspended physical frame's handler table.
The byte exit receives only a borrowed byte span; supported nested public VM
entry creates its own root/frame and restores the outer trampoline, and child
policy writes use frame-local COW. The context output writer forbids reentry.
Queueing an interrupt changes the pending word, not its receiving handler
table. Thus inherited default-HALT remains a terminal alternative; authored
handler/backward edges remain in the CFG and the site's ingress cone.

This fact does not remove HOST callbacks, queued signals, nested execution,
value/binding havoc, physical-frame observers, output failure or opaque output
effects. Unknown/incomplete, selector, unreviewed host, invocation and hidden
ABI/departure routes do not receive it. Explicit instruction policy writes are
separate effects and remain. Arbitrary mutation through private debugger/VM
internals is outside the published callback contract. Transfer policy closure
and callback destination projection consume this native-owned fact; the
inliner and normalization gates receive no mnemonic exception. Focused
controls pair canonical output under the checked entry premise with an
authored POSIX_INT backward handler, unsealed entry and an unenrolled-provider
callback. The real `io-branch` CLI no-call/output acceptance remains open until
the coordinator qualifies it, including independent retained-call obligations.

The absolute site collector refuses every possible callback after capture with
`target.flow.site.call.callback.phase.unproved`. Pointer identity preservation
alone does not prove payload definedness or exhaustive dirty origins: a hook
may transport dirty components. Defined and dirty argument controls both retain
that refusal, as do independently authored lookalike and HOST hooks. The
relational inliner consumes the separate unchanged normalization identity and
source-to-candidate event relation; it does not weaken this absolute fact or
substitute a foreign-defined value. Ordinary transfer keeps unknown callback
availability explicit across shared globals and escaped aliases as well.

The maintained `target-scopefree-inline` ordinary-CALL cases (including the
explicit `all-legal` policy) must be rechecked after removing the obsolete
prior-a0 recycler callback-phase gate; other exact source and callback proofs
remain independent. The literal SIGNAL control
refuses earlier with `target.cfg.ingress.local.destination.unproved`: its
incomplete callback-policy inventory propagates through the child invocation
and retains an unknown local CFG destination, which default-HALT cannot close.
Native CALL clears the provider's
reused a0; INLENTER clears caller-owned recipe slots and cannot substitute that
event. This dependency predates the sequential ledger change. Independent
explicit INLENTER/return ABI and runtime positives remain in their owning
fixtures; none authorizes bypassing the CALL replacement obligation.

**Approved decimal context synchronization contract (INL-STEP04/05).**
Adrian approved a value-only, non-signalling and non-reentrant decimal
`syncNumericContext` contract. Synchronization may read the supplied numeric
settings (or the documented default when the pointer is null) and update only
the provider instance's private cache of their effective values. It must not
raise or clear a signal, change diagnostics, call VM/native/host services,
reenter execution, read or change register/value state, mutate the supplied
context, retain/observe its pointer identity, or make call count/order visible.
It is total for valid contexts and performs no allocation/resource operations,
I/O or abnormal exit.
Other decimal operations must leave the synchronized effective configuration
intact between sync calls, restoring temporary internal adjustments before
return, including failure-result returns.
The host may omit a redundant synchronization when effective numeric settings
are unchanged. Other decimal operations, native hooks, frame recycling and
cleanup retain their separate effects and proofs. This increment establishes
the contract and gate; it does not add dispatch-time sync suppression.

Decimal plugin registration advances the existing tightly coupled ABI from 2
to 3 without changing `decplugin` or `value` layout. A rebuilt provider opts in
with `REGISTER_DECIMAL_PLUGIN`, exposing `_register_rxvm_decimal_plugin_v3`
for dynamic loading and the corresponding checked static registration API.
Its descriptor carries exact structure size, ABI version 3, decimal type,
`value-only-nonreentrant.v1` synchronization contract, factory name and factory.
Validate the descriptor before publication and before writing any decimal
extension fields. A factory whose returned type or required sync entry differs
is rejected. Legacy `_register_rxvm_plugin`/`register_rxvmplugin` remains a
Unicode path and must reject decimal providers atomically; rebuilding the host
alone cannot confer this contract on an old binary. Dynamic incompatibility
has a distinct load result and diagnostic. Existing live instances/catalogue
remain intact after rejection; descriptor and library cleanup keep the current
transaction owner. Bundled db/mc decimal factories explicitly use ABI 3.
A library exposing both registrars may contribute new decimal and legacy
Unicode factories in one atomic transaction; invalid versioned registration
never falls back, and a legacy decimal contribution rejects that transaction.

The focused native gate is implemented and checked: constructed old-decimal
rejection, malformed new descriptor rejection, legacy Unicode acceptance,
rebuilt bundled decimal load and private instance/lifecycle controls. Independent checks
compare equal numeric values in distinct context objects, null/default sync,
changed values and diagnostic preservation for both bundled engines. The native
canonical catalogue may advertise this enforced contract; shared CALL proof
must distinguish decimal context synchronization from independent value/frame
resource hooks. Root owns all build/test execution and broader INL acceptance
remains open.

**Retained caller ABI composition (INL-STEP04/05; implementation/QA todo).**
Direct expansion removes the callee activation; it retains the caller's physical
frame and its eventual normal/terminal cleanup. The current root
`target.abi.parent.unproved` result is not permission to use the native nested
`INTERNAL_PARENT` retirement row: root copyback/external-argument destruction
is not represented by that row. Keep the ordinary raw-root ABI query unknown
and possible. A separate shared Level L transformation certificate may prove
that this retained boundary remains equivalent; callback absence and callback
preservation are distinct outcomes.

Preserve a typed pending boundary ledger even when a conditional ABI row cannot
be published: exact owner/profile/CFG epochs, physical caller, source edge and
cause/phase, scope prefix and root-entry kind. Never classify it by diagnostic
reason text. A provisional direct draft may defer only caller-owned retained
boundaries after its removed callee ABI, callbacks, initializer ingress and
other semantic obligations are proved. Public final permission stays false
until candidate validation closes both this ledger and static diagnostics.

The certificate binds the frozen source flow, exact frozen candidate IR and
checked expansion correspondence. For the initial zero-delta slice the candidate
operational view is derived from that correspondence and the proved provider
plan; a caller-supplied candidate profile cannot assert the required equivalence.
It compares all observable inputs and order at every paired
retained cleanup boundary: original storage/argument/result identities and
resources, context, policy/scope, reference lifetimes and root role. Additional
clone cells need their own full-capacity hook-absence, nonescape and recycled-
activation proof; fresh numbering alone proves none of these. Declared physical
frame/code/metadata observers, including caller continuations and callbacks,
must be proved unchanged or absent. An incomplete observation inventory cannot
be projected away. An opaque tail is preserved only by complete observable-input
identity; otherwise its missing ABI/observation contract remains a named gap.
Normal later caller returns and abrupt routes both retain these obligations.

The first typed positive uses an ignored-result, zero-local leaf in a physical
root with no added clone cells, no root argument/result mutation and no physical
observer. It must retain raw-root uncertainty while independently proving the
unchanged caller tail. Negative controls alter a caller ABI input, add a
resource-bearing clone cell, introduce a physical/code observer, mismatch a
cause/phase or scope prefix, and reuse a stale owner/correspondence. A callee's
unproved departure cannot borrow the caller certificate. Charge the ledger,
paired checks and added-cell proofs to existing work/byte budgets, preserve the
last-good output on refusal, and invalidate/reprove after cleanup or later waves.
The first executable L-client control uses an independently constructed complete
operational observation profile. Canonical marked-main and ordinary raw-root
admission remain CALL while their entry-resource/observer premise is unproved:
the program-default entry marker alone does not establish clean argv/native
payloads, complete hook observations or an unobservable physical frame. Native
entry APIs must supply a checked premise, or the native catalogue must publish
the complete retained-boundary observation contract, before that control can be
generalized to ordinary RXAS input. The initial expansion certificate permits no
new caller cells, arguments, result mutation, policy/context writes or callback
effects in the provider; these are generic operational restrictions, not an
opcode allowlist. A source-only direct plan grants draft permission only. The
diagnostic checker consumes its return/restoration plan, avoiding a dependency
on final CALL-to-clone permission. The final paired certificate validates every
unchanged caller record and control target outside the exact certified clone,
including later physical returns and abrupt cleanup paths. A renamed label or
rebased diagnostic anchor is checked by its mapped identity, never assumed.
The initial single-site theorem also requires no pre-existing static frame and
only normal continuation edges at the removed CALL. An exceptional CALL cause
cannot be erased merely because the retained cleanup is opaque: its pairing or
the approved infrastructure-opportunity removal needs a separate typed cause
correspondence. Until then it reports
`target.retained.call.departure.correspondence.unproved`. Existing later caller
RET/terminal cleanup remains present and is preserved by exact state/control
identity; its raw ABI fact remains unknown/possible.

The shared interface is `profiledraft.boundary(functionid,observation)` for a
complete independent physical-cleanup observation inventory (omission and all
canonical profiles remain unknown), `analysis.directactivation(callid)` and
`analysis.directplan(callid)` for draft-only permission, and
`analysis.directequivalence(callid,diagnosticclone,work,bytes)` for final checked
publication. The last fact binds the frozen summary/profile/CFG/source and
candidate owners, exposes `allowed/preserved` separately from raw ABI absence,
and returns the original module on refusal. `diagnosticclone` owns provider
instruction/source correspondence; the final checker additionally verifies all
unchanged module records/labels and active source projections, allowing only
the exact certified clone and its typed diagnostic records. The source-only
plan is the diagnostic checker's return/restoration input, not its conclusion.
Its cost shares the pre-reserved diagnostic envelope: extra work is bounded by
`(source_records + candidate_records + source_operands + 1) *
(max_payload * 12 + 160)` and extra bytes by
`(source_records + candidate_records + 1) * (max_payload * 4 + 128)`, with the
same checked arithmetic and outer allowance. The first focused controls are
`levell_target_retained_caller_rxbvm` and
`levell_target_retained_caller_native`; their QA remains coordinator-owned.
The abstract stdout operation in the constructed control has an explicitly
complete empty activation-relative observation inventory through
`profiledraft.relocation(printed,'none')`. Its external intrinsic footprint
describes the output effect and grants no observer/ABI premise. A physical or
unknown relocation inventory retains CALL; canonical SAY/HOST callbacks keep
their normal native observation obligations. The correspondence binds original
instructions, labels and address-bearing TRACE/runtime metadata on their
original side of the source CALL, with CALL-entry anchors exactly at clone
entry and continuation anchors after the entire clone. Hostile controls move a
post-CALL label/TRACE event before a read-only looping clone, move the clone
across a caller global write, and corrupt only numeric caller SOURCE_STEP
fields. Source projections use canonical `.srcfile`/`.srcline` records and
typed integer indices; empty integer byte payloads are never equality proof.
This requirement keeps INL-AC02/04/05/07/09 and INL-STEP06 open.

When TRACE is known on at assembly/link time, selection may disable inlining and
other optimizations. This is a compile/link selection policy, not an assertion
that runtime TRACE remains off. Runtime TRACE may become active after the image
is produced; admitted clones therefore retain typed inline-frame/source
diagnostics and the retained-boundary observer proof still covers runtime
observation. No new runtime opcode or alternate execution path is implied.

Direct diagnostic planning uses activation mode 2, with an explicit pending
diagnostic obligation. Its semantic permission is separate from final
`allowed()`: a typed frame/map alone grants no permission. A candidate clone
certificate binds the source analysis/call/provider, frozen candidate owner,
root static frame and complete source-instruction-to-candidate-region
correspondence. Check source anchors, every copied/lowered instruction and
root/descendant map coverage before publishing diagnostic equivalence. Compare
the active `SOURCE_STEP` at every copied and lowered instruction, including an
explicit unavailable step at provider entry; matching origin spans alone are
insufficient. Each descendant frame definition must sit at the mapped source
definition's next-instruction/procedure-end coordinate. Every copied label operand use that same checked physical-boundary map:
renamed targets must reach the first copied/lowered source instruction or its
mapped procedure end, preserving the entry-capture boundary. Uniform label
renaming alone cannot certify a target moved earlier or into caller code.
Numeric entry getters
and return setters must correspond exactly to the licensed restoration fact:
saved fields use distinct appended cells captured once in field order, return
setters follow result transfer in field order with the exact saved cell or
proved literal, then the shared continuation branch. Check their source steps
and parent/root map attribution at the same checkpoints. This
certificate addresses terminal diagnostics only; physical-parent observers,
callbacks, policy, storage and ABI obligations remain independently binding.
Implementation and focused positive/negative qualification remain open. The
decimal entry callback's contract decision is approved and the ABI 3 gate has
focused native controls; Level L still must prove its separate CALL causes.

The final diagnostic checker uses one reserved, bounded envelope per proposed
direct site. Before building the draft, derive a conservative candidate IR-op
bound from the frozen source IR operations, four times its source-record count,
and, for each selected direct body, five times its provider-record count, eight
times its body-cell plus return/context-lowering-cell count, twice its register
reference count, twice its allocated-slot count, and 32 fixed operations. The
static-frame bound is the source frame count plus each direct body's provider
frame count and one new root frame. The shared summary owner computes work and
byte limits from those bounds and the checked payload/operand scan. Reserve
that per-site allowance for every direct site from the outer request before
draft construction; the draft receives only the remainder, and each final
candidate verifier spends its own reserved allowance. Checked arithmetic,
unknown bounds or exhaustion retain the original CALL and roll back the draft;
the same allowance must not be counted again as fresh work at publication.

The approved v3 decimal registration contract separates two native CALL
causes. `FRAME_CALLBACK`/`bytecode-frame-hook` covers only resource clearing
of a0 and optional recycled private backing, with canonical callback mask
`OUTPUT|CAPACITY`; it remains a possible hook. A distinct
`CONTEXT_SYNC`/`bytecode-decimal-sync` cause at frame preparation carries
`PLUGIN_SYNC`. Each typed CALL cause publishes its canonical callback mask
through `callcausecallbacks(opcode,row)`; preparation is `OUTPUT|CAPACITY`,
provider execution is `INVOCATION`, and noncallback ingress failures carry
zero. Level L does not infer these classes from condition-name strings.
The catalogue capability `contextsynccontract()` publishes
`value-only-nonreentrant.v1` only together with the mandatory v3 decimal
registration gate. Legacy decimal providers cannot inherit that permission;
Unicode's independent legacy ABI remains unchanged. The accepted contract
makes sync non-signalling/non-reentrant and value-only, excluding pointer
identity and observable call counts/order; redundant equal-value sync may be
omitted. Resource hooks and provider execution retain their independent
storage, policy and physical-observer obligations.

The canonical invoking CALL family (plain result/no-result, counted direct or
dynamic, fused pointer/status prefixes and fixed CALL1–4) and all five RET
forms publish complete callback inventories. Completeness removes only the
generic UNKNOWN inventory bit: CALL retains INVOCATION, its resource routes
and every typed cause; RET retains DEPARTURE, with result ABI additionally
present for register/string returns. RET departure includes checked-invocation
result validation/failure cleanup, interrupt-action/diagnostic cleanup and
external-root argument copy/free. These use resource hooks rather than an
unclassified user completion callback. Their actual absence, ABI checkpoint
coverage, terminal behavior and permitted execution context remain separate
facts; complete inventory grants neither purity nor direct activation.

Entry-demand extraction separates body observations from deferred physical
departure ABI uncertainty. Explicit callback actions still invalidate state
and observation completeness at their exact checkpoint. A final physical
RETURN ABI or a terminal edge without an in-procedure continuation still
invalidates state and publishes every possible typed ABI row, but does not
retroactively erase earlier body-read coverage. Edges with an in-procedure
continuation retain observation invalidation; later reads after any havoc
remain unknown. Direct activation must independently discharge the exact
caller-instantiated ABI, including caller-result resources and parent-frame
retirement. The restore-numeric row excludes plugin callback uncertainty only
under `value-only-nonreentrant.v1`; context effects and other ABI rows remain.

For a canonical resource callback, consume the native
`callbacknormalpreservesframe` contract only for complete resource-only
callbacks: retain checked pointer/base/slot identities while invalidating
pointees, numeric state, lifetimes, capacity and logical-scope ownership.
This describes a callback that returns to the VM, independently of whether
the interrupted instruction completes or reaches a handled failure checkpoint.
Explicit instruction binding actions and their partial/unknown phase still
invalidate bindings independently. A resumed handler retains value uncertainty;
it cannot borrow the interrupted instruction's successful receiver postwrite.
Callback presence, observations and policy/control paths remain explicit.
An explicit callback invalidates state at its operational event, once; it
does not schedule another state barrier after the normal CALL return
projection. Deferred physical-return ABI has its own pending barrier. A
possible canonical frame hook after fixed-pointer capture invalidates the
captured actuals' exact pointee values while preserving their pointer identities
and may-entry origins. Normal callee return projection then accounts for the
later result transfer. No result is projected onto failure/skip edges, and raw
callback observation/ABI uncertainty remains explicit. Scalar postwrite replay
still requires the independently licensed successful-normal setter contract.
At this exact canonical frame event, the typed frame-callback cause must have
a known nonempty resource-only callback mask before preserving checked caller
pointer/base/slot identities across the returning resource hook. The whole CALL also
has an invocation route; that separate route does not turn this frame-resource
event into an arbitrary pointer-table mutation. Retain pointee/context/lifetime,
capacity and logical-scope havoc and the real observation veto. Exceptional or
independently authored callbacks cannot borrow this typed event contract.
`callbackpostwriteoperand` licenses one captured receiver and computed scalar
postwrite: snapshot its ordinary component result before the appended hook
events, then restore only those component cells and guaranteed native-payload
absence after successful-normal instruction callback havoc. It grants no reference/attribute/capacity
absence, and cannot discharge its own incoming hook condition. Exceptional,
handled and independently authored callback programs retain their value and
observation barriers. State copies and joins keep scope ownership independent of pointer
identity; explicit cells above unknown defaults meet by their cell values,
including equal postwrites on a diamond. Complete demand-only entry coverage
and the existing exact unchanged-result identity certificate remain separate
from unconditional callback-observation permission.
Unknown availability absorbs a join in either operand order. Test the original
incoming availabilities before constructing a mixed entry/defined result;
unknown is not a proved may-dirty entry alternative. A focused two-arm control
swaps known and unknown writes while keeping physical bindings intact and
requires the same conservative value fact in both orders.
Flow qualification separates normal point values from whole-function entry
normalization. Canonical callback-free numeric getters provide defined integer
actuals and alias-conditional writes without assuming successful resource
setters; raw LOAD/COPY/LINKREF variants retain their real callback/handler and
unknown-inventory refusals. A literal return can prove its normal result with
unknown actual values only while its result binding remains checked. Full
private-return, skipped-call, dirty-owner and callee-certificate controls use
an explicit complete no-hook operational CALL program. They do not confer that
premise on a canonical invocation. A checked fixed-actual no-hook/handler premise
for full entry normalization and unresolved-actual-vector return projection
remains shared transfer/flow work under INL-STEP03F; the intended positive
theorem is not replaced by weakening raw callback or policy gates.

The five numeric context GET operations publish a complete empty intrinsic
Rexx-value observation footprint: each reads only its documented numeric
context field and writes the destination integer field directly. Keep their
context reads, opaque/external optimizer barriers and separate signal/effect
contracts. Fixed no-prefix/no-window local CALL descriptors capture explicit
actual pointers at handler entry, before all initializer/frame checkpoints.
Their may-executed provider reads may therefore use that exact capture on an
aggregate UNKNOWN exceptional edge when the independent frame-hook condition
is disproved; this does not license counted/fused capture reconstruction or
unknown callbacks before capture. Counted local calls snapshot the count after
initializer ingress and before frame preparation; normal public resource hooks
preserve pointer-table coordinates, while foreign initializer/host/handler
intervention retains its unknown capture condition.

INL-STEP03F still needs an independent oracle and repair for the signal-CFG
fixture's fixed-capture zero-demand assertion when an aggregate UNKNOWN
exceptional provider path retains a possible raw frame hook. Its diagnostic
clone controls are also masked by a strict context-restoration check on
ordinary LOAD/IADD payload hooks before source/map correspondence is checked.
Resolve the raw coverage expectation and let diagnostic identity validation
report its own errors without granting semantic callback permission. These
issues are separate from the incidental recycler-timing classification.

For a canonical function with an unknown-extent window reference, flow
inventories its declared physical `r` and module-global `g` coordinates plus
the always-present `a0` before solving point facts. A finite counted window
may cross those banks even when a coordinate has no explicit operand use.
Other functions retain the explicit reference inventory, avoiding unused rows
in every state copy/merge of ordinary fixed-call compiler output.
Within one solved block or one frozen instruction replay, reuse an instruction
transfer only for the same exact source operation, normal/exceptional mode and
checkpoint phase over its unchanged before-state. Retain a typed state snapshot
and independent ABI cursor; reset the cache before another block/solver round
or frozen source point. Each signal edge still performs its own ambient-cause,
scope-exit, unwind, handler-resume and boundary publication processing. Before,
after, partial and unknown phases never share a checkpoint. Charge cache lookup,
snapshot/cursor copies and retained bytes to the unchanged allowance, releasing
temporary bytes when that cache is discarded. Multiple same-phase signal causes
must not multiply a complete instruction action replay. A focused authored
multi-edge control checks equal-phase values and distinct-phase uncertainty;
the two-arm CLI roundtrip remains a resource-limit control under the current
direct expansion pipeline.
The solver visits dirty blocks in the checked RPO rather than rebuilding all
joins during every global round. A changed incoming edge schedules its target;
an exact same-owner direct CALL also subscribes its block to the selected
callee's RET-before checkpoints. A new or changed subscribed checkpoint
schedules that caller even when its incoming state is unchanged. Other stable
blocks neither rebuild joins nor replay their outgoing paths. Preserve every
edge/cause and its state/ABI processing when its source block runs, and perform
the complete frozen observation pass after convergence. Charge dirty vectors,
reverse dependency rows, state comparisons, scheduling and RPO scans to the
unchanged work/byte allowance. Caller-before-callee and transitive-return
controls must retain later refinement; unknown/foreign calls gain no theorem
from this scheduling dependency.
Inventory grants pointer/base/slot identity only, never defined
payload, resource absence, global isolation or an unproved `aN` tail extent.
The existing work/byte allowance bounds every slot and state. Callback-bearing
unchanged callers consume the exact source/candidate normalization receipt and
its identity entry certificate; the ordinary whole-function callback-observation
veto remains conservative. Scoped fixtures use this same receipt path rather
than treating an ordinary normalization stamp as callback-preservation proof.
The native postwrite query also admits binary payloads under its existing
complete inventory, one explicit shared setter, no binding/implicit effects
and output-only callback conditions. The admitted literal binary load captures
its receiver/immutable constant before native cleanup and writes the binary
bytes after successful buffer preparation. Its intrinsic failure contract
remains unknown, with no exceptional or resumed-handler postwrite replay.

CALL policy completeness composes a bounded exact-local provider closure over
canonical callback masks. Resource-only causes keep their wildcard signal
producers while preserving the suspended structural policy; host, selection,
unknown, unresolved provider and initializer paths keep explicit policy gaps.
This is not callback absence. The transfer profile leaves provider execution
with its existing call action and emits the independent frame resource-hook
action. Shared flow, rather than CFG, can discharge that action only from its
owned backing/recycler resource theorem. Extend that theorem compositionally
for fixed prefix-free same-owner calls with a private result, clean child
backing and clean returned resource projection; borrowed/unknown resource
returns, foreign writes, callbacks and cycles retain refusal. RET's separate
caller-result ABI proof remains mandatory. Monotone child dependencies and
argument-presence checks use the existing bounded closure and budgets; no
second value evaluator or fresh-frame assumption is introduced.

The [short architecture](doc/inlining.md) records the shared call/return/signal
exit model, Level L graph, reusable procedure summaries and component/flag
proof requirements, early
read-before-write clear/warning, and minimal post-link cleanup. Do not admit
ordinary mnemonics one at a time through scalar transfer functions. Preserve
bounded prior results as regressions while replacing that admission gate.
Existing ordinary-clone work is unqualified foundation, not complete call or
signal coverage. The earlier mirrored `INLINE_ENTER`/`INLINE_LEAVE` family is
superseded as an unapproved proposal. The small runtime context/ISA, dynamic
argument-bank and private-global binding contracts need concrete owner review
before product edits; the later approved direct boundary is specified above.
Adrian has selected completion and focused QA of all shared Level L capabilities
required by this architecture before integrating them into the inliner. The
approved native opcode-database read/write extension follows this Level L
foundation; present catalogue gaps yield explicit unknown facts during its QA.

## Vision and boundaries

The wider proposal is four cooperating tools in Level B with Level L
infrastructure:
a nonoptimizing RXAS assembler, an RXBIN reader/writer (reassembler) ready for
future middle passes, a module linker, and a common RXAS inliner. A naive,
semantics-preserving instruction expander is its correctness baseline; the
selected standalone outcome also includes proved transport simplification and
bounded post-expansion cleanup.
One owned target-module/image model carries instructions, records, references,
module boundaries and provenance through every service. Expose callable services
and thin CLI adapters; clients never capture another command's stdout.

This is a demanding final validation/debugging opportunity for Level L at this
stage: grammar/source/diagnostics, owned structures, transformation, validation
and atomic publication must work for real target tools and independent clients.
It does not establish universal Level L completion or close the separate
[parent research](../PLAN.md) or [C compiler](../compilers/PLAN.md) criteria.
Current develop baseline QA/publication remains independent of this proposal.

User requests execute B/L algorithms, without native assembler/linker encoding
backends, native source-compiler services, subprocess fallbacks or provider
bridges to those algorithms. Native B bootstrap, prepared canonical-schema
extraction, native VM execution and independent native test oracles are allowed.
Retain current product tools and defaults. No new ISA, runtime, source-language
semantics, platform, search/autoload policy or production replacement is implied.
The requested design also considers small linking, metadata, ABI/ISA or
language-contract additions where they materially simplify correct inlining;
the architecture compares them with the current-contract baseline. Their design
is in scope; adopting/implementing an extension requires its explicit decision.
Research has no formal performance-completion gate; correctness, bounded work,
code quality and actual sanitizer obligations retain their existing owners.

## Existing services and ownership

`rxfnsl-rxas` supplies `llrxascatalog` and the single-function checked records,
register assignment and renderer in `llrxas.program`; it is not an assembler,
module model or RXBIN writer. Its canonical view is extracted from existing
opcode/effect/signal owners. Extend that checked view for missing wire operand
domains and feature rules rather than hand-copying native numeric identities.
`llbinmeta` supplies bounded 007 envelope/LZSS and selected metadata reading;
its callable projection omits executable/graph data and is not a lossless codec.

Use actual L source, lexer/parser, owned AST/IR, transform, validation, diagnostic
and writer services/representations through their APIs, not copied frameworks.
Use existing `llir` ownership/freeze/revision/atomic-result and `llirgraph` services
where applicable. `llflow` models source-AST flow, not a machine CFG. Module/body,
typed metadata, relocation and target CFG/storage/base-binding/exception evidence
are deliberate integration deliverables, not assumed existing capabilities.
Missing shared capabilities and generic defects are repaired/extended at their
L owners with independent consumers; target defects stay with the target owner.
No private arena/transaction/graph framework or product-to-research edge is allowed.

Reusable target-model, codec, relocation and expansion facilities belong to the
optional L target owner. Research `common/` holds adapters/profile/orchestration;
`rxbin/`, `assembler/`, `linker/`, `inlining/`, `cli/`, `tests/` and `doc/` hold
research tooling/integration. Factor binary primitives with their current owners.
Proposed services: `llbinformat`, `llassembler`, `lllinker`, `llinline`; commands:
`llrxbin`, `llrxas`, `llrxlink`. STEP01 decides exact migration/package/API layout.
The wider toolchain delivery follows assembler, reassembler, linker, integrated
inliner; internal codec construction precedes the first assembler output and is
not a second framework. The standalone INL increment does not wait for complete
assembler, codec or linker delivery: its RXAS reader/model/writer are shared
target facilities, with existing native tools used only as independent checks.
The broader [inlining design](../../../compiler/docs/inlining_design.md#future-inlining-boundary-review)
provides background. The selected proposal separates expansion from a bounded
machine-cleanup service; it does not commission a general optimizer framework.
The nonoptimizing assembler and linker retain their contracts. A later driver
may explicitly invoke the same inliner/cleanup services around them.

## Selected standalone inliner outcome

### Generic inliner admission review (INL-STEP04/05)

Review the current standalone expander against the shared Level L target
contract before further opcode fixes. Remove duplicate mnemonic/category,
physical-observation and textual-last-instruction admission gates when the
existing checked target flow, CFG, activation and remap facts already decide
the actual obligation. Do not transplant these broad gates into Level L: a
shared refusal must describe a concrete missing effect, reference, control,
callback or storage mapping fact. Keep body availability, exact executable
binding, resource/policy limits and runtime representability checks in the
driver. Replace local count/implicit-window proof calculations with shared
typed layout and storage facts where they provide the same phase and binding
guarantees. Preserve a precise refusal when a retained child call cannot be
represented by the current ABI. Review same-module global mapping on the
scope-free path: direct global identity must survive, while imported globals
use the scoped selected-provider binding. Add focused positive and negative
checks for each changed obligation, then run one representative handwritten
RXAS expansion through assemble, link and VM. This is a development increment;
INL-STEP06 and INL-AC01–09 remain open until their own qualification.

Normalization is a required interface for every supported instruction form:
it either publishes a canonical form with complete typed remap/scope ownership
or reports a normalization error. The inliner does not keep a call because an
old opcode-specific admission rule lacks a case. Optional direct mapping,
value and cleanup proofs may fail without rejecting generic scoped expansion;
the inliner checks certificates, selected executable binding and the actual
runtime ABI's representability.

The admission review removed those four duplicate inliner gates. Shared
Level L now owns the direct argument/global mapping fact and reports an
unrepresented implicit reference or missing captured/qualified scope owner as
a normalization error. The canonical `QGLOBBIND` effect is a classified alias
binding with its actual before-write signals. Typed `INLENTER` entry uses its
ABI checkpoints rather than a second ordinary result-write callback. Open
shared work: make callback observation and preservation point-specific, so a
possible callback elsewhere in a procedure does not invalidate every selected
site. Keep actual resource and ABI uncertainty as an ensuring gate until that
shared proof exists; do not reintroduce an opcode admission rule or weaken
positive controls to make them pass.
Next, add exact callback-point facts, checked source/candidate correspondence
in target remap, and bounded activation/normalization preservation queries.
Independently test matched callbacks and changed phase, alias and physical
observer negatives. This does not infer a raw-root argument count or clean
backing; those need separate entry/recycle evidence. Changing an actual
physical observer's behavior remains a VM semantics decision gate.

The first bounded callback-point slice exposes an immutable before-action
point/edge fact and a point-local component-delta preservation query. An
independent operational callback may describe its complete single-component
observation projection; canonical callbacks retain unknown projections until
their owner supplies one. The initial positive requires the exact same frozen
flow/event/phase/activation and distinct proved current-frame private storage.
Aliases, physical/unknown observers, missing checkpoints and cross-owner
correspondence remain unknown. The fact preserves callback havoc and does not
authorize an earlier entry clear, complete normalization, or expansion by
itself. Normalization must compose intervening dependencies and retain a checked
source/candidate correspondence before consuming that local obligation.
The same point-local query may consume an exact checked correspondence across
two flow owners with unchanged activation: both checkpoint bindings and every
scope entry must map, and the callback phase, projection and possibility must
agree. This remains conditional on the supplied delta being the complete
point-local difference; normalization separately composes its earlier entry
clear through intervening actions. It does not certify call-frame removal or
an asserted clone instance, which retain the activation-mapping obligation.
Owned-entry demand coverage is a separate owner-bound aggregate: every typed
owned read must be proved defined or contribute its original dirty entry
base/component, while any missing binding, origin, child projection or possible
exceptional phase leaves an explicit unresolved row. Callback havoc stays in
the executor; a callback-only preservation veto does not erase otherwise known
demand rows, and a later unproved read still rejects coverage.
For canonical `INLENTER` count capture, classify its typed register observation
at the exact post-preparation, pre-capture checkpoint: a dirty owned count
contributes its original demand and a defined count contributes none. A missing
or phase-ambiguous checkpoint remains unresolved. This is shared flow work,
with no inliner opcode admission rule.
A candidate needs complete coverage with no remaining dirty/unresolved demands. Each inserted
entry whole-cell clear must be licensed by a source dirty demand, then pass the
separate callback-preservation proof. Neither aggregate alone changes the
existing unconditional normalization certificate or authorizes call expansion.
Implement this as one shared Level L development increment: expose exact
callback-point/edge/preparation facts and an observer-specific disjoint-delta
proof; publish checked source/candidate correspondence and conditional
normalization preservation without changing unconditional procedure completeness;
describe the existing frame-unwind ABI at the canonical database owner; then
exercise independent matched-event and alias/phase/physical-observer negatives
plus the affected handwritten RXAS boundary. Keep callback havoc, unknown
root backing and host-entry argument count explicit. The full inliner acceptance
and broader QA remain open.
For an inserted-clear transaction, the adapter consumes a conditional
normalization certificate only with
its exact checked source/result correspondence and still asks the shared
activation proof for CALL-to-logical-scope equivalence. It never substitutes
the certificate for callback havoc, a physical-observer guard, or root
resource/count evidence.
For that transaction, split exhaustive owned-entry read coverage from
callback observation preservation. The latter may hold even if callback havoc
leaves a later read's entry provenance unresolved. Publish an owner-bound
coverage view with explicit unknown demands, and prove every reachable demand
already defined or dominated by a correct clear on the exact candidate CFG.
The adapter must retain and revalidate that coverage certificate and its
source/result proof owners; a preserved-callback boolean alone grants nothing.
To integrate that transaction, the shared normalizer must construct a
tentative candidate from complete source entry-demand coverage even when the
old unconditional callback veto makes `change.complete(fn)` false. It reuses
the existing clear placement, warning, remap and bounded transaction path;
each clear is licensed by an owned source demand. A separate exact-result
query on the resulting entry certificate binds the live source/candidate
owners, candidate function and mapped result identity for downstream use.
The tentative candidate, map and warnings are private proof inputs. A checked
commit publishes them together only after the matching entry certificate
covers every clear staged for that one source function; failure leaves the
original result, identity operation map and no warnings.
Entry coverage also publishes direct procedure dependencies for dirty return
origins. Candidate completeness and entry certificates discharge that dependency
closure against the same final result: an unmodified owner with a dirty demand
keeps dependent callers incomplete, while a proved clear or already empty
owner satisfies the dependency. The bounded closure is shared Level L work;
source extraction readiness alone never proves candidate discharge.
Neither API changes the old unconditional completeness result or grants
CALL-to-logical-scope activation equivalence.
The POC adapter retains a request-lifetime normalization proof bundle: frozen
canonical source and candidate modules, their exact operational profiles,
CFGs and flow owners, source summaries, canonical-to-candidate correspondence,
callback preservation and an entry certificate for every modified procedure.
Use the existing ordinary path when no conditional batch is needed. Otherwise
stage every eligible dirty-demand function directly from enrolled canonical S
into one candidate C, including eligible ordinary clears. That draft stays
tentative through candidate/native owner enrollment; result, operation map and
warnings publish only after all modified procedures' exact S/C theorems are
checked together. Any required proof failure is an atomic normalization error,
with no candidate, warning or partial output publication. Compose raw-to-final
mapping as raw-to-explicit-canonical followed by canonical-to-C; warnings are
created against that final map and retain their original observation provenance.
Stamps retain the bundle and revalidate its exact
result/function proof; they do not copy a preserved boolean. One request/session
budget reserves construction and all retained analysis/certificate envelopes
once, settles subsequent queries in those same owners, and leaves failed or
limited staging at the last good immutable result. Canonical fact admission,
target selection and activation/ABI/physical-observer proofs remain independent.
Native enrollment relates one canonical owner to one final candidate owner.
The additive batch entry-candidate API therefore stages requested procedures
in one shared transaction and commits only with exactly one live certificate
per modified procedure against that same final identity. Requested IDs and
certificates must be unique, live and complete; missing, extra, duplicate or
foreign proof refuses atomic publication. Allocate every owner/proof array
slot before lending references and keep those arrays fixed thereafter.
Level B copies attribute values without rebasing internal references. Receipt
factories therefore return an unprepared value which the controller first
places in its stable local/field/array slot; only then may preparation create
borrowed proof links. A prepared receipt is never copied or boxed as an owning
substitute. Context certification borrows and revalidates that exact receipt
slot, whose controller lives through the request. The legacy unconditional
normalization convenience functions remain separate from this explicit receipt
lifecycle. No opaque back-reference or strong receipt/change cycle is needed.
For an inserted clear followed by ordinary computation, propagate its possible
effect through canonical action dependencies to private outputs with a bounded
worklist. Check every resulting changed cell against reached callbacks; tainted
control, external, binding, exceptional or unknown effects remain explicit
obligations. Do not replace this with a blanket intervening-read veto or a
mnemonic evaluator.

Adrian approved the one-sweep implementation direction on 30 October 2026.
One shared target service handles all feasible call, return and signal shapes
through the canonical Level L graph/effect model. The approved runtime design
uses a logical activation owner and emits only the context operations needed
at a site; a checked ordered pointer view handles dynamic argument banks.
The linker selects and seals the exact body and may bind a provider-private
global by module identity and slot in executable form, without public exposure
or inliner proof metadata. Native opcode effects are completed as one canonical
contract; uncovered effects and opcode defects are repaired at their owner,
not worked around by a mnemonic admission list. Expansion must be correct
with optional cleanup disabled. Selection separates availability, legality and
profitability, with disabled and force-legal controls. The assembler runs its
normal optimization after local expansion; the linker runs bounded cleanup on
changed callers, including proved call setup and inverse-swap removal.

Implementation order within this single increment: freeze exact minimal
operation/operand and RXBIN 007 validation contracts; complete the canonical
effect, signal and relocation description; implement one call recipe and
generic target remap/activation/departure path; integrate assembler and linker
contexts; then run the focused normal-configuration/one-VM panel and independent
source/RXAS review. Preserve exact refusal reasons for unavailable or dynamic
bodies, unrepresentable physical-frame observation and deterministic resource
limits. Treat any other representable refusal as incomplete implementation,
not an accepted exclusion. The existing INL-STEP06 remains the broader
standalone integration milestone; this approval does not close INL-AC01–09 or
authorize a default switch, publication or release claim.

The reviewed minimum runtime extension uses RXBIN 007 feature bit 8 and three
semantic operations in four opcode forms, with assembler-validated typed operands. `INLENTER`
receives a typed version-1 call recipe and normal continuation; the readable
RXAS source spells result, exact procedure, count, continuation, and an ordered
actual/owned-local/preparation recipe. The assembler parses the recipe once;
the VM never interprets source text. Only `none`, one checked `SETTP`, one
checked `SWAP`, or their ordered combination may be captured as fused
preparation. Entry performs that preparation and exact argument capture at the
original call boundary, checks the sealed selected body and image generation,
performs normal initializer ingress, and pushes a lightweight logical scope
only for proved context, dynamic argument-bank, or qualified-global obligations.
`LINKARGV` resolves either a register or immediate selector against the
captured ordered pointer bank; the immediate form avoids an added scratch-load
signal point. `QGLOBBIND` binds one private proxy register to the exact selected
module/global slot; the scope restores both active and base pointers on every
departure. Foreign ordinals are image-local and resolved against the frozen
linked image, never the mutable context-wide module order. A plain local leaf
that proves all scope obligations absent emits none of these operations.

A retained nested call from an inlined foreign body may target a provider-private
procedure. Its ordinary CALL/SIGCALL opcode keeps a typed qualified `P` operand
(`QPR1`) naming the sealed image-local module and provider procedure ordinal;
this is an executable operand, not a public export or proof record. The linker
validates it against the frozen selection, and the loader resolves it to the
provider's runtime procedure at image materialization. An unsealed/mismatched
qualified operand fails before execution. Handwritten RXAS has an explicit
qualified procedure spelling; the exact surface spelling is finalized with
the assembler/disassembler contract. Ordinary CALL handlers are unchanged.

The existing RET value/ownership forms close the current physical frame's
innermost logical inline scope and continue at the recipe target; there is no
extra leave instruction or intervening signal point. The VM's common departure
owner drains scopes for inherited `SIGRET`, signal branch unwind, cancellation,
terminal exit and real frame unwind. Same-scope handlers, ignored/resumed
signals and CALL handlers retain the appropriate scope. The checked RXBIN
feature gates the typed recipe, selected module/procedure/global references,
and loader validation; unknown or mismatched forms fail before execution.
The native and Level L writers/readers, linker, VM, disassembler and handwritten
RXAS syntax must agree before the extension is claimed. Focused cost evidence
must confirm the plain-leaf zero-op fast path and account for scoped entry,
argument capture, global binding and departure overhead.

**Retained variable-argument child calls.** Adrian prefers existing call
instructions here. Use `CALL1`–`CALL4` for up to four known explicit argument
registers, and ordinary counted CALL with the required setup/swaps when an
exact contiguous physical window is proved, including a directly forwarded
one. The existing counted CALL already has a count operand; a new count-only
`CALLN` would not represent an unknown-length scattered pointer tail.
`CALLVIEW` (an ordered pointer-view call using the existing frame owner) was
considered but is not in this increment's ISA. If the current call site has
no proved finite or contiguous representation, retain its *outer* call with
the exact `child.argument.count.unproved`, `child.argument.outer-count.unproved`
or `child.argument.window.outside.captured.frame` reason. Measure this gap in
G/C and handwritten RXAS before reconsidering an ISA extension; any
new operation retains the explicit user ISA gate.

The optional link-time driver uses a transient checked selection manifest,
`llinline.selection.v1`, produced only after native module selection and
signature validation. It names the frozen transaction/catalog identity,
ordered selected module owners and their disassembled RXAS snapshots, exact
procedure identities, and only call-to-body bindings that the linker can seal.
The B/L service returns one complete ordered candidate mapping; the native
driver validates every candidate's format, owner, selected binding and feature
rules before replacing any selected module. Failed or unsealed candidates keep
the previous output intact. The manifest is orchestration data, not RXBIN
metadata or a compiler-supplied proof. Standalone `--body` input without such
a selection retains a foreign call with `binding.unsealed`.

The proposed [architecture and mapping rules](doc/inlining.md) implement one
producer-independent B/L target pass, callable separately from compiler,
assembler and linker orchestration. The first standalone adapter reads ordinary
UTF-8 RXAS and writes current RXAS for the unchanged-contract profile, preserving
separate module artifacts. An extension-enabled profile requires matching
approved directives/records and assembler/linker/VM support; it cannot claim
compatibility with unchanged consumers. Already optimized bodies are valid
inputs. A checked constructed-model client is the first development slice, not
the finished standalone proof of concept.
RXBIN input/output reuses the future complete codec and is not a prerequisite.
No runtime request shells out to native tools to parse, expand or optimize.

The complete selected outcome covers every case representable under the approved
ABI and preservation contract, including any selected extensions, rather than
stopping at scalar leaf calls. Stage growth through aliases/rebinding,
omissions/varargs, nested calls, multiple returns, objects/binaries/references,
state and imported helpers.
Specific unproved cases remain calls while implementation is incomplete;
they are not automatically reclassified as infeasible or accepted exclusions.
Native/unresolved/dynamically loaded targets without one selected executable
body and physical-frame observations without an equivalent are current
boundaries. Cross-module private globals are intended to map to the selected
provider's storage; the exact checked representation is open. Review exact-site
initialization ingress and useful language effect/ownership facts without
turning `.meta` into an inliner proof side channel. Recursion/growth policy is
distinct from semantic feasibility. API/ISA, language/platform changes and
production default replacement require separate decisions.
Inlining preserves deterministic language values, control, ownership, delivered
signals, logical source and cancellation/unwind. It need not reproduce slight
differences in resource-pressure failure paths caused solely by removed or
rearranged infrastructure, such as allocation of a call frame that no longer
exists; such hypothetical opportunities alone do not retain a call.

### Capability sequence to complete the standalone inliner

The foundation already includes the generated opcode catalogue, shared Level L
flow/remap/normalization and procedure facts, a checked RXAS reader/writer,
optional `rxas` and `rxlink` service adapters, typed entry/private-global and
static diagnostic records, substantial scoped-call controls, a narrow legacy
direct theorem and a broader default relational checker. Those are implemented
increments, not seven new features to rebuild. The remaining work is the
**effective direct path** through that same foundation. The legacy direct
theorem admits only zero-argument, ignored-result, zero-local/no-state-delta
cases. The default relational route can construct broader clones but still
retains broad callback and exit refusals. Complete the following
capabilities in dependency order, marking a row done only after its stated
boundary works. The wider TGT toolchain replacement is separate.

| Capability | Current state | Completion boundary and dependency |
|---|---|---|
| **INL-CAP01 — generic direct equivalence** | **critical gap:** the default relational checker exists but still refuses ordinary changed-activation/callback and exit paths; a separate legacy direct theorem is narrow | Complete and qualify the default source-to-clone relation, removing duplicate broad entry-cleanliness/body-callback/all-local-resource vetoes where the relation proves the actual published effect. Copied ordinary instructions retain the catalogue's effects; removed CALL/RET, changed storage/activation and published observations need proof. An ordinary direct body expands correctly with cleanup off, while an explicit physical observer retains its call. Retire the narrow legacy permission only after its compatibility uses are accounted for. This is the prerequisite for wider direct admission. |
| **INL-CAP02 — call and return ABI** | **partly implemented:** fixed, counted/fused, rebinding, finite selection, nested calls and multiple returns have scoped/constructed controls | Connect those existing mappings to CAP01 for direct expansion: exact count, presence, actual order, active and entry bindings, aliases, result copy/move and retained child windows. No `INLENTER` allocation is introduced on the ordinary path. A genuinely unrepresentable argument window remains a call with a precise reason. Depends on CAP01. |
| **INL-CAP03 — context, signals and lifetime** | **partly implemented:** numeric/entry facts, G-style signal slices, dirty-entry and ownership analyses exist | Establish only the semantic differences created by removing a frame: necessary numeric/signal restoration, G blocks and C layering, normal and nonlocal exits, and eventual once-only ownership cleanup. Do not simulate exact asynchronous checkpoints, recycled-cell initial contents or incidental finalizer timing. Depends on CAP01–02. |
| **INL-CAP04 — exact module binding** | **infrastructure present:** optional linker adapter, selected-provider authority and `.inlinedprivate` storage contract exist; direct foreign-provider proof is open | Use the same direct service after exact linker selection, map public/private globals to the selected module, respect initializer readiness and process selected link-time chains leaf first. Assembler-local calls run before assembler optimization. Unresolved, native and dynamically loaded providers remain calls. Depends on CAP01–03. |
| **INL-CAP05 — diagnostics and metadata** | **mostly implemented:** typed inline-frame records, RXAS/RXBIN/VM handling and source controls exist; full clone correspondence remains open | Complete source/TRACE/frame-map correspondence and relocation for direct local and selected imported clones, including retained child diagnostics. A corrupt map fails; an actual published physical frame/code observer keeps the call. Depends on CAP01 and CAP04 for imported clones. |
| **INL-CAP06 — cleanup and admission policy** | **partly implemented:** shared liveness/cleanup, swap-window and status proofs, work limits and KEEP diagnostics exist | Run bounded cleanup after a correct no-cleanup clone: remove proved outer setup/inverse swaps, dead status/branches and unused register lifetime while keeping alias snapshots and nested windows. Separate semantic refusal from recursion, resource and profitability policy; the normal direct path adds no per-call heap scope and is assessed against ordinary CALL cost. Depends on CAP02–05. |
| **INL-CAP07 — standalone acceptance** | **todo:** focused slices pass, final whole-POC panel has not run | Freeze the above and run one compact panel through the existing assembler-local and linker-selected adapters, generated and handwritten producers, one VM, cleanup off/on, structural output, retained-call reasons and atomic failure/lifecycle checks. Repair any independently established Level L fixture defect before declaring the panel green. Only then close INL-AC01–09 and INL-STEP06. Depends on CAP01–06. |

The critical path is CAP01 → CAP02/03 → CAP04/05 → CAP06 → CAP07. The
first CAP01 repair is at the shared proof boundary: canonical callback events
currently carry an unknown phase/observation, while `bodyrelation` refuses a
copied possible callback unless both become precise. Pair unchanged source and
clone events through their checked operation/storage map, retain their unknown
value and signal effects, and ask separately whether a *published* physical
observer can distinguish the removed frame. Extend the native catalogue's
generic observation contract only where that distinction is missing; do not
add opcode names or a second callback evaluator to the inliner. Then prove
the removed CALL/RET actions and any introduced storage/restore operations as
the actual changed boundary. This is the first implementation increment, not
one repair per fixture.

The `levell_target_signal_cfg_rxbvm` red assertions are two Level L QA investigations,
not new inliner capabilities: establish the fixed-capture entry-coverage oracle
without erasing a possible raw frame hook, and separate diagnostic map checking
from semantic context-restoration permission. Repair the shared owner or the
assertion according to that independent evidence; do not add opcode-specific
inliner gates.

The inliner does not recreate a removed frame's allocation-failure opportunity,
exact asynchronous interrupt checkpoints, an old recycled cell's contents, or
incidental finalizer timing. It preserves values installed by the selected
execution, their ownership and eventual once-only cleanup. The native opcode
database owns an ordinary copied instruction's side-effect contract; incomplete
catalogue facts are Level L errors, not per-mnemonic inliner exclusions.

| Stable criterion | Observable completion and verification |
|---|---|
| **INL-AC01 — shared service and real adapter** | One B/L checked target-model inliner serves an ordinary RXAS CLI, a callable B client and an independent handwritten/DSL-style producer. No AST inline payload, copied parser/framework or native backend is required. Input/output, unknown/unsupported forms and atomic publication have explicit contracts. |
| **INL-AC02 — exact call/storage mapping** | All admitted direct, counted, fixed and fused forms preserve actual evaluation, counts/presence/defaults, active and entry argument bindings, slot identity/permutations, status components, explicit reference/value behavior, nested calls and return copy/move ownership. Independent executable and structural controls include repeated actuals, mutable aliases and abrupt exits. |
| **INL-AC03 — full representable profile** | The canonical effect inventory, reusable procedure/point facts and checked target profile support rebinding, varargs, multiple returns, aggregate/reference lifetimes, context/cleanup and exact-bound imported helpers. Procedure summaries distinguish direct/transitive and proved/conditional/unknown effects; their use does not turn optional value precision into a structural-cloning admission gate. Every retained case has a specific semantic, missing-proof, body-availability or policy reason. Unimplemented representable categories keep this criterion open. |
| **INL-AC04 — module and observation integrity** | Globals keep their module/storage owner, including linker-selected private globals without public exposure or inliner proof metadata; initialization, poison/cycle ingress, dependencies, numeric/signal state, reference expiry, source/TRACE/diagnostic and declared physical-frame observations remain equivalent. Record the checked cross-module binding and minimal runtime compatibility contract before implementation. Cases lacking an approved equivalent stay calls. The maintained `rxpa_invocation_inline_sigret` fixture covers a checked `rxinvoke.prepare`/`invoke` target that raises inherited `SIGRET`, completes once, resumes its caller and cleans scope/policy/argument storage. Initialization and the remaining signal/ownership interactions keep this criterion open. |
| **INL-AC05 — effective bounded cleanup** | Expansion is correct with cleanup and assembler optimization disabled. Separately prove status/transport/copy elision, constants/presence, branch/reachability cleanup and alias-aware storage reuse on changed callers. Eliminate caller setup and reverse call-window swaps made redundant by direct actual-register mapping, using storage/permutation proof across the entire call region. A simple reordered-actual leaf has no original call or outer setup/restoration swaps; an alias/snapshot case retains required copies, and a retained nested call keeps a valid window. Preserve once-only actual order, result/entry/base/slot identities, abrupt-exit restoration and swaps required by body semantics. Previously optimized modules are covered without assuming their old analysis facts remain valid. |
| **INL-AC06 — deterministic policy and resources** | Separate legality, availability and profitability decisions. Explicit recursion, depth, code/register/metadata growth and work limits terminate predictably, support diagnostics and forced-legal/disabled controls, and preserve ordinary calls on policy refusal. Malformed input or failed transformation publishes no partial output. |
| **INL-AC07 — metadata and lifecycle** | Relocate executable and observation references together; keep runtime graph/provider/task/source contracts, source/module identity and initializer dependencies. Prune only under explicit reachability/consumer rules. Invalid/repeated/interleaved/resource/cancellation/I/O cases retain owned results and last-good publication. |
| **INL-AC08 — reviewed research delivery** | Early coherent implementation and representative RXAS review, focused development checks and a final compact integration panel qualify both expanded and retained cases against independent expectations. Guides, coverage/limits and this plan are current. No production-speed, full toolchain, wider platform, SAN-clean or publication claim follows automatically. |
| **INL-AC09 — shared Level L target foundation** | A checked canonical transfer/reference view, typed normal/exceptional machine CFG, reusable typed remapping, storage/component/individual-flag point and edge facts, liveness/lifetime, early entry-dependent read clear/warning, and direct/transitive procedure summaries serve independent target clients before inliner adoption. Unknown, recursion, malformed data, stale owners and resource limits never imply an absent effect or known value. Focused constructed and ordinary RXAS controls qualify the Level L algorithms against independent expectations; opcode completeness remains a separate database gate. |

| Stable step | Status, dependency and decision gate |
|---|---|
| **INL-STEP01 — architecture and contract** | done for the governing one-sweep POC design; AC01–09. Adrian approved assembler-local and link-selected placement, one shared service, exact module/private-global ownership as an outcome, checked dynamic argument views, database-driven instruction handling and bounded cleanup. The existing scoped operation/RXBIN contract remains separately recorded; it does not authorize a static private-global alias for the default direct path. Adrian approved the concrete `.inlinedprivate` borrowed-slot RXAS/RXBIN binding; its implementation and qualification remain open. Implementation and qualification criteria remain open. |
| **INL-STEP02 — shared model and conservative expansion** | first constructed-model slice done; after 01; AC01/02/04/06/07. Optional `rxfnsl-target`, L-owned frozen records/descriptors, exact binding and conservative direct scalar-return expansion under the checked closed-root signal profile are implemented and focused-qualified, using fresh identities and no copy elimination. Broader model/call coverage and cleanup remain open; this does not complete the full INL outcome. |
| **INL-STEP03 — standalone RXAS client** | initial adapter done; after the first 02 slice; AC01/02/04/06/07. Generated-L RXAS reader/writer, callable client and single-output CLI are focused-qualified with a real generated caller, independent handwritten expansion, optimized input and atomic-failure controls. Broader exact-bound imported helpers and profile/qualification remain open. Full assembler/codec/linker delivery is unnecessary. |
| **INL-STEP03F — shared Level L foundation** | in progress; after 03 and before further 04/05 integration; AC02–05/07/09. The checked target transfer/edge, remap, fact, lifetime/normalization and procedure-summary stack has focused one-VM controls. Entry-demand extraction, callback preservation, tentative clear publication, exact result binding, contextual CALL intrinsic reads, per-operand value-writer metadata and whole-op callback inventory have focused controls. Canonical CFGs distinguish intrinsic from callback control and propagate nonresource policy uncertainty along reachable paths; complete resource-only callbacks retain structural handler policy while pending interrupts and delivered handlers stay explicit. Native incoming-resource and captured normal-postwrite facts and an explicit exact-entry HALT/IGNORE policy view are additive; `BPOFF` pending-bit control is separate from structural handler policy in the catalog and consumers. Unknown writers/signal contracts, older disconnected aggregate completeness, raw-root recycled resources and call-to-logical-scope activation remain open. The ordinary native optimizer graph panel and inliner integration remain open. |
| **INL-STEP04 — complete representable expansion** | in progress; after 03; AC02–04/06/07. Scalar/metadata/batch, transactional private normalization of selected root same-module scalar counted SWAPCALL/SETTPSWAPCALL/SETTPCALL under the default-HALT profile, straight-line callee LINK/UNLINK/SWAP, one retained same-module scalar nested leaf in straight-line and acyclic branch bodies, finite LINKARG/known-passing ICHKRNG, checked runtime LINKARG with at most three choices and a straight-line scalar or bounded acyclic suffix through multiple reached RETs including a conditional retained same-module scalar CALL1 under its leaf gate, private same-module scalar globals, acyclic direct-branch scalar multiple returns, finite padded optional scalar prologues, ordinary UTF-8 string LOAD/SCOPY/whole-return and immediate-string return, ordinary binary LOAD/BCOPY/whole register return, six signed integer predicates, REQ/RNE loose equality, CONCAT/SCONCAT and register-backed typed string TRACE in the exact-bound scalar profile, exact global-free straight-line and acyclic branch imported scalar bodies with proved private locals plus one retained already-imported exported scalar child, existing public-global mapping for imported bodies, and bounded caller-owned unbound G-style signal scopes through the full 29-name catch-all, bound event capture and one guarded post-join owner projection around a checked range leaf are implemented and focused-qualified. Direct target CFG transport/validation, detached-scope closure, stable-register scoped META re-entry on direct acyclic calls and checked runtime-selector suffixes, and stable nonempty REG rN/aN/CONST runtime prefix scopes and selected LINKARG-group META/source/TRACE staging are focused-qualified. Complete broader imported, aliasing, dynamic varargs, group-local aN/gN re-entry, changing-storage metadata, recursive nesting, aggregate/reference and state/cleanup categories. Classify each hard boundary; do not leave feasible categories silently outside the finish line. |
| **INL-STEP05 — proof and cleanup** | in progress; after each coherent conservative category, with final qualification after 04; AC02–07. Shared access/liveness, canonical fact extraction, scalar transport, typed assignment/checking, per-path scalar status/storage/activation meets, bounded integer success across acyclic joins, ABI-bit presence and branch refinement, ordinary string/binary component and recycler proof with retained nonplain ENDLIFE and failure-capable binary LOAD, and the fresh-root private-scalar reuse slice are focused-qualified. The acyclic closed-root r-only graph path removes proved mapping and plain-lifetime glue, simplifies proved branches and assigns distinct reached cells densely; linear cleanup remains separately qualified. Retained branch-child calls have root graph access summaries; reached same-module counted children with one to four proved actuals now use explicit CALL1–CALL4 pointers, removing proved caller setup/inverse swaps. Reached exact same-module counts above four now use a fresh linked pointer window under the checked profile; zero, unknown and unproved implicit forms remain open. Shared backward liveness uses reverse RPO and transport query charges match bounded block suffixes; larger CLI requests can select an explicit per-stage work allowance. Runtime prefix metadata uses original typed operands, one stable StorageId ledger per root site and original incoming checkpoints through the shared bounded reconciler; argument-bank root reuse remains outside this proof. Private same-module globals retain a conservative expanded caller under explicit cleanup guards. Typed end-anchor observations, changing scoped bindings, broader control/component, alias and lifetime reuse, and checked cross-block sharing remain open. Selected fused-root preparation is preserved conservatively and its proved outer setup/restoration is removed through the existing cleanup service; mixed fused batches retain exact original owners and site decisions on refusal. Retain independent no-cleanup results; inspect simple and alias-heavy output, not only execution. |
| **INL-STEP06 — standalone research acceptance** | todo; after 03F–05; AC01–09. Freeze and run the compact agreed integration panel through the matching assembler/linker/VM profile (current tools for the unchanged contract; explicitly approved owner extensions when selected), plus CLI/service/DSL, negative/lifecycle and structural cases. Use one normal VM; broaden only for specific unresolved risks. Commit reviewed authorized paths locally; no push/default switch. Wider TGT criteria remain open. |

Per-feature checks and execution allocation follow the repository instructions;
INL-STEP06 is the named standalone integration milestone.

### INL-STEP05 scoped cleanup implementation contract

**Status: scoped cleanup increment implemented and focused-qualified; broader
INL-STEP05 and INL-AC02/04/05/06/07/09 remain open.**
This is one bounded development increment under INL-STEP05, serving
INL-AC02/04/05/06/07/09. Adapt the shared target storage proof and cleanup to
the approved logical activation, rather than make `INLENTER` an ordinary
conditional branch or admit its body through scalar mnemonic tests. The
existing runtime operations and RXBIN recipe are sufficient; no new ISA,
wire form, runtime algorithm or external API decision is selected. Necessary
additive target-analysis queries remain within the approved shared services.
INL-STEP06 and the complete INL acceptance criteria remain open.

1. **Typed recipe roles.** Extend the checked recipe view at its owner so
   consumers can distinguish count/ordered-actual capture, result mode,
   `a0`, proxy destination/argument ordinal, owned destination and ordered
   preparation. Use the generated catalogue's recipe, scope-continuation and
   binding descriptors to select these semantics; publish the existing
   `INLENTER` result and target operand roles in that catalogue without
   classifying it as an ordinary CALL. Do not add a second opcode
   table. Preserve the assembler's validation and exact capture/preparation
   order. Register enumeration alone is insufficient: an active pointee, an
   owned cell's base and its slot identity are different remapping roles.
   The Level L model retains typed `r`/`g`/`a` recipe selectors. The RXAS
   writer converts them to the existing physical `r<index>` INL1 spelling
   only with final caller-local and module-global extents; the reader inverts
   those physical capture/preparation selectors before register enrollment.
   Scope-owned destinations remain local identities. Roundtrip and cross-bank
   argument-window controls must prove that packing never grows the local
   frame or shifts global/argument bases. The native wire is unchanged, with
   no preparation copy, added runtime instruction or interrupt boundary.
2. **One scope CFG.** Consume the checked `lltargetcfg` projection, including
   its source-operation mapping and scope owner/continuation evidence.
   `INLENTER` enters the cloned body; its encoded continuation is not a
   normal bypass. A logical `RET` reaches that scope's continuation, while
   the outer `RET` departs the physical procedure. Preserve normal, resumed,
   handler, asynchronous, unwind and terminal alternatives and their signal
   phases. A response that exits a scope applies its departure transfer;
   an ignored/resumed signal retains the scope. Unknown policy correlation
   or destinations remain explicit and cannot become absent observations.
   Do not duplicate the graph or synthesize an emitted leave operation.
   The sealed `profile.closedmain(owner,rootfunction,executioncontext,work,bytes)`
   adapter binds the exact immutable owner and `main` to the explicitly
   declared `closed-main-default-halt.v1` standalone execution context.
   Its premise remains canonical; `executionroot()`/`executioncontext()`
   identify the checked premise. Only inherited foreign alternatives at that
   root become the known terminal default-HALT response; authored policy
   alternatives, unknown paths and pre/partial-entry uncertainty remain.
   Reject initializer/startup uncertainty and malformed contexts. This is a
   caller-declared standalone contract, never an inference from RXAS text or
   a context certificate for arbitrary assembler/linker inputs.
   The standalone POC may explicitly bind its caller-declared execution
   contract through a sealed `profile.closedcalls(owner,rootfunction,
   executioncontext,work,bytes)` witness. This distinct factory preserves the
   existing root-only `closedmain` contract. It checks the exact immutable
   module/catalogue and `main` root, rejects startup/runtime-observer
   uncertainty, and checks a bounded same-module direct-call closure. Only a
   closure with fully described call/control effects and no authored policy
   mutation can inherit default HALT in its reached procedures. Dynamic,
   unresolved or foreign callees, recursive closure, unknown control/effects
   and policy changes do not produce that stronger witness. Every incoming
   path in the declared root invocation must belong to the checked closure.
   The declaration supplies the HALT policy; the VM's mixed fresh default
   responses are not inferred to be all HALT. CFG consumes its checked
   `defaulthalt(functionid)` query; ordinary canonical alternatives remain for
   functions without a witness, and local/failure/asynchronous departures
   retain their canonical transfer semantics.
   The POC passes this witness through additive normalization/context APIs;
   `--profile` supplies the execution declaration, not proof from RXAS text.
   Canonical rewriting must recreate and recheck the witness for the exact
   published result before binding it to the inliner context. Stale owners,
   another module/root and ordinary assembler/linker contexts cannot borrow
   it. Normalization and finite child-window proofs consume the same checked
   operational context, with no scalar evaluator or inferred default handler.
   Independent controls cover a finite positive under an explicit witness,
   unbound canonical refusal, incorrect/stale ownership, policy/call-closure
   refusal and genuinely unknown or out-of-frame argument windows.
   An authored policy contributes only while its logical installation owner is
   in the committed scope ancestry. A synchronous failed entry has no newly
   entered owner; an asynchronous after-entry edge includes the committed
   owner. Departure restores the inherited table, so an inner installation
   cannot affect earlier entry or observations after that scope has left.
   Within-owner path/timing correlation remains conservative and explicit.
3. **Activation/departure storage facts.** At entry, apply the original fused
   preparation, capture the result and ordered actual pointers, and snapshot
   the sparse owned active/base/slot-identity bindings before establishing
   count/proxy bindings. Private locals start dirty; an argument borrow does
   not define its pointee, and an absent optional proxy retains its own
   backing cell. On a normal logical return, account for the existing
   component setter/COPY/MOVE result semantics, then restore only the owned
   bindings and end only inline-owned lifetimes. Preserve caller-owned
   pointee mutations. Abrupt departures restore the exited scopes; partial
   or failed entry has no committed scope state. Meets retain exact storage
   only when incoming bindings and scope stacks agree; disagreement is
   unknown, never value equality or an invented canonical binding.
   The canonical return-result effect must include setter side effects on
   published components, flags and payload ownership: string literal return
   invalidates facts its VM setter releases or resets. Shared flow consumes
   that descriptor rather than adding an inliner-only opcode exception.
   Availability and may-dirty entry provenance are distinct from literal
   identity. Shared flow preserves bounded ordered alias-conditional write
   summaries and resolves them against captured actual StorageIds. Unknown
   aliases/effects never prove definedness; a definite repeated-actual write
   can define the returned borrow, while a distinct dirty actual retains its
   entry witness. This reuses canonical actions and the existing flow owner,
   without a second instruction evaluator or call solver.
   Ordinary same-module return summaries preserve bounded callee-owned entry
   origins separately from mapped `aN` actuals. When a caller's typed result
   observation demands such a component, resolve its exact frozen callee RET
   point and owned entry cell, publish the entry-read witness there, and record
   a bounded dependency on that callee's normalization certificate. A caller
   certificate cannot bypass an incomplete required callee. Failure/skip
   alternatives remain in the meet; unknown effects, missing/stale source
   points, unknown origin/location and recursive or depth-32/limit boundaries
   remain explicit unknowns. This introduces no arithmetic evaluator or
   inferred actual argument.
   Transitive observation projection resolves actuals only for borrowed entry
   origins that an observation needs. A defined origin-free local/count
   observation is neutral even with an unknown actual vector; unknown aliases
   or borrowed origins still refuse proof. Ordinary canonical operations emit
   all input/context reads and typed observations before output writes; ordered
   binding recipes retain their own preparation/capture/write phases.
   Shared work accounting charges a first incoming candidate for its actual
   state copy and reserves the full meet cost for true joins. An unchanged
   incoming state skips transfer unless a subscribed callee RET-before point
   changed; explicit reverse dependencies revisit callers after such changes.
   Dirty-block scheduling avoids rebuilding stable predecessor joins. Strict
   budgets remain unchanged. Indexed-write and caller-before-callee controls
   must cover both bounded cost and later return-summary improvement.
   The frozen witness pass replays only instructions whose checked typed
   action list contains an `observe` or `call` action. The fixed-point transfer
   still processes the complete instruction stream. This selects demand work
   from the existing operational owner, without a mnemonic admission table or
   a second transfer algorithm.
   The canonical checked-integer inventory covers every add/subtract/multiply,
   divide/modulo, increment/decrement, sign change and power form together.
   Checked add/subtract/multiply and divide/modulo fail before register writes;
   successful `set_int` forms release reference/native payloads while preserving
   ordinary side components. Direct-field increment/decrement/sign-change and
   power writes preserve those payloads. Power has an explicit partial integer
   failure write because invalid negative exponents write zero, while a positive
   overflow can retain the old destination. No inliner opcode admission rule or
   arithmetic evaluator is added. Unchecked C shift domains retain unknown
   contracts; this metadata repair does not define their missing semantics.
   Normalization publishes exact per-procedure certificates through
   `change.complete(functionid)` and `change.unknownreason(functionid)` bound
   to its immutable result owner. Unproved procedures remain exact; unrelated
   incomplete bodies cannot veto a selected proven site. Inliner admission
   requires certificates for both the selected caller procedure and callee,
   preserves the exact incomplete-family reason, and never treats a partial
   module certificate as proof for an unproved body. This certifies only owned
   entry-local clearing, including its vacuous zero-local case. Canonical
   effect, CFG/signal and ABI legality remain separate admission obligations.
   **Signal-policy normalization proof (STEP04/05).** Entry normalization
   needs a sound may-read witness over every possible local CFG path, not an
   exact correlation of installed policies with paths. Its CFG certificate
   therefore requires complete possible local destinations; the independent
   `policycorrelationcomplete(functionid)` query retains policy-path unknowns.
   Preserve the same local, resumed, failed/partial, inherited, asynchronous
   and outward edges with their existing uncertainty and exposure. Flow must
   still prove observation, origin, component and binding completeness before
   certifying a procedure or inserting an owned-entry clear. Unknown event
   payloads, aliases or local destinations cannot license a clear. G-shaped
   push/branch/pop, C-shaped bound protection/relay, genuine dirty reads and
   unknown/stale/resource controls independently qualify this separation;
   no new policy solver, ISA or inliner-specific bypass is introduced.
   The canonical opcode observation descriptor treats `SIGBRV`'s bound
   register as a stored destination-slot selector, matching the VM handler;
   installation does not inspect its payload. Preserve conservative may-read
   storage/liveness effects and the later event-binding uncertainty.
   `SIGNALORIGIN` transports its complete payload without a scalar observation;
   its origin operand has an explicit known whole-structure observation.
   The signal descriptor's static names are unconditional possible alternatives
   unioned with any operand-selected name. Its `INVALID_SIGNAL_CODE` and
   `INVALID_ARGUMENTS` validation alternatives retain before-write phase and
   no explicit register writes. Requested payload/address/diagnostic context
   remains observable. Unknown event-binding values remain a separate proof.
   The focused `io-policy-shapes` integration control requires G-style inner
   policy cloning/restoration and a compact C bound relay in uncleaned/cleaned
   output. Both selected calls disappear; execution must preserve the terminal
   tag, payload and captured logical source. Unknown values remain distinct
   from complete may-entry-read provenance. The full emitted C attribute/helper
   closure and full INL-AC04 signal/call-exit coverage remain open.
4. **Whole-region transport and remapping.** Expose these source-point/edge
   facts through the target proof owner and let `llcleanup` remove proved
   caller setup and inverse swaps across the complete region. Rewrite
   executable operands, TRACE/META references and embedded recipe operands
   together, using capture versus destination/base roles. Preserve result,
   entry/base and slot identities, ordered actuals, fused preparation and
   abrupt restoration. A retained counted child keeps its exact physical
   window or the already supported proved explicit/fresh-window lowering.
   A counted outer invocation may retain a counted child when the shared
   canonical flow proves its finite count at the post-preparation capture
   checkpoint. Preserve the complete original local/global prefix and every
   argument position that window reaches, including authored swaps and fused
   preparation. A window reaching `a1` or above also requires the outer
   captured argument count to cover its last position; an unknown or shorter
   outer frame cannot be replaced by fresh proxy backing. Unknown child
   counts, unknown required outer counts and out-of-frame windows retain
   distinct keep reasons. A known count beyond this adapter's 4096-position
   bound reports `inline.child.window.argument.limit` as a resource limit,
   not a failed proof or a new semantic limit on ordinary CALL. This is
   layout proof from the existing solver,
   not a second instruction evaluator or a restriction on ordinary CALL use.
   Body swaps, aliases and snapshot copies remain whenever their semantic
   effect is required. Ordinary instructions use canonical effects and
   typed remapping; missing optional value precision does not become body
   admission, and an unknown implicit reference is never an empty use.
   Complete the shared Level L adapter for the opcode database's ordered
   binding recipes (including fused value/flag/binding order) before claiming
   general binding cleanup. A recipe that is not yet interpreted stays an
   explicit shared-proof gap; it is not repaired by adding an opcode-specific
   inliner admission or cleanup shortcut. Until an ordered recipe has a
   complete shared transfer, its unknown binding and value/component effects
   must invalidate dependent facts together so a later UNLINK cannot expose
   stale value knowledge.
   The ordered adapter validates the complete canonical recipe in a bounded
   first pass before materializing any typed references or actions. A second
   pass emits one sequence for binding changes, scalar/flag transport and
   writes, lifetime effects and addressed selectors. Immutable operation-local
   captures preserve `captureint -> unlink -> putint` and captured index order;
   a later source register resolution cannot replace the captured value.
   Fused calls prepare first, capture result/count/ordered actual pointers
   next, and apply the callee transfer last. Normal, failed and resumed paths
   consume the same phase-qualified sequence. Unsupported recipe semantics
   publish one atomic generic unknown transfer; malformed selectors and stale
   owners remain distinct errors. Addressed references whose values, aliases
   or extents are unresolved remain symbolic uses with explicit uncertainty,
   never absent effects or an opcode-specific inliner admission.
   Shared cleanup queries must derive discardability, typed placement
   footprints and captured call layout from these exact owner facts. Dead
   output removal requires complete written domains and preserved signal,
   control, context, external, ownership and lifetime effects; failure-capable
   binary LOAD or arithmetic remains executable without that site proof.
   Footprints cover explicit/implicit/recipe roles and fixed, distinct and
   window constraints; call layouts retain preparation/capture order and
   normal/abrupt restoration, selecting equivalent variants or preserved/fresh
   windows only with a proof. Missing facts, limits or stale owners cannot
   imply an empty footprint, zero actuals or a removable operation. Reuse the
   shared CFG/access/liveness/summary/remap owners; these query obligations
   remain open until independently verified, without a second flow algorithm.
5. **Access, liveness and placement.** Use the shared Level L access/liveness
   owners on the same projected CFG, accounting for captured borrows,
   result transfer, owned binding restoration, references, lifetime and
   metadata observations on every applicable edge. Possible writes do not
   kill components on failed/resumed paths. Separate an exact storage
   permutation proof from optional value/dead-definition simplification.
   Scope transfer and access share one post-preparation count query: a proved
   finite register count enumerates the same physical window as a literal
   count, while unresolved counts keep access and transport explicitly unknown.
   Initially pin scope-owned destinations and any unresolved identity or
   lifetime participant during placement; this increment need not invent a
   new cross-scope reuse algorithm to remove the redundant outer swaps.
   Re-encoding must retain every implicit recipe reference in the frame
   extent and reject role collisions introduced by placement.
6. **Bounded refusal and publication.** Distinguish malformed/stale owners
   from missing recipe-role, scope-edge, storage/alias, implicit-window,
   observation or policy-path proof and from work/entry/byte/frame/code
   limits. Keep the precise owner reason rather than relabel a scope as
   `profile.callee.cfg.pending`. Unknown facts retain the affected operation
   or region; no refusal can publish a partial cleanup. Revalidate the
   complete candidate and preserve the last valid expanded owner on failure.
   Scope omission is a separate caller-point proof and is not used to avoid
   this scoped cleanup obligation.

**Native B argument receiver identity repair.** Unqualified attributes in a
method-call argument retain the caller's receiver identity through native B
inline cloning and emission, including readonly object actuals whose value
capture is elided. Matching the receiver's class is insufficient when caller
and callee use different instances of that class. Repair this at the native
compiler receiver/capture owner and preserve indexed, scalar and typed-view
access semantics; do not require source-local snapshot workarounds. A maintained
Bag/Item control must produce the independently expected source value in normal
and no-opt modes, and a representative RXAS assertion must show the argument
access using the source receiver rather than the destination receiver. Reuse
the affected receiver/copyback controls; this does not commission broad native
qualification or close INL-STEP06.

Focused QA uses one prepared normal configuration and one VM, owned centrally
by the coordinator. Construct independent typed-recipe/scope-flow controls for
capture versus owned/base roles, logical return versus physical departure,
resume versus abrupt restoration, incompatible joins, stale inputs and tiny
budgets. Maintain a handwritten reordered-actual leaf with meaningful TRACE:
cleanup-disabled expansion executes correctly; cleanup-enabled output has no
original CALL or outer setup/restoration SWAPs and preserves the independently
expected result and caller values. Add an alias/snapshot control that retains
its required copy, a retained nested counted-call window control, and one
logical-return/abrupt-signal restoration control. Inspect the representative
RXAS early, then run only the affected target owner, cleanup and CLI checks
after source review and freeze. No per-increment sanitizer matrix, broad
functional rerun, performance verdict, publication or INL-STEP06 closure is
implied by this source-only proof increment.

**Next shared call-site/action proof sweep (INL-STEP04/05).** This is one
cohesive implementation increment serving INL-AC01–09, with final acceptance
still owned by INL-STEP06. Complete the shared source-owner interfaces below
before changing their consumers. Internal additive analysis descriptors and
queries do not change the public language/service/native callable ABI, RXAS
syntax, INL1/QGR1/QPR1 wire, RXBIN feature bits or runtime algorithms. Preserve
existing callable signatures and metadata; do not add a parallel inliner plan,
scalar evaluator, call solver or mnemonic admission table.

- **Canonical selection versus invocation (opcode database/catalogue owner).**
  `rxop_call_invokes(int opcode) -> int` and
  `rxop_call_target_components(int opcode) -> unsigned int` own activation and
  target observations. DIRECT/DYNAMIC invoke; a direct typed `P` observes no
  scalar target, while DCALL observes its integer procedure pointer, never its
  string component. Retain the existing FACTORY enum/`RxOpCallContract` ABI as
  a compatibility view, derived from one `RxOpSelectionContract` returned by
  `rxop_selection_contract(int opcode)`. That descriptor owns method and
  factory selection kind and the result/descriptor/receiver/count/window
  operand roles. The generated internal catalogue exposes
  `callinvokes(opcode=.int)->.int`, `calltargetcomponents(opcode=.int)->.int`,
  and `selectionkind/result/descriptor/receiver/count/window(opcode=.int)->.int`;
  kind comes from the owning enum, operand ordinals are one-based and zero
  means absent. Existing success-write/clear/definition and signal-phase
  descriptors describe selection's result clear plus integer pointer write;
  selection failure preserves the old result. Method receiver structure and
  factory count/argument-type/status dependencies remain observations even
  though no callee is invoked. Resolution/autoload/context uncertainty is a
  separate effect. Selectors emit typed target provenance, never call capture,
  activation or an unknown callee clobber. Audit every call-contract consumer,
  including metadata cross-checks, CFG/flow/summaries and native discovery.
- **Scoped diagnostic-chain transport (existing VM owner).** A committed
  INLENTER stores its physical entry call-site in the existing inline scope.
  Extend the one diagnostic walker to interleave active logical providers with
  physical call/callout frames, innermost outward, suppressing duplicate
  frame/callout segments and interrupt pseudo-edges. Its visitor distinguishes
  physical source/range owner from logical provider/procedure: source-step
  filtering uses the physical procedure start. Raw signal module/address stays
  physical; logical provider and source labels remain separately truthful.
  Reuse detached diagnostic count/build, COPY/SIGNALORIGIN, empty/unavailable
  and allocation-failure behavior; no live scope pointer escapes into events.
  Failed entries add no logical frame. Focused two-frame, nested and retained-
  child OTHER controls plus BRV unwind/copy/rethrow verify one PANIC, ordering,
  complete ordinary caller-chain equivalence and no duplicates. This repairs
  the approved logical activation contract through the existing walker, with
  no ISA/RXBIN/public API or second diagnostic algorithm. Once the matching VM
  walker and exact source mapping are qualified, activation equivalence can
  admit scoped cloning; default HALT is not a no-observation premise. No-scope
  diagnostic equivalence remains a separate proof/map obligation and stays
  unknown while observable asynchronous/semantic terminal paths lack it.
  New scoped-inline output assumes a VM with the corrected INLINE_SCOPES
  diagnostic semantics. This repairs the existing feature contract without
  changing RXBIN/ISA; older same-feature VMs may render incomplete caller
  context. Exact release compatibility qualification remains open.
- **Deferred public decision: guarded INL2 entry (unapproved).** The
  [owning architecture](doc/inlining.md#deferred-proposal-guarded-provider-equivalent-entry-inl2)
  records the bounded alternative to physical-frame virtualization or a
  genuinely checked closed-root execution premise. Corrected INL1 diagnostic
  walking does not virtualize parent/code observers; unknown inherited
  handlers and reentrant host hooks retain their current refusals. INL1 cannot
  acquire a silent fallback to its provider because authored clone bodies may
  differ. A new opt-in entry/recipe/required-feature decision would share the
  native original fixed/count/zero-result preparation, capture, initializer,
  failure and single mailbox checkpoints; an O(1) policy/observer guard chooses
  either a statically proved clone or the real provider frame before synthetic
  setup. The static facet consumes canonical effects and provider bundles,
  including transitive policy invariance and physical/code observations,
  without an opcode whitelist or rejecting ordinary scalar reads. Minimal
  fast/fallback/alias/initializer/recovery/mailbox/host-hook/private-global and
  cleanup controls plus simple output/compatibility checks are required.
  This is a deferred proposal only, outside the approved direct-or-retain
  effective path. No INL2 product changes precede a demonstrated cost benefit,
  exact guard/invalidation contract and Adrian's ISA decision.
  For the first guarded proof POC, the recommended semantic baseline reserves
  the real provider frame/backing once before the guard. A guard hit borrows
  that same backing for the clone and retires it through the ordinary call
  owner; a miss activates the prepared provider frame. This preserves recycled
  resources and allocation/finalizer checkpoints without claiming clean entry,
  at the cost of retaining much frame work. A synthetic fast scope is admitted
  only with exact entry/retirement resource certificates. Eager VM frame
  retirement is a separate observable semantics and performance decision.
  The Level L prerequisite is an owner-bound **call-site equivalence
  certificate**, not a blanket whole-caller normalization receipt. It binds
  the original call descriptor and provider, guarded clone, exact original
  provider slow path, shared continuation, source/candidate graph and operation
  correspondence. Outside-region operations remain identical; the fast path
  simulates the call under the checked guard, and a miss executes the original
  activation. It checks each preparation/capture/initializer/mailbox checkpoint,
  result and borrowed-storage identity, dirty-entry clears, normal and signal
  departures, and resource/finalizer effects. Existing whole-procedure entry
  coverage retains its clear/warning purpose; an unrelated later host callback
  cannot veto a locally certified replacement. INVOCATION and DEPARTURE need
  typed call/return contracts rather than automatic HOST uncertainty; actual
  native, initializer and finalizer callouts remain separate obligations.
  Existing target action/summary/scope facts are inputs, while one-to-many
  guarded-region mapping and branch-conditioned entry/exit facts are new
  shared proof work. This proof-interface design can proceed before the public
  INL2 decision, but cannot authorize an ISA or VM change.
  The separate existing counted-CALL/DCALL VM defect is repaired under the
  current call ABI: after initializer return, one checked count snapshot drives
  frame allocation, telemetry and actual-pointer binding. This removes the
  former reread after recycled callee `a0` reset could finalize a native
  payload and reenter, making allocation and binding disagree. Result and
  actual pointer capture order is unchanged. The focused recycled-a0
  finalizer regression covers direct/dynamic result and unused-result forms
  and a fixed-call control. `CALL P` remains the zero-actual form. This repair
  is independent of the pending INL2 decision.
- **Private native selector authority (implementation gate).** The transient
  `llinline.selection.v1` manifest may add a `selections` array with `caller`,
  `procedure`, `instruction_ordinal`, `graph`, `descriptor`, `provider`, `target`
  and `target_ordinal`. Owner/graph strings and nonnegative original native
  constant/ordinal/descriptor IDs are locators and diagnostics, never sealing
  evidence; descriptor zero is valid. Existing direct `bindings` are unchanged.
  Native selection authority is host-owned enrollment: the adapter itself
  calls fixed trusted L staging entries with its frozen native snapshot bytes,
  rather than accept owners parsed from caller-supplied JSON. The private L
  boundaries are `llrxinlinestage.read(snapshots=.object[])->.object` and its
  request-owned `session.canonicalize(work)` / `session.normalize()` methods;
  the existing L
  reader/normalizer remain the algorithm owners. The native host selects those
  entries by their exact owning service/library identity. A private native
  resolver pins each executable trusted PROC/META_FUNC and its exact loaded
  module/member before invocation; no context-wide exported-name fallback
  chooses the reader, stage methods, module accessors, execution or response
  reader. Imported/forwarding or duplicate/hostile owners refuse. Exact calls
  reuse RXVML's checked save/restore invocation with the receiver prepended for
  methods, and reject wrong/stale contexts.
  Before any pin or enrollment, `rxvml_inline_seal_context(ctx)` checks the
  complete explicitly loaded trusted service/runtime closure: concrete
  EXPOSE_PROC keys must agree with the existing resolved native winner,
  distinct duplicate bodies refuse, and imported declarations do not pretend
  to be providers. Pins and callbacks retain module-count/semantic-generation
  identity; later module mutation refuses rather than resealing the invocation.
  This prevents an extra runtime export from hijacking the pinned stage's
  transitive ordinary helper imports. Target RXAS remains data in this VM.
  The noninstalled C interfaces are `rxvml_inline_module_count(ctx)`,
  `rxvml_inline_resolve_owned(ctx,first_module,last_module,member_name,
  descriptor,method,&opaque_pin)`, `rxvml_inline_call_owned(ctx,pin,
  receiver_or_NULL,argc,args,&result)` and `rxvml_inline_procedure_free(pin)`.
  Module ranges are one-based inclusive: the count before a trusted artifact
  load plus one through that load's returned last module. Resolve the exact
  member within that range, not merely the last member of a linked artifact.
  Each pin verifies its original executable PROC/META_FUNC offset/signature,
  nonimport body and runtime binarySpace module, and retains exact context and
  module/file identity. Teardown invalidates the context before pins can be
  reused; duplicate/wrong-owner readers/getters never mint capabilities.
  A request-lifetime
  session retains the actual immutable raw/result/provider owners and checked
  operation maps in stable live object storage. Its fixed trusted reader is
  `llrxinlinestage.read(names=.string[],sources=.binary[])->.object`; the
  native opener supplies same-length parallel arrays from its frozen snapshots,
  and the reader validates count/pairing and total bytes before publication.
  Heterogeneous nested `.object[]` casts are not a typed transport contract. References to staging locals
  or temporary array cells are insufficient: retaining a reference cell does
  not keep an expired target alive. Only the host enrolls returned identities
  and mints an ops-stamped native-payload `.object` capability, tied to its
  active context/invocation, exact frozen native graph/member/procedure tuple,
  catalogue and checked L owner/IR revisions. B has no mint/enroll-arbitrary-
  owner procedure. Whole-value COPY preserves the capability; bytes, BCOPY,
  serialization, names and pointer-shaped integers cannot construct one.
  The private read-only native boundaries are
  `_rxlinkinline.selectiontoken(caller=reference .lltarget..module,
  selectorop=.int,provider=reference .lltarget..module,procedureid=.int,
  typeoperation=.int)->.object`
  (retrieve only an already enrolled capability) and
  `_rxlinkinline.selectioncheck(capability=.object,
  caller=reference .lltarget..module,selectorop=.int,
  provider=reference .lltarget..module,procedureid=.int,
  typeoperation=.int)->.int`. Type operation zero denotes a direct binding;
  nonzero identifies a native-enrolled receiver-type stamp candidate, never
  an asserted runtime type. Flow selects among such candidates only from the
  effective receiver's joined sealed type provenance.
  They require the adapter's live context registry, exact live owner references
  AND the trusted immutable owner/IR revision check; replacing a module inside
  the same reference cell invalidates it. An absent authority returns an
  empty/unproved result. A private context hook, not a general installed
  provider or public `rxvml.h` API, supplies these callbacks. Lower transfer
  enrollment is `profile.certifyselection(selectorop=.int,
  provider=reference .lltarget..module,procedureid=.int,capability=.object)->.int`
  before materialization, and revalidates through that checker. Its read-only
  `selectiontarget(selectorop=.int)->.lltargettransfer..selectionbinding`
  exposes valid/status/reason/provider/procedure/selection/bindingidentity to
  flow without importing the inliner. No targetfact/status constructor or
  caller boolean bypasses native enrollment. Changed owners require another
  host-driven trusted normalization/enrollment roundtrip that verifies exact
  source-to-result mapping and preserved selector descriptor/receiver/count
  roles before enrolling the new result; copied wrappers or new text cannot
  reassert an old seal. Tear down tokens/registry before session storage expires.
  Provider existence alone is insufficient. The capability also certifies
  either one canonical native SUCCESS target across every permitted runtime
  selector outcome, or an exact receiver/type/count/status specialization
  witness checked at the immutable shared input point. Reuse native method/
  factory resolution, with no second resolver. Overloads, ambiguity, autoload,
  match callbacks, native targets and rebinding uncertainty remain unproved
  unless that exact outcome theorem covers them. The first admitted dynamic
  outcome is same-graph method dispatch: the immutable native type/member row
  resolves its exact callable/provider/procedure-start identity, and shared
  flow proves the effective receiver at the selector's input still carries
  that sealed SETOBJTYPE graph/type identity with OBJECT_UNINITIALIZED clear.
  This is a distinct type-provenance fact; scalar ATTRIBUTES/status knowledge
  or lexical adjacency is insufficient (ACOPY does not copy object_type).
  Aliases, callbacks and signal paths participate in the ordinary meet.
  Unrelated loads may invalidate the native cache but rebuild the same exact
  graph/member/procedure-start mapping. Context-wide fallback is unproved.
  Factory providers are resolved from the target runtime's context-wide
  interface registry; the service VM's registry or a selected-graph singleton
  is not that runtime universe. Factory selection remains unknown without a
  separately checked closed target-runtime provider premise, including later
  loads/native/autoload additions. Flow installs this provenance
  only on successful selection; failure/ignore/resume preserves the prior
  pointer fact and participates in the meet before DCALL admission. A negative
  starts with another procedure pointer, ignores a failing selector, and must
  execute that prior target at the following DCALL; a singleton-success token
  cannot erase this reaching failure value.
  Before service selection/snapshot, existing sealed INL1/QGR1/QPR1 references
  must be rebased from their exact original image graph/member into the frozen
  selected image. The service's old QPR ordinal is not a `modules[]` index;
  ordinary relink qualification alone does not qualify this service path.
  Enrollment resolves type/member/callable tuples in a host-built frozen graph
  over the exact selected inputs. The candidate validation graph must preserve
  that semantic dispatch through exact type/member/callable/procedure-owner
  mapping; dense IDs from an old input graph are not selected-image identities.
  Same-name types/providers and reordered/overlapping old IDs have independent
  negative controls.
  Missing authority, unknown outcome, stale/mismatched owner, unproved
  normalization rebind or image rebase retains a specific
  `target.selection.authority.unavailable`, `target.selection.outcome.unproved`,
  `target.selection.owner.stale`, `target.selection.owner.mismatch`,
  `target.selection.normalization.rebind.unproved` or
  `target.selection.image.rebase.unproved` reason. This bridge remains a
  reviewed implementation gate until the private hook, stable owner enrollment
  and outcome checks exist and are independently qualified; no-token paths
  remain explicitly unknown. No native callback product edit precedes source-
  owner review of this handshake, and no new ISA/public API/wire is selected.
- **Exact target and ordered action points (transfer/flow owners).** Transfer
  remains the static checked action/reference authority. Shared flow adds
  `actionstorage(sourceop=.int,actionOrdinal=.int,refId=.int,edgeId=0)
  ->.lltargetflow..storagefact`,
  `actioncomponent(sourceop=.int,actionOrdinal=.int,refId=.int,domainId=.int,
  edgeId=0)->.lltargetflow..valuefact`, and the distinct
  `actioncapture(sourceop=.int,captureActionId=.int,domainId=.int,edgeId=0)
  ->.lltargetflow..valuefact`. Storage/component queries are before-action:
  prior ordered preparation/binding actions have occurred, while the queried
  action has not. `actioncapture` returns the immutable value produced by its
  named capture action; a path on which that capture did not complete is
  unknown. Edge zero selects the normal path; a nonzero ID must name a CFG edge
  whose source is `sourceop`, preserving before/partial/after applicability and
  weak writes. A capture ID is never interpreted as a reference ID. Build
  bounded sparse point rows in the same action executor, not full-state copies
  or replays per query. The existing `projection()`/`liveness()` remain the sole
  access authority; addressed/capture/resize/status accesses consume those
  phase bindings. Resolve finite locations through the shared storage owner,
  retaining active/base/entry/slot distinctions, symbolic reference/attribute
  paths and exact module-owned globals. Unknown aliases/paths/extents remain
  explicit uses, not empty footprints or proof of disjointness.
  `flow.analysis.targetat(sourceop=.int)->.lltargetflow..targetfact` returns
  the exact invocation target from canonical typed selection provenance at
  its capture point. Its checked `provider()->reference .lltarget..module`,
  `procedure()->.int`, `selection()->.int` and `bindingidentity()->.string`
  identify the immutable source selection and exact provider/image binding;
  they are usable only with `status()='proved'`. Arbitrary integer constants,
  equal pointer-shaped values, exported names or asserted inline body IDs
  cannot manufacture this fact. Unknown/ambiguous/native/unavailable or
  rebindable targets remain explicit. Existing sealed native selection evidence
  supplies foreign binding authority; ordinary clients cannot assert it.
  Exact foreign direct bindings carry the native-enrolled provider's canonical
  transfer profile through `selectionbinding.providerprofile()` and checked
  `targetfact.transfers()`. Request staging preallocates the provider bundles;
  `summary.useprovider(flowref,summaryref)->.int` enrolls only live canonical
  flow/summary/profile/owner identities, with exact summary.flow matching.
  Repeating the same immutable bundle is idempotent; stale/mismatched peers
  refuse. Registry construction is bounded and charged. Native target binding
  is required independently: a borrowed provider summary is not authority to
  select it. Activation composes reusable procedure-scoped provider closures,
  without rebuilding a whole foreign module for every call site.
  Staging owns one aggregate work/byte allowance across all selected modules,
  indexed instruction ordinals and native proof buckets. Stable phase-specific
  profile/CFG/flow/summary arrays are preallocated before borrowing refs and
  memoized thereafter. Until the lower normalization remap/draft transactions
  publish total counters, staging reserves disjoint conservative whole-stage
  envelopes; unavailable counters are never treated as zero and no per-module
  analysis silently renews the request allowance. Failure publishes no code.
- **Canonical type and callback boundaries.** `OBJECT_TYPE` is a distinct
  internal component from ACOPY's status `ATTRIBUTES`; whole copies/resets
  include it, scalar/status setters preserve it unless explicitly described.
  Type readers, including failure diagnostics, consume that component.
  The generic ordered `callback` action is separate from an inert opaque
  external residual. Native descriptors distinguish resolver match callbacks,
  legacy host exits and conditional source-copy/destination-release resources.
  `selectioncallbacks` describes only factory resolver match invocation;
  method-negative never proves absence of result finalizers. SAY/SAYX legacy
  exits may reenter; the newer output_writer contract is leaf, but an
  unqualified profile cannot assume that is the selected route. ADDRESS/native
  calls and payload hooks keep their actual call/ownership owners.
  Per-operand component-write knownness distinguishes a known pointer-only
  slot write from an unknown pointee write, including within a compound recipe.
  IGETUNLINK keeps both coarse written operands while its source unlink has a
  known empty component-write set; the destination writes only INTEGER. This
  cannot suppress hooks of compound operations that use an actual setter.
  ISETUNLINK likewise has only direct INTEGER transport followed by slot
  restoration, with no opaque residual or payload hook; ILOADSETUNLINK's
  actual integer setter retains its release/finalizer obligations.
  Capture conditional top-payload/active-child resource witnesses at the exact
  BEFORE-action point with existing immutable capture IDs. A final callback
  action (may/all/unknown phase) consumes that captured witness; only proved
  absence discharges it. Otherwise forget value, binding, lifetime, context
  and scope facts on normal and exceptional prefixes, after ordinary writes,
  so a mid-COPY hook cannot be followed by falsely restored normal facts.
  Unknown physical child capacity is an unconditional unknown boundary;
  logical child-count zero does not prove inactive capacity has no hooks.
  `flow.callbackpossibleat(sourceop,actionOrdinal,edgeId=0)->int` is the
  single cached condition owner for executor, access and summary; zero means
  a proved no-hook before-action witness, and every invalid/unknown case is
  possible. Return/copyback and scoped entry/departure have hidden ABI
  resources; absent an exact owned-backing/caller-result checkpoint they
  retain an unconditional callback boundary rather than infer no effects from
  the lack of explicit output operands.
  Current hidden-ABI fallback is deliberately conservative: ordinary
  RET0/immediate can still release owned backing, register return can copy or
  move, and string return can finalize the caller result. Until a shared
  complete backing/result no-hook witness discharges those obligations, this
  may retain otherwise pure calls and is an open emitted-quality blocker,
  not evidence that the plain-leaf no-scope criterion is complete.
  Closure requires a shared site proof of every callee backing tree and old
  caller result resource at the exact RET checkpoint, or an exact replacement
  setter/finalizer-order equivalence; then inspect structural plain-leaf RXAS
  under INL-AC05 rather than treating the conservative barrier as completion.
  Missing compound/scope checkpoints likewise cannot use an unrelated active
  cell's absence. Callback access is explicit incomplete observe-all until a
  checked extent/effect summary exists, with no liveness kills. Callback-capable
  factory signals are unknown; callback-free method lookup retains its
  precise before-result failure rule. This is one canonical action executor,
  not factory/SAY dispatch in flow or an opcode-specific inliner refusal.
- **Hidden ABI descriptor (native catalogue owner; implementation in progress).**
  Replace the coarse ABI/departure booleans as proof inputs with a small
  `RxOpAbiContract`/`RxOpAbiStep` table describing existing VM helpers, not a
  second action language. A row carries native enums for operation/checkpoint,
  source/destination storage role, source/destination tree extent, conditional
  rule and repetition. Roles distinguish current active return source, its
  exact original owned backing, captured result, recipe/scope saved backing,
  physical-frame backing and external argument source/destination. Extents
  distinguish top payload, active tree and full retained owned capacity.
  The finite rules cover result-present/ignored, same-storage no-op,
  owned-original-base MOVE else COPY, committed scope and external-root args;
  they are native semantic enums, never client-supplied predicates or recipes.
  Proposed generated queries are `abistate(opcode,mode)->string`,
  `abisteps(opcode,mode)->int` and per-row
  `abioperation/abicheckpoint/abisource/abidestination/abisourceextent/
  abidestinationextent/abirule/abirepeat(opcode,mode,row)->string`.
  Mode is entry, physical-return, scoped-return, scope-leave or frame-unwind;
  generated names map the native enums. Capture-before-init uses the separate
  validated-stage rule; successful-entry applies only after initializer ingress.
  The owned-begin through owned-end rows are one repeated body per slot, not
  separate whole-array loops. Physical value-return numeric/plugin sync precedes
  physical retirement; void RET has no explicit decimal-sync row in this VM.
  Unknown mode/family/row is explicitly
  unknown, never an empty no-effect sequence. Existing component/flag setter
  masks remain the sole field-effect authority. No ISA/RXBIN field changes.
  Entry maps existing typed preparation, then stage capture before initializer
  ingress, then successful commit's `clear_value` of each saved owned backing
  (full capacity), a0 setter and proxy publication; failed entry has no new
  scope. Partial/reentrant commit is unknown unless its checkpoint is resolved.
  RET result publication precedes departure: integer/float use direct fields;
  string uses `set_const_string`; register return uses MOVE only for that exact
  owned slot whose active pointer equals its original base, else COPY. COPY
  may invoke source hooks and touch destination capacity; MOVE destroys the
  complete destination tree without source copy hooks. Same-storage means
  exact current native source/destination pointer equality at the pre-transfer
  checkpoint; it suppresses only the COPY/MOVE result row. Ignored results
  likewise omit only result publication, never subsequent cleanup/restoration.
  Common scope-leave rows repeat per checked exited scope, innermost first:
  first clear/release the local signal table/stack and restore saved signal
  policy, stack and native interrupt modes; next release/clear each saved
  original backing at full capacity and restore its slot in the actual
  per-slot order; then restore numeric context and synchronize the decimal
  plugin; finally pop/discard the scope. These are distinct ordered checkpoints:
  a cleanup finalizer can observe restored policy before numeric restoration.
  This template also serves signal return/branch/unwind edges; pre/partial
  entry cannot invent a committed scope. Physical `free_frame` instead retains
  reusable local storage and retires reference/policy/frame state; it is not
  `clear_value`. External-root modes stay distinct and unknown until represented:
  ordinary RET copyback and argument free repeat using live base-a0; terminal
  `free_external_entry_arguments` cleanup repeats using `frame.number_args`;
  signal RETURN frees arguments without ordinary result/argument copyback.
  Unmapped abrupt/plugin restoration obligations remain unknown. Generic L
  consumption uses existing typed recipe,
  prepared capture, saved scope base/result, active source and exact CFG exit
  identities; it needs sparse pre-ABI checkpoints and storage/generation-bound
  full-capacity absence before replacing the conservative final barrier.
  Native/catalogue implementation is approved under this bounded contract;
  shared checkpoint consumption and focused qualification remain in progress.
- **Owned backing resource induction (shared flow owner).** A procedure's
  first physical frame starts with `value_init` owned backing. Reused frames
  need no new VM clean-entry premise when every reachable canonical action and
  CFG path preserves absence of reference/native payload and the entire owned
  child tree, including retained capacity. The shared flow theorem is bound to
  the immutable module/revision, catalogue, transfer profile, CFG and checked
  activation context. It seeds only those resource absences; ordinary scalar
  components remain dirty/unknown. Exact native write/clear/transport masks,
  typed actions and owned-set binding closure prove preservation; conservative
  ALL may-write masks are not release/creation semantics. An absent precise
  write mask is unknown unless `writecomplete` or exact checked pointer-only
  action evidence proves preservation; zero never means no resource effect.
  Borrowed resources,
  escaped mutation, unknown imported effects, returning handler/host mutation,
  unknown edges and unresolved call/SCC obligations retain named refusal.
  Reentry that allocates a distinct frame is not intrinsically unsound; the
  proof must instead account for every route that can mutate a cached or active
  owned backing. The first bounded implementation accepts checked internal
  members of the existing acyclic closed-call context with complete paths and
  no unresolved resource/callback/escape effects. Other invocation contexts,
  including raw root external-argument cleanup, remain unknown. Owned rN entry
  absence and external-argument departure are separate: a future checked first-
  physical-root premise may prove newly allocated owned entry slots without
  proving raw-root departure. Existing closed-main/default-HALT profiles do not
  carry that fresh-root authority. Scalar borrowed reads may preserve the
  invariant; resource transport, binding, writes or escape through arguments
  remain unproved. The initial theorem requires every referenced aN to be
  present on all incoming canonical fixed-actual vectors; counted/unknown
  ingress retains an argument-presence refusal, using no second count evaluator.
  Writes/addresses of a0 or materialized owned tails likewise remain unproved,
  rather than silently omit their backing. This is a
  bounded sufficient theorem, not a body-admission list or a second evaluator.
  `flow.resourceclean(functionid)->int` and `resourcereason(functionid)->string`
  expose that owner-checked result. The same theorem supplies entry resource
  cells and conditional callback/access/site proofs; private clean departure
  is distinct from RET_REG/string caller-result copy/finalize and scoped owned
  backing checkpoints. It never discharges those independent obligations.
  A fresh standalone no-host-hook context would additionally have to prove that
  no reachable native/plugin installer or imported callback changes the hook
  route; default HALT, `main` spelling and current CLI hook absence alone are
  insufficient. That remains an explicit context gate. Independent controls
  cover a scalar internal leaf, owned binding transport, borrowed resource and
  host/unknown-edge refusals, dirty scalar availability, limits and stale owner.
- **Bounded original-backing entry/recycle follow-on (design; coordinator
  review pending).** This shared INL-STEP05 extension serves
  INL-AC02/04/05/06/07/09. Admit resource facts before a retained counted call
  in a checked internal closed-call member without introducing a public root
  profile, fresh-root assumption, VM clear, or different frame lifecycle.
  A clean source prefix alone is insufficient: a later descendant can leave
  payloads in recycled backing for the next invocation. The proof therefore
  validates a resource-only entry/recycle invariant for each exact original
  private backing, including retained child capacity, before publishing its
  entry seed. It need not assume every cell stays clean throughout the body;
  every path that can recycle that backing must reestablish the invariant.
  Ordinary scalar entry components remain dirty/unknown.
  Flow owns a bounded outward resource-preservation/no-escape relation for
  exact acyclic same-owner callees, composed from the canonical typed actions,
  native ABI rows and existing CFG/state/worklist. It instantiates the existing
  post-preparation result/count/ordered-actual captures, original backing and
  lifetime identities, repeated actual aliases and hidden return/departure
  resources. Same-module initializer timing and synchronous failure/skip paths
  remain explicit. A general component may-effect, AliasWrite definedness
  summary or absent callback row is not this resource relation. No second
  scalar evaluator, copied call resolver or mnemonic admission list is added.
  Every possibly mutating/reentrant route participates: unknown target or
  extent, foreign/native effect, alias or escape, returning handler, callback,
  resource checkpoint, physical observer, recursion, incomplete path or limit
  retains a named refusal for the affected backing. Unknown reentry may not be
  ignored because it occurs after the queried prefix. Prove result setters,
  native/plugin synchronization and COPY/MOVE/tree obligations independently;
  a private-backing proof grants neither scope omission nor signal/diagnostic
  equivalence. Full activation and transport uncertainty remains intact.
  Before-point facts and immutable callback captures come from the same flow
  executor and capacity ledger only after the entry/recycle proof stabilizes;
  provisional hypotheses never escape to cleanup. Keep existing
  `resourceclean/resourcereason` meaning and resource/ABI query validity.
  An entry-only/per-backing theorem must not enable a whole-procedure no-hook
  shortcut; each hook consumes its exact before-point resource proof.
  Facts/cleanup must borrow the exact owner/revision/profile/CFG/flow context
  for the internal member, rather than reconstructing that member as MAIN.
  The additive internal borrowing interface must be frozen with its owners
  before product edits. All call relations, validation, caches and retries use
  the existing cumulative work/byte allowance and stale-owner checks.
  Independent positives use a zero-owned root that invokes an internal helper
  twice; the helper prepares counted 0/4/5 layouts and retains an exact local
  leaf. Preserve captured count kind, result behavior, repeated/ordered actual
  identities and fresh-window placement or original zero-count form. Check
  resource absence separately from dirty scalar entry. Negatives let a later
  child dirty/escape an actual or result before frame reuse, use unknown or
  host/returning-handler effects, hide backing through LINK/SWAP, and exercise
  result/actual alias plus RET_REG/string obligations, stale owners and tiny
  limits. A first-invocation-only positive cannot replace recycled-entry proof.
  The separate public fresh-root/no-host-hook profile decision remains open;
  this follow-on neither supplies nor infers that authority. Implementation
  and focused qualification remain pending coordinator review.
- **Entry/recycle additive owner interface proposal (freeze pending).**
  Keep existing factories/procedures and their callable signatures unchanged.
  The proposed checked borrowing entry points are:

  ```text
  lltargetfacts.analyseflow(
    points=reference .lltargetflow..analysis,
    functionId=.int,
    work=1000000, bytes=67108864) -> .lltargetfacts..analysis
  llcleanup.transportfacts(
    facts=reference .lltargetfacts..analysis,
    work=1000000, entries=1000000, bytes=67108864,
    frame=65536, cells=1000000) -> .llinline..change
  lltargetflow.analysis.callresourceat(callsite=.int, edgeId=0)
    -> .lltargetflow..callresourcefact
  ```

  `analyseflow` retains the exact live flow reference and its original owner,
  revision, canonical premise/catalogue, profile and CFG identities; it checks
  the requested member against that execution context and never changes the
  execution root. It owns one cached summary whose `flow()` is that exact
  reference. Existing `siteflow()` returns the borrowed reference; additive
  `analysis.function()->int` identifies the checked function view. No solver or
  module wrapper is copied. `transportfacts` borrows that exact facts view,
  revalidates its owners and retains existing full access/transport gates.
  Mismatch, replacement or expiry refuses; rewritten publication requires a
  new matching proof before any later edit. Limits are ceilings on the same
  enclosing facts/request ledger: charge built owners once, subsequent lazy
  flow/summary deltas and cleanup work once, never reset per helper/query.
  `callresourcefact` has `valid/status/reason/points/operation/edge` and
  `preserves(storageId=.int)->int`. It is published only by the flow owner for
  one canonical invocation's normal path (`edgeId=0`) or exact owned CFG edge.
  `proved` means a complete resource relation, not that every backing is clean;
  `preserves` is one only for a checked original backing whose resource
  absence/lifetime/no-escape obligation survives. Unknown, missing, stale or
  wrong-source/edge storage returns zero. A supplied StorageId/status or fact
  constructor cannot enroll proof. Use existing `targetat` authority, captures
  and aliases within the bounded acyclic same-owner closure; the same executor
  validates entry/recycle candidates before exposing point/ABI facts.
  Call-entry decimal `syncNumericContext` and value-return parent sync remain
  possible callbacks. Default HALT, an empty leaf/RET_VOID or service VM plugin
  state supplies no target-runtime theorem. Missing canonical no-hook or
  inaccessible-backing proof stays unknown, with no caller boolean/string
  bypass. Add wrong-member/context, stale/replaced wrapper, wrong-edge and
  cumulative lazy-budget controls to the preceding tests. These signatures
  await coordinator/owner freeze; implementation remains held.
- **ABI callback point fact (implementation; local QA pending).**
  `flow.abicallbackat(sourceop,edgeId=0,?invocation)->abicallbackfact` aggregates
  the native owner's ordered ABI resource rows for one normal/CFG-edge path.
  It validates the exact flow/module/revision/profile/catalogue, source/edge
  ownership, active scope snapshot and lifetime identity; `valid/status/reason`
  and `possible()` distinguish no hook from unknown. Resolved read-only rows
  retain native checkpoint, resource role, operation, tree extent and StorageId.
  ABI row ordinals are not transfer-action or reference IDs. The native typed
  descriptor rows and the shared owner signatures are coordinator-reviewed.
  `callbackpossibleat`, access, summaries and site proofs consume this one
  fact. Edge consumers also consult it when a non-RET instruction unwinds
  scopes; each exact `scopeexits` entry applies the same leave plan inner first.
  Existing recipe roles, `preparationat`, prepared capture points, immutable
  activation result/actual vectors and saved owned-base snapshots identify the
  resources. Return source uses the live active pointer at RET, after body
  mutations; captured result identity does not follow a rebound result token.
  Physical RETURN chooses MOVE only at its exact original local backing;
  scoped RETURN uses owned membership and that activation's saved backing.
  Missing bindings/aliases/scope identity remain unknown.
  The executor publishes sparse before-ABI checkpoints, preserving ordered
  preparation, capture, initializer ingress and commit, then return transfer,
  and each exact leave. Leave restores saved signal policy first; for every
  saved owned slot it releases the reference lifetime, clears that backing,
  and restores that slot before proceeding to the next. Numeric context and
  plugin synchronization follow those per-slot actions; the scope is popped
  last. Resource and observer facts use those individual checkpoints, not one
  final restoration point. Failed entry has no committed snapshot; partial
  preparation retains its possible prefixes. Current coarse scope action rows,
  post-unwind `componenton`, and physical `entrycomponentat` cannot substitute
  for the saved owned backing immediately before commit or leave. The minimal
  new resource evidence is StorageId/point/lifetime-bound absence of the full
  owned tree including retained capacity; existing `resourceclean` is one
  sufficient witness for the exact original backing it covers, not for an
  arbitrary active rN or mutable base. No native ISA/component is selected.
  Exact current native value-pointer equality at the pre-transfer checkpoint
  suppresses only that COPY/MOVE transfer row. It does not suppress later
  backing cleanup, signal/numeric/plugin restoration or scope departure.
  Otherwise COPY requires its native
  source/destination payload conditions and active child-copy/reset obligations
  (the proved zero/zero child-count case is distinct from capacity). MOVE's
  destination destruction requires full retained-capacity absence. A string
  setter uses its exact old-result release conditions and retains all ordinary
  setter effects. Caller-result copyback, private backing cleanup, external
  argument copy/free, context/plugin restoration and callback timing remain
  distinct rows and proof obligations. External-root ordinary RET uses its
  live base-a0 count/copyback path; terminal cleanup uses the frame's saved
  argument count, and signal RETURN does not perform ordinary return copyback.
  These paths stay unknown unless their exact native row sequence, live extent
  and resources are modeled; one root-path fact cannot certify another.
  Unknown resource, initializer, callback,
  handler or phase preserves the conservative boundary on every affected path.
  No-hook proof alone licenses neither scope omission nor physical-frame
  equivalence, a fresh-root/no-host premise, or changed VM lifecycle semantics.
  Cache construction and row queries share the existing work/byte allowance;
  no replay or full-state copy per query. One transient path cursor accompanies
  the existing transfer and scope-departure execution; after convergence the
  same executor republishes its authoritative normal/edge rows from frozen
  incoming states. Reset preparation/capture rows on each authoritative normal
  attempt. Public queries read those sealed ledgers, not transient stage caches
  or a second reconstruction of phase/scope exits. A logical return consumes
  only its matched exited-scope prefix; subsequent async/unwind exits remain
  ordered obligations. Physical frame unwind is unknown until its native plan
  is represented. Contextual resource substitution preserves this row order
  and requires exact caller capture, provider/premise, non-mutating borrow
  provenance and bounded acyclic activation identity; private StorageIds alone
  cannot equate recursive physical frames. Focused controls cover mutated/rebound
  actuals, saved backing hidden by LINK/SWAP, same-storage transfer, COPY/MOVE
  tree/capacity distinctions, string side components, failed/partial entry,
  normal and signal leave, external-root refusal, tiny limits and stale/wrong
  edge owners. Existing structural positives remain the acceptance oracles.
  Exact consumed-inner/remaining-outer leave traversal qualification remains
  open: the current nested async-return control proves conservative refusal
  and the terminal alternative, while unknown root resources prevent it from
  independently reaching the full ordered leave-row path. A checked entry
  resource premise is required for that positive checkpoint control.
- **Intrinsic effects and ambient continuation cleanup.** One sealed ABI
  path ledger retains the instruction's intrinsic prefix separately from an
  ambient continuation suffix. The CFG owns `edge.asynchronous()->int`; only
  its canonical signal producer's asynchronous-origin bit can mark newly
  executed response/scope/frame cleanup ambient. Phase, exposure spelling,
  terminal role and signal names are insufficient. Entry/return/normal ABI
  rows already executed by the instruction stay intrinsic. Synchronous failure
  cleanup and unknown origin remain intrinsic, including same-phase/name
  controls. Shared fact queries `intrinsicstatus`, `intrinsicreason`,
  `intrinsicpossible`, `intrinsicrows` and `intrinsicrow` expose the independently
  proved intrinsic slice under the same owner/lifetime. Full ABI facts,
  outward effects, activation and transport preserve every ambient obligation.
  Discardability's semantic `safeifdead` and movable instruction footprint may
  use the intrinsic slice; actual removal still requires complete shared
  all-output liveness. Access publishes known opcode-local rows while unknown
  continuation observations explicitly pin the affected function's liveness.
  This applies to unknown synchronous and asynchronous cleanup, without
  dropping either path. Full-state equivalence is not inferred from an
  intrinsic permission. A function-wide continuation pin is a named current
  conservative limit: canonical async physical cleanup can still prevent
  register/swap removal until a bounded observation footprint or checked
  execution/resource premise proves a narrower constraint. Existing cleanup
  positives and full initiative acceptance remain open. Focused controls pair
  certified async versus synchronous cleanup, known local rows versus pinned
  full liveness, semantic discard permission versus no actual deletion, and
  unchanged full ABI/activation refusal.
- **Contextual activation ABI composition (shared summary owner).** Cache the
  generic per-procedure remap/CFG/policy/host/observer proof separately from
  bounded pending native ABI source/edge obligations. A generic procedure fact
  with pending ABI obligations is not a final activation permission. At an
  exact `activationequivalence(callid,scoped)` request, derive the caller's
  checked `flow.invocationat(callid)` and resolve each provider obligation via
  `flow.abicallbackat(sourceop,edgeId,invocation)` from the same executor.
  Cache only the final exact callid/mode decision; retain invocation, target,
  provider and ABI fact lifetimes. No per-tuple cache or second evaluator.
  Nested callees require their own checked invocation, with existing cycle,
  depth and work bounds; the outer caller cannot supply a descendant's result.
  Invocation facts may be proved relative to an exact explicitly supplied
  independent operational profile, as other reusable L facts are. They retain
  the exact transfer profile/premise/owner and checked target/capture/binding
  identities; no omitted native effects are inferred from that premise. The
  canonical CALL inventory deliberately defers `catalog.call.callee.unknown`
  until exact target facts are composed. Invocation construction accepts only
  that named canonical deferred row after checked direct-target identity,
  capture descriptor, references/bindings/order, reached captured actuals and
  bounded callee recursion checks; other unknown or independent call rows
  retain their veto. This supplies capture identity, not callee-effect purity
  or retained-caller unwind equivalence. Focused canonical positive and
  independent lookalike/unknown negatives qualify this narrow composition.
  Native inliner admission separately requires the canonical native profile, so a
  relative fact cannot certify RXAS execution or manufacture native binding.
  Pre-call result absence is insufficient when a callee can mutate an alias:
  the ABI owner must supply current pre-RET resource/identity evidence or unknown.
  Outward summaries remain unknown without that invocation-specific evidence.
  Summary, discardability and footprint consume exact scope-exit ABI edges even
  when a non-RET instruction has no callback action. Unknown edges/resources
  never become an empty footprint or no effect, and all lazy provider costs
  remain within the existing composed allowance.
- **Independent reusable site facets (summary owner).** Add
  `analysis.discardability(sourceop=.int)->.lltargetsummary..discardfact`,
  `analysis.footprint(sourceop=.int)->.lltargetsummary..footprintfact`,
  `analysis.calllayout(sourceop=.int)->.lltargetsummary..layoutfact`, and
  `analysis.scopefree(sourceop=.int)->.lltargetsummary..scopefreefact` to the
  existing summary analysis over shared flow. The independent
  `analysis.activationequivalence(sourceop=.int,scoped=1)
  ->.lltargetsummary..activationfact` supplies `valid/status/reason/allowed`
  for BOTH scoped and scope-free cloning, caching the modes independently.
  Mode one requires the native-owned `catalog.scopeddiagnostics()` capability
  for the corrected matching INLINE_SCOPES VM plus complete source/remap proof;
  mode zero cannot borrow that runtime scope. Scopefree queries mode zero.
  It traverses the exact callsite and
  checked bound closure CFG, retaining unknown and asynchronous alternatives.
  A semantic or asynchronous terminal event that can observe the removed
  ordinary caller chain requires native diagnostic equivalence or a proved
  no-observable-terminal premise; default HALT alone is insufficient. Current
  physical-only diagnostic walking is not repaired by retaining INLENTER.
  Without that proof retain the original call with
  `target.activation.terminal.diagnostic.unproved`, rather than substitute a
  body-opcode list or caller assertion. Scopefree also requires this facet.
  Every facet supplies
  `valid()->.int`, `status()->.string` (`proved`, `unknown`, `invalid`, `limit`)
  and `reason()->.string`, bound to exact owner/catalogue/profile/CFG/binding
  identities. Facets are independent: an unknown return literal cannot poison
  otherwise complete placement, layout or activation evidence. Queries on
  all instructions support discardability/footprint. Layout covers canonical
  invocations and approved scope-entry recipes, consuming their existing
  post-preparation invocation or scope-capture checkpoint; it never invents
  another count/capture algorithm. Target and scopefree remain invocation
  facets, reporting `unknown` with `target.site.not.call` otherwise. Procedure
  direct/transitive and outward summaries instantiate the same action-point
  bindings, rather than
  widen after a prefix LINK or interpret opcodes again.
  `discardfact.outputs()->.int` and `outputdomain(ordinal=.int)->.int`
  enumerate every written storage/component/flag domain using the exact shared
  access projection's domain IDs. `safeifdead()->.int` is the semantic predicate
  proving every residual signal/control/context/external/ownership/lifetime/
  observation effect harmless if those outputs are
  dead. The consumer separately requires existing shared liveness to prove
  ALL enumerated output domains dead on every applicable edge; semantic safety
  contains no implicit liveness assumption. The generated
  `residualobservable(opcode)` is a conservative obligation to prove residual
  safety, not an unconditional discard veto. `opaqueeffects(opcode)` and the
  typed `external`/`catalog.opaque` action retain observable opaque behavior,
  including literal output with no scalar references. Existing typed clear/
  lifetime actions and canonical component-clear masks describe conditional
  ownership release: shared before-action absence of reference/native payloads
  can discharge it, without a LOAD or other mnemonic exception. Conditional
  signals require complete applicable-edge/success proof; unresolved effects
  remain unknown. These predicates never gate structural cloning. It does not
  evaluate arithmetic or
  drop allocating/failure-capable operations because their value is dead.
  `footprintfact` enumerates typed roles and fixed,
  distinct and window constraints, including implicit/recipe refs, captured
  borrows and active/base/entry/slot identities; repeated pointee aliases do
  not collapse distinct argument ordinals or slots. Its `roles()->.int` and
  `role(ordinal=.int)->.lltargetsummary..rolefact`, plus `constraints()->.int`
  and `constraint(ordinal=.int)->.lltargetsummary..constraintfact`, enumerate
  that checked footprint; an incomplete fact never masquerades as empty.
  Cleanup preserves connected physical binding components containing scope
  identity, external storage or a nonerasable compound action; their operands
  remain slot-relative. Only complete pure bindings outside those components
  may disappear under the all-edge region certificate. Facts owns one cached
  canonical summary/liveness for site domains; legacy scalar IDs never mix
  with canonical output IDs, and lazy cost deltas are charged before publication.
  Canonical `catalog.callvariant(opcode,count)->.int` returns an equivalent
  existing invocation opcode or zero when unavailable, preserving target/result
  kind, preparation and initializer/capture checkpoint. Counted-to-fixed forms
  crossing that checkpoint remain unavailable absent a separate shared site
  theorem; preserve or construct the proved physical window instead.
  `layoutfact.countknown()->.int` distinguishes a known zero from unknown;
  `countkind()->.string` (`fixed`, `captured`, `unknown`) and
  `resultkind()->.string` (`absent`, `ignored`, `captured`, `unknown`) distinguish
  typed absence from an unknown binding. A fixed count has no count StorageId;
  an ignored/absent result need not supply a result StorageId, and neither
  absence poisons a proved layout. When several prerequisites are unknown,
  retain the first causal refusal in deterministic proof-check order; a later
  result/actual binding absence must not overwrite an earlier count/ingress
  failure. Resource-limit refusal still takes precedence over a partial fact.
  `count()->.int`,
  `result()/countstorage()->.lltargetflow..storagefact` and ordered
  `actual(ordinal=.int)->.lltargetflow..storagefact` expose the existing captured
  count/pointers, physical window and preparation/capture/ingress phases, plus
  per-CFG-edge normal/abrupt restoration obligations. No separate count
  algorithm is added. `scopefreefact.allowed()->.int` is a separate caller-point
  permission: every applicable normal/failure/asynchronous response must be
  equivalent without logical activation, with no inherited SIGRET/unwind,
  frame/parent/runtime observer, policy/numeric restoration, argument-view,
  initializer ingress or reference-expiry obligation requiring it. A pure body
  or absence of authored policies alone is insufficient. Retain INLENTER with
  the precise residual reason whenever that permission is unproved; scope
  omission never becomes a prerequisite for already representable cloning.
  Scope omission also certifies return mapping. No-scope integer/float literal
  returns use a fresh private temporary LOAD followed by ICOPY/FCOPY into the
  result, preserving its unrelated components, flags and reference/native
  payload; direct LOAD into that result is not equivalent to RET_INT/FLOAT.
  Charge the extra temporary and emitted cells. Whole-register returns require
  exact owned-unlinked MOVE versus borrowed/rebound COPY evidence, not a bank
  name alone. `scopefreefact.returnmode(sourceReturnOp=.int)->.string` reports
  `copy`, `move` or `unproved` under that exact site certificate; the consumer
  freezes each needed mode while its charged shared proof owner is live and
  retains scope if any mode is unproved. String literal returns retain scope
  until their setter's exact
  effect equivalence is proved. Independent side-component and native-payload
  controls qualify these return-family rules before zero-context admission.
- **One consumer path (inliner/cleanup and native boundary owners).** CLI,
  callable and assembler/linker selection discover invocations through the
  canonical invocation/target fact, including proved exact-bound dynamic calls.
  Preserve executable selection and its failures; remove only the invocation
  whose exact target was proved. Existing `llinline.context.scopefree(binding)`
  delegates to the shared facet and validates the selected provider/image and
  normalization stamps. `llcleanup` and any legacy fact adapters consume the
  same access/summary/site facets for transport, discardability and placement;
  replace global/exceptional or opcode-family blanket guards only when their
  corresponding owner proof is complete. Layout chooses descriptor-equivalent
  CALL1–4, preserves an exact counted window, or creates a proved fresh linked
  window. Known zero retains an equivalent existing form unless the catalogue
  supplies a valid variant. Preserve body-owned swaps, snapshot copies,
  result/prep overlap, side components/flags, module storage and every departure;
  an incomplete unrelated function or optional value fact cannot veto a proven
  site. Unknown-length scattered tails retain the accepted existing-ABI
  boundary and the exact child/outer-count/window reason recorded above;
  finite representable refusals remain open implementation work, not exclusions.
- **Initializer and sealed-image ownership (existing VM/linker contracts).**
  Preserve each original call kind's capture timing around initializer ingress:
  counted CALL captures after ingress, fixed CALL1–4 before it, while INLENTER
  stages pointers/count before ingress. Prove equivalence for the admitted site
  before converting list/window representation or removing preparation; an
  initializer-time/callback ingress gap retains an exact capture-phase reason.
  Do not infer equivalence merely because normal startup initializes modules
  before main. Failed/poisoned/cyclic entry cannot commit a scope or execute
  the clone body. Preserve the original call's canonical response, including
  its caller continuation on ignore/skip/resume, branch/unwind/terminal
  alternatives and preparation phase. A resumed rejected INLENTER must not
  fall into its physical next instruction inside the unentered clone. Ordinary
  relinking of existing sealed output uses RXBIN007 unchanged:
  typed INL1/QGR1/QPR1 references contribute exact
  provider dependencies, resolved by `(original image graph, old member)` rather
  than export name or a new ordinal. Precompute the complete old-member-to-final
  selected-module mapping before rewriting, remap provider procedures through
  their original owner, preserve self-relative QGR, and distinguish overlapping
  ordinals in separate input images. Validate recipe/P agreement before
  resealing; malformed mismatches are rejected, never repaired by relocation.
  Selection, graph rebasing and publication retain one atomic native owner.

Implementation order is canonical descriptors and independently checked action
points; shared target/site/summary facets; inliner/cleanup/native adapter adoption;
then one focused normal-configuration/one-VM interaction panel. Coordinator review
of these interfaces precedes product edits. Source ownership is opcode metadata/
catalogue and native linker selection/rebasing at their existing native owners;
`lltargettransfer` for static actions; `lltargetflow` for point/edge/capture storage
and target provenance; `lltargetsummary` for site/procedure proof facets; and
`llinline`/`llcleanup` for consumers. No lower owner imports the inliner to resolve
its own binding; do not introduce dependency cycles or a second algorithm.

Independent controls cover integer-target versus string-target observation and
selector success/failure without invocation; exact dynamic selection plus
ambiguous/rebound/native negatives; capture→UNLINK→consumer and captured address
indices; finite/unresolved LINKARGV; flag/side-component preservation and dead
arithmetic with observable overflow; repeated actuals and counted children of
0/4/5 arguments with prep/result overlap; and all-edge outer-swap removal while
body swaps/snapshots remain. A sealed pure-leaf positive must show no original
CALL, INLENTER, LINKARGV, QGLOBBIND or redundant outer swaps when the complete
scopefree certificate permits it; the same body without that context and under
inherited SIGRET retains activation. Private/global/exceptional interactions
cover normal and abrupt exits, scoped ignore/SIGCALL, numeric restoration,
complex return ownership, once-only initializer state and failure poison/cycle.
Ignored initializer FAILURE and rejected-count OUT_OF_RANGE controls must resume
at the original caller continuation, never run an unentered clone, and preserve
preparation/caller state; branch-only failure controls do not qualify resumption.
Cover ordinary relink with private typed dependencies and colliding image ordinals.
A relink prepends an unrelated rooted module so provider ordinals change and a
root shadow procedure cannot capture a private target; independently inspect the
new QPR/QGR operands. Two images with old ordinal zero must remain distinct.
Missing providers, wrong procedure ordinals, unsealed foreign references,
recipe/P disagreement and feature loss reject atomically without resealing an
invalid old identity into a valid one.
Reuse the maintained checked-invocation scoped-SIGRET fixture and its request
completion/cleanup oracle; native ordinary scoped SIGRET alone is insufficient.
Stale/wrong-owner/wrong-edge, malformed binding/recipe, tiny budgets, cancellation
and failed native service/publication must retain exact owners and last-good
output. Review representative RXAS with cleanup disabled and enabled; reuse
unchanged green controls and qualify only affected owner/boundary inputs before
the existing INL-STEP06 milestone. This contract does not claim opcode completeness,
production speed, wider platform/sanitizer qualification or acceptance closure.

### INL-STEP03F implementation contract

This is one coherent Level L owner increment, not a series of opcode admissions.
Before source edits, use the following ordered interfaces and keep existing
bounded inliner behavior unchanged:

1. The optional target owner exposes a checked, frozen instruction transfer
   view derived from the generated canonical catalogue; independent sealed
   rich effect profiles may exercise the same Level L algorithms before
   native catalogue completion, but no target mnemonic table fills its gaps.
   The view includes typed
   explicit/implicit references, ordered compound actions, component/flag
   may/must writes, numeric and signal dependencies, failure phases and
   activation observations. Must availability/kills are qualified by the
   actual edge and exit; a success-only write does not kill state on a resumed
   before/partial-write path. Missing catalogue precision is a named unknown,
   never an empty effect.
   An opaque host/output action has two independent footprints: intrinsic
   Rexx-value reads/writes, which the opcode database may describe completely,
   and an external host callback/signal effect, which remains possible unless
   separately proved absent. Generic Level L external-demand extraction may
   omit only the intrinsic unknown-read sentinel when that typed footprint is
   complete. The native SSA, signal and speculation barriers keep the external
   residual. Apply this contract to the whole SAY/SAYX output family together,
   with direct register reads where present; unknown external operations remain
   unresolved and callback havoc is unchanged. Remap correspondence compares
   this intrinsic-footprint fact so a candidate with unknown coverage cannot
   borrow a source's complete footprint.
   A CALL's opaque intrinsic-read footprint is contextual, not a static
   catalogue `complete` bit: the checked direct target, prepared argument
   capture, provider CFG and bounded recursive observation projection must
   establish all reachable callee reads. Only after that local proof succeeds
   may entry-demand extraction discharge the delegated CALL external sentinel.
   Unknown/native/foreign/dynamic targets, unresolved actuals or phases,
   recursion, missing effect families and limits stay explicitly unresolved.
   A call callback, ABI event and final havoc remain independent obligations;
   no later success resets an earlier unknown entry-demand fact.
   The same opcode database owns an explicit per-operand value-writer kind:
   raw field/copy, resource-releasing typed setter, or unknown/special. Derive
   reference/native-payload prewrite clear and possible finalizer callback
   from the setter kind, after inventorying the VM's shared setter handlers;
   do not infer release from an integer result or a mnemonic range. Unknown
   writers keep conservative resource effects. Preserve existing signal
   uncertainty until separately proved; the writer repair cannot silently
   remove it. Cross-check positive setter families and negative raw/copy
   controls against the generated catalogue and ordered Level L transfer.
   The native writer-kind inventory and generated catalogue now cover explicit
   shared scalar/string setters, reviewed direct field/copy writes and unknown
   helper/hidden writes. A stem string helper can succeed with its destination
   aliased to its source and skip release; its output is therefore an unknown
   writer with a possible callback, not a guaranteed prewrite clear. The
   existing stem must-clear assertion was corrected to that contract. This
   increment does not resolve raw-root recycled payloads or the remaining
   unknown opcode signal contracts.
   Before refining unknown signal rows, the canonical database must expose a
   complete/unknown whole-op callback inventory derived from typed resource
   input/output, retained capacity and selection/host/ABI/departure routes.
   Missing inventory fails closed; `OPAQUE` is not itself a callback theorem,
   and a reviewed raw getter may prove no callback. Intrinsic signal metadata
   stays separate from effective execution control: a possible finalizer can
   queue an interrupt through the public VM API, so native and Level L CFG
   leaders/edges, native SSA normal/failure transfer and effect/policy versions
   must represent that callback control and havoc independently of `signal NONE`. An intrinsic
   before-write failure cannot restore stale pre-callback state.
   Review native motion/removal/permutation proofs together, retaining a
   positive only with independently proved absence of every source,
   destination, capacity and subjectless callback route. First repair incoming
   versions; no no-hook premise may be derived circularly from post-setter SSA.
   A raw local-base `ENTRY` component is dirty, not proved resource-absent;
   native cleanup may use only an independently established `ABSENT` incoming
   fact for that conclusion. The Level L transfer must publish a callback
   residual for every canonical obligation route not already represented by
   its typed callback actions, including invocation with no result operand.
   `LOAD` setters cannot be unconditionally speculatable, and a signal-free
   callback cannot make temporary register permutation unobservable. Focused
   callback interrupt, source-copy/capacity, stale-state negatives and positive
   raw-field/pointer controls precede any signal-precision batch.
   The next native proof increment is additive and uses only incoming,
   epoch-owned storage facts: discharge a resource callback route only for an
   exact absent native payload or zero active count on every relevant input and
   output. Capacity, unknown, host, selection, ABI, departure and invocation
   routes remain unresolved. An ordered, complete single-setter profile may
   retain guaranteed normal-path writes on the physical receiver captured
   before callback havoc. It may establish absent native payload after the
   final reset, but cannot infer absent reference when a finalizer can reinstall
   one. For a normally returning public-ABI resource callback with complete
   inventory limited to input, output or capacity routes, the VM preserves the
   current frame's locals/baselocals/local_identity pointer coordinates; the
   callback may still mutate their pointees and all resource, lifetime,
   context, policy and pending-signal facts. Invocation, host, selection, ABI,
   departure and unknown routes do not get that coordinate premise. An ordered
   setter may then write to its captured receiver on the normal path only;
   exceptional, handler and resumed paths remain unknown. Keep this fact
   independent of graph construction: do not use post-setter SSA to
   prune callback CFG edges or effect versions circularly.
   In the Level L CFG, intrinsic and callback-caused unknown paths have distinct
   boundary identities. Callback control must not inherit a pre-callback
   default-HALT proof for unknown/host/ABI/invocation routes: those routes may
   change policy before control resumes. A complete public resource-only
   finalizer can queue an interrupt but cannot edit the suspended frame's
   handler table before dispatch; preserve that structural policy on its
   normally returning path. Delivery and resumed handler code still have
   their ordinary signal-flow effects. Unknown callback routes retain resumed
   and outward alternatives and incomplete local coverage. Scoped flow must preserve
   residual callback obligations rather than counting only the typed ABI action.
   Possible non-resource callback policy changes propagate to later reachable
   signal producers, including loop backedges, by flow reachability rather than a
   whole-function unknown flag that poisons unrelated earlier paths. Use an
   internal per-source-point may-change fact seeded by the exact nonresource
   callback obligation bits. Propagate it through staged local successors to a
   fixed point, restage signal alternatives against each producer's incoming
   fact, and repeat when new handler/resumption edges appear. Preserve raw
   intrinsic phases and distinct callback/ambient identities. Charge sweeps,
   edges and slots to a work budget; exhaustion yields no partial CFG proof.
   This increment gates nonresource callback-policy completeness by reachability.
   Older aggregate completeness paths for disconnected scope-entry `SIGRET`,
   authored policy and unresolved constructed/operational rows still need a
   separate reachable-row projection before whole-procedure completeness can
   ignore every disconnected source. Keep their explicit rows and incomplete
   result until that shared projection is qualified.
   The native graph uses the same cause/phase distinction through an internal
   exact entry-policy profile. Do not infer all-HALT from a procedure name or
   fixture: VM defaults differ by signal. Preserve immutable intrinsic signal
   contracts and add typed callback producer/checkpoint provenance. A complete
   resource-only callback queues control for dispatch after opcode completion;
   `DISPATCH` defers pending delivery while the active frame is already an
   interrupt handler, so exact entry profiles cover only an ordinary
   root/normal activation with no handler-entry ingress. Handler/unknown
   activation remains conservative. The after-completion checkpoint is before
   policy delivery, not a promise of immediate delivery;
   intrinsic synchronous before/partial failures remain separate. Solve
   structural policy reachability from a sealed entry seed with a monotone
   worklist before SSA, adding HALT terminal, IGNORE posthook resume, and
   handler/unknown conservative alternatives as admitted by each reachable
   policy. Join and backedge growth may enable more edges before freezing the
   graph view. Never use postsetter resource facts to justify a control edge.
   SSA/effect/use facts consume the frozen view; native/action handlers havoc
   state before resume unless independently summarized. Work exhaustion or a
   stale/foreign profile returns the existing conservative graph, not a partial
   proof. The first implementation supports exact entry profiles and
   resource-only HALT/IGNORE; interprocedural handler summaries remain open.
   The opcode database distinguishes structural handler-table writes from
   pending-signal bit control. `BPOFF` clears a pending BREAKPOINT bit in the
   VM; it does not remove an installed handler or establish IGNORE. Retain its
   typed pending effect as `RXOP_SIGNAL_PROP_PENDING_WRITE` (32), distinct from
   structural `POLICY_WRITE` (8), through the existing signal-properties field;
   the pending-only row carries no handler-policy dependency. Remove the false
   structural-policy-write fact, and update native consumers and focused
   controls together. Level L CFG collects only structural writes as handler
   installations. Transfer emits an existing external action for pending
   writes, so their observation/barrier survives without a false policy action.
   Summary, fact and flow consumers retain conservative effects for both
   property classes.
   The ordinary raw native graph's historical positive proof fixtures remain
   an open qualification gate. After the common CALL/RET closure contract is
   fixed, integrate callback-aware local proofs without a circular bootstrap:
   build conservative SSA/effects S0 over a frozen exact-policy view, derive a
   checkpoint-bound incoming no-hook ledger H0 only from S0, then rebuild
   SSA/effects once using H0 for normal callback havoc and ordered postwrites.
   Local proof admission uses H0 and the unchanged intrinsic signal contract,
   not an unconditional effective-NONE test. HANDLED paths and unproved
   capacity/unknown routes keep havoc. Migrate old fixtures by theorem with
   explicit prerequisite assertions and raw/handler/dirty-resource twins;
   never invent clean entry resources or reduce a negative to a generic reason.
   After that consumer repair, classify the reviewed shared families together:
   pure integer bitwise/boolean and direct branch/status getters with no
   intrinsic signal; short branch counters with possible partial integer
   writes on overflow versus staged long forms with before-write failure; and
   fixed-width binary getters with checked range (and float-platform failure)
   before writes. Other unknown handlers, including shifts and opaque host
   operations, remain unknown until their actual VM contract is proved.
2. One authoritative target machine CFG validates normal/table, signal-skip,
   handler, resumed, asynchronous-entry, unwind, terminal and unknown paths.
   Each edge identifies its role, source operation, successor or boundary
   identity, signal phase and per-normal/per-exceptional completeness.
   The shared Level L graph consumes typed edges and edge-sensitive transfers.
   Nonlocal departures are boundary exits, not illegal cross-function
   successors; handler registration is not an executed branch.
3. Reusable remapping and point/edge facts distinguish register spelling,
   active/base/entry storage, slot identity, canonical published component
   and individual-flag domains,
   derived values and availability. Liveness and lifetime
   include implicit windows, TRACE/metadata, aliases, references and all
   departures. Possible writes are not definite kills.
4. A separate early target normalization diagnoses proven entry-dependent
   owned-local reads and inserts a safe whole-cell `NULL` at that owned
   local's activation entry before authored writes, or an equivalent
   path-specific clear proved not to erase a valid component. It does not move
   a clear across effects, clear linked borrowed targets or argument/global
   storage, treat a copied but unobserved unknown component as an error, or
   publish a partial rewrite. Before this source edit, the coordinator and
   source reviewer freeze the exact warning trigger, placement, provenance,
   idempotence and async-entry contract. Native assembler adoption remains
   a later integration action.
   A callee's repairable dirty return can make its caller's availability
   unknown. Resolve this dependency by staged private normalization, using
   only witnesses from procedures whose entry-normalization proof is complete
   in that iteration. Rebuild the same shared transfer/CFG/flow after each
   partial canonical stage; never infer that an unknown return is defined.
   Once every procedure has a complete proof, validated entry clears establish
   the normalized postcondition directly, preserving constructed operational
   premises without substituting canonical semantics. Incomplete constructed
   premises retain with an exact reason. Every changed
   iteration must clear at least one previously uncleared owned local, so the
   original owned-local count bounds progress. Charge all requested analyses,
   remapping, retained candidates and ID/warning composition to one cumulative
   request budget. Reach a complete fixed point with no dirty observations
   before publishing; unresolved facts, no progress, limits or failed validation
   retain the exact original module with no staged warnings. Compose original
   operation IDs through every stage, and relocate each warning to its final
   observation while preserving its original source contributors.
   Direct dirty-return owner IDs are an extraction fact, separate from the
   recursive proof that all owners have been discharged. Candidate batch
   normalization may clear the complete requested owner set, then certify
   every changed owner against the same final candidate identity. The
   unconditional entry-normalization result remains conservative until its
   dependencies are discharged. For an independently sealed exact static
   call, the existing return/observation transfer may run only when effects,
   calls, references, bindings, order, components and observations are
   complete, the result and target references match the instruction, and a
   must whole-result clear precedes a must argument capture and one must call
   of that target. Callback and unknown effects keep their ordinary barriers.
   An independently described RET may project its pre-RET source only when
   its complete ordered action prefix through a single final must-return
   preserves storage and bindings; a possible callback, write, rebind,
   unknown effect or control departure before that read makes the returned
   component unknown, without substituting the prior caller value.
   The focused fixture checks this no-hook mechanism and separately checks
   that a canonical callback-capable call remains unproved when its caller
   resource cleanliness is not established.
   A normalization receipt borrows the exact source summary owner but does not
   force summary construction for an ordinary normalization that consumes no
   summary facts. The reference and flow-owner link remain checked on every
   use; conditional callback certificates force and validate the summary
   before consuming it. The reserved analysis allowance is not renewed.
   A separately named zero-dirty identity certificate may satisfy only the
   entry-normalization obligation for an unchanged procedure. It requires
   complete zero-demand source and candidate coverage, exact S/C owner,
   premise, CFG and operation correspondence with no insertion or rewrite in
   that procedure, plus discharged candidate dirty-return dependencies.
   Since no operation in that procedure changes, this identity theorem needs
   no callback-preservation summary; it grants no callback absence, activation
   or call equivalence. The ordinary function-wide completion result stays
   conservative. A changed procedure still requires its full certificate.
   A canonical result-clear recipe must retain the exact operand/storage and
   ordered pre-clear checkpoint in generic transfer metadata. `value_zero`
   traverses the active value/attribute tree; a proof of an empty active tree,
   absent reference and absent native payload at the exact live generation may
   discharge that checkpoint. Unknown callbacks, handlers, binding changes and
   unproved entry resources still prevent it. The existing subjectless
   capacity/checkpoint sentinel cannot supply this theorem; any separate
   callee-result or host callback residual remains possible until separately
   proved. Dirty scalar entry does not alone imply resource absence: the VM
   recycler can retain native payload and active children, and the next
   `value_zero` can invoke a finalizer. A resource-absence seed requires
   proved first physical allocation or a procedure/base recycle invariant
   covering all earlier exits and live aliases; a dirty-read warning or an
   ordinary post-entry clear cannot prove the pre-clear hook absent. A raw-root caller
   has no clean-resource seed merely because its scalar value is defined;
   nested canonical calls require a resource-preservation summary for the
   selected callee and return ABI, not a blanket no-call restriction.
5. An immutable procedure-fact service derives direct and transitive may/must/
   unknown summaries from the same graph/facts, including numeric, signal,
   return, symbolic argument/global, alias/ownership, call and external
   effects. Recursive SCCs use conservative seeds and bounded fixed points;
   no-return guarantees are nonvacuous. Exact owner/revision/catalog/binding/
   premise checks invalidate stale results and affected callers.

The focused owner panel constructs independent graph/effect cases for signal
phases and resume/departure, overlapping swaps and per-flag/component values,
owned-local clear/warning versus borrowed cells with idempotence, conditional
and conflicting normal returns, no-return recursion, numeric effects internal
to an activation versus caller-visible effects, argument/global aliases,
direct/recursive/unknown procedure calls, changed dependencies, stale/foreign
owners and tiny budgets. One handwritten RXAS client invokes the new services,
checks their results, then compiles, assembles, links and executes on one
normal VM. Existing affected target-CFG/constructed controls run only
where the shared inputs change. Correct Level L inference from checked
premises is the result of this step; complete native opcode coverage is not
claimed until the approved database extension and its own cross-check.

**Open shared policy-path precision:** the canonical procedure-wide union of
installed signal policies proves possible local destination coverage, not the
feasible correlation between policy state and data state. Entry normalization
may use that supergraph only with the independent flow observation/origin/
binding proof described in the STEP04/05 contract above. Clients requiring
exact policy correlation still receive an explicit incomplete certificate.
A future bounded correlated worklist may improve that precision through
policy install/push/pop and handler-stack markers; it is not required merely
to prove a may-read over complete possible paths. Unknown values, local
destinations, budget or stack-depth overflow remain explicit unknowns.

The reviewed increment entries below describe what the *current bounded
implementation* proved or rejected at the time. Their per-opcode admission
rules, private-global retention and proposed activation ISA are historical,
not the governing architecture. The short [inliner design](doc/inlining.md)
and INL criteria above supersede them for the integrated implementation.

### Reviewed direct fused-call normalization increment

This bounded INL-STEP04/05 development increment serves INL-AC02/05/06/07
and is implemented and focused-qualified under its reviewed contract. It admits
selected linear-root `main` calls to exact same-module scalar
bytecode bodies under `closed-main-default-halt.v1`, with a proved finite
count/window and the existing ordinary counted-call body, component, alias,
recycler, numeric and normal-return proofs. This first slice has no module
globals. Existing admitted scalar bodies and ordinary retained children keep
their current proof gates; fused calls inside cloned callees, imported/native
or unresolved targets, dynamic dispatch, handler lanes and physical-frame
observations are not newly admitted. These remain specific proof/body/scope
refusals, not an inherent impossibility claim. INL-STEP06 and the complete
INL criteria remain open; the demand audit establishes no speed benefit.

Use one private, checked defusion projection before the existing ordinary-call
analysis and expansion, rather than widening ordinary-call mnemonic tests in
the facts and cleanup owners. Match the canonical opcode/signature and replace
only requested fused sites, in this exact order:

- `SWAPCALL R,P,R,R,R`: `SWAP op4,op5`; `CALL op1,op2,op3`.
- `SETTPSWAPCALL R,P,R,R,I,R`: `SETTP op4,op5`; `SWAP op4,op6`;
  `CALL op1,op2,op3`.
- `SETTPCALL R,P,R,R,I`: `SETTP op4,op5`; `CALL op1,op2,op3`.

Each projected record is owned target data, not reparsed source text. The
ordinary analysis captures result storage, count storage/value and ordered
actual pointers only after all preparation. Preserve the counted CALL's lack
of zero-argument result clearing. SETTP retains the VM's low-32-bit public-band
write semantics; status belongs to storage and is observable through aliases.
SWAP exchanges both active pointers and slot identities. Repeated operands,
self-swaps, result/count/actual overlaps and distinct slots aliasing one value
are permitted when that post-preparation proof succeeds. In particular, do not
reuse a pre-swap count fact or capture the result pointer before the swap.

The projection and subsequent expansion form one public transaction. Preserve
ordinary-only batch behavior. For a batch requesting any fused site, publish a
changed result only if every requested fused site is ultimately expanded;
ordinary requested sites may still be kept unchanged with their existing
per-site reasons. If any requested fused site is kept by scope, facts or policy,
discard the entire projected/expanded batch and return the exact original
owner. Thus an ordinary site provisionally expanded in that batch is also
rolled back. Do not attempt retained-fused reconstruction in this first slice,
and never expose a merely defused retained call. Unrequested fused records stay
unchanged and may cause an existing analysis refusal; normalization does not
silently broaden their scope.

Keep public site IDs and request order original. On the fused-batch rollback,
preserve actual keep reasons for independently refused sites; change every
otherwise provisional `changed` decision to `keep` with
`batch.fused.site.retained`. The overall keep reason is the first refused fused
site's actual reason in request order. If shared facts refuses the candidate
set without a site-specific cause, report that shared reason for its affected
sites instead of inventing individual diagnoses. Invalid/limit failures keep
their existing failure status and original-owner behavior, and cannot leave a
`changed` site decision. No rollback result lists a changed caller.

Maintain original-record to projection-record and projection-record to final
record maps, plus the original selected fused ID to its projected ordinary
CALL ID and original body to projected body identities. Rebind checked requests
to the private owner, then compose maps and translate decisions back before
publication. A successfully expanded original fused call maps to zero under
the existing removed-call convention; other original records map through
composition. Original labels enter before the first preparation instruction.
Original source/TRACE point records remain before that preparation, preserving
the original fused-point register observation; preparation and CALL keep the
original logical origin. Existing callee contributing origins, fresh labels,
scopes and metadata relocation retain their owners. `inspect` uses the same
projection/admission path and charges its preparation. Public bind/inspect/
expand/expandmany/change and CLI signatures remain unchanged; no new ISA,
RXBIN feature, metadata assertion or producer proof is selected.

Charge both private stages, maps, complete copied records/origins/literals and
every added encoded cell within the request's work/entry/byte/frame/cell caps;
do not grant the second stage a fresh allowance. Keep immutable input and
last-good publication on all scope, proof, policy, resource and I/O refusals.
The default-HALT profile has no resumed nonlocal continuation; no handler
admission follows from defusion. Preserve delivered signal tag/payload and
logical fault source under the approved observation policy. Inherited branch/
SIGRET and resumable departures still need their own preparation/window and
unwind closure proof.

An unreferenced root label is an inert source/address record, not an alternate
execution path. The existing complete-normal-CFG and single-route linear proof
already excludes unsupported reachable control, cycles and executable detached
tails. Admit the label as a zero-effect transfer in that checked linear lane,
retain its original relocation/address mapping, and keep the existing CFG
proof for referenced entries. The fused controls retain an unreferenced label
before source/TRACE and preparation; a separate direct-branch root control
retains the exact original owner with `profile.caller.cfg.pending`, since
branched-root expansion is outside this slice's ordinary linear proof.
This prerequisite repairs facts
coverage without weakening either observation or control-flow expectations.

Conservative expansion retains preparation and normal caller restoration.
Separately disabled cleanup receives the resulting primitive stream and may
remove outer setup and reverse swaps only through its existing complete
storage/permutation, status, snapshot, result and observation proof. Retained
child windows and body-authored swaps stay governed by their current facts.
Do not let new expansion support accidentally authorize cleanup of a retained
compound call. A simple reordered leaf must lose proved outer setup/restoration
in cleaned output; aliases/snapshots must retain necessary transport.

The focused panel uses constructed-model and handwritten RXAS controls for
all three forms, plus one actual native-generated fused consumer through the
existing reader. Set independent expected values/status before native
comparison. Cover post-swap count/result capture with initial
`r0=1,r1=10,r2=2`, `swapcall r0,sum(),r0,r0,r2`, then `swap r0,r2`: a two-argument
sum must leave `1,10,11`. Its SETTPSWAPCALL counterpart must write status on the
original r0 storage before that storage becomes argument two. Add repeated
mutable aliases versus a required snapshot, self-swap, repeated activations,
zero/high-bit status writes and zero-count void-return result preservation.
Unknown post-swap count despite a known pre-swap count, invalid window,
imported/native target and handler/physical-observer controls retain precise
reasons and the exact original owner. Mixed batches cover all-fused success,
ordinary keep plus fused success, a fused keep rolling back another fused and
an ordinary provisional expansion, and unchanged ordinary-only behavior.
Assert original IDs/per-site decisions, label/source/TRACE mapping and atomic
work/entry/byte/frame/cell refusal. Inspect conservative and cleaned operations;
execute both with native `rxas -n` and one normal VM. Reuse prepared normal
tools and compile only the changed target/service fixtures and the consumer;
broader functional/platform/sanitizer campaigns are not commissioned by this
source-only increment. Coordinator review of this contract and the first
coherent source/representative RXAS precedes freeze and focused execution.

### Reviewed finite counted-window increment

The next INL-STEP05 slice covers a retained, exact same-module counted child
with a proved finite count greater than four under the existing restored-root,
closed-main-default-HALT profile. The proof captures the pre-call result and
count StorageIds and every ordered actual StorageId. All must belong to the
original caller frame. The one-to-four explicit-call rewrite stays in place;
unknown counts retain their distinct `argument.count.unproved` reason.

For a straight-line caller, the existing linear facts first identify a reached
exact same-module three-operand `CALL` with count greater than four. A charged
per-call lookup then dispatches that caller to the same graph transport, using
only the remaining work allowance; one-to-four, zero and unknown-count paths
retain their existing dispatch. Graph transport reserves a fresh, physically
disjoint contiguous window
`W0..WN`. It links `W0` to the proved count storage and `Wi` to actual `i`,
calls the same target with the original result storage and `W0`, then unlinks
every window cell on the normal continuation. Original count, result and
actual storage may alias; repeated actuals remain repeated pointers. No value
copy, status write or borrowed-value lifetime operation is introduced. The
original call address, source and TRACE attachment stay on the call; labels
enter before window setup. Added frame, records, bytes, work and code cells
are checked before atomic publication. Assignment must preserve the new
implicit window's adjacency. Ignored signals may resume within the same call
region and reach normal closure. The existing admitted policy must exclude any
nonlocal resumable handler departure that bypasses the UNLINKs; such handlers
require a separate window-closure proof.

Independent constructed and RXAS controls cover five reordered actuals,
repeated mutable aliases and snapshots, result/count and result/actual overlap,
multiple outer activations, source/TRACE, full normal UNLINK closure, and
work/byte/frame/cell refusal. Inspect removal of proved outer setup and
inverse swaps, then execute unchanged and cleaned artifacts through `rxas -n`
and one VM. This increment does not complete INL-STEP05 or INL-AC02/03/05.

### Reviewed exact imported-leaf increment

**Current direct-path integration contract (31 October 2026).** The native
linker supplies one exact frozen caller/provider bundle and selected executable
binding to the same Level L direct-or-CALL core used during assembly. A checked
body view borrows the provider's original module, transfer profile, flow and
summary owners; it does not copy the provider into a synthetic caller owner or
turn a manifest/name into selection authority. Validate the invocation's target,
provider/procedure/profile identities and the enrolled provider bundle before
reading executable records. Keep caller capture/layout/context facts in the
caller owner, and provider control/storage/normalization/callback/return facts
in the provider owner. Owner replacement, missing provider facts or stale
native epochs retain CALL with an exact reason. Unprojected same-owner checks
remain binding.

The first frozen wave admits global-free foreign bodies only through complete
shared proof. It introduces no instruction whitelist or second effect evaluator.
Private-global storage is separately represented by the approved borrowed-slot
contract; initializer readiness, policy inheritance, abrupt departures and
numeric restoration must still be proved at the exact invocation. Retained
procedure operands use the existing sealed selected-provider spelling. Static
diagnostic validation compares provider source/TRACE/frame records with the
clone while retaining the caller's original call-site/parent chain. Shared
proof unknowns retain the original CALL; default policy never substitutes
INLENTER. Input provider modules remain unchanged and candidate failure keeps
the last-good caller.

Link-stage leaf-first waves remain part of INL-STEP04/05. After a provider body
changes, native selection must validate a one-to-many executable correspondence
from the enrolled frozen source to the candidate, including copied retained
calls, and re-enroll the candidate owner/profile before another caller may clone
it. Each wave rebuilds affected facts and exact target bindings under the same
bounded outer request; no old capability is copied to a fresh owner and no
metadata proof channel is introduced. Until that native re-enrollment interface
is available, a frozen-wave result cannot authorize a subsequent wave. The
remaining interface and qualification stay open in this plan.

Independent controls cover an exact global-free foreign positive, unchanged
provider and caller import contracts, source-ID/frame collisions, retained
sealed children, dynamic/native/missing/stale binding negatives, module/policy/
initializer barriers and tiny-budget atomicity. Coordinator-owned preparation
and one-VM execution qualify the source increment; bottom-up selected A→B→C
and recursive-SCC controls qualify the later re-enrolled waves. INL-AC01–09
and INL-STEP06 remain open.

**Approved typed `main` entry contract; implementation/QA open.** Adrian
approved the program-entry and uniform initializer-entry contracts. This
replaces the proposed whole-image closed-host opt-in. The contract defines
program-visible entry state independently of the launcher. A typed executable
declaration binds one exact `main` procedure to the versioned default-entry
contract. RXAS/RXBIN and checked entry APIs implement the approved semantics;
exact feature assignment/API names remain implementation details. It is a runtime entry
contract, not inliner proof metadata, a no-callback assertion or a new opcode.

Approved handwritable syntax follows existing procedure-header options:

```rxas
main() .locals=2 .entry=program-default
  ret 0
boot() .locals=0
  .meta "example.boot"=".initializer" ".void" boot() ""
  ret
```

Only program `main` needs a new marker. Use one typed `META_ENTRY_STATE`
record with owning PROC reference, contract version 1 and program-default kind
1, under required `RXBIN007_FEATURE_ENTRY_STATE` bit 11. Its native layout is
`meta_entry base; size_t procedure; uint32_t version; uint32_t kind;`.
The header option follows `.locals=N` and any existing `.expose=name` option;
the record's code address is its owning procedure's start. One record is
permitted per module, naming the local procedure `main`. Unsupported versions,
kinds, duplicate/misplaced records and nonlocal/non-main owners are malformed.
The assembler validates the procedure name/unique declaration; the linker
preserves/remaps the PROC and rejects ambiguous selected program entries.
The canonical executable procedure-operand inventory rejects known child entry to a
marked main during assembly and selected-image linking, before and after the
trusted inline service. Dynamic ingress remains a checked VM/API obligation.
Load/prepare validates the record and feature before entry; source/inline
stripping preserves it. Older runtimes reject the required feature. Initializer
identity already has the exact typed `META_INITIALIZER` owner and needs no
redundant `.entry=initializer-default` or second entry-state record.
The checked program root and module initializer are separate entry roles:
one PROC cannot own both `META_ENTRY_STATE` and `META_INITIALIZER`. Assembly
rejects that semantic conflict before output opening, and the typed RXBIN
validator rejects it in either metadata order before loader/linker use.
Disassembly emits each typed initializer at its canonical metadata-chain
position, including non-exposed local procedures. It must not reconstruct
initializers only through imported callable signatures or reorder/duplicate
them by symbol; reassembly preserves startup behavior and declaration order.

The bounded Level L integration preserves this declaration as an optional
typed procedure-header entry kind (`0` ordinary, `1` program-default), with
kind 1 restricted to `main`. The RXAS reader/writer retain the exact approved
header spelling, including exposed main, and normalization, remapping,
expansion and cleanup preserve the header kind. Unknown, duplicate, reordered
or non-main declarations fail before publication. Focused IO controls cover
roundtrip, malformed headers and bounded output; executable qualification
remains open. Separately, scope-free proof reuses its mandatory invocation-
qualified activation/RET ABI certificate instead of repeating a context-free
ABI callback veto. Explicit ordinary callbacks keep their independent checks;
neither repair asserts clean root locals or supplies a new execution profile.

- **Observable initial state.** Enter a logical program root with no program
  parent, handler-save stack or active signal-handler state. The canonical
  root signal table has no user handler/branch/return bindings, with responses
  taken from the VM-owned default table. Numeric state is DIGITS 18, FUZZ 0,
  SCIENTIFIC form, LOWER case and COMMON standard, using one canonical owner
  shared by runtime and generated analysis. Argument/result ABI and
  worker-local module storage retain their ordinary contracts; root entry does
  not declare recycled locals clean.
- **Checked entry handshake.** VM, loader, rxvml and debugger may establish or
  emulate exactly that observable state using private machinery. Idle-VM or
  ordinary-launcher entry is not a semantic prerequisite. Isolated reentrant
  entry is permitted when the implementation can establish the declared root
  view, preserve/suspend the outer state and restore it on every exit. An
  ordinary CALL/DCALL/SIGCALL/task/native invocation cannot silently treat the
  marked `main` as an inheriting child: it must use the checked entry handshake
  or reject before argument/result preparation or other side effects. The first
  implementation may reject entry paths it cannot enforce. Static references
  and dynamic entry checks belong to the linker/loader and common VM entry
  owner, including debugger and rxvml paths; a name alone grants no permission.
  Use a checked `program-entry` operation distinct from ordinary callable
  invocation. A forbidden static child target rejects assembly/link/prepare;
  forbidden dynamic child entry raises `INVALID_ARGUMENTS` with an entry-
  contract diagnostic at the invoking instruction, before preparation/clear/
  swap/capture or frame commit, and resumes its original continuation if the
  policy handles it. An API handshake failure returns an entry-contract error
  without invoking the body; freeze exact API names/codes during implementation.
- **Pending signals and debugging.** Preserve queued external events and
  deliver them under the declared initial policy at a defined entry boundary;
  do not erase pending delivery to create the premise. The debugger handshake
  distinguishes private implementation state from program-visible overrides.
  Later debugger-visible policy changes, signal injections and callbacks are
  explicit effects, with their true observation footprint; entry certification
  cannot hide them or imply that later policy remains default.
- **Separate readiness and identity.** Initializer success/readiness,
  poison/cycle behavior and capture timing remain independent obligations.
  Entry neither certifies arbitrary initializers nor initializes an untaken
  clone. Bind selected entry evidence to exact procedure/module/IR, catalogue
  and native request epochs; validate and remap it through the existing image
  owners. Unknown/stale evidence, incompatible runtime or unsupported entry
  rejects explicitly. A changed body still needs checked native re-enrollment
  before a later link-stage wave uses its selection facts.

Level L uses this contract only to seed the actual root's incoming state, then
derives per-site policy/context through canonical effects and exact selected
bindings. It does not assume no other module, dynamic load, callback or helper
entry; their effects remain explicit, and native/dynamic/unresolved targets
remain calls. Ordinary unmarked `main` retains its current entry behavior.

A global-free foreign leaf does not inherently require this flag. An exact
selected provider with no initializer has a checked no-user-code ingress; a
literal integer return need not depend on the inherited numeric values.
Shared context-parametric proof may remove its CALL when the exact caller
facts also discharge every callback and delivered-signal/activation observation.
Unknown inherited SIGRET or physical-observing handlers remain real barriers,
including asynchronous delivery at CALL/RET boundaries; global-free syntax
alone is insufficient. Extend the shared bodyview/site proofs rather than
requiring a global closed-root premise for every foreign site. Qualify both
this flag-independent path and the marked-entry path, with wrong/stale entry,
unsupported reentry, pending delivery, debugger-visible changes, initializer
and arbitrary inherited-policy negatives. Implementation and focused
qualification remain open under INL-AC04/07 and INL-STEP04/06.

**Approved initializer observable-entry contract; implementation/QA open.** Strengthen
the existing typed `META_INITIALIZER` contract uniformly; do not add another
initializer flag. Apply
the same launcher-independent canonical signal/numeric root view to each
module initializer body through a checked VM/loader/debugger handshake, bound
to that exact initializer/module/instance epoch. This seeds analysis inside
the initializer; it does not assert successful completion or certify callees
ready. Declaration order, once-only readiness, failure poison, cycle detection,
reentrant dependency initialization and outer-state preservation remain with
the existing initializer owner. A later call can consume a separately proved
completion/readiness fact; it cannot derive success from the entry marker.
Pending delivery and subsequent visible debugger/policy effects follow the
same rules as `main`. Entry API, enforcement and focused qualification remain
open; approval does not claim implementation.

**Approved initialization-readiness direction.** Each worker/thread module
instance initializes exactly once successfully before ordinary procedure
entry. Ordinary calls require READY; FAILED stays poisoned with no retry.
INITIALIZING admits helper calls only within that instance's checked initializer
lineage; external/reentrant ingress to the still-initializing instance rejects.
The VM initializer/entry owner must enforce lineage, not infer it from a
context-global current-module pointer or `target == caller` alone. This is
separate from proposed `main`/initializer entry-state flags and from the
structural fact that a module has no initializer body. The latter proves no
user-code startup work, not an arbitrary runtime READY state. Implementation,
lineage/reentry negative controls and worker-isolation qualification remain
open; the coordinator owns native VM repairs.

The initial focused-qualified INL-STEP04 slice admits an imported, global-free, zero-private-local,
straight-line scalar leaf called from fresh standalone `main`. An opt-in
`closed-main-static-exact-default-halt.v1` profile uses the existing service
profile string and CLI `--body` interface. It requires the fixed caller plus
supplied providers to be the exact immutable sources subsequently assembled,
linked and loaded, with no alternative provider or pre-entry invocation. This
is an explicit delivery contract, not an RXBIN identity witness. The old
profile keeps foreign calls. No RXAS wire, VM ISA or source-language change is
selected by this increment.

Adrian prefers `.meta` not to become an inliner proof side channel. Essential
compiler call signatures, startup, linker and VM contracts are appropriate,
provided an ordinary handwritten RXAS producer can realistically express
them. This increment adds no new `.meta` inlining or provider-attestation
protocol. It uses checked target records and the existing import/export
contract, reads existing startup metadata to prevent an unsound static-entry
claim, and preserves source/TRACE observations. Any later private-global
binding must meet the same handwritten-producer and compatibility tests.

The shared service checks each selected call's unique caller import and exact
provider export, rejects local-name collisions and duplicate exports/imports,
requires complete caller-import resolution over the distinct provider owners
present in its binding batch, and requires caller/provider globals to be zero.
The CLI separately rejects unused `--body` providers before invoking that
service. All selected modules must lack initializer/autoload or unknown
startup metadata; providers have no imports or `main` body. Selected provider bodies have no
nested executable procedure references, policy/physical-frame observers or
unproved side effects. The initial zero-private-local slice avoids inventing
recycler freshness; the reviewed follow-on below preserves declared private
locals and uses the shared history proof. Pre-entry activation remains
excluded, and initializer ingress remains open. Existing scalar proof decides
remaining legality.

The implementation makes a private checked-target projection with one
surrogate per exact provider arena/procedure pair, rewrites selected call
targets only there, and runs the existing same-owner proof/expander. It then
rebuilds the original caller with its imports and module identity but no
surrogate definitions or surviving surrogate references. Caller and provider
origin/source caches are separate; provider spans, bytes and contributor
chains survive, with fresh module-local observation IDs. Compose old to
projection to expanded to published record/caller mappings. Charge each
projection, proof, expansion and publication stage against explicit work,
bytes, entries, frame and cell allowances. Every refusal or failed rebuild
returns the unchanged caller; no partial RXAS is published.

Focused service and separate caller/provider RXAS controls cover repeated
mutable aliases, result overlap, two call sites bound to one provider body,
colliding numeric source IDs, truthful source/TRACE, unchanged provider and
retained caller import, unique/missing/mismatched binding, startup/global/
nested-body refusal and tight-limit atomicity. Conservative and cleaned
callers are assembled with `rxas -n`, linked with those exact supplied
providers and run on one VM. This slice does not close broader imported
body/global/initialization coverage or INL-AC03/04/07.

### Canonical default-entry catalog projection

**Approved internal plumbing; source frozen, focused QA open.** One
VM-owned `interpreter/rxvmentry.h` X-macro inventory declares all 31 valid
canonical signal names and their existing root responses, plus the five
numeric members/context domains/default constants. `frame_f` expands those
inventories after clearing root handler bindings, preserving its current
mixed IGNORE/HALT/SILENT_HALT behavior and numeric values. In particular
`INVALID_SIGNAL_CODE` remains IGNORE, as in the existing initial fill;
`CANCEL` remains SILENT_HALT. The invalid sentinel `RXSIGNAL_MAX` is not a
catalogued signal. Existing `rxvalue.h` defaults own numeric values, including
a named COMMON-standard default. No opcode, RXBIN record, enum value, frame
layout, startup/readiness rule or pending-signal behavior changes.

The generated `llrxascatalog.catalog` exposes:

- `programdefaultcontract()` returns `program-default.v1`.
- `programdefaultsignals()` returns every valid exact canonical name in
  native signal-code order.
- `programdefaultresponse(name)` returns `IGNORE`, `HALT` or `SILENT_HALT`
  only for an exact whole canonical name; empty, wildcard, unknown, padded,
  compound and case-variant inputs return `unknown`.
- `programdefaultnumeric(contextBit)` returns the native numeric value only
  for one exact DIGITS/FUZZ/FORM/CASE/STANDARD mask, respectively 18/0/1/1/1;
  zero, combined, unknown or nonnumeric masks return -1.

The generator consumes the same VM inventories and native signal/numeric
owners, validates complete unique contiguous signal coverage and supported
response tokens before publication, and participates in the existing atomic
generator output lifecycle. Its shared CMake fingerprint/dependency list
includes those VM inputs, so native selected evidence and generated L queries
cannot silently retain different default revisions. Independent native/L
controls cover all names, mixed responses, five numeric values and invalid
queries; existing program-entry integration exercises actual main/initializer
numeric setup. Source review checks root wiring to the one owner. Preparation
and execution remain coordinator-owned. The catalog supplies values only:
marked entry identity, later policy effects, context-qualified activation,
readiness and private-resource proofs remain separate open consumers.

### Reviewed imported private-local follow-on

**Status: implemented and focused-qualified.**

After the exact global-free leaf slice, admit a straight-line imported scalar
leaf with a finite private local count under the same static-exact profile,
without changing metadata, ISA or public service signatures. The checked
projection must preserve the provider's declared local count on its one
interned surrogate per exact provider arena/procedure, and the shared facts
owner must prove initial component state, repeated activation/recycler history,
whole-value ownership and every normal return before expansion. Do not split
aliases into fresh surrogate owners: two imported names for one provider body
share one recycled local history. Default-HALT abrupt exits do not continue
into another activation; caller-owned resumable handlers remain a separate
signal proof. The existing expander closes fresh locals on each normal exit.

Independent controls cover an owned private snapshot returned after mutating
the actual (MOVE and reset), a borrowed local link returned by COPY (actual
still live), and repeated calls to one provider with a private register whose
integer is overwritten but flags survive across activations. The latter must
retain the whole batch with a precise recycler-history reason, not silently
inline either call. The constructed model also checks multiple bindings to
one body; native RXAS permits one import per exposed index, so the handwritten
case calls that import twice. Conservative and cleaned exact linked RXAS run on
one VM, with unchanged caller/provider on negative and tight-limit cases and
source/TRACE identities. Direct DAG labels are covered by the following
increment; globals, initialization and dynamic binding remain open.

### Reviewed imported direct-DAG increment

This focused-qualified INL-STEP04 slice admits a finite direct acyclic scalar branch body
from an exact static imported provider under the existing profile and delivery
contract. The selected body's normal CFG and per-return storage/status history
must pass the existing same-module DAG proof; this is no new graph solver.
Imports, provider readiness, globals, startup, nested executable references,
physical observers and dynamic binding retain their existing limits. One
surrogate per exact provider arena/procedure still carries its private locals
and recycled state across every selected site.

Before replaying each provider body into the private projection, allocate a
bounded label map keyed by that interned provider/procedure pair and original
label bytes. Cover every declared label, including forward, alias and detached
labels, before any branch is emitted. Fresh names must avoid all caller and
provider labels, `.const` aliases, procedure names and previously generated
names; `bodydraft.emit` resolves `L` operands against constants. Rewrite only
label declarations and canonical `catalog.branch(opcode,k)` operands. Other
symbols, metadata names, literal bytes and source/TRACE records are unchanged.
A missing map or malformed input refuses atomically. Charge map/name storage,
collision scans and lookups before growth within the existing preparation
work, entry and byte allowances. The existing cloned-site label relocation
then owns the second stage; final deprojection removes every surrogate and
retains the original import/module identity and exact provider selection.

Independent controls include two feasible branch outcomes with actual native
values and result/actual overlap; owned private MOVE versus borrowed COPY
across returns; two distinct providers with identical label spellings and
numeric source IDs; collision with caller labels and `.const` aliases; a
forward/alias label and a two-target branch. Source/TRACE must stay at their
original logical points. A later activation observing unreset private status,
cycle, undefined condition, incompatible join, branch-local metadata and
nested body remain precise whole-batch refusals; duplicate/missing labels
inside one source are invalid input. Tight work/name, byte, entry, frame and
cell limits preserve the exact caller and CLI last-good output. Compare
original, conservative and cleaned handwritten RXAS through `rxas -n`, the
same linked provider images and one VM. No public API, ISA, metadata or wire
change is selected. Broader imported and exceptional cases keep INL-AC03/04/07
and the final INL-STEP06 milestone open.

### Reviewed imported retained exported-child follow-on

The next INL-STEP04 slice admits a direct scalar nested child call in an exact
global-free, all-ready imported outer body. The child is a same-provider
bytecode leaf with a real export, and the caller already has a uniquely
resolved `.import` for that exact provider/child identity. No import, exported
symbol or child definition is synthesized in published RXAS. Both bodies are
interned by exact provider/procedure pair, so repeated outer sites and nested
activations use one proof/history owner. Existing call normalization and facts
prove fixed/count windows, argument aliases, return ownership, branch paths,
signal policy and recycler history. Only direct finite two-level shapes are
admitted; deeper/private/unresolved/native/dynamic children and recursion keep
precise reasons. No new metadata, API, ISA, linker or VM contract is selected.

Project provider labels and observations for the bounded pair closure before
replay. Rewrite nested direct procedure operands to the child surrogate during
analysis. At publication rewrite only those validated retained child CALL
targets to the caller's existing import alias, then remove every surrogate;
any other surviving surrogate reference refuses atomically. Provider readiness,
module ownership and exact-source identity remain unchanged. The published
child call is again imported, so current caller-only cleanup cannot see its
body: retain conservative expanded RXAS and precise cleanup keeps until a
separately reviewed provider-aware cleanup lifecycle exists. This increment
does not claim private imported nesting or outer counted-window cleanup. In
the selected counted example, linear expansion replays the authored swap pair
and conservatively replays its reverse again at outer RET. The latter pair is
redundant for that restored window; the following INL-STEP05 generated inverse
normalization owns its removal without requiring provider facts, with a
structural check that keeps the child-required swap and inverse. Broader outer
transport cleanup still needs the provider-aware lifecycle.

Independent constructed and handwritten linked controls check repeated fixed
aliased actuals and counted windows with required swaps, a two-arm acyclic
outer body with a retained child on one arm, source/TRACE, result overlap and
shared provider-pair identity. Verify original/conservative/cleaned executable
outcomes, removal of the outer CALL, retention of the child CALL to the exact
original import, no surrogate or new import in output, and unchanged provider
images. Missing/misbound child import, hidden child, deeper/recursive/dynamic
or unproved windows, startup/global/physical-observer barriers and tight limits
retain exact caller/provider owners and CLI last-good output. INL-AC03/04/05/07
and the final INL-STEP06 panel remain open.

### Reviewed imported shared public-global follow-on

**Status: implemented and focused-qualified.** This INL-STEP04 imported-body
slice uses the existing handwritten RXAS
public-global contract, without adopting a private alias record. A provider
may declare `gN .expose=counter.value`, and the caller may already declare
`gM .expose=counter.value`. The VM links these declarations to one worker-local
value cell; different module-local register numbers are expected. Preserve
both module artifacts, every original exposure and ordinary procedure import,
the exact provider selection and module load order. No caller global, symbol,
metadata or initialization directive is synthesized. Matching exposure names
across modules intentionally share storage, so they are not duplicate callable
providers.

Under the current static-exact, all-ready fresh-main profile, map every
referenced provider global through its unique within-provider public symbol
to an existing unique within-caller exposure of that same symbol. Validate
indices and declarations before projection. Remap all typed instruction,
metadata and TRACE global references to the caller's `gM` before the existing
same-owner scalar proof; that proof must use the caller's persistent global
StorageId, including when an actual aliases the mapped global. Require
canonical caller global bindings, and retain the existing rejection of
callee global LINK/SWAP/UNLINK, global lifetime operations, unproved
components/ownership and non-scalar state. Projection retains the validated
exposure records, and published RXAS keeps the original declarations.
The imported projection and static-exact facts profile validate exposures;
ordinary facts profiles still refuse them. Existing startup/initializer, autoload, dynamic target,
provider-import and physical-observer exclusions stay in force. Cleanup stays
conservative where global or exceptional access is not proved. There is no
new ISA, RXBIN record or inliner `.meta` proof fact in this slice.

Independent constructed and handwritten caller/provider controls must use
different global layouts and prove shared mutation, a global actual alias,
an unrelated caller global, repeated calls, return copy/status, metadata and
TRACE relocation. Compare original and expanded exact linked artifacts
through `rxas -n`, `rxlink` and one VM; inspect the emitted global indices,
unchanged exposures/imports, retained provider and absence of global
lifetime cleanup. Private/unexposed provider global, absent or mismatched
caller exposure, duplicate/out-of-bounds or cross-kind external declarations,
an original-provider nested counted window, changed global binding,
initializer ingress, unresolved callable provider and tight resource
limits retain the exact inputs with specific reasons and CLI last-good output.
Foreign private globals retain calls with `import.global.private.pending` under
the selected current-format profile. A private linker binding is not selected
for this POC. This focused slice does not complete INL-AC03/04/07 or INL-STEP06.

### Reviewed generated inverse-swap normalization

This focused-qualified INL-STEP05 slice simplifies only the linear inverse SWAP replay
generated at an expanded outer `RET`. After mapping each recorded pair to its
actual output register identities, cancel adjacent equal unordered pairs in
that contiguous generated replay. This bounded lookahead does not claim a
general permutation normal form.
The VM SWAP exchanges both value pointers and local identities, so repeating
the same physical pair is identity even for aliases. This proof crosses no
source/TRACE record, call, signal point or authored operation. Preserve the
result COPY/MOVE, every authored swap around a retained child, lifetime and
binding closure, and remaining inverse swaps. Charge bounded preparation/work
for the replay; refusal remains atomic. No provider body, new public API,
metadata, ISA, VM or linker change is selected.

The imported counted-child control should have exactly its authored pre/post
swaps across two activations in conservative output with `rxas -n`, and still
produce the independently expected values. A distinct overlapping pair and
noncancelling permutation retain their necessary inverses; aliases and result
overlap retain their values. Structural assertions inspect child-window order
and source/TRACE. Caller-only cleanup still returns `body.unavailable`; the
broader provider-aware cleanup and INL-AC05 remain open.

### Reviewed VM signal-marker prerequisite repair

A minimal handwritten two-tag RXAS control on current `hotfix2` aborts at the
VM's branch-unwind marker assertion after `SIGPOP` removes the top saved tag
while another installed branch still holds that node as its stack marker.
This is a VM signal-stack defect, not an inliner proof rejection. Named saved
entries can also be removed below the head; branch unwind can free younger
nodes while another handler remains active. The VM owner must maintain the
invariant that every same-frame active or saved handler marker is null or a
node still in that frame's saved-handler list. Before freeing any node during
named pop or branch unwind, rebase exact marker references to that node's
successor in the private active table and all remaining saved-entry snapshots.
Do not allocate during unwind or change whole-frame stack teardown. No new
ISA, metadata or signal response is selected.

A permanent handwritten RXAS regression must prove top and non-top removal,
active and saved handler references, branch and branch-value delivery, nested
restoration and G-style shared-marker ordering. The former assertion reproducer must
reach the expected handler exactly once, restore policy and exit normally.
Run the affected ordinary VM signal checks, then a focused maintained-sanitizer
check for the pointer-lifetime path. This prerequisite does not itself admit
G catch-all inlining; asynchronous state and return/cleanup proof remain open.

### Signal coverage and practical-demand decision

Adrian confirmed on 29 October 2026 that signal design should be tuned to real
language and optimization needs. A specifically characterized, low-value signal
form may rationally retain its call. This is not approval to reject every body
with signal policy, nor to close INL-AC03 by treating an unimplemented
representable case as infeasible. Any accepted exclusion or narrower preservation
profile needs its exact semantics, retained-call reason, independent negative
control and an explicit update to the affected acceptance contract.

INL-STEP04/05 must account for Level G simple-DO `ON SIGNAL` blocks: named and
catch-all handlers, optional bound `.signal`, nested policy, normal and handler
departures, `RETURN`, `LEAVE`, `LEAVE WITH`, relay and crossed reference/
call-window cleanup. Earlier source clauses retain priority when a later
catch-all overlaps.
The B/G emitter installs all `SIGPUSH` saves before related `SIGBR`/`SIGBRV`
branches and restores in reverse order; the VM's branch-handler stack marker
depends on that ordering. Repository G HTTP and LLM library code uses these
structures. This establishes a real consumer shape, not its workload frequency.

Level C needs a separate contract, not a relabelled G block. Classic
`SIGNAL ON/OFF` and delayed `CALL ON/OFF` use activation-owned condition policy;
direct `SIGNAL` is a same-invocation control transfer. Their lowering differs:
native C lowers SYNTAX to VM branch handlers and other Classic conditions
through a typed `CLASSIC_CONDITION` dispatcher.
Research L C adds protected-invocation capture, selection and origin-preserving
relay. Its emitter installs a standard VM policy capture layer in ordinary
generated procedures, even without authored `SIGNAL ON`. Preserve the Classic
activation, one-shot and delayed delivery rules, original event and
clause/SIGL/RC observations when selecting
or moving a compiled call. Source-level SIGNAL counts alone would miss these
generated effects.

Before selecting new ISA or declaring a rare-form retention policy, inventory
named independent B/G libraries/applications and representative static/dynamic
C programs separately from conformance fixtures. Reuse prepared producers to
inspect emitted RXAS at reachable candidate call sites: inherited and installed
policy, possible response, continuation, nested calls, cleanup and exact keep
reason. Report affected programs and candidate sites separately from observed
signal firing; a seldom-triggered condition can still make frame removal wrong.
Treat an absent form in that bounded sample as *not observed*, not impossible
or proven rare. Keep transient counts/results in conversation, and retain only
durable coverage decisions and limits here.

The bounded nonfixture producer shortlist for that survey is G
`driver/crexx_build_worker.crexx` (small named handler),
`lib/rxfnsg/rexx/llm.crexx` (fallback/object return) and
`lib/rxfnsg/rexx/http.crexx` (nested handlers, relay, references and cleanup);
B `examples/franz-lotto/crexx/lotto.crexx` with `lotto_logic.crexx`
(caller catch-all and imported stream-owning helper),
`examples/php/php_reader.crexx` (imported read under catch-all), and
`lib/rxfnsb/rexx/rxjson.crexx` (named conversion scopes); and Classic C
`packages/rexx-csv/classic/rexx_csv.rexx` with
`packages/rexx-csv/examples/classic_demo.rexx` (internal calls and external
source calls). The C pair has no authored SIGNAL in the selected source. Full
L C capture/protection depends on signal handlers or dynamic execution, but
its emitter still installs a per-procedure bound fallback policy; dynamic
source calls add a second child layer. Confirm emitted sites rather than
inferring a signal-free body. These source candidates are not workload
frequency evidence. Maintained signal/dynamic fixtures supplement, but do
not substitute for, the nonfixture set.
The bounded prepared G sample contains real unbound, bound, nested and
catch-all scopes around both direct and dynamic calls. It also shows source
TASK lowering can create many helper calls; source-call counts alone are
insufficient. Raw `SIGRET` and action handlers were not observed in that
sample, which supports a provisional specific retention policy, not a claim
of rarity or absence. B Lotto and C CSV have source-only evidence here; the
C emitter's generated protection still needs emitted-site inspection.
The available prepared B RXJSON RXAS retains its named conversion scopes but
has already reduced the protected source calls to native `bclear` instructions.
Lexical call counts therefore cannot stand in for actual RXAS candidates.
The selected real G/B sites are chiefly bound, nested, object/reference,
stream or cleanup cases; the bounded range-leaf catch-all slice has no
observed match in this purposive sample. It remains a semantic prerequisite
rather than measured application acceleration.

Try the existing ISA first for proved unchanged context and bounded structured
policy scopes. An isolated signal save/restore pair cannot generally reproduce
inherited `SIGRET`, whose VM response returns the current physical frame, or
branch handlers bound to a physical frame/stack marker. Compare a representative
G block and C capture/relay lowering against a versioned active logical-call
region (`INLINE_ENTER`/`INLINE_LEAVE` with typed descriptor). Such a region
would need a logical return continuation and dispatcher-owned unwind/cleanup;
it must reuse VM signal/frame/lifetime owners, enter atomically and avoid
allocation during unwind. It is optional for simple bodies and remains an
explicit ISA/runtime/assembler/linker decision gate, not an approved opcode.
Specific raw `SIGRET`, action-handler or physical-observer sites may
provisionally remain calls under a selected profile while G blocks and
C-generated protection are developed.

### Reviewed caller-owned named-signal increment

This focused-qualified INL-STEP04 slice uses the existing ISA and frozen target schema for
one Level G shaped, caller-owned named handler around a same-module direct
`CALL1`. The fresh standalone `main` saves one literal `OUT_OF_RANGE` policy
with `SIGPUSH`, installs an unbound `SIGBR` to a same-procedure label, executes
the call, then follows distinct normal and handler `SIGPOP` paths to one join.
The selected leaf has zero private locals, no policy/context mutation, and
exactly a before-write `ICHKRNG a1,0,10` followed by `RET a1`. Prove the one
actual/result are plain scalar storage; reject alternate entries, nested
scopes, backedges, other raising instructions and physical observers. The
ordinary caller and callee instructions, including policy save/install/pop
ordering, remain the source of truth; no assertion or inliner `.meta` record is
accepted. Preserve `SIGPUSH`'s possible failed-save behavior by retaining its
original instruction, not by assuming successful lexical restoration.

Derive a bounded exact-owner exceptional overlay in the facts owner: checked
push/install/handler/normal-pop/handler-pop/join IDs, literal tag, canonical
failure phase, and normal versus handler entry state. Do not treat the handler
as normally unreachable or feed its reads into a normal-only liveness result.
At abrupt entry, the expander closes only its fresh, disjoint argument/result
anchors and cloned metadata before the original handler body; it must not
copy a return, expire borrowed actual/result storage or change the original
policy instructions. Structural proof must cover partial setup, normal return
and handler entry. Source/TRACE and skipped-result observations remain truthful.
The current cleanup graph has no exceptional edges, so this slice retains
conservative expanded RXAS and returns a precise cleanup-pending reason.
General exceptional cleanup needs a reviewed derived graph/owner extension;
this increment does not change public target or generic IR APIs, wire/schema,
VM or language syntax. `INLINE_ENTER` remains a separate gate for inherited
`SIGRET` and physical logical-call unwinding.

Independent focused controls use input 7 (normal result 7, one normal
observation), input 12 with preloaded result 99 (one handler observation,
unchanged result 99 and actual 12), both policy pops, the same join, absent
selected call, abrupt closure before handler work and a truthful fault source.
Inherited `SIGRET`, callee policy mutation, unresolved/cross-procedure handler,
reference/native payload and crossed counted windows retain specific reasons.
Tight resource failure must preserve the original module and CLI last-good
output. The first representative emitted G RXAS is inspected before edits;
constructed and CLI outputs then assemble with `rxas -n` and execute on one VM.
This does not close bound, catch-all or nested G handlers, general signal
cleanup, Level C capture/relay, or INL-AC03/04/05.

### Reviewed dynamic named-signal follow-on

This INL-STEP04 slice admits a defined, entry-independent scalar actual
whose value may be unknown at analysis time under that same one unbound named
`SIGBR` scope and zero-local `ICHKRNG a1,lo,hi; RET a1` leaf. Literal finite
bounds need not be fixed at 0 and 10. Preserve the original policy rows and
fresh-anchor cleanup; no public profile, API, target schema, metadata, ISA or
VM change is selected. The caller may use a proved no-signal `GETANDTP` to
produce the runtime value. Other named failures, nested scopes, callee policy,
nonplain storage and physical observers retain precise reasons.

The facts owner must snapshot caller cells, bindings and recycler history at
the call. Derive feasible normal/check-failure paths from the canonical
before-write `OUT_OF_RANGE` effect: known values can select one, a proved
in-range interval selects normal, and a defined value crossing a bound keeps
both. Existing invocation and return transfer apply only to the normal path;
the handler starts from the pre-call state with no result write. Process each
path separately, then meet component/status, frame and history at the join
using the existing owners. A preloaded result preserved on failure prevents a
false definite kill or constant after the join. Normal-only access/liveness
remains incomplete and exceptional cleanup retains its precise keep reason.
Never use the side overlay to justify dead-store deletion or storage reuse.

Constructed and handwritten controls use two runtime values from the same
analysis-unknown producer, one in range and one out of range, with a preloaded
result and result/actual overlap. Independently check both outputs, actual and
result state, normal versus handler observations, source/TRACE, original pop
ordering, abrupt closure and joined later reads. Undefined actual, other
potentially failing caller instructions and tight resource limits preserve the
exact caller and CLI last-good output. Incompatible binding/history-meet
refusal remains open until a later admitted shape can reach such a join; this
profile rejects binding-changing caller instructions before analysis. Run one
normal VM with `rxas -n`; broader G catch-all,
bound `SIGBRV`, asynchronous cancellation and general exceptional cleanup
remain separate INL-STEP04/05 work.

### Reviewed multi-name unbound signal follow-on

The next INL-STEP04 slice admits a bounded explicit list of distinct named
policies sharing one unbound handler around the existing exact same-module
`CALL1` and zero-local `ICHKRNG a1,lo,hi; RET a1` leaf. `OUT_OF_RANGE` is the
only leaf signal; another synchronous named policy such as
`INVALID_ARGUMENTS` exercises G's common-marker ordering without inventing
another feasible callee failure. Read ordinary RXAS policy rows: all pushes
precede every install, every install names the same local handler, normal and
handler paths pop precisely the reverse pushed list, and both reach one join.
Retain the policy instructions and the existing before-write scalar split,
fresh-anchor handler closure, source/TRACE relocation and exceptional-cleanup
refusal. No wire, metadata, API, ISA, VM or language change is selected.

This is a named-list prerequisite, not full G catch-all. The G emitter expands
catch-all into 29 named registrations, including `FAILURE` and `CANCEL`; `ANY`
is not a VM signal name. Adrian accepted on 29 October 2026 that reasonable
inlining may change error paths caused by infrastructure pressure when it
removes or reshapes that infrastructure, including the `FAILURE` opportunity
from allocating an eliminated call frame. This does not excuse deterministic
language failures from retained work or other call-entry semantics. `CANCEL`
can arrive after setup or return writes, so a single pre-call handler snapshot
is insufficient. Full catch-all
still needs interruptible path/cleanup proof; a runtime logical activation
fallback remains a separate API/ISA decision if that proof requires one.
Handwritten metadata cannot replace unwind state.
The bounded named-list slice excludes `FAILURE`, `CANCEL` and other externally
delivered asynchronous tags; their frequency in real programs is unmeasured.

Constructed and handwritten controls check normal, out-of-range and dynamic
alias results, exactly one handler observation, the original pop ordering,
selected-call removal, cloned source/TRACE and fresh closure before handler
work. Missing, duplicate, mismatched or interleaved policy rows and alternate
handlers/entries retain exact input with specific reasons. Resource refusal
preserves the caller and CLI last-good output; the cleanup guard stays active.
The full catch-all, bound `SIGBRV`, nested scope and Level C protection layers
remain open under INL-AC03/04/05.

### Reviewed bounded G catch-all follow-on

**Status: implemented and focused-qualified.** This INL-STEP04 signal slice
admits the G emitter's complete, explicit
29-name catch-all around one same-module `CALL1` in fresh standalone `main`.
The selected callee is exactly zero-local `ICHKRNG a1,lo,hi; RET a1`, with
literal bounds, a proved defined plain scalar actual and result, and no
callee policy, context, nested call or physical-frame observation. The ordinary
RXAS rows are authoritative: every distinct name is pushed before any install,
all unbound branches target the same local handler, and normal and handler
departures pop the exact reverse list. Preserve every policy instruction and
its ordering. No synthetic `ANY`, `.meta` proof, new API, ISA or VM change is
selected for this inliner slice.

The inliner may map the read-only `a1` directly to its caller actual storage:
the leaf does not rebind, overwrite or expire it. `a0` is unused, the callee
has no locals, and the normal `RET` needs only the proved borrowed-result COPY
to the caller result. This avoids fresh call-window anchors and their cleanup,
including an unnecessary handler prologue. Result/actual overlap remains
valid under the same return-action proof. Retain the call if any status,
ownership, entry-binding or alternate-control fact needed for this mapping is
unproved. Keep the existing exceptional-CFG cleanup guard; no normal-only
liveness fact may delete or reuse a handler-visible write.

Prove the interrupted state at every original policy row and every generated
instruction boundary, not only at the old call. Before the result COPY, a
handler sees the pre-result caller value; afterward it sees the committed
result. A synchronous failed `ICHKRNG` reaches the handler without the COPY.
Earlier writes and observations remain visible. A second event during reverse
pops must match the original policy and VM response, including a repeated-pop
failure in the current G emitter pattern; this increment does not assert
successful recovery from that existing failure. The eliminated call-frame
allocation may no longer raise `FAILURE`;
other retained deterministic failures and language control stay exact.
Relocate call-site, callee and caller-continuation source/TRACE records at
their actual phase boundaries, with the normal result event only after normal
completion. A separate VM signal-marker repair is prerequisite to testing
delivery through pop prefixes; it does not establish this state proof.

Constructed and handwritten controls must cover normal, range-fault and
analysis-unknown values, preloaded result, result/actual alias, asynchronous
delivery before and after the result commit, another event during restoration,
policy priority, source/TRACE, cleanup refusal and atomic resource refusal.
Missing/duplicate/interleaved policies, alternate handlers or entries, bound
branch, nested scope, nonplain actual, callee mutation, and physical observers
retain precise reasons. The focused slice does not close general G blocks,
Classic C protection, inherited `SIGRET`, general exceptional cleanup or
INL-AC03/04/05. It is not a production performance claim.

### Bounded caller-owned bound-event increment

**Status: implemented and focused-qualified.** This
INL-STEP04 follow-on serves AC02/03/04/06/07 without changing the existing
ISA, target wire schema, public service API or VM/library event ABI. It is a
bounded event-delivery and consumption prerequisite for G bound handlers and
C capture, not qualification of their complete producer lowerings or evidence
of application acceleration. Keep INL-AC03/04/05 and INL-STEP06 open.

Keep the preceding full 29-name, fresh standalone `main`, zero-global,
single-scope contract and exact same-module zero-local
`ICHKRNG a1,lo,hi; RET a1` leaf. All installed rows are `SIGBRV` with the same
local handler and the same caller `rEvent`; all saves precede every install,
and both departures retain their exact complete reverse-pop lists. Preserve
the original tag order, policy instructions, labels and departures. Mixed
bound/unbound installs, multiple event destinations, smaller bound tag sets,
nested scopes, additional control entries, callee policy/context or retained
calls remain separate categories. The existing smaller unbound profiles keep
their contracts.

Require canonical, distinct caller cells for `rEvent`, `rRaw`, `rField` and
each projection output; none may alias the selected actual or result. Actual
and result may still alias each other under the existing borrowed-return
proof. `rEvent` must have an explicit whole reset before the scope and may
then receive ordinary scalar preloads; it has no source write in the protected
region. `rRaw`/`rField` have reset owned bases and no active links at scope
entry. No other binding change, reference or object operation is admitted.
Keep the read-only direct actual mapping, call-site `CNOP` and normal whole
result `COPY`; introduce no fresh frame/window cells or handler prologue.

The `lltargetfacts` owner extends its existing bounded overlay with a private
VM-event shape, not a general object algorithm. `SIGBRV` installation records
an encoded destination slot and does not define its value. On each exceptional
predecessor, resolve that slot's current active cell, apply the VM's whole
runtime-event overwrite, then meet handler states. Derive the event shape from
the executable installation and existing VM handlers, never from `.meta`
attestations or a copied opcode/code table. The event is nonplain/nonordinary;
do not grant it scalar component facts or plain lifetime/reuse permission.
Its raw wrapper, tag/code and message/payload remain owned by the existing
`rxsignal_populate_raw_interrupt`/`rxsignal_populate_runtime_signal` ABI and
`lib/rxfnsb/rexx/signal.crexx`.

After the handler's entire reverse-pop list, admit at most one projection of
each of code, name and message, in source order. Each uses an immediate-index
group beginning `LINKATTR1 rRaw,rEvent,5; LINKATTR1 rField,rRaw,k`. For `k=1`
(code) or `k=4` (name), retain `COPY rOut,rField`, matching the ordinary scalar
fields constructed by the VM and C's maintained `rawcondition` pattern. For
`k=5`, retain `SCOPY rOut,rField` for the string message view only; the typed
payload and its object/native/reference ownership are neither copied nor
classified as ordinary. Every group ends with `UNLINK rField; UNLINK rRaw;
NULL rField; NULL rRaw`, and the handler then executes `NULL rEvent` before
its existing branch to the join. Outputs are distinct ordinary owned cells;
only their proved integer/string/status components survive the join. Preserve
partial-copy semantics and string certificates independently of integer and
whole-cell facts. All projection, unlink and reset operations remain executable.
The exact VM-produced attribute extents establish successful `LINKATTR1` here;
no general exception-to-success specialization is selected.

No branch or alternate entry may enter a projection group. Its scratch links
cannot outlive that group or escape through metadata/TRACE operands. Raw
module/address fields (`k=2/3`), dynamic or other attribute indices, field
mutation, whole-event/payload copies or returns, relay, arbitrary calls and
runtime-event register observations retain `signal.bound.consumer.pending`.
Event/scratch/output alias or noncanonical binding retains
`signal.bound.mapping.pending`; unproved reset/component or joined state
retains `signal.bound.state.unproved`. Malformed/duplicate policies retain
their existing specific diagnostics. These are current missing-proof limits,
not inherent G/C language exclusions.

At every original policy row and generated instruction boundary, use the
actual already-performed caller writes and observations. A failed check has
no result COPY; asynchronous delivery before COPY sees the old result and
delivery after it sees the committed result. Each delivery overwrites `rEvent`
with that new event before any handler read, including re-entry during a pop
prefix. Projection starts only after all scope policies have been restored,
so such re-entry cannot strand projection links. Preserve existing failed-save,
installation and repeated-pop outcomes and the checked outer default policy;
do not infer successful saves or repair G restoration semantics. The accepted
removed-frame infrastructure-pressure difference remains unchanged.

Source/TRACE remapping keeps the callee check's authored origin, fresh source
identities, original call-site boundary and restored caller continuation;
the original result TRACE executes only on normal completion. Do not assign
the caller location to the event's fault or invent a value event for transport.
The approved physical-traceback difference does not admit raw address or
register-bank observation. A test-only observer may resolve the delivered
origin through the existing metadata owner and compare authored source fields;
it must not become an inliner fact or widen the admitted consumer lane.

Independent constructed and handwritten controls cover known and
analysis-unknown normal/fault actuals, preloaded result/event/output cells,
result/actual overlap, all three consumed projections and scratch/event closure.
Paired ordinary original/expanded RXAS through `rxas -n` and one VM must observe
the expected code/name/message, scalar values/status and normal-path preloads.
Inject deterministic pending CANCEL with a nonempty message before/after COPY
and after an earlier normal observation; retain the existing priority and
second-event/restoration terminal controls. Check logical fault source and
normal-only result TRACE independently. Constructed/handwritten negatives cover
event/actual/result/output alias, rebound or unreset scratch, bad/physical or
dynamic projections, missing/unordered unlink/reset, escaped event/payload,
alternate entry, mixed installs, nested policy and callee mutation. Keep one
retained negative executable against its independent expected behavior.

Reuse the existing signal fixtures/runners and prepared producers. Inspect the
first conservative output early: original call absent, one required return
COPY, unchanged policies/projections, no fresh cells or lifetime elision.
`llcleanup` transport/storage must still return their exceptional-CFG pending
reasons; handler observations cannot justify normal-only deletion or reuse.
Charge event/projection facts, predecessor snapshots, joins, work and bytes
before growth; tight work/entry/byte/cell limits, replay and failed publication
retain the exact source owner and CLI last-good output. The coordinator owns
the focused build and this bounded panel on one normal VM; this plan draft
commissions no broader functional, sanitizer, platform or performance campaign.

### Runtime-event owned-copy increment

**Status: implemented and focused-qualified.** This
INL-STEP04 follow-on serves AC02/03/04/06/07 within the preceding full 29-name
bound scope and exact read-only range-leaf contract. It admits one existing
authored `COPY rSaved,rEvent` after the handler's complete reverse-pop list,
before its projections. This is the owned-event capture needed by G HTTP
cleanup and research C `signal.capture`; it does not admit those complete
producer bodies. Keep INL-AC03/04/05 and INL-STEP06 open. No ISA, target wire
schema, public API, VM algorithm or inliner `.meta` proof is added.

`rSaved` is a canonical owned caller cell, distinct from `rEvent`, scratch,
outputs, actual and result, with an explicit pre-scope whole NULL and no
subsequent write before capture. Require its whole-reset state and absence of
live attribute/reference aliases into its destination tree at capture: COPY
into a previously linked attribute tree can write through borrowed links.
Preserve the one authored COPY row, its operands and its exact executable
position. Do not synthesize a copy,
move it across a pop, add a capture guard or assert once-only first-event
capture. Capture before complete restoration retains
`signal.snapshot.phase.pending`: delivery during the pop prefix can re-enter
the handler and change dynamic COPY/finalizer counts, and a register guard
cannot close that interruptible gap. Keep existing save/pop failure and
re-entry behavior, with the same checked outer default policy.

The existing VM `copy_value` owner recursively copies value components and
attributes and retains reference descriptors. It may allocate and invoke native
copy/finalizer hooks, including diagnostic ownership. The result owns its copied
storage/descriptors; it is not a detached clone of referenced targets or native
resources. Native retention/sharing follows the existing hook contract; copying
a reference descriptor does not extend its target's language lifetime.
The built-in raw diagnostic hook
retains its existing immutable diagnostic owner; it does not recapture source
or stack. A copied descriptor still shares its target, and source-owner reset
can invalidate it. Retain the
exact authored COPY and resets, including their order and hook effects, without
elision, conversion to LINK/MOVE or a second event-copy implementation. Prove
the completed-copy normal successor separately from any abrupt departure;
neither allocation success nor a resumed partial-copy state follows from
the event shape or the opcode's normal catalogue row.

`lltargetfacts` adds only the bounded exact-owner transfer for this checked
COPY row. Its destination inherits runtime-event shape/provenance and opaque
payload ownership, not ordinary/plain, scalar, independent-referent or reusable
storage facts. Preserve integer/string/status/component uncertainty, logical
source origin and owned diagnostic carrier. A later overwrite/reset of `rEvent` does not by itself erase
the copied wrapper/attribute shape in `rSaved`; it also cannot certify payload
referent validity. No analysis materializes or traverses the runtime payload.
Native/reference callback, abrupt-state or physical-observation obligations
must be justified by the unchanged instruction placement and existing owning
contracts, rather than inferred from the public event class name.

The bounded code/name/message projection groups now read `rSaved`, using the
same immediate fields, COPY versus SCOPY semantics and complete scratch
unlink/reset groups as the preceding increment. Each group still begins only
after full policy restoration, with no entry/interleaving or escaped scratch
link. Require both an authored `NULL rEvent` after capture and an authored
`NULL rSaved` after the last saved-event use, before the existing handler
branch to the join; preserve their source order and positions. `rEvent` may
be reset before saved-event projection only when the latter uses the saved
wrapper's own code/name/message fields, with no payload dereference or target
liveness inference. At the join both event cells are ordinary reset cells on
the handler path; normal-path preloads retain their existing scalar proof.
Carry no opaque event into the continuation in this increment.

NULL recursively resets/releases nested ownership and restores owned attribute
pointers. ENDLIFE releases reference identities/descriptors while leaving
ordinary/native contents intact; it cannot replace either NULL. An existing
ENDLIFE after the corresponding explicit reset may remain executable, with
no new lifetime deletion or storage reuse. Neither copy nor reset may execute
with open attribute links into either event. Do not borrow a payload pointer
through the copy or extend a target's reference lifetime to make a control pass.

Retain precise current reasons for malformed scope and projection shapes.
Saved/event/scratch/output alias, rebinding or foreign-bank destinations retain
`signal.snapshot.mapping.pending`; unproved reset/component or joined state
retains `signal.snapshot.state.unproved`; missing or premature closure retains
`signal.snapshot.cleanup.pending`. Additional event copies, MOVE, whole-event
or payload return/argument/global escape, payload COPY/dereference/mutation,
raw module/address reads, arbitrary calls/relay and unsupported observations
retain `signal.snapshot.consumer.pending`. Existing direct-event projection
inputs keep their earlier contract. These limits do not classify the pending
G/C forms as inherently infeasible.

Delivery still writes `rEvent` at each actual exceptional predecessor before
handler entry, never at installation. Preserve caller state before/after the
normal result COPY and every prior write/observation; an older copied event
must not become the new event until its authored COPY completes. Preserve
all policy rows and every possible second-event outcome during restoration.
Point source/TRACE remains attached to the real capture/reset/projection
instruction boundaries. The saved event preserves tag/payload and logical
fault origin under the approved observation contract; no physical traceback,
raw-address or new semantic result-event guarantee is introduced.

Independent constructed and handwritten controls cover normal/fault/unknown
actuals, ordinary event/output preloads, result/actual overlap, saved
code/name/message projections and explicit closure of both event cells and
scratch. Include a
case that resets `rEvent` before projecting `rSaved`, with empty and nonempty
messages. Check name whole-COPY state and message SCOPY's preserved numeric
component/public status separately. A paired delivery/observation harness may
supply nested object, reference and native-hook payloads outside the admitted
transformation input: verify descriptor retention and target invalidation
against the maintained VM rules, diagnostic/native hook counts and reset
ordering, without asserting independent or prolonged referent lifetimes.
Use test-only reference identity/lifetime probes, and distinguish NULL's native
release from ENDLIFE's lack of native finalization. Include a destination-tree
alias refusal; whole NULL must not be replaced by a scalar overwrite.
Compare each ordinary original/expanded artifact through `rxas -n` and one VM.

Reuse existing pending-CANCEL before/after result commit, earlier-observation,
priority, restoration terminal, logical-source and normal-only result-TRACE
controls. Add independent negatives for before-pop capture, alias/rebound or
unreset saved storage, duplicate copies, missing either NULL, ENDLIFE substituted
for NULL, open links at copy/reset and escaped event/payload. Execute one
retained negative with independent expectations. Inspect conservative output
early for the unchanged authored event COPY/NULL rows, unchanged policies and
projections, absent selected call and no fresh cells or lifetime elision.

Keep the exceptional-CFG transport/storage guards and exact-owner atomic
publication. Charge added facts, snapshots, joins, work/entries/bytes before
growth; tight limits, failed output and replay preserve the frozen input and
CLI last-good output. Reuse prepared producers and the current signal runners;
the coordinator owns the focused one-VM development checks. G method selection,
general object/reference aliases, C field-5 whole payload transport, capture
before restoration, Classic activation/relay and atomic restoration remain open.
This contract does not commission broader functional, sanitizer, platform or
performance qualification.

### Guarded owned event across the handler join

**Status: implemented and focused-qualified.** This
INL-STEP04/05 increment serves AC02/03/04/06/07 by carrying one opaque owned
event across the existing normal/handler join to one proved flag-guarded later
consumer. Keep the full 29-name, fresh zero-global `main`, exact read-only
range leaf and post-restoration authored event COPY of the preceding contract.
No field-5 whole payload COPY, G method selection, Classic activation/relay,
new ISA/public API/target wire schema or inliner `.meta` proof is selected.
INL-AC03/04/05 and INL-STEP06 remain open.

Add one canonical caller `rHave`, distinct from event/saved/scratch/output,
actual and result. Before the first save, require explicit `NULL rHave; LOAD
rHave,0` and `NULL rSaved`, with no later write to either before handler
capture. The normal departure reaches the original join with `rHave=0` and
empty saved storage. After its complete reverse-pop list, the handler keeps
the authored `COPY rSaved,rEvent; NULL rEvent; LOAD rHave,1` and branches to
that same join. COPY still requires the exact whole-NULL disjoint destination
tree without nested aliases. Both paths retain canonical active/base/identity
bindings. The flag commits only after the completed copy and event reset; it
does not assert atomic first-event capture or alter any restoration failure.

The join's first executable instruction is `BRF owner_closed,rHave`. Its
fallthrough contains exactly one current checked code/name/message projection
group from `rSaved`, with the established immediate fields, COPY versus SCOPY
component rules and complete scratch UNLINK/NULL closure. There is no other
branch, entry or owner/flag write between join and `owner_closed`. The false
edge skips the consumer and reaches that same closure label. Both paths then
execute authored `NULL rSaved; NULL rEvent` before the ordinary scalar tail
and sole RET. The source flag and scalar output may be observed after owner
closure. No opaque value escapes into that tail; ENDLIFE cannot replace NULL.
All COPY, flag, branch, projection and reset rows remain executable in their
original order. Add no storage, capture guard or synthesized cleanup.

The smallest maintained handwritten producer-shaped fixture is
`tests/io-signal-guarded-owner.rxas`, with this existing-ISA suffix (the save,
install, call and normal restoration retain the preceding exact 29-tag rows):

```rxas
  /* pre-scope: null rSaved; null rHave; load rHave,0 */
  /* normal reverse pops */
  br joined
caught:
  /* complete handler reverse pops */
  copy rSaved,rEvent
  null rEvent
  load rHave,1
  br joined
joined:
  brf owner_closed,rHave
  linkattr1 rRaw,rSaved,5
  linkattr1 rField,rRaw,4
  copy rName,rField
  unlink rField
  unlink rRaw
  null rField
  null rRaw
owner_closed:
  null rSaved
  null rEvent
  /* observe scalar name/flag/result and return */
```

This fixture must actually retain `rSaved` over `joined`, clear delivery
storage before the join, consume the saved name later only on the handler
path, and retain a preloaded name on the empty normal path. It represents the
capture/join/guard/use/cleanup lifecycle in G HTTP `first_problem` and C
capture, without claiming admission of their other calls, globals or relay.

`lltargetfacts.namedshape`/`boundshape` recognize only this additional suffix.
Use the existing `llirgraph` index and canonical branch catalogue to validate
same-procedure labels/edges, the guarded block, common closure and absence of
alternate ingress/backedges. Source metadata may split the consumer's address
groups: follow only a bounded same-function chain with one exact predecessor
and successor per group, no label thunk or repeated group, and exactly the
false-edge label thunk plus final consumer group entering common closure.
`namedrun` retains its existing normal and handler
`.cells`/`.frame`/`.history` snapshots. Before `joinpath` erases path detail,
establish the exact-owner relation: the normal predecessor has flag 0 and
zeroed saved storage; every feasible handler predecessor has flag 1 and a
completed owned runtime event. The relation belongs to this frozen module,
call/join/guard and the exact flag/saved StorageIds and bindings. Equal values,
a producer declaration or a merely nonzero unknown flag cannot establish it.

Preserve both alternatives through the join. On the proved BRF successors,
reuse the matching snapshots for the empty and event paths, applying the
existing checked projection transfer only on the event path. Do not restore
runtime-event shape from a scalar flag after an ordinary boolean meet has
lost the relation. Meet changed scalar/component/status, binding and recycler
facts with `joinpath` at `owner_closed`, then perform both common resets.
Incompatible history or storage bindings retain the call. This is a bounded
extension of `namedrun`, not another CFG/worklist framework or general predicate
solver. The target's exceptional normal-only access projection stays incomplete;
transport/storage retain their exceptional-CFG pending guards. No normal-only
liveness may delete the flag, skipped consumer, owned copy or cleanup.

Keep payload/native/reference ownership opaque under the preceding reviewed
COPY contract: hooks and descriptor identities are preserved, referenced
targets gain no language lifetime, and diagnostics retain their existing owner.
Native-hook purity, bitcopy safety, allocation success or resumed partial-copy
state must not be inferred from the catalogue's scalar COPY row. The user
accepts slight infrastructure-pressure failure differences generally, including
elimination of call-frame allocation FAILURE; such an opportunity alone is not
an admission blocker or a new failure-injection campaign. Deterministic language
state, delivered signal tag/payload/logical source and CANCEL/unwind remain exact.

Retain all existing delivery-phase and restoration proofs: handler feasibility
does not disappear for a known-passing check, and a delivered event reflects
the actual caller writes before/after the result commit. After restored policy,
an interrupt between capture and flag commit must keep its existing default
response; no path may reach guarded use with an invented event or stale flag.
Source/TRACE follows original call/check/join/guard/consumer/closure boundaries;
normal result TRACE stays conditional on normal completion, and no physical
traceback/raw-address observation or new semantic result event is introduced.

Independent constructed records in `signal-inline.crexx` and the handwritten
fixture cover known/unknown normal and fault actuals, result/actual alias,
normal empty owner/flag 0 with unchanged output, handler owner/flag 1 with the
expected later code/name/message, and both common cleanup paths. Keep a normal
execution whose skipped LINKATTR1 would fail if wrongly reached. Check whole
name COPY and message SCOPY's preserved integer/public status separately.
Reuse paired original/expanded `rxas -n`/one-VM execution, pending CANCEL before
and after result commit and after a normal observation, restoration/priority,
logical source and native/reference ownership probes from the current runners.

Negatives cover missing/wrong initial flag, wrong committed value, extra flag
or saved writes, unknown/mismatched guard, reads before the guard or on its false
edge, alternate entry/backedge, owner/flag aliases or rebound storage, early
owner reset, missing common cleanup and opaque escape/unsupported consumers.
Use specific `signal.owner.guard.pending`, `.mapping.pending`, `.state.unproved`
or `.cleanup.pending` reasons; retained scope/snapshot shapes keep their current
precise reasons. Execute one retained negative with independent expectations.
Inspect the first conservative output for unchanged authored guarded ownership
and observations, no original selected call, no fresh cells and no elision.
Charge correlation/snapshot/graph work and bytes before growth; tight limits,
replay and output failure retain exact input identity and CLI last-good output.
The coordinator owns the affected build and bounded development panel, reusing
prepared producers; broader functional/platform/sanitizer/performance QA is not
commissioned by this increment.

### Pending atomic signal-scope decision

The current G emitter's duplicate normal/handler reverse-pop lists cannot
reliably recover from delivery partway through normal restoration. A handler
may pop an already removed name, raising `INVALID_SIGNAL_CODE`; with an outer
same-name save, it may instead consume the outer scope silently. Marker
rebasing fixes pointer lifetime but cannot identify lexical ownership.
Making `SIGPOP` lenient, or recording pop progress in an ordinary register,
does not close the interruptible update gap or protect an outer same-name save.
The bounded inliner may preserve the existing RXAS behavior but cannot claim
that it repairs this G producer defect.

Consider a versioned, frame-owned atomic scope acquisition and restoration
contract, expressed by handwritten RXAS such as `sigscope rAcquired,"A B"`
and `sigrestore scopeLabel`. Acquisition would prepare the private policy and
all saves before publishing an identified scope, return a checked success
flag for a producer-chosen failed-acquisition continuation, and expose no
partial policy on failure. Restoration would validate the current scope and
restore it in one VM instruction without allocation or an intermediate
dispatch, never searching by name into an outer scope. The assembler/linker
would treat `scopeLabel` as a relocatable same-procedure identity; inline
clones need fresh identities. Exact failure, pending-native-signal, signal
effect, RXBIN feature and G lowering contracts require review before any
API/ISA/runtime adoption. This would solve policy restoration, not inherited
physical `SIGRET`, arbitrary callee cleanup or Classic activation relay.

### Approved borrowed-register private-global binding (INL-AC04)

**Approved by Adrian; implementation and focused qualification open.** The
handwritable procedure-level declaration is
`.inlinedprivate rK,"exact-provider",gN`. This is executable ownership/linkage
metadata, never an instruction, public exposure or compiler proof payload.
`.locals` remains the complete `r` extent. The declarations cover every slot
in one contiguous borrowed suffix; the first declared register is the owned
prefix length. Each suffix slot has exactly one binding. Repeated targets in
different slots are permitted and remain distinct slot/permutation identities.
No declaration changes the module's ordinary `gN` bank or public symbol set.
Declarations follow their owning procedure header and precede its first
executable instruction; their code address must equal that procedure's start.
Labels and nonexecuting metadata do not change that entry coordinate.

Append `META_INLINED_PRIVATE` under RXBIN007 feature bit 10
(`RXBIN007_FEATURE_INLINED_PRIVATE`). Each typed record contains the owning
PROC pool reference, an existing typed `QGR1` provider selector pool reference,
the local slot, provider global index and owned-prefix length. The assembler
infers and writes the same prefix length into every declaration of a procedure.
Validate procedure membership, total/owned/suffix bounds, complete coverage,
duplicates/conflicts, selector syntax, feature support and reference kinds.
The native linker resolves an exact selected module spelling, or an already
sealed image-local ordinal, and checks the provider's global bound. Re-linking
rebases old image-local bindings through the original image's module identity.
Unresolved/ambiguous/unsealed runtime bindings fail before execution. Retain
selected provider dependencies without inventing a public name or visibility.

The loader prepares per-procedure worker-local borrowed pointers once all
selected globals exist. Resolution allocates/indexes storage but runs no
initializer. Physical frame activation/reuse installs those pointers into both
active/base suffix slots and resets their permutation identities. The frame
allocates value backing only for the owned prefix and `a0`; aliases add no
per-call instruction or heap allocation. Reuse must not zero provider values,
even with SAFE_RECYCLED_STACKFRAMES. LINK/SWAP/UNLINK retain their normal active,
base and permutation semantics; the next activation reinstates each declared
suffix binding. Native register optimization must preserve suffix ownership
and binding identities, including declarations that have no remaining uses.
The bounded native cleanup panel obtains possible borrowed pointer transport
from the central binding/operand inventory over the complete procedure,
including backedges. Peepholes wait for that census. A record touching borrowed
storage, a possible borrowed carrier, or an implicit/opaque register window is
an observation barrier; unaffected owned-local records still use the existing
peephole and flow optimizers. The suffix is pinned and temporary-register
growth across it is disabled. This deliberately leaves provider-aware storage
reuse and movement across borrowed boundaries for later shared proof work;
it is not a whole-procedure optimizer bypass or an opcode admission list.

Every normal/abrupt teardown path skips borrowed suffix backing. Reference
owner lookup identifies it as original provider global storage, and frame exit
must not expire that lifetime. RET from a borrowed encoded local always COPYs;
MOVE is restricted to the owned prefix at its own base. An owned active slot
linked/swapped to borrowed storage also cannot acquire ownership. Explicit
value mutations still affect provider state; implicit frame cleanup must not
clear/free/expire it. Provider initialization, poison/cycle ingress and final
teardown remain with the provider. Alias setup cannot initialize an untaken
clone early; inlining still needs the separate initializer-readiness theorem.

Focused qualification covers handwritten optimized/unoptimized roundtrip,
feature/reference/corrupt-suffix negatives, exact selection/relink relocation,
worker isolation, ordinary and recycled frames, borrowed RET, LINK/SWAP/UNLINK,
reference expiry, abrupt teardown and initializer non-eagerness. Root owns all
build/test execution. This local feature does not close INL-AC04/STEP04/STEP06.

### Reviewed shared CFG prerequisite

INL-STEP02/04 extend the existing generic L IR owner for real machine CFGs.
`operation.control(flow,minimum=0,maximum=0)` declares structural `next` or
`terminal` flow independently of semantic effects. Default `next/0/0` preserves
existing manifests and behavior. Terminal extensions end a block and declare
an inclusive bounded successor count, including zero and table fanout.
`opdraft.successor(block)` appends an ordered edge only after exact live owner,
same-function, arity and prospective budget checks; repeated targets are legal.
Freeze and decoded-input validation reject missing/extra edges, cross-function
targets, operations after terminators and unterminated blocks identically.
Use existing owned edge rows and serialization envelopes, with nondefault
control declarations participating in exact schema compatibility.

The generic optimizer must preserve terminal extensions and control contracts;
structural admission alone does not authorize generic execution. The existing
L graph index consumes their declared edges. One independent non-RXAS graph
fixture plus existing core IR controls qualifies this shared API. Its enduring
account belongs in `langtools/doc/ir-design.md`. This shared API is implemented
and qualified for the selected ordinary owner checks; inliner integration and
broader INL acceptance remain open. There is no target-private graph framework.

### Reviewed lexer preparation repair

INL-STEP03 exposed excessive shared `llregular` preparation cost for the
unchanged RXAS Unicode lexer. The selected repair stays at that L owner and
preserves exact byte classes, DFA images, rule priority, first-discovery order
and existing NFA/DFA limits. Each DFA state derives its consuming-state list
once from the canonical sparse set; byte-class transitions scan that list.
Acceptance keeps the first/minimum-ID accept, with a separate seen flag rather
than assuming an accepting kind is nonzero.

The existing public value-argument `closure` contract remains; one internal
borrowed scratch walk serves last-use determinization callers and the public
compatibility adapter. Public subset sorting keeps the same stable merge
algorithm using packed i64 work buffers and pointer swaps, then the existing
i32 result encoding. Preserve integer ordering before final encoding and
retain public input ownership. Internal determinization has a stronger proved
invariant: its initially zeroed membership and unique seed queue correspond
exactly, and each newly visited state updates both once. After the shared walk,
emit canonical IDs by ascending membership scan, validating extent, 0/1 bits,
expected count and result bounds. Public callers retain queue-sorting behavior;
arbitrary membership/queue arguments do not acquire that stronger precondition.
Add no persistent cache, patched-NFA sharing, new graph framework or reduced
Unicode profile.

The owning `testGenerated` fixture adds an independently constructed exact DFA
image and ownership/duplicate-order controls. The repair and selected owner
checks are complete; real RXAS adapter integration remains open. This is a
bounded research preparation fix, not a production performance verdict or
wider language/platform campaign.

The initial adapter normalizes source-level leading plus signs and decimal
suffix case at its own boundary; generic integer encodings remain canonical.
Decoded string payloads, including constructed inputs, use the existing L
UTF-8 validator at the checked target owner. Embedded NUL remains valid and
binary literals retain arbitrary bytes. Preflight payload size and charge work
before decoding, copying or quoting; stop binding promptly after draft failure.
These checks belong to the adapter/target owners and do not change the grammar
or shared L encoding contracts. `draft.preflight(work,bytes)` delegates to the
existing shared target fuel and arena prospective-reservation checks without
reserving bytes or duplicating accounting. String decoding uses one count/emit
walk; it charges the source scans and checks decoded extent before allocation.
Trusted AST byte spans bound token conversion before its first owned copy;
private decoder inputs cannot bypass that prospective work check.
Constructed decimal literals require canonical lowercase `d` as emitted RXAS.

### Reviewed reusable machine-proof boundary

The inliner and cleanup are first consumers of reusable L machine analysis
intended to inform and merge with later RXAS optimization. The existing C
optimizer remains in place. Its pass demand/capability, immutable analysis
epoch, storage/component/use facts, proof-plan and atomic-mutation structure
informs this boundary; its conservative coverage is not the new pass's ceiling.

Existing optimizer code, metadata and tests are evidence, not correctness
oracles. Reuse maintained behavioral cases, check their expectations against
the intended semantics, and classify disagreements in the fixture, VM, current
optimizer or L consumer. The existing `storage_identity_runtime`,
`entry_alias_runtime` and `copied_xtoy_component_placement_runtime` fixtures
provide relevant alias/permutation, failure-mapping and independent-component
cases. Keep independent L expectations; do not copy an old defect or treat a
conservative refusal as a language restriction.

Register numbers are names, not storage or value identities. Track physical
slots/permutations, active and entry/base storage bindings, and independent
component ValueIds/presence/status. Swaps and links move or alias storage;
copies may preserve a component's value identity while creating distinct
storage and ownership obligations. Equal integer components do not establish
whole-cell equality, reference validity, attribute equality or release safety.
Keep absent, unknown and unavailable facts distinct.

Reuse `llirgraph` for declared CFG, traversal, dominance and loops. Add the
smallest checked finite-domain access projection to its existing bounded
liveness solver, bound to exact analysis/module owner, revision/schema, profile
and operation IDs. Separate reads/observations, possible writes and definite
kills; incomplete descriptions conservatively retain storage. Preserve the
existing core-slot adapter and qualify the generic extension independently.
Target attributes do not become implicit machine reads merely by indexing IR.

The generic API is `llirgraph.access(reference index,profile,entries,bytes,work)`
with `domain(functionId,key)`, single-assignment extension
`put(operationId,reads,observations,writes,kills)` and `seal()`. Existing slot
IDs remain reserved canonical `core.slot.ID` domains; callers cannot replace
core semantics. Validate owner/function/ranges, vector uniqueness and kills
as a subset of writes. Missing extension descriptions pin all domains in the
function. `index.accessliveness(reference access,work,bytes)` reuses the same
`live` solver and adds domain before/after/in/out queries. Existing default
liveness remains unchanged. Completeness describes access premises over
declared control, not executable lifetime or result availability; kills must
hold across every continuation. No edge-specific override is implied.

`lltarget` owns typed executable/metadata/TRACE reference enumeration and
remapping. `lltargetfacts` owns RXAS storage, component, presence, status,
context and observation transfers over those generic algorithms. Canonical
opcode/effect/signal data remains with its current generated owner; extend
extraction for needed clear/copy/derivation/context facts rather than creating
private mnemonic tables. Append compatible generated catalogue queries
`clearcomponents(opcode,operand)`, `samecopyisnoop`, `derivation`,
`derivationsource`, `derivationcomponent`, `contextreads` and `contextwrites`.
Keep existing columns and operand conventions; absent derivation source is
`-1`, and derivation-source indexes remain zero-based.
`llcleanup` selects rewrites using immutable checked
plans, validates expected records and publishes atomically. Structural changes
invalidate affected facts; rebuilding the changed caller is acceptable.

Checked arithmetic may leave a destination unwritten on a signal continuation.
For example, default-root overflow can be ignored. A possible write is not a
definite kill or a new available component: require success/range evidence or
an exact history/restoration proof before expansion or elimination relies on
it. Retain specific missing-proof outcomes while extending representable
coverage. Independent controls cover renamed/aliased storage, equal integer
but different other components, partial writes and required release effects.
The current scalar slice need not implement general SSA, wider machine CFG or
the complete optimizer; those remain open work through these same seams.
The generic access/liveness extension and compatible catalogue extraction are
implemented and qualified by their selected independent owner checks. Target
adapter and cleanup integration remains in progress.

Contributor preservation uses one additive L construction API:
`arena.contributingorigin(sourceId,first,last,kind,parents)` accepts ordered
prior-origin IDs, and `sourcefile.contributingorigin(first,last,kind,parents)`
accepts exact-owned origin handles. Validate all inputs and prospectively
reserve the existing origin/contributor rows before mutation; preserve order
and multiplicity. Existing single-parent `arena.origin` delegates to the same
algorithm with zero/one parent, keeping its bytes and wire contract unchanged.
The target facade delegates; copying retains all parents, and inline provenance
contributes both the complete copied body origin and call-site origin. No
private provenance structure, flattening or parent-count admission guard is
needed. Qualify this shared API independently before the target consumer;
batch the resulting generator refresh after target source/owner checks.
The shared contributor API and its independent compatibility/failure controls
are implemented and qualified; target consumer integration remains open.

### Reviewed scalar and transport increment

After the initial RXAS adapter is qualified, INL-STEP04/05 admit real compiler
metadata and multiple direct scalar leaf calls under the existing checked
closed-main/default-halt profile. Extend `llinline.expandmany(bindings,profile,
budgets)` to analyze one exact frozen caller and clone selected sites atomically;
the existing singleton entry delegates to it. Keep integer arithmetic as the
original checked VM operations, preserve argument COPY versus local MOVE/reset,
and admit numeric setup only when the complete context tuple matches the
proved caller tuple. Unknown policies/contexts retain calls.

Add independently callable `llcleanup.transport(module,procedure,profile,
budgets)` using the existing change result. It reconstructs the whole caller's
storage, active/base bindings, slot permutation, captured result, components,
status and observations; it can remove binding-only LINK/UNLINK and redundant
outer setup/reverse SWAPs only after remapping every intervening operand and
metadata observation and proving continuation equivalence. It needs no private
expansion tags. Keep required snapshots and machinery for retained nested calls.
The reordered-actual argument-return leaf must use direct COPY and add zero
frame cells. General allocation remains with the existing RXAS owner.
INL-STEP05 still includes factoring that owner's assignment/checking interface
for typed target records, then qualified reuse of proved private scalar storage.
Removing dead anchors while leaving sparse fresh register numbers is not full
storage-reuse closure. Its controls must cover nonoverlapping inline temporaries,
required snapshots, retained contiguous call windows and hidden cleanup; stronger
target facts may improve admission without duplicating the allocator algorithm.

Register-reference enumeration/remapping, source-step/clause identity freshening,
scoped META_REG/clear names, TRACE reference domains and L contributing origins
belong to `lltarget`; callable and opaque inline transport declarations remain
with the original callable. Transient checked storage/context facts may live in
`lltargetfacts`, with cleanup in `llcleanup`; there is no second graph/provenance
framework. Independent checks cover reordered actuals plus metadata, retained
snapshot with aliased mutation, two generated arithmetic calls including the
documented overflow signal, and an intact retained counted-call window.
Conservative and cleaned RXAS receive one coherent review before freeze.
This scalar/transport slice is implemented and focused-qualified; broader CFG, nested/writable/
optional/variadic, global and context coverage remains open.

Metadata identities use their actual unsigned 32-bit domains, including fresh
source-step/clause exhaustion. Normalize native source spellings at the text
boundary before constructing canonical records. TRACE code bytes are data, not
an invented enum. The typed raw-code reader/writer supports literal control and
DEL bytes. Backslash token-boundary handling remains a named adapter gap;
nonnegative `K` raw constant-pool offsets require logical constant binding.
These representable cases remain open; preserving an unrelocated number would
not establish preservation, even with expansion disabled.

The adapter treats TRACE fields 1/3/4/5 as raw quoted code bytes,
not ordinary escaped strings. Strip only their surrounding quotes, preflight
conversion from the trusted AST span, and render admitted bytes literally with
exact `2+length` output accounting. Preserve empty/default roles and unknown
codes without an enum whitelist. Raw control/DEL byte controls are qualified;
backslash token boundaries still need an L-side proof. Native source restrictions and a missing writer
proof are distinct. No grammar change is assumed. A stable `K` reference needs
typed logical constant binding plus native alias syntax for that field;
FLOAT/DECIMAL constants also need representation. Existing 007 relocation can
remain the wire owner. That syntax adoption retains its explicit decision gate.

### Reviewed straight-line rebinding increment

After the scalar/transport batch is qualified, extend the same target owners
for callee LINK/UNLINK and ordered SWAP. Active storage, immutable entry/base
bindings and slot permutation identity remain separate. Native SWAP changes
active pointers and slot identities; UNLINK restores the base of its original
encoded slot. A preceding swap does not change that base. Extend the immutable
per-call binding plan rather than introducing another mapping framework.

Give the expander untouched argument entry and local base anchors, retain the
entry-captured result pointer, and lower each UNLINK through its original base.
At each return, choose MOVE only when the original encoded slot is local and
its active pointer equals that slot's own base; an owned value in a different
swapped slot still copies. Transfer the result, restore permutation and active
bindings, then end only inline-owned lifetimes. Reverse admitted swaps as
needed to restore actual VM slot identities; relinking alone cannot do that.
The later cleanup may remove this transport only with its whole-region proof.

Independent controls cover swapped arguments followed by UNLINK, local link/
mutation/unlink with both owned and still-linked returns, a result aliasing an
actual while that formal is rebound, and repeated calls. Reuse relevant
maintained alias/permutation fixtures after checking their intended behavior.
Inspect COPY/MOVE and restoration-before-cleanup structurally. This straight-line
rebinding category is implemented and focused-qualified with conservative and
cleaned executable controls. Broader CFG, nested calls, optional/variadic and
aggregate/context cases remain open.

### Reviewed typed assignment and reuse seam

Factor the existing `llrxas.program` first-fit assignment and independent
checker once into a typed `llrxas` kernel. Keep text parsing/rendering and
target record remapping in their respective adapters; the RXAS owner gains no
target dependency. Rows carry stable record IDs and complete reference,
read/write/definite-kill/observation sets. Preserve fixed cells, retained
lifetimes and contiguous call windows, with current conservative control
boundaries and protected intervals. Missing uses cannot authorize reuse.

Separate identity preservation, dense unique placement and private scalar
reuse. A cell may disappear only when complete rows and all retained/fixed/
window/lifetime requirements prove its absence; validate absent mappings
independently. Preserve the existing text API's required-cell contract.
Target facts must prove candidate entry state, successful initialization,
absent hidden payloads and dead status/base/identity/lifetime obligations.
Return ordered typed NULL-reset requests with the mapping, preserving origins;
the independent checker must verify interference, constraints and required
resets without trusting allocator order. A live snapshot or release obligation
prevents reuse. Qualify sparse metadata remapping, disjoint versus overlapping
temporaries, retained windows and corrupted-map/missing-reset/resource failures.
Existing text META/TRACE adapters must either enumerate their supported uses
correctly or retain identity for unproved rows; their current conservative view
does not limit the complete target projection. No second allocator or generic
L solver is needed for this straight-line seam.

The additive kernel is `llrxas.assignment(virtualLocals,recordIds,
registerBudget,work,bytes)`. Stable positive record IDs are declared in source
order; rows and vectors are one-based, while virtual/physical cells are
zero-based. `row(ordinal,refs,reads,writes,kills,observations,origin)` is assigned
exactly once per record; validate unique in-range vector members, access sets
within refs and kills within writes. Borrow inputs through preflight and retain
owned copies, without borrowing target/module handles. Typed zero-cell input is
valid; the text adapter keeps its existing positive-budget contract.
`candidate`, `pin`, `fixed`, `window`, `retain`, `boundary` and `protect` carry
the checked constraints. Retain defaults true for legacy compatibility; pin
preserves distinct storage while fixed also preserves its index.

`finish(identity|dense|reuse)` returns an owned `assignmentresult` with
valid/error, frame size, virtual-to-physical mapping and ordered typed reset
requests (virtual/physical cell, arrival record ID, origin). Failure exposes no
partial mapping/reset list. `validate(mapping,locals,resets)` independently
reconstructs required cells, inclusive interference, fixed/window/identity
constraints and reset order. Rendering remains outside the kernel. Scoped
META_REG bindings protect storage through matching clear, while point TRACE
observations extend the relevant interval. Unknown frame observations retain
identity; a raw integer write is not itself a whole-value kill or entry proof.

The target adapter is separately callable `llcleanup.storage`, with the same
module/procedure/profile and work/entry/byte/frame/cell limits as transport,
returning the existing atomic change result. Reanalyze the frozen caller after
transport; derive complete typed executable, metadata and implicit-window uses,
checked candidate/entry/lifetime constraints, then validate and emit the mapping
and reset requests through existing target builders. Preserve complete origins
and source/TRACE references. The CLI invokes it after transport for each changed
caller; disabled cleanup and disabled expansion keep their existing controls.
Pin preserves storage distinctness; only actual fixed/window/frame-observer
obligations constrain physical numbering. Keep scoped metadata live through its
clear, and reject or retain identity when observation completeness is unproved.

For the focused executable reset control, admit the existing non-signalling
`GETANDTP_REG_REG_INT` in the closed-root scalar facts profile. It reads the
source status component and successfully writes only the destination integer
component; it neither releases a payload nor kills the destination's whole
cell. The transfer may mark the result integer defined but unknown, so no
constant-status assumption is needed. Preserve source/destination alias reads
and the existing component access projection. A constructed disjoint-lifetime
case must first copy a cell whose compiler flag band is 256, then reuse its
physical cell for a fresh second cell with a checked `NULL` before its first
partial write. Use `GETANDTP` with mask 768 on both copied outputs and execute
the transformed RXAS through the ordinary unoptimized assembler and one VM;
expect 256 and 0 independently of unrelated protected/VM-private flag bits.
The existing register-flags VM fixture and handlers are evidence for these
expectations, not an oracle for the L transformation or a blanket admission of
other status operations.

The same scalar storage proof may cross `ENDLIFE` on a proved fresh root cell
whose admitted history contains no reference identity, reference/native payload
or attribute tree: the VM lifetime handler has nothing to release there. Keep
the instruction and its reference interval; allow reuse only after that
interval ends, with the usual reset before a different fresh cell arrives.
Mapping aliases, calls, imported/recycled cells and hidden or unproved lifetime
state still need their separate ownership proof. Add a positive plain-root
`ENDLIFE` control; a blanket refusal is not a semantic rule.

### Reviewed retained nested-call increment

The next INL-STEP04/05 slice expands a straight-line outer body while retaining
an exact same-module scalar bytecode leaf call inside it. Admit fixed `CALL1`–
`CALL4` and counted `CALL` nested forms through the same checked closed-root
profile, including windows based in the outer callee's local or argument bank.
Keep the nested call opcode and its normal setup/reverse swaps. The first slice
does not infer argument presence from count, expand the nested call recursively,
admit an imported/native body or add a control-flow or exception mechanism.

`lltargetfacts` owns a transient checked call-shape normalization in the
**current frame**: form, entry-captured result pointer and zero-argument result
clear, count, ordered actual slot/storage pairs, and exact counted-window
bounds. Add frame slot-ordinal helpers rather than assuming the root's `r`
bank. A private nested invocation simulates the exact selected leaf and its
ordinary return/side effects, with a bounded active-procedure/depth check
before consulting per-procedure recycled owned bases. It must not publish a
second root access row or overwrite root-call facts using a nested record ID.
The existing root-indexed getters continue to describe only original root
sites. A readonly `orderedslots(rootCallId)` result requires the existing
contiguous local/`a0`/argument active-slot and untouched-anchor layout even
when no explicit callee LINK/SWAP was seen. The expander remaps the retained
nested opcode's typed registers; the outer return action still uses its
original encoded slot, current active storage and exact own base. Reanalysis
after expansion observes the retained nested window for placement.

Independent executable controls include a fixed nested call that mutates an
aliased actual after an outer integer snapshot (starting 5, child adds 7,
outer result 17 and caller actual 12), and an argument-bank counted window
whose outer swaps are restored (starting 10/20, child mutates the second actual
to 21 and returns 10; repeat reaches 22). Check conservative and cleaned RXAS
through native `rxas -n` and one VM, plus typed window/return/snapshot structure.
Unknown count, unresolved/nonbytecode body, active recursion and unproved
signal continuation retain the original outer call with specific reasons and
last-good atomic output. Current refusals are missing implementation/proof,
not infeasible language forms. Use maintained optional/vararg and signal-unwind
fixtures as behavioral evidence, checking their expectations independently.

Typed B may pad omitted fixed optional formals with real ABI slots whose
`REGTP_VAL` presence bit is clear; `REGTP_NOTSYM` is separate. Count is not
presence, and raw RXAS calls synthesize neither placeholders nor defaults.
Optional prologues and finite dynamic `LINKARG` remain subsequent representable
categories requiring status/control and selector/range proofs. Preserve the
producer's existing range check; raw `LINKARG` does not add one.

### Reviewed finite argument-selection increment

The next INL-STEP04/05 slice proves straight-line `LINKARG` selection in the
existing same-module, globals-free, closed-root scalar profile. It changes no
ISA, linker or source-language contract. Admit immediate selectors and
register-plus-immediate selectors only when the selector and checked sum are
known before destination rebinding, and the resulting index is within the
captured entry argument-bank extent `0..N`. `N` is the actual call arity, not the
mutable integer currently held by `a0`; index zero selects the current active
`a0`. A raw `LINKARG` has no implicit bounds signal. The selected source is the
current active `a[index]` pointer, including prior LINK/SWAP effects, never its
entry/base anchor.

`lltargetfacts` records a per-root-call, per-callee-record checked selector;
repeated activations of the same body cannot overwrite another site's proof.
It snapshots selector inputs before changing the destination binding, models
the active link and marks outer rebinding. Existing per-slot base and return
ownership facts still determine UNLINK and COPY/MOVE. A retained private leaf
may simulate a finite selector in its own frame but does not publish a second
root rewrite row. The expander lowers only the selected outer body's LINKARG
to ordinary LINK between its mapped destination and active argument slot.
Unknown, overflowed or out-of-bank selection keeps the original call with a
specific proof reason and exact original result. A proven-passing scalar
`ICHKRNG` may remain in the cloned body; an unknown/failing guard retains the
call under the current signal profile. This slice does not infer presence from
count, add optional defaults, or admit branch/handler paths.

Independent conservative and cleaned RXAS controls assemble with optimizer
disabled and execute on one VM. They cover active-versus-entry selection after
SWAP (starting `10/20/30`, mutate selected original second actual to `99`,
return `10`), mutable `a0` versus captured three-slot bank, selector/destination
aliasing with pre-link snapshot, original return ownership, repeated selected
sites, and exact retained-call reasons/atomicity. Inspect source and rewritten
LINK/UNLINK/window structure as well as values. Existing compiler vararg tests
are behavioral evidence, not a correctness oracle for L legality. Producer
branches/status presence and signal paths remain for the later CFG slice.

### Reviewed finite runtime argument-selection follow-on

**Status: implemented and focused-qualified; broader varargs and milestone QA
remain open.** This is a bounded INL-STEP04/05 vararg increment,
serving INL-AC02/03/04/05/06 without completing them or creating a milestone.
It changes no ISA, RXBIN schema, linker or source-language contract. Start with
one runtime register-plus-immediate `LINKARG` in an otherwise admitted
same-module, globals-free, straight-line scalar callee, with a read-only suffix
whose sole executable instruction is `RET` of that LINKARG destination. Keep
the existing exact entry arity and closed-main/default-OUT_OF_RANGE-HALT proof;
unknown call counts, inherited/resumable policy, loops, nested calls and a
second runtime selector retain their existing or specific pending-proof reason.
The first dispatch has at most three possible selected indices. A wider finite
domain is a growth-policy limit, not an ABI impossibility. The existing exact
selector path continues to handle its currently admitted broader suffixes.

The maintained native producer is `pick` in
`compiler/tests/rexx_src/inline_test_vararg_dynamic.crexx`: `arg ix=.int,
...=.int; return arg[ix]`. Its current `OP_ARG_VALUE` emitter in
`compiler/rxcpemit.c` emits `ICOPY rD,a0; ISUB rD,rD,F;
ICHKRNG aIx,1,rD; LINKARG rD,aIx,F`, where `F` is the visible fixed-argument
count. For `pick`, `F=1`; captured arities two through four give one through
three possible tail slots. This producer evidence does not imply that the
fixture's currently constant caller indices exercise a runtime-unknown fact.
The maintained handwritten `tests/io-linkarg-unproved.rxas` does: its defined
GETANDTP result is deliberately unknown to facts and its unguarded selection
must remain `argument.selector.unproved`. `tests/io-linkarg.rxas` separately
owns current-active selection, selector/destination overlap and repeated-site
controls. Extend these maintained owners rather than inventing another
producer or treating the float-loop/ref-mutating vararg fixtures as covered.

The new proof is a checked normal-successor refinement, not an assumption that
a runtime integer has a finite domain. Require the authored `ICHKRNG` immediately
before LINKARG apart from inert, already admitted metadata. Resolve its checked
operand and bounds through current StorageIds before changing any binding;
the checked operand must be the LINKARG selector and both bounds must have
independent known integer values at this activation. In particular, the native
upper-bound computation must prove its actual successful integer write from
the current active `a0`, rather than substitute captured arity for a mutated
`a0`. Prove a nonempty inclusive normal interval, checked selector-plus-offset
addition at both ends, width at most three, and every selected index in the
captured bank `0..N`. The bank extent remains entry arity `N`; index zero still
selects current active `a0`. Intersect existing integer bounds where useful,
but do not infer status, whole-cell equality, presence or ownership from an
integer interval. A completely failing guard is not needed for this first
admission and may retain its precise existing failure-proof reason.

Retain the exact authored range-check opcode, operands, order and logical
source. A runtime value outside its bounds delivers the same OUT_OF_RANGE and
halts before dispatch or result commit under the checked policy. Values on
that failing path need not have a safe raw LINKARG index, because that path
cannot execute LINKARG. Refinement applies only to the normal continuation;
an IGNORE/branch/call response or an alternate entry bypassing the check cannot
use it. Raw LINKARG itself neither checks bounds nor signals; never add an
invented range error to justify an unguarded, overflowed or out-of-bank access.
Unproved bound values, selector definition, guard provenance or signal policy
retain the call with distinct proof reasons and atomic original output.

This needs new bounded target control facts in `lltargetfacts`: the existing
exact-value ICHKRNG proof and one selector integer per original record are not
enough. The shared `llirgraph` remains the sole CFG/index owner. Record the
guard's normal-successor certificate and a per-root-call, per-selector-case
proof row containing its exact selected index, active binding and return
action/restoration. Each arm starts from the same immutable pre-LINKARG state,
refined to that selector value, and uses the existing storage/component,
history and return owners. Repeated activations cannot overwrite another
site's cases; neither can several cases overwrite the single original RET's
proof row. Never weaken `frame.compatible` or meet divergent active pointers
into an integer-value union. Return all arm results to the caller through the
existing whole COPY semantics for borrowed selected storage, then meet only
the restored caller states. Each case retains its true COPY, MOVE or ignored
return action; mixed owned/borrowed actions may retain the precise
`argument.selector.ownership.pending` proof gap in this first slice.
The selected actual and result may alias, actuals
may repeat, and prior admitted LINK/SWAP effects still select the active slot,
not its immutable entry anchor.

Emit the body prefix and range check once. After the check, capture the selector
integer in a fresh owned scratch before any destination LINK, including when
the destination holds the selector or a bound. Keep that snapshot stable and
use a separate initialized owned compare cell for every test. Use ordinary nonfaulting scalar
dispatch, for example checked-small-difference ISUB/BRF tests against each raw
selector candidate, followed by the proved final arm. Each arm emits ordinary
LINK to its exact current active argument slot and its own cloned return
commit/restoration/lifetime closure before the common caller continuation.
All generated arithmetic must independently prove success; a three-value
interval makes candidate differences small without changing caller state.
Scratch initialization/status and cleanup follow the existing owned-cell
contract. No selected borrowed cell is moved or ended. Preserve source/TRACE
events and scoped metadata through the existing fresh-identity owner, and
restore caller observation state before the continuation. Charge cases,
scratch, cloned return rows, labels, records, bytes and analysis work against
the existing request limits before publication. A representative emitted
dispatch and its proof/metadata rows require coordinator review before product
implementation is frozen.

The focused development panel extends constructed argument selection/keeps and
one maintained handwritten RXAS fixture. Produce runtime-unknown selectors with
two existing root BINEQ interval0..1 observations followed by checked IADD,
giving a straight-line unknown interval1..3, and execute every normal choice
with independent expected values. Cover the native `N-F` range shape, nonzero
offset, index zero with mutable active `a0`, prior argument SWAP, repeated
actuals, result/actual overlap, selector/destination overlap, two selected call
sites and repeat activation. Observe whole returned components and public
status as well as integers; the dispatch must leave unselected actuals intact.
A paired out-of-range witness requires original and expanded OUT_OF_RANGE,
the same logical check source/TRACE, and no continuation/result commit.
Negatives retain unguarded unknown selection, overflow, partly out-of-bank or
too-wide normal domains, unknown/changed bounds, a changed checked selector,
policy allowing range-check resumption, bypass entry and mutable/use suffixes.
Keep valid original modules exactly on refusal and exercise one bounded
work/growth rejection. Structural checks require no selected CALL/LINKARG,
one authored prefix/check, finite direct-LINK arms, pre-link selector capture,
COPY rather than borrowed MOVE, exact arm closure and truthful fresh metadata.
Run conservative and cleaned artifacts through native `rxas -n` and one normal
VM, reusing prepared producers; inspect the generated shape before expanding
the small panel. Cleanup may simplify dispatch only with its existing complete
access/storage proof; failure to prove cleanup keeps the conservative expansion.
The current runtime-selector fixture takes that keep path with
`argument.range.unproved` in cleanup, while the conservative expansion runs
correctly with and without the cleanup request. Its branch-aware range proof
and storage reduction remain separate INL-STEP05 work.
No broad compiler, sanitizer or performance campaign is commissioned here.

The named next representable step is a dependent scalar suffix that reads or
mutates the selected alias before UNLINK/RET, including the maintained
`inline_test_ref_vararg_runtime.crexx` use shape. It needs path-specific tail
cloning or an equally exact reviewed alternative through binding restoration
and every return, with status/alias/ownership effects proved for every case.
General loops, additional selectors, dynamic arity and deeper retained calls
remain separate open work. The first read-only return slice must not be
reported as general dynamic vararg acceptance; none of these current proof
gaps requires a new ISA merely because the present frame meet is exact.

### Reviewed dependent selected-alias continuation

**Status: linear scalar, retained CALL1 and single/multiple-RET acyclic suffix
slices focused-qualified; metadata re-entry and milestone QA todo.** This
INL-STEP04/05 continuation serves
INL-AC02/03/04/05/06/07 by carrying each finite runtime LINKARG binding through
its actual scalar consumers, writes and returns. It extends the qualified
width-at-most-three selector contract above; it does not complete general
varargs, INL-AC03/05 or the standalone integration milestone. No new ISA,
RXBIN/target wire record, language rule, type/proof META or native algorithm is
proposed. Keep the current linear closed-main caller, exact same-module
bytecode body, zero globals, captured arity, successful prefix and authored
ICHKRNG normal-successor certificate. Unknown counts, a second runtime
selector, loops and inherited/resumable signal policy remain outside this
increment with precise retained reasons.

Implement this contract in coherent development slices under the same plan:
first, a whole straight-line scalar suffix with one reached RET, including
selected writes, authored UNLINK and case-aware return/summary ownership;
second, one retained exact same-module scalar child CALL1 and its abrupt-exit
proof; third, bounded acyclic suffix branches with per-case graph, metadata
and per-RET proof. The maintained indirect-mutation producer is a required
completion control for the second slice, not evidence that the first slice
covers nested mutation. These stages do not defer any criterion of the full
inliner outcome or rename a representable case as infeasible.

The continued region starts immediately after the one checked LINKARG and
ends at every reached RET. It may read or write the selected alias, use
ordinary scalar temporaries, restore a link with UNLINK, and return either the
still-selected cell, a separate owned/borrowed cell, an immediate or no value.
Use the existing target transfer owner for every operation and component;
this is not an opcode-specific rewrite of one IADD or a source-name match.
Include straight-line suffixes and bounded acyclic direct branches already
understood by the existing DAG owner. A source branch may remain dynamic
within a selector case. It needs its existing defined-integer or ABI-bit
condition proof and per-edge state; integer equality does not make different
active pointers compatible. Joins inside one case still require the existing
frame/base/identity and history compatibility. Unproved branch-local binding
joins retain the call; separate reached RETs retain independent proofs rather
than forcing such a join. The qualified exact-selector and read-only runtime
paths remain valid inputs.

The maintained `inline_test_ref_vararg_runtime.crexx` is a required producer
control, not an oracle for target legality. Its `mutateTailIndirect` suffix is
`LINKARG r3,a1,1; CALL1 r4,bumpOne(),r3; UNLINK r3; TRACE; RET r4` in both native
optimized and unoptimized RXAS. An arithmetic-only tail does not close this
shape. Admit one existing exact same-module scalar bytecode leaf CALL1 in the
continued region, using the current retained nested-call normalization and
`invoke(...,publish=0)` owner separately in each arm. The child is not expanded
by this change. It has no additional call, selector, policy/context change,
physical-frame/address observer, native/dynamic dispatch, aggregate/reference
operation or opaque effect. Preserve its explicit result and borrowed actual
pointers, including result/actual overlap. Its ordinary scalar reads/writes,
return components and successful arithmetic must all be proved for the
current selected storage. Retain counted windows, other unproved child forms,
recursion/depth violations and further nesting with their exact current or
`argument.selector.tail.call.pending` reason; do not silently replace the child
with a duplicate arithmetic algorithm. The complete native mutation control
remains open if its nested-call or abrupt-exit obligation is not established.

`llirgraph` remains the sole graph/index/dominance/cycle owner. Validate the
linear prefix, guard, selected operation and entry to the continued region
against that exact frozen graph, including structural label thunks and
metadata address-group splits. Every executed route into the selected region
must have passed the same check with the same selector/bound StorageIds;
reject bypass edges, backedges and alternate ingress. Before any selector
snapshot, use the existing child preparation owner to reserve all possible
retained-child owned bases and history identities. Cases and repeated sites
must share the native procedure's existing recycler identity, not allocate
fresh child bases for each alternative or let a later case publish foreign
StorageIds into earlier snapshots.

For each original root call and raw selector case, preserve immutable entry
cells/frame/history plus the guard certificate and selected active StorageId.
Resolve selector and bounds before rebinding, keep the owned selector snapshot
stable, then transfer the continued region in that case's own state. Extend
the existing DAG transfer with a checked continuation checkpoint when needed;
do not materialize another executable graph, solver or target module just to
obtain suffix facts. Reuse `step`, nested `invoke`, `finishreturn`, existing
component/status meets and history/recycler checks. All following reads and
writes use current StorageIds: LINKARG does not copy the value, and UNLINK
restores the encoded destination's immutable own base, not the selected entry
slot or a copied snapshot. Subsequent admitted LINK/SWAP operations keep their
existing active/base/identity semantics in each arm.

Do not meet selector cases while their active bindings differ. Within an arm,
preserve once-only operation and actual-evaluation order, all selected-cell
mutations and aliases to other actuals/result. Returning writes the original
captured caller result storage after the source suffix's effects and before
inverse permutation, UNLINK and owned-base lifetime closure. Repeated actual
pointers remain one cell; equal integers in distinct cells remain distinct.
Observe integer, ordinary string/binary components and public ABI status
according to their existing transfer contracts. Integer arithmetic alone must
not replace whole state or presence; scalar writes preserve unrelated string,
binary and status components as the VM does. No selected borrowed cell may
be moved, nulled or ended merely to close the synthetic frame.

Retain per-case, per-original-RET live/branch/action/restoration evidence. The
same source RET can COPY in one case and MOVE in another; a bare RET can leave
the captured result unchanged while earlier alias writes still affect that
same cell. Remove the first slice's uniform-action restriction only when the
case-aware return owner and emitter represent those alternatives truthfully.
A common call summary takes the union of feasible reads/writes and definite
kill guarantees only when every feasible case/path establishes them. A legacy
single return-action or branch-choice query is usable only for a uniform
answer; it must not select case1's ownership/edge as a general premise.
Consumers lacking the case-aware proof keep their conservative result.
Merge only exited caller states and corresponding history after each normal
arm's result and teardown obligations; do not weaken `frame.compatible`.

The original range check still executes once before dispatch. Its invalid
selector path retains OUT_OF_RANGE/HALT without running the suffix or committing
a return. This certificate does not prove suffix arithmetic: every IADD,
ISUB, IMULT and retained child's `+7` must prove its actual integer write
succeeds for every feasible selected interval. Default OVERFLOW_UNDERFLOW is
IGNORE, so an unproved write must retain its precise arithmetic reason rather
than assume an exit, value or reset. Keep deterministic failures and source
state exact; the accepted variation in infrastructure-pressure opportunities,
including an eliminated outer call-frame allocation FAILURE, is a separate
preservation rule and does not discharge a signal-policy or unwind obligation.

In particular, a retained CALL1 can receive CANCEL or a terminal signal before
the authored UNLINK while the selected local is still linked. Require an
independent check against the existing VM call/unwind and owned-base cleanup
owners: default terminal policy cannot resume into the missing normal
closure; native and expanded teardown must release their own original bases
without resetting, ending or finalizing a borrowed actual. No outer handler,
host completion callback or declared physical observer may inspect synthetic
scratch or resume after a partial restoration. Prove the caller mutations
already performed, tag/payload/logical source, result-commit boundary and
retained-child behavior on that abrupt route. Do not synthesize normal UNLINK,
copyback or ENDLIFE on a borrowed cell during unwind. If this cannot be proved
under the current checked profile, retain the nested subshape with
`argument.selector.tail.unwind.pending`; keep direct scalar-tail admission
separate and the native indirect-mutation acceptance open. A required VM/ISA
change is an explicit decision gate, not an assumed part of this plan.

Emit the qualified single prefix/check and stable scalar dispatcher, then
clone each case's reached continued region into that arm. Relocate all source
branch targets and labels with a `(call,case,source-label)` identity, never
cross-link two cases because they share an original label. Use that arm's own
per-RET action and inverse permutation, full active/anchor UNLINK and owned
ENDLIFE closure before its common caller continuation. Do not omit authored
UNLINK or reset rows solely because another arm restores its binding. A
later transport reduction needs the existing independent complete proof.

The existing metadata/provenance owner must also become case-aware where a
continued region has different reached declarations or returns. Preserve the
shared prefix's observations, re-establish its live scoped bindings at each
arm when physical metadata layout requires that, and close each case's live
bindings without suppressing a later arm's declarations. Preserve logical
source and once-executed TRACE for reads, writes, retained CALL1, UNLINK and
RET; distinguish the LINKARG source from later suffix source steps, including
an empty initial source context. Use fresh physical identities where the
metadata contract requires them; no producer proof annotations. Every normal
exit restores the caller's observation state before its original result TRACE
or next instruction. An abrupt transfer suppresses the result event and
continuation it prevents; prefix or child TRACE already executed before that
transfer remains observable. Unproved branch-local metadata reconciliation
retains the existing specific metadata reason.

Extend the maintained constructed runtime-selection controls and handwritten
runtime RXAS runner. Keep the current straight-line BINEQ/IADD unknown-selector
producer and execute all three choices with independent expected values.
Required positives are direct selected `+7` followed by read/RET; mutation then
UNLINK and return of a separate owned value; the native CALL1/UNLINK/TRACE/RET
shape with expected selected mutation and untouched other actuals; repeated
actuals; result/actual and selector/destination overlap; prior SWAP; component
and public-status preloads; repeated sites/activations; and mixed owned/borrowed
RET actions. Include a bounded authored branch suffix with distinct feasible
RET/closure obligations. Test each actual cell as well as the result so a
correct result cannot hide wrong alias mutation. Reuse unchanged read-only,
int64-boundary and complete guard-failure controls.

Paired terminal CANCEL controls inject at child entry, after child mutation
before child RET, and after child return before the authored UNLINK; the
after-mutation/before-RET boundary is required. Current default CANCEL is
silent HALT: the existing RXAS runner can assert termination, pre-signal
output and absence of continuation/result TRACE, but cannot inspect local
storage after teardown or report tag/source from stderr. Tag/logical-source
attribution and owned-base-versus-borrowed cleanup need source/VM owner proof.
The existing scoped initializer-fault sink is a possible bounded test-harness
route to capture the delivered event and a selected payload at the signal
boundary; adapting it to main must be reviewed and would not by itself prove
post-teardown local state.
If exact post-teardown mutation/commit/ownership observation is required to
close the nested shape, design and review a test-only native/embedding probe
as an explicit gate; do not claim that the current runner supplies one or
change the product entry contract merely to make the test pass. Keep a range-failure
before mutation and a retained suffix-arithmetic failure with independent
expectations, rather than weakening either oracle for native parity.
Negatives cover missing/bypassed/changed guard, unsafe domain/offset, unknown
or potentially overflowing selected arithmetic, incompatible binding joins,
extra selectors, loops, unknown/native/dynamic/deeper or counted children,
policy resumption, unproved CANCEL cleanup, hidden payload/reference escape
and unsupported physical/metadata consumers. Preserve exact original input
and CLI last-good output on refusal; use specific existing reasons plus bounded
`argument.selector.tail.*.pending` gaps instead of a blanket suffix refusal.

Charge case state/history/proof snapshots, all scalar and child transfers,
checkpoint/graph work, retained proof copies and publication, per-case labels,
source/TRACE/scoped metadata, scratch and cloned exit/closure rows before any
growth. Sealed queries borrow immutable case/RET rows and consume no handed-off
work; do not deep-copy a whole case on each query or assume compiler COW/move
elision. Work/bytes/frame/code/entry limits remain atomic and deterministic;
wider domains stay growth policy rather than ABI impossibility. Inspect one
representative emitted direct-mutation and retained-child arm layout early,
then use the existing separate serial runtime-selector panel with native
`rxas -n`, one normal VM, conservative and requested-cleanup variants. Reuse
prepared producers and valid existing results. Cleanup may keep conservative
code until its separate range/access/storage proof is complete. The coordinator
owns preparation/execution; no broad compiler, sanitizer, platform or
performance panel is commissioned by this increment.

#### Second slice: one retained scalar CALL1 in the linear suffix

**Status: implemented and focused-qualified; broader branch/return shapes and milestone QA todo.**
Keep the first slice's one reached outer RET, linear caller/callee,
width-at-most-three checked selector, exact arity, zero globals and closed-main
default-terminal policy. Admit at most one suffix `CALL1` whose target resolves
to one existing same-module bytecode body. Validate this child explicitly as a
straight-line scalar leaf with one reached RET: no child call, LINKARG, branch,
handler/policy change, changed numeric context, initializer ingress,
native/dynamic target, reference/aggregate/native-payload operation or declared
physical/frame/address observer. Existing checked identical `NUMSCI` context
setup and source/TRACE/scoped metadata remain valid. Do not inherit the broader
forms accepted by ordinary `invoke` merely because that owner can transfer
them. Other suffix calls retain `argument.selector.tail.call.pending`; an
unproved terminal policy/cleanup obligation retains
`argument.selector.tail.unwind.pending`. Keep existing specific scalar,
arithmetic, depth, metadata and budget reasons when those are the failed proof.
Prefix calls, a second runtime selector, suffix branches and deeper nesting
remain later slices. Exact-selector inputs keep their existing path.
The first bounded child gate also retains child binding changes, ACOPY and
explicit lifetime operations with the call reason. These ordinary-scalar
forms can be assessed under the same ownership contract in a later extension;
their retention is a profile boundary, not an ISA impossibility.

Thread the original root `outerCallId`, invocation depth and tracking mode into
`runtimeselection`; do not substitute the child's source record ID for the
root's publication identity. Before taking any selector state/history/proof
snapshot, run `preparechildren(outerBody,state,history,1)` once through its
existing bounded owner. Its cached child local/a0 StorageIds must be present in
every fork, including an arm which has not yet activated the child. Preparation
is not activation or admission. Each arm then transfers the retained row with
`invoke(row,armFrame,armCells,armHistory,0,outerCallId,depth+1,0)` and keeps that
arm's resulting cells and recycler history through authored UNLINK and outer
`finishreturn`. The child uses the same immutable owned bases as other arms and
repeated sites; it must not allocate a fresh symbolic recycler per case.
Existing recursion/depth, uncertain recycled-cell reads, component availability
and child-return COPY/MOVE/none proofs remain binding. Balance `_active` through
the existing invocation owner and do not publish child call/proof/selector rows
as independent root sites or mutate global `_plain` while exploring one arm.
The existing full refresh from the merged exited caller cells owns tracking.

The CALL1 result and actual are resolved to current StorageIds before entering
the child, just as VM fixed calls capture explicit pointers before activation.
Repeated actual aliases and a result which aliases that actual remain one
cell. The child mutates the selected cell before its own return commit; the
outer captured caller result commits only at the reached outer RET. A child
result written into the LINKARG destination therefore follows that slot's
current borrowed pointer, whereas the maintained producer's separate `r4`
result remains its own storage until outer RET. Authored `UNLINK r3` restores
r3's immutable original base, without copyback or resetting the selected
actual. Use the existing per-case return/restoration rows and common aggregate
summary; never substitute case1 for a nonuniform result, binding or kill.
Reads/writes include all feasible child effects and definite result kills are
claimed only when all cases prove them. The child's `IADD a1,a1,7` requires a
successful write for every feasible selected interval; the selector ICHKRNG
does not settle overflow, whose default response is IGNORE.

The emitter keeps the child definition and target binding, and replays exactly
one remapped CALL1 in each finite outer arm. Only its explicit result/actual
register operands are mapped; no child register bank, body expansion, counted
window, reverse-window swap or arithmetic replacement is synthesized. Reuse
the first slice's scoped-binding checkpoint, arm source/TRACE relocation,
authored UNLINK, per-case normal teardown and caller-source restoration.
Preserve the CALL1 source step separately from the child definition's steps,
the following function-result TRACE and outer RET. Charge leaf validation,
preparation, every per-arm invoke/return/history transfer, proof construction
and all additional emitted rows before growth; sealed queries continue to
borrow immutable proof rows.

Default terminal CANCEL may arrive in the retained child before outer UNLINK.
The owning VM proof is `rxvm_release_frame_reference_lifetimes`, `free_frame`,
`clear_frame` and final frame-pool draining in `interpreter/rxvmintp.c`:
local/a0 lifetime release and final value destruction visit immutable
`baselocals`, not active linked pointers, and incoming a1+ pointers are borrowed.
The retained child has the same borrowed actual and native recycler in both
artifacts. The removed outer frame's hidden owned bases become caller-owned
synthetic bases, whose final teardown likewise cannot expire a selected
borrowed value. Under this scalar-only, no-observer, no-resume profile, neither
route resumes and neither needs synthetic normal UNLINK/copyback/ENDLIFE on
the interrupted path. Existing whole-cell/reference/payload guards remain
essential to that argument. Prove deterministic tag, payload and logical source
from the unchanged retained-child instruction or correctly relocated outer
origin, including the boundary before outer result commit. Do not demand the
same physical stack or infrastructure allocation opportunities. The selected
resource-pressure tolerance remains separate from deterministic signal/state
preservation.

Required controls extend the maintained constructed and handwritten runtime
selector panel, using its BINEQ/IADD unknown interval 1..3 and two repeated
calls. Copy the maintained native noopt
`mutateTailIndirect` guard/LINKARG/CALL1/UNLINK/TRACE/RET shape and `bumpOne`
leaf, with independently preloaded 10/20/30, ordinary text and public status.
Execute all choices and check V+7 then V+14, final selected V+14, unchanged
other actuals and preserved text/status. Require `argumentcases > 0`, finite
dispatch, the original child target, one CALL1 per arm, authored UNLINK and
separate return closures. The unchanged native fixture's literal `which=2`
caller is a producer/old-exact-path control, not evidence of runtime-selector
admission. Include repeated actuals, prior active-slot SWAP, selected
result/actual overlap, separate result, selector/destination overlap, repeated
child recycler use and supported per-case outer return actions. Inspect the
representative emitted arm before extending the panel. Missing/deeper/counted/
native child, child branch/selector/context/policy/observer, unsafe arithmetic
and finite-work/byte refusal controls keep exact input and last-good output.

Mandatory abrupt controls use ordinary CLI main original/expanded pairs with
test-only CANCEL injection after transformation at child entry, after child
mutation before child RET, and after child return before outer UNLINK. Keep
the same stable logical injection source in both artifacts. Fresh test scratch
may print the boundary selected integer/text/status without changing the
selected cell; verify independent boundary values, rc0, empty stderr and no
prevented result/continuation TRACE or output. Pair this executable evidence
with the owned-base teardown and result-commit source review above. Default
CANCEL is silent HALT, so the runner does not observe its tag/source in stderr
or read root locals after teardown, and these controls make neither claim.
Keep the existing invalid-selector-before-mutation and overflow-IGNORE refusal
controls. Authored SIGNAL probes are injected only into paired transformed
artifacts; they do not expand the admitted child opcode/policy profile.

An exact event/payload/source observer is optional supplementary evidence and
requires a separately reviewed bounded native test-harness change. The existing
scoped initializer-fault sink captures SILENT_HALT before teardown only when
`ext_proc == invocation_fault_entry`; it cannot be armed for implicit-main
entry by setting a sink alone. A possible test-only adapter selects the same
main as explicit entry, reproduces CLI a0=1 and its one argv-array a1, leaves
ext_ret NULL/default policies, and restores all sink/entry/external-call fields
on every exit. It must call ordinary `run`, not initializer execution, which
rewrites a captured silent fault into initializer status. A boundary CANCEL
payload can snapshot selected scalar state and the owned event can be inspected
after run; this is not post-teardown root storage observation. No new VM/public
API, production profile, signal policy, resolver algorithm or native mutation
helper is authorized. Lack of this optional observer alone does not retain a
case whose mandatory paired controls and ownership/source proof are complete;
any concrete failure of that proof is reported before product admission.

#### Third slice: an acyclic suffix with one shared RET

**Status: implemented and focused-qualified; metadata re-entry and
milestone QA todo.**
This INL-STEP04/05 increment admits conditional branches
after the checked runtime LINKARG, with one shared reached source RET. Keep
the qualified linear closed-main caller, exact same-module body/arity, zero
globals, width at most three, successful linear prefix and default terminal
policy. Reuse the current scalar suffix operations and at most one static
retained CALL1 row with its existing exact scalar-leaf gate. The call may be
conditional; a skipped child neither activates its recycler nor defines its
result. Multiple source RETs, prefix branches/calls, extra selectors, loops,
resumable policy and broader child forms remain explicit follow-ons under the
unchanged full selected outcome. This slice is a reusable DAG continuation,
not a special rewrite of one diamond or one arithmetic opcode. No ISA, VM,
public embedding API, wire record, proof META or native algorithm is needed.

Use only the frozen `llirgraph` index for block/operation order, successors,
reachability, dominance and cycle checks. The selected operation divides the
body into a proved single-entry linear prefix and an acyclic suffix. Every
executed route to the LINKARG must pass the same authored ICHKRNG with the
same current selector/bound StorageIds, and its next executable operation must
be that LINKARG. Metadata splits and structural label thunks do not require
separate instructions or invalidate this relation. No normal edge may bypass
the check, enter the suffix from another prefix path, or return to the check,
LINKARG or earlier prefix. Every feasible normal suffix route must terminate
at the same original RET, after compatible branch joins. The shared RET need
not be last in source layout: an acyclic graph may contain later source blocks
which branch to an earlier RET, subject to the scoped-metadata boundary below.
Additional reached RETs retain
`argument.selector.tail.return.pending`; their case-by-RET proof/publication is
the next slice, not a permanent exclusion.

`invoke` currently routes any branched `_dagbody` to whole-body `dag` before
linear runtime selection. Add a controlled runtime-selector interception to
that existing DAG owner, at the checked guard's current transfer point, while
preserving ordinary DAG/exact-selector behavior for other calls. The guard's
unknown normal path must not reach ordinary `step` before case refinement.
Save an immutable guard-entry checkpoint containing cells, active/base/identity
frame, history, prefix proof and original call/depth/context before the unknown
ICHKRNG and before LINKARG rebinding. Separately record the continuation cursor
immediately after LINKARG; it is an operation position, not an already-rebound
common state. That cursor can be inside a block; resuming at the block head
would repeat prefix/check/write effects, while starting at its successor would
skip remaining suffix operations. Factor the existing DAG transfer to seed
that position from each case's transferred state and continue its existing
RPO/edge/meet logic. Do not flatten a
branched body into `_linear`, restart a partially transferred prefix, create a
second executable graph/solver, or activate the outer frame again on resume.
An unrelated or exact guard must continue through its unchanged existing path.

Before saving the forkable guard-entry checkpoint, validate the suffix and
prepare the retained child's owned StorageIds/history through the existing
owner, including a child reached on only one edge. The immutable checkpoint
includes those prepared cells and history dimensions without activating the
child. For each feasible raw selector, clone the pre-check guard-entry
checkpoint, refine and transfer the original guard's normal successor, transfer
any intervening metadata and the checked LINKARG exactly once, then seed the
separate post-LINKARG continuation cursor with that case's resulting state.
Each suffix block owns case-local
cells/frame/history; child invoke remains publish0/tracking0 with the original
root call identity and correct depth. Use the existing frame-compatible joins,
component/status meets and history rules inside one case. Equal scalar values
cannot reconcile different pointers or slot identities. A child-not-activated
path remains neutral in the recycler-history meet, without granting a value
definition or reset. Finish the shared RET, exact result/alias commit and
normal closure separately in every case; only exited caller cells/history are
merged across selector alternatives. Refresh tracking from that final merge.

Admit the existing DAG branch forms `BR`, `BRT`, `BRF`, `BRTF` and the checked
ABI masks of `BRTPANDT`, using the index's successor ordering. BRT means nonzero,
BRF zero; an unknown condition can keep both edges within the same selector
case. Use only refinements already supplied by that owner, including its ABI
edge refinement. Do not invent integer interval refinement from a branch or
treat a possible arithmetic failure as a terminal edge: every feasible write
still needs the existing success proof because overflow defaults to IGNORE.
The selector's case equality is an entry fact, not a sticky invariant. Later
writes through that storage or an alias update ordinary cells facts; later
conditions must see the new value. The emitted dispatcher retains its stable
pre-binding selector snapshot even if the source selector/destination changes.

Extend the existing sealed selection rows with borrowed `(call,case,record)`
live/branch-choice queries. The emitter must not use union liveness or a
case-aggregated branch choice to select an individual case's code. Since every
case reaches the same source RET, current per-case action/restoration and
`caseexit` aggregation can remain exact; mixed COPY/MOVE/none actions remain
case-local and legacy nonuniform queries remain unavailable. Reads/writes are
unions and definite kills require all feasible edges/cases. Multi-RET support
later extends these rows to `(call,case,RET)` plus an aggregate union of return
sites; it cannot select the first reached RET or reuse case1 as the summary.

Emit the prefix/check and stable finite dispatcher once, then iterate cases
outside the suffix's source-record traversal. Emit the entire reached suffix
of one case, including source blocks after its RET, before starting another
case. A RET emits that case's proved result action, complete inverse/binding
and owned-base closure, and a branch to the common caller continuation; it
does not advance the case loop. Allocate every suffix source label and branch
target under `(call,case,source-label)` before emission, with no cross-case or
prefix target. A known case edge may be simplified by its own proof; a dynamic
edge remains dynamic. Keep the original retained child definition/target and
only the reached case's ordinary CALL1. No borrowed selected cell is moved or
ended by generated closure, and authored UNLINK keeps immutable-base meaning.

Scoped metadata checkpoint/resume reconciles cases, not different paths inside
one case. Retain branch-local opens/clears lacking the existing dominance/order
proof with `metadata.scope.cfg.pending`. For this slice, live scoped bindings
must have consistent common entry/join state and their admitted opening/clear
rows must dominate the shared RET as the current DAG metadata owner requires.
Clone source/TRACE references coherently for each case, preserving each live
record's original lexical source state even when an adjacent dead record is
omitted; metadata address state is not a condition that executes at runtime.
Restore the saved LINKARG entry state, including an empty source context, at
each case, and caller source before continuation TRACE. Every generated normal
RET exit closes its live scoped META before the branch to continuation, with
matching scope bookkeeping. A later-layout reached block could consequently
need its original lexical binding state re-established at that physical
address; inter-case checkpoint/resume alone does not prove this. In this first
slice, retain RET-before-later-block layouts requiring scoped-binding
restoration with `metadata.scope.cfg.pending`. Such layouts without that
obligation still use whole-case emission; do not reject their source/TRACE-only
graph shape. General per-block scoped-state restoration remains explicit
metadata follow-on work. The first implementation conservatively keeps every
early-RET layout with a reachable scoped META opening, including an opening
already closed by a dominating clear. A constructed negative records this
precise `metadata.scope.cfg.pending` boundary until the metadata owner proves
the actual live lexical entry state. Freshen only identities whose metadata owner requires
physical uniqueness; retain truthful logical clause/source relationships and
once-executed observations.

Required independent constructed/handwritten controls reuse the straight-line
BINEQ/IADD unknown-selector producer, scalar/status preloads and prepared
ordinary assembler/VM. One diamond computes `selected - 10` in reset scratch,
adds 7 on its zero edge and adds 9 otherwise, then joins with the same active
map and returns selected storage. Selector cases for 10/20/30 must yield
17/29/39; two repeated calls yield final 26/38/48, with unchanged ordinary
text/status and untouched other actuals. A separate BINEQ-controlled initial
10 or 11 exercises both feasible edges within selector case 1, with independent
first results 17 or 20. Assert three sealed selector cases, known different choices across cases
and a genuinely unknown case-local choice; native constant callers alone do
not exercise this lane. Include one common RET physically before a reached
later source block, and structural checks for private case labels/targets,
single prefix/check, all reached blocks, one normal closure per case, matched
META and restored caller source. One source/TRACE control distinguishes the
two branch clauses and the common return, including a removed dead neighbor.

Additional focused controls cover a conditional retained child versus an
explicitly defined fallback (including skipped then reached child at repeated
sites), selected result/actual overlap, a selector-storage write before a
later branch, and compatible value/status joins. Negatives include guard
bypass/extra ingress, a cycle, a differing active-pointer join despite equal
integers, undefined condition/result on one edge, one-edge arithmetic overflow,
branch-local META change, multiple RETs, extra selectors/children and atomic
work/byte/frame/code limits. Use existing precise guard, branch, join,
arithmetic, metadata and policy reasons; use
`argument.selector.tail.entry.pending` only for the new unproved continuation
entry/region shape. Retain exact input and last-good output on refusal. A
separate RET would make some binding joins representable, so keep that next
case-by-RET extension explicitly open instead of weakening `frame.compatible`.

Keep the qualified scalar/default-terminal unwind proof. Reuse unchanged child
CANCEL evidence and add paired post-transformation cancellation after mutation
on each newly exercised branch route, before common normal closure. Observe
independent boundary integer/text/status, rc0/empty stderr and no prevented
result/continuation marker. Source/VM owned-base review supplies the teardown
and logical-origin argument; no post-teardown root-local or silent-event
observer is assumed. The accepted variation in infrastructure-pressure paths
does not discharge deterministic failures, mutation, source or cleanup proof.

Charge the checkpoint and intra-block cursor validation, all graph/region work,
coexisting original/case block snapshots, meets, child preparation/invokes,
live/choice/return proof copies and aggregate publication before growth. Charge
per-case label/source/TRACE/scoped operand copies by their actual lengths, plus
all cloned branches and normal closure rows. Sealed queries borrow immutable
rows and do not perform uncharged copying after facts publication. Width stays
at most three; existing finite work/byte/frame/code limits bound suffix size,
without a new arbitrary one-diamond rule. Inspect one coherent emitted graph
early, then freeze for the coordinator's focused serial constructed/RXAS panel,
sharing prepared producers and valid unchanged scalar/child results. No broad
compiler, sanitizer, platform or performance panel is commissioned here.

#### Fourth slice: multiple RETs in the checked acyclic suffix

**Status: implemented and focused-qualified; metadata re-entry and milestone
QA todo.**
This INL-STEP04/05 increment removes the shared-RET limit
from the qualified runtime-selector DAG continuation. Keep width at most three,
the linear closed-main caller, exact same-module body/arity, zero globals,
successful linear prefix, one authored guard/LINKARG pair, acyclic suffix and
default terminal policy. Keep the existing scalar operations, branch forms
and at most one static conditional CALL1 under the same scalar-leaf/depth gate.
The existing guard-entry checkpoint, separate intra-block continuation cursor,
stable dispatcher snapshot and prepared child recycler identities remain
unchanged. No new graph/solver, ISA, wire record, producer proof META, public
embedding API, signal policy or native algorithm is required. Multiple RETs
are useful representable coverage; the bounded producer survey found no real
nonfixture runtime-LINKARG/acyclic/multi-RET site and supports no application
frequency, acceleration or performance claim. Loop-based native vararg uses
and branch-local metadata remain separate open categories.

Every feasible normal suffix route must reach a proved RET. RET count is
bounded by the existing source-record/work/code allowances, not a new two-RET
shape rule. Unreachable source RETs contribute no feasible exit. Continue to
use the original frozen `llirgraph` topology, per-case block cells/frame/history
and existing conditional-edge facts. At a real pre-RET CFG join, the existing
`joinpath` binding/base/identity and history compatibility still applies. Two
different RET blocks instead finish independently; do not force their sibling
active maps through a synthetic frame join. Thus a path returning a still-linked
borrowed cell and a path restoring a link and returning an owned temporary can
both be represented, even when joining their active maps before return would
fail. This does not admit incompatible bindings that actually merge before
the same source RET.

`callproof` remains the owner of each case's reached record/choice rows and
per-original-RET action/restoration. Extend `selection` and its sealed facts
adapter to expose the feasible `(original call, selector case, original RET)`
set, a RET-live predicate and exact result action/inverse permutation for each
member. Validate original owner/body/RET identity and membership in that case;
a RET reached only by another case is infeasible here, not action `none`, an
empty restore or an assumed exit. Case record liveness and branch choices stay
the union of that case's feasible paths, as needed by the existing whole-case
emitter; no new RET-conditioned executable graph is needed. Enumerate reached
RET IDs deterministically in original source order, with no duplicate row.

Replace singleton `caseexit` aggregation with one whole-case proof absorption
per feasible selector case. Union record liveness, feasible branch outcomes
and distinct reached RET IDs. For a particular RET, compare action/restoration
only among cases which reach it; absent cases add no `none` action. Keep an
aggregate action only when all reaching cases agree, and aggregate restoration
only when every reaching case proves the same ordered pairs. Nonuniform rows
remain unavailable to legacy queries. Possible reads/writes use the existing
conservative union; result/component definite kills are the intersection over
every feasible case and exit, including bare and component-only returns.
Absorb each case's already-complete write/kill summary once, rather than
reinitializing summary state by invoking the old singleton routine for each
RET. A global first return, first case or first action is never a summary.

Remove the singleton dependency in `selection._return` and the runtime action/
restoration queries. The emitter must request the exact current original RET ID
for every reached case exit. Separate successful runtime-selection completion
from `invoke`'s current `returned = proof.site(1)` shortcut. Store the call's
singleton `_return` only when its aggregate has exactly one feasible RET;
otherwise store zero and use aggregate enumeration. A public singleton guard
does not justify retaining a misleading first site internally. Legacy singleton queries may be used only when
their documented query scope has exactly one feasible RET; otherwise they
must give the existing unavailable/error result, not pick a representative.
Legacy per-RET action queries remain valid only for a uniform reaching-case
answer, and restoration queries keep their existing nonuniform guard. Preserve
the qualified single-RET/linear/exact paths and old callproof consumers. All
new membership/enumeration/action queries borrow immutable rows after sealing
and consume no uncharged deep-copy work.

At each reached RET, `finishreturn` resolves that path's current StorageId,
captured caller result pointer, action and inverse permutation before any
exit merge. Retain register COPY/MOVE, integer/string component return and bare
RET semantics, including an ignored result under an existing call form. A bare
RET leaves the captured result's previous components, while earlier writes
through an aliased actual still affect the same cell. Whole returns preserve
ordinary integer/string/binary components and public status; component returns
do not invent whole-cell kills. Owned MOVE resets only its actual owned source;
neither result overlap nor a different sibling exit authorizes moving or
ending borrowed storage. Merge only normally exited caller cells/history,
then refresh all affected tracking from that merge. Outer recycler reset
guarantees must hold on every normal exit; conditional child history keeps
its existing never-activated-path semantics.

Reuse the current whole-case source-order emission and `(call,case,label)`
namespace. Filter each RET by its case-live row, then emit its own proved
result commit, inverse restoration, active/anchor UNLINK, owned closure and
branch to the one caller continuation. A RET does not advance the case loop
or suppress later reached source blocks. Do not share an exit epilogue whose
binding/action/ownership obligations differ. The emitted number of closures
is the number of feasible `(case,RET)` rows, not width times all static RETs.
An explicit case/RET proof with an unexpected action must fail validation;
the emitter must not silently treat unavailable or mixed action as COPY.

Keep the scoped-metadata boundary with an explicit whole-original-CFG gate
before any selector fork. For the selected outer body, every structurally
reachable scoped opening or clear must dominate and precede EVERY structurally
reachable outer RET, using the frozen index and same-block operation order.
This obligation is independent of case feasibility, case record liveness and
which exit a refined case reaches; retained-child metadata keeps its unchanged
owner. Do not replace this gate with per-case dominance or omit a row because
it lies on another case's untaken control edge. Scoped META describes lexical
address state, not an operation whose effect is suppressed by a branch.

Also retain any structurally reachable outer scoped opening together with an
early-layout outer RET followed by another structurally reachable outer
instruction, even when a dominating clear already closed that opening. Use
`metadata.scope.cfg.pending` for either failed gate. A generated RET closes
active META before branching to continuation; inter-case checkpoint/resume
does not prove the original lexical entry state at later source addresses
inside the same case. The existing per-case DAG metadata check remains useful
defense but cannot establish this whole-CFG lexical obligation. In particular,
disjoint branch-local openings followed by separate RETs must retain even
when each selector case chooses one branch and passes its own dominance check.
Consequently first multi-RET positives use source/TRACE without those scoped
bindings; ordinary supported procedure declarations and the original retained
callee definition remain intact. Do not strip authored metadata to obtain
admission. Source/TRACE step/clause/register relationships, a removed dead
neighbor's lexical context, distinct RET source steps and caller restoration
stay exact on every emitted route. General block-local scoped-state
re-establishment remains a separate proof increment.

Required independent constructed/handwritten controls extend the maintained
runtime branch owner with genuinely unknown selector facts. The baseline
suffix computes `selected - 10`, branches to an add7/borrowed RET on zero, and
otherwise performs add9, whole-copies to an owned result, restores the selected
link and returns that owned result. For initial 10/20/30, first results are
17/29/39 and repeated calls leave 26/38/48; text/public status and unselected
actuals remain unchanged. Check COPY versus MOVE per exact source RET. A
BINEQ-controlled initial 10 or 11 makes both RETs feasible inside case1, with
first result 17 or 20. Also assert different cases' distinct feasible RET sets,
aggregate source-order enumeration, and an infeasible case/RET action query's
rejection. Do not report width times static RETs as reached coverage.

A constructed matrix must exercise the same RET with MOVE in one reaching
case and COPY in another, plus a sibling bare RET reached by a different case;
verify unavailable legacy action/restoration where nonuniform and no false
whole/result kill. A separate seeded bare/integer/string return control checks
preserved components, status and the caller's old result. Include repeated
actuals, selected result/actual overlap, prior SWAP and per-RET inverse pairs,
and a conditional retained child versus a defined fallback at distinct exits.
Repeat sites/activations to exercise owned MOVE/reset and skipped/reached child
history. Convert the current constructed multi-RET refusal to an intended
positive only with the new proof. The handwritten old multi-RET negative
contains `pick.which` META and can legitimately remain a metadata refusal;
new admission controls must remove that fixture-only binding explicitly or
use a later independently proved metadata layout. Keep an explicit metadata
negative so that this distinction cannot hide behind correct values.
One required constructed negative has case-selected disjoint branches with
`open A; RET A` versus `open B; RET B`. Each case individually reaches only its
own opening/RET, but neither opening dominates the other original RET. It must
retain with `metadata.scope.cfg.pending` through the pre-fork gate, regardless
of per-case branch simplification. A branch-local clear of a common prefix
binding is subject to the same whole-CFG rule.

Negatives retain guard bypass/extra ingress, cycles, an incompatible binding
join before a shared RET despite equal values, unproved branch conditions or
integer reads on one route, one-route arithmetic overflow, extra selectors,
unsupported/deeper children, resumable policy and hidden payload/physical
observers. A fresh ordinary null can be returned whole; it becomes a missing
integer proof only when an actual integer consumer requires that component.
Every feasible arithmetic write still needs success because overflow defaults
to IGNORE. No RET proof is fabricated for an exceptional/dead-end route.
Preserve exact input owner and last-good CLI output on malformed or bounded
refusal, using existing precise reasons and the specific return-summary keep
only when the new summary cannot be established.

Default terminal signal/CANCEL preservation retains the scalar owned-base VM
proof. Add paired post-transformation cancellation before the borrowed RET and
before the owned RET after their mutations, checking independent boundary
integer/text/status, silent rc0/empty stderr and no prevented result or caller
continuation. Reuse unchanged child-entry/inside-child/after-child CANCEL
evidence when its profile is unchanged. Distinguish alias-visible mutation
from a return commit; neither silent stderr nor destroyed root locals is an
event/post-teardown state oracle. No optional native observer becomes a gate.
Accepted infrastructure-pressure variation does not relax deterministic
source, signal, mutation or ownership obligations.

Precharge all per-case RET membership/site/action/inverse rows, full-case proof
absorption, transient/retained constructor copies, source-order enumeration,
exited state/history meets and per-feasible-exit emitted closure before growth.
Reservations may conservatively bound width times static RET count, but emitted
and published coverage must describe only feasible rows. Include restoration
vector lengths and metadata operand lengths; do not assume COW/move elision.
Atomic work/byte/frame/code refusal controls use existing limits and unchanged
publication owners. Inspect one representative mixed-exit RXAS region early,
then freeze for the coordinator's focused serial constructed/handwritten panel
and affected existing paths, reusing prepared producers and valid results.
Broader INL-STEP06, metadata re-entry, loops and initiative acceptance stay open;
no compiler-wide, sanitizer, platform or performance panel is commissioned.

### Scoped lexical metadata re-entry increment

**Status: direct acyclic and runtime-selector suffix/stable-prefix/LINKARG-group
slices implemented and focused-qualified; milestone QA todo.**
This INL-STEP04/05 increment replaces the
conservative scoped-META dominance/early-layout gates only where the original
lexical address state can be reproduced with stable observed storage. The
direct acyclic scalar path and checked runtime-selector follow-ons share one
projection/reconciler with the same case-local publication contract. Retain all
existing caller,
arity, operation, successful-arithmetic, child/depth, module and terminal-signal
gates. There is no new source-language rule, RXAS syntax, ISA, wire record,
linker/VM algorithm, producer proof META, graph solver or register-reuse slice.
The original `.meta "name"="b" "type" rN` declaration and
`.meta "name"` clear express the contract for handwritten RXAS too.
The direct-DAG slice admits new REG `rN` re-entry only. Preserve the qualified
common-entry `aN` path and its shared closure. The bounded runtime prefix
follow-on below admits stable prefix `aN`; newly suffix-local/changing `aN`
metadata retains its existing unsupported boundary.

A bounded static source-layout sample found 243 scoped acyclic definitions,
39 with multiple reachable RETs and 8 with branch-local openings in the 112
unoptimized goldens; the 112 optimized goldens had 122/19/3, and the maintained
Prolog RXAS example 77/49/19. These counts indicate practical layout demand;
they are neither eligible call-site counts nor application/hot-path rates.

Build one bounded target-owned lexical projection from the frozen original
procedure's source-ordered address groups. Carry supported ordinary REG/CONST
descriptors and matching clears, their complete operands, declaration identity
and origin. Include authored rows on skipped or structurally unreachable
machine groups when they determine the state at a later surviving address;
META does not execute conditionally. Keep the incoming state before each
address group distinct from that group's ordered authored records. Detached
body-end rows retain their existing non-cloning contract. The projection uses
the shared target record/address identities and existing scope owner; it is
not another CFG or predecessor-state solver. At a CFG join, the desired scope
state belongs to the original lexical address, rather than being a meet of
the scopes encountered on incoming execution paths.

Use the existing path-local frame/cells proof to observe every register
descriptor live at every reached original address, including one whose opening
record's machine group is skipped. Preserve whole-state entry independence
and the current stable `_scopestorage` obligation per original declaration
through its matching clear: all feasible observations, including selector
cases, must resolve the same StorageId. Keep `metadata.binding.change.pending`
when that storage changes; keep the existing precise recycler/undefined or
runtime-observer refusal when a live observation lacks its proof. A declaration
point alone cannot discharge its whole lexical interval. Constants need no
invented register observation. Preserve all existing reference, ownership and
physical-observer gates. Changing-storage scope splitting and DAG reuse remain
separate follow-ons, even if conservative physical glue could represent more.

Reconcile the currently emitted lexical descriptors with the projected incoming
state before each surviving original group, after applying the existing
site/case register and metadata-identity maps. Emit only the required delta:
one opening replaces a different descriptor, and one clear removes a descriptor
which should be absent. Never synthesize a clear followed by a reopening of
the same name at the same address. Keep names fresh against every destination
declaration using the existing owner; do not let closure clear an authored
caller binding. Preserve the current direct-DAG shared closure when its one
carrier already closes the whole clone before continuation. Runtime selector
exits retain their existing per-(case,RET) closure, followed by the same
reconciler at any later emitted original group and at the next case. A RET
neither terminates source-order traversal nor makes later groups conditional
metadata. Restore the caller's original lexical/source state before its
continuation observations.

Same-address order is part of the contract. `METALOADDATA` returns records in
canonical forward-chain order; `rxdb`/`symbol` scan addresses backwards and
accept the first matching scope row in a group. A last-record-wins fold is
therefore invalid. Copy an admitted authored group's records in their original
order. The first slice admits at most one distinct scoped transition for a
name in one original address group; repeated identical rows keep their order.
Conflicting same-name REG/CONST/CLEAR rows at one original address retain
`metadata.scope.address.pending` until their consumer-specific ordered meaning
has a separate proof. This boundary does not change existing linear support.

When a synthetic delta must restore inherited state visible to an `addr-1`
lookup at the first resumed instruction, attach it to one generated `CNOP`
immediately before the entire original address group and its alias labels.
The prior generated exit clear and this reopening then occupy distinct
executable addresses. Labels still target the mapped original instruction;
metadata does not require execution of the carrier. Emit a carrier only when
the required incoming-state delta needs that separate address, sharing it
across all names in that delta. Keep the original group's directives on its
mapped original instruction address, rather than moving its authored opening
or clear to the earlier carrier. Give generated carriers truthful generated/
contributing origins, without duplicating authored source-step or TRACE
events. Preserve source-step/clause identities, TRACE register operands, a
removed neighbor's lexical source context and caller restoration through the
existing source owner. Cleanup must preserve these address carriers and scope
observations; it may keep the conservative expanded result when its independent
transport/storage proof is unavailable.

Independent constructed and paired handwritten RXAS controls establish an
expected lexical descriptor table at original logical anchors, including
lookup at the mapped instruction address and its preceding address. The
small direct-DAG panel covers: a common live entry binding with an early RET
before a later reached block; a binding already closed before that RET, which
must not be spuriously reopened; disjoint branch openings/clears and separate
RETs; an opening or clear on a skipped machine group which still governs a
later live address; and the same name redeclared at distinct addresses with a
different descriptor. Include an original missing clear deliberately: its
binding remains lexically visible in a later arm even when execution skipped
the opening. Do not invent branch-local lifetime semantics to tidy that case.
Use stable observed storage, independently expected return values and current
borrowed COPY/owned MOVE/none behavior. Two inline sites and an authored suffix
collision check fresh names and caller-scope restoration.

After that direct-DAG result is reviewed, reuse the same controls on the
current width-at-most-three runtime selector, including two feasible RETs in
one case and different feasible RET sets across cases. Convert the maintained
`pick.which` early/multiple-RET and disjoint-scope controls to positives only
where the lexical/storage proof now succeeds. Assert exact case labels,
matched feasible exit closure, no cross-case metadata bleed and correct
source/TRACE on distinct returns. Compare conservative and cleaned artifacts
with native assembler optimization disabled and one normal VM. Inspect the
native metadata readback in a separate existing-style probe; this does not
admit physical metadata/frame observers into the expansion profile. Reuse
unchanged scalar, child, return/CANCEL and guard proof rather than expanding
this metadata-only increment into their full panels.

Negative controls cover conflicting same-name transitions at one original
address, a lexically live but recycler-dependent/undefined register after a
skipped opening, changing observed StorageIds before a clear, unsupported
metadata/physical observers and exact input retention. Existing incompatible
joins, cycles, unsupported control and child/selector gates stay intact. Charge
original group/declaration indexes, complete descriptor operands, every
coexisting scope snapshot, per-case remapping/reconciliation and each emitted
delta/carrier before growth. Include variable name/type/literal lengths and
constructor copies; no assumed COW or uncharged sealed-query copies. Tight
work/entry/byte/frame/code controls must retain the exact input owner and the
CLI's last-good output without partial publication. Inspect one coherent
re-entry region early, batch fixes, then freeze for the coordinator's focused
serial constructed/handwritten panel using prepared producers. INL-STEP06,
changing-storage metadata, loops, wider cleanup and initiative acceptance stay
open; no broad compiler, sanitizer, platform or performance panel is selected.

#### Runtime-selector scoped lexical metadata follow-on

**Status: suffix-only, stable-prefix and LINKARG-group follow-ons implemented
and focused-qualified; milestone QA todo.** Extend the qualified original-address
projection/reconciler to the checked runtime LINKARG acyclic multi-RET path.
Keep the existing linear closed-main caller, exact same-module body/arity,
zero globals, width at most three, successful linear prefix, one authored
ICHKRNG/LINKARG pair, acyclic suffix, admitted scalar operations and branch
forms, at most one static conditional CALL1 under its exact leaf/depth gate,
and default terminal signal policy. Reuse the current per-case cells/frame/
history and case/RET facts. No new source rule, ISA, wire record, VM/linker
algorithm, proof META, graph solver, reuse or signal category is selected.
New re-entry remains ordinary REG `rN`/CONST plus clear. Preserve qualified
legacy common-entry `aN` behavior; a body requiring new re-entry of `aN`/`gN`
retains its existing metadata/storage boundary rather than silently stripping
or changing those declarations.

Enable this first follow-on only when the original common prefix through the
complete selected LINKARG address group contains no scoped opening or clear:
no supported four-field REG/CONST declaration and no one-field clear anywhere
in that source-order span, independent of case feasibility or whether a pair
would leave the final state empty. Ordinary procedure metadata and the current
source/TRACE records retain their existing handling. Prefix scoped rows leave
the existing runtime metadata path and its gates intact: already-qualified
layouts remain admitted, while layouts needing new lexical re-entry retain
`metadata.scope.cfg.pending` (or an earlier, more specific existing refusal).
Do not newly reject qualified common-entry `aN` or claim it gains re-entry.

Prepare the same frozen original lexical projection once before the runtime
fork, including the checked empty-scoped prefix/entry and full suffix. Do
not make its incoming/observed rows case-specific or rebuild them from a
case's visited META. Every reached original group observes its projected
register scopes through the existing path-local frame/cells: the common
prefix, each case's transferred guard/LINKARG rows, and every suffix route.
The existing guard-entry checkpoint and separate post-LINKARG cursor retain
their execution-state meanings; neither is a substitute for lexical group
incoming state. Prefix effects and emitted guard run once, while case proofs
still refine and transfer their own normal guard/LINKARG successor exactly
once. Retained child scopes keep their unchanged owner.

Keep one `_scopestorage` obligation per original declaration across the whole
call, all feasible selector cases and exits. A case can establish the first
observed StorageId but cannot clear that ledger before the next case; every
later observation must agree and prove whole-state entry independence.
Different values may inhabit that same storage. Equal values in different
storage do not discharge the obligation. In particular, a declaration over a
LINKARG-selected cell normally retains `metadata.binding.change.pending`
when different cases select distinct actual StorageIds; repeated actuals may
pass when the existing alias/binding proof identifies the same storage on
every observation. Keep the existing recycler/runtime-observer refusal for
missing independence. Do not turn this increment into per-case scope splitting
or broaden aggregate/reference/component proof.

Emit the common prefix, authored check and stable selector snapshot/dispatcher
once. Keep their qualified source/TRACE placement and the existing case-entry
binding operation; no LINKARG-group record staging or relocation is selected.
The prefix eligibility check establishes an empty scoped entry checkpoint,
distinct from the existing possibly nonempty lexical source context. At each
case entry reconcile scoped descriptors to that empty state using the same
fresh call mapping, without reactivating the runtime frame or replaying prefix
effects. Reuse the existing case labels, source checkpoint and checked binding
placement. Labels still name the mapped original address, not a preceding
scope carrier. No authored observation is duplicated around the dispatcher.

Each case owns its emitted lexical cursor. Case 1 starts from the actually
emitted prefix state; a later case starts from the previous emitted region's
actual lexical end state, then reconciles to the same empty scoped entry state.
Reset source-group traversal bookkeeping at every case. Do not seed it from
case 1's visited records, stale emitter activity, or a new activation of the
runtime frame. The new profile needs no nonempty prefix scope replay; preserve
the qualified legacy checkpoint path. Emit the
whole reached suffix of one case before another case begins. A scoped opening
or clear in skipped code can still govern a later live group of that case,
even when that opening lies on another case's untaken edge. Query group
survival from that exact case's instruction liveness, never aggregate union
liveness. Retain original missing-clear semantics.

At every feasible `(case,RET)`, perform its existing result action, inverse
restoration and owned closure, close that exit's active clone bindings before
its branch to caller continuation, and update the emitted scope state. Later
source-layout blocks in the same case must re-establish their projected
incoming descriptors through the same reconciler; an early RET cannot leave
them cleared. Case transitions likewise remove residual suffix bindings and
restore the empty scoped checkpoint. Use one delta carrier before the resumed
group/entry labels when needed for `addr-1`, with at most one effective
transition per name at an address: replace by one opening or remove by one
clear, never clear+reopen together. Preserve canonical authored group order
and `metadata.scope.address.pending` for conflicting same-name transitions.
Reuse the qualified all-META address protection, including unexecuted carriers
and omitted NUMSCI groups, and charge carriers through the shared code ledger.

Preserve each case's original lexical source state, including an empty entry
context, removed neighbors, distinct branch/RET source steps and original
clause/TRACE/register relationships. Dispatcher/carrier glue gets truthful
generated/contributing origins without invented authored TRACE events. Restore
caller source before continuation TRACE even when no fresh caller source row
intervenes. Fresh scoped names remain collision-free against caller and other
inline sites; no case closure may clear an authored caller name. Fresh physical
identities follow the existing owner rather than inventing a logical inline
stack. Conservative output remains the correctness baseline; independent
cleanup may retain it for its existing range/storage proof reasons.

Remove the runtime whole-original-CFG dominance/early-layout META refusal and
the per-case DAG metadata gate only when the no-prefix-scope eligibility and
shared lexical proof are enabled and complete for the exact body/case. An
incomplete projection, unsupported
bank or failed observation keeps the original with its precise existing
metadata/storage reason. All guard/entry, arithmetic, frame-join, cycle,
selector/child/depth, signal and per-RET summary gates remain unchanged. In
particular, case feasibility cannot justify omitting an original lexical row,
and a per-case-only StorageId check cannot replace the whole-call obligation.

Independent constructed and paired handwritten controls use genuinely unknown
selector facts and the maintained three-choice value/status oracle. Cover
suffix branch-local opens/clears, two feasible RETs inside
one case and different feasible RET sets across cases, an early RET before a
later reached block, skipped lexical openings/clears and an original missing
clear. Assert empty scoped state at every case entry while retaining the
qualified possibly nonempty source context. Include a
skipped/reached retained child under its unchanged gate, repeated activations,
caller name collisions, and one repeated-actual positive versus equal-valued
distinct-actual negative. Check exact per-case return actions and feasible
closure membership; do not count width times static RETs as reached coverage.
Use the existing separate native metadata probe to inspect scopes both at
mapped anchors and at preceding addresses across case entries/early exits,
plus caller continuation source/TRACE. Assert carrier survival in cleaned
output, prefix/check/snapshot uniqueness and no cross-case labels or scopes.

Negatives retain a prefix scoped opening, a prefix clear, a prefix pair which
is already closed, and a scoped transition sharing the selected LINKARG group;
pair these with an unchanged qualified legacy prefix positive and assert
`metadata.scope.cfg.pending` for the new re-entry layouts. Also retain mixed
same-address transitions, changing StorageIds across
two cases or within one interval, a recycler-dependent scope in one repeated
activation, unsupported branch-local `aN`, incompatible joins, guard bypass/
extra ingress, cycles, extra selectors/children and resumable/physical
observers. Charge the one original projection and prefix eligibility scan,
all simultaneous case-entry/emitter snapshots, descriptor operands/origins,
per-case observation/reconciliation work, every feasible exit delta and every
carrier before growth. Include constructor copies and variable operand lengths
without assumed COW; sealed queries borrow. Work/entry/byte/frame/code refusal
keeps the exact frozen input and CLI last-good output with the specific limit
diagnostic, including a one-below-output-cell cap that counts all carriers.
Review one coherent three-case emitted region early, then freeze for the
coordinator's focused serial constructed/paired panel on one normal VM with
native assembler optimization disabled, reusing qualified direct-DAG and
unchanged return/child/signal results. Broader selectors, changing-storage
scope splitting, loops, INL-STEP06 and initiative acceptance remain open.

#### Stable nonempty runtime-selector prefix scopes

**Status: implemented and focused-qualified; the LINKARG-group follow-on below
is also focused-qualified; milestone QA todo.** This
INL-STEP04/05 slice serves INL-AC02/03/04/06/07
by extending the qualified suffix projection/reconciler to stable scoped
bindings established in the common prefix. Keep every existing runtime-selector
gate: linear closed-main caller, exact same-module body/arity, zero globals,
width at most three, one successful linear prefix and authored ICHKRNG/LINKARG
pair, straight-line or acyclic suffix through its feasible RETs, admitted scalar
operations and
branches, at most one static conditional CALL1 under its unchanged leaf/depth
gate, and default terminal signal policy. No ISA, RXAS syntax, wire/metadata
format, VM/linker algorithm, proof annotation, graph solver, signal category
or storage-reuse widening is selected. Handwritten producers continue to use
ordinary `.meta "name"="b" "type" rN` or `aN`, ordinary CONST declarations
and one-field clears.

New prefix re-entry admits ordinary REG `rN`/`aN` and CONST descriptors, plus
their clears, at original addresses strictly before the selected LINKARG
address group. Prefix open/clear pairs which leave no binding live at LINKARG
are admitted too; do not recreate those closed declarations at case entry.
The suffix retains its qualified REG `rN`/CONST profile and may carry, clear
or reconcile a prefix `aN` descriptor. A newly opened suffix-local `aN`, `gN`
or another unsupported descriptor does not gain new re-entry support here.
Preserve qualified legacy common-entry metadata layouts and their existing
admission; do not replace a supported old path with a broader new refusal.

The initial stable-prefix slice qualified an observation-free selected LINKARG
group. The bounded group-staging follow-on below now owns observation-bearing
groups. Membership still belongs to the
frozen original next-instruction address identity, not textual adjacency, CFG
block membership, case liveness or the location of the last source row. Preserve
qualified suffix-only and legacy admission. Alias labels retain their checked
relocation/entry contract; no alternate ingress into the guard or selected
operation is admitted.

Extend the existing scope projection owner to preserve each original typed
REG operand, including its bank and index, rather than storing only `index+1`
and reconstructing an `r` operand. Resolve it through the current path-local
`frame.storage` at every feasible observation. In particular, `a0` names the
owned mutable argument-count cell, and `a1` names its current active argument
binding; neither is an `r` slot or an immutable entry anchor by assumption.
Retain the existing whole-state entry-independence requirement. Constants need
no register observation. Root-graph cleanup and its r-only storage/assignment
profile remain separate and unchanged; extending inline observation does not
admit argument-bank root reuse or another storage resolver.

Maintain one declaration-to-StorageId obligation keyed by the exact selected
root call/site and original declaration, shared by the common prefix, every
feasible selector case, all suffix paths and exits of that call. Do not key
new argument-bank obligations only by original body/declaration: two separate
root sites may legally bind the same prefix `a1` declaration to different
actual cells. Do not reset this ledger per case, RET, block or source-order
re-entry. Within one call, different values in one stable cell are allowed;
equal values in different cells do not satisfy the obligation. A changed
binding retains `metadata.binding.change.pending`; a missing whole-state/
recycler proof retains `metadata.runtime.observer.unproved`, and unresolved or
unsupported storage retains its existing specific boundary. Keep the existing
root-cleanup declaration lookup separate. This slice neither splits one
declaration by case nor weakens frame/history joins or reference ownership.

Prepare one immutable original-address projection for the body, including
lexical rows on skipped machine groups. The case-entry scoped checkpoint is
the projection's incoming descriptor row at the selected original LINKARG
address, before any records of that group; it is not `_scopes[LINKARG]`, an
emitter activity snapshot, guard-entry cells/frame state or case 1's visited
META. Bind its full descriptors and origins through the fresh call mapping
once, or borrow sealed original rows and remap them under the same mapping.
Queries validate the exact call/body/original-record owner even for a skipped
declaration. Keep incoming and ordered per-group observation state distinct,
including clear-before-removal semantics and detached end rows.

Emit common prefix/check effects and the stable selector snapshot/dispatcher
once. Case 1 begins with the actually emitted prefix descriptors. Before each
later case, compare the actual lexical state left by all previously emitted
source groups with that same immutable incoming checkpoint, then use the
qualified delta reconciler. Remove residual suffix names, replace different
descriptors with one opening and reopen only missing prefix bindings; leave
identical descriptors alone. Do not call legacy clear-all/replay `resume`
for this new nonempty checkpoint. When a delta needs a separate address, emit
one counted CNOP immediately before the whole resumed case-entry group and
its labels. Never synthesize same-name clear+reopen at one address. Case labels
target the checked binding instruction, not the preceding carrier. Restore
the qualified lexical source checkpoint separately, then emit that case's
one direct binding operation. Reset `lastscopeaddress` and original-group
traversal bookkeeping per case; do not reactivate a frame or replay prefix
instructions, opens/clears or TRACE events.

Within a case, retain the original-address suffix reconciliation and exact
case instruction liveness, including skipped lexical openings/clears which
govern later surviving groups and original missing-clear semantics. At every
feasible `(case,RET)`, preserve its proved result action, inverse restoration
and owned closure, close active clone names and update emitter bookkeeping
before the continuation branch. Later source-layout blocks in that same case
must still regain their own projected state. A later case restores the prefix
checkpoint from the actual preceding lexical state, even if a later-layout
block left one prefix name active; neither a previous RET nor an inactive
snapshot may suppress the required delta. Keep canonical authored order and
`metadata.scope.address.pending` for conflicting same-name transitions at an
original address. Fresh names cannot collide with caller or other site names.

Preserve source/TRACE on the single emitted prefix and each surviving suffix
group, existing source-step/clause/register mappings, empty source context,
and caller source restoration before continuation TRACE with no intervening
caller row. The excluded LINKARG group has no authored observation to move
onto dispatcher scratch or clone onto case bindings. Generated carriers and
dispatcher glue receive truthful generated/contributing origins without new
authored source-step or TRACE events. Existing all-META address protection
must retain unexecuted carriers and omitted NUMSCI groups through cleanup;
conservative output remains the independent correctness baseline.

Enable removal of the old whole-CFG/per-case META dominance and early-layout
gates only after this precise prefix/group profile and the shared lexical/
storage proof succeed. Keep suffix-only and qualified legacy paths intact.
Retain incompatible joins, guard bypass/alternate ingress, cycles, extra
selectors or children, width/depth limits, unproved arithmetic, physical
observers and resumable signals under their current precise reasons. New
suffix-local `aN`/`gN` layouts requiring re-entry retain the legacy
`metadata.scope.cfg.pending` or an earlier specific metadata/storage refusal.
These unsupported-profile reasons remain distinct from a proven storage change.

The focused constructed lane and one paired handwritten RXAS control must
include a stable prefix `a1`, a distinct `a0` count descriptor, an ordinary
prefix `rN`/CONST and an already-closed prefix pair; carry live descriptors
through suffix branch-local re-entry and multiple feasible RETs. Include two
root sites with different actual cells as a positive, versus one site's
cases resolving one declaration to different StorageIds as a negative. At
least one later emitted block must leave a prefix name active before the next
case, discriminating delta replacement from clear-all/replay. Reuse the
qualified genuinely unknown selector, repeated-actual and per-RET ownership
controls. Assert independent values/status/mutations, one prefix/check/
snapshot, matched feasible closures and no cross-case names/labels. A separate
native address probe checks prefix descriptors before and at each case binding,
later anchors and caller continuation source/TRACE in conservative and cleaned
output with native assembler optimization disabled. Return checks identify
actual result-commit groups and their original source rows independently;
retained source rows from removed RETs do not imply execution or live scopes.
The per-case source-derived minimum return membership is required even when
later-site summary imprecision conservatively retains additional exit groups.
The group-staging controls below cover each formerly excluded LINKARG-group
record kind. Pair changing/recycler-dependent bindings with exact retention
reasons; retain an unchanged legacy prefix positive and unsupported suffix-local
`aN`/`gN` controls.

Charge the typed operand projection, group eligibility scan, per-site
declaration ledger, saved/borrowed checkpoint mapping, every simultaneous
descriptor/origin snapshot and constructor copy, variable operand lengths,
per-case observation/reconciliation and all emitted deltas/carriers before
growth. Sealed queries borrow; no assumed COW or uncharged repeated full-prefix
copies. All carriers use the shared code emitter/`inline.code.cells` ledger.
Focused work/entry/byte/frame/code controls, including one below the independently
counted output-cell total, must preserve the exact frozen input and CLI
last-good output with the specific limit diagnostic and no partial publication.
Review one coherent source change and representative case transition early,
then freeze for the coordinator's small serial constructed/paired panel using
prepared producers and one normal VM. This development slice leaves
INL-STEP06, changing-storage splitting, broader banks/reuse and initiative
acceptance open.

#### Runtime-selector LINKARG address-group staging

**Status: implemented and focused-qualified.** The group, scope, prefix and
branch panels passed with the former multi-RET refusal now checked as a paired
positive; broader initiative acceptance remains open.

This INL-STEP04/05 slice serves INL-AC02/03/04/06/07 by removing the
stable-prefix profile's empty-selected-group restriction where the following
group-staging proof succeeds. Preserve every qualified
runtime-selector, signal, storage, child and cleanup gate from the preceding
slice. There is no new ISA, RXAS syntax, metadata format, VM/linker algorithm,
producer annotation, graph solver or storage-reuse profile. INL-STEP06 and
broader initiative acceptance remain open.

Index or borrow the complete selected original LINKARG address group through
the existing frozen next-instruction address owner. Its bounds include all
ordered preceding records assigned to that instruction, including alias labels,
`.meta`, `.srcfile`, `.srcline`, `.srcstep` and `.traceevent`; textual adjacency,
block membership and case liveness do not determine membership. Keep that
ordered original group distinct from its immutable incoming scoped projection
and incoming lexical source checkpoint. Stage the group before emitting any of
its labels or observation rows. Do not save a checkpoint after applying its
opens, clears or source rows, and do not rescan a full body for each staged row.

Emit the common prefix, authored range check, stable selector snapshot and
dispatcher once, before the staged group. For each feasible case, reconcile
the actual preceding emitted lexical state to the same mapped incoming scope
checkpoint using the qualified delta owner. Any required counted carrier
precedes the entire case-entry group. Then emit its generated case label and
the original group's cloneable rows/alias labels in original relative order,
followed by that case's direct LINK. Every case and original alias label still
targets the binding instruction; no label targets dispatcher scratch or the
preceding carrier. Continue each case through the existing suffix/RET proof,
closure and source-order traversal. Mark the staged original group consumed
once per case so it cannot be replayed by the ordinary suffix traversal.

TRACE records on that binding address retain the original typed active-slot
operands through the existing register remapper. Their event batch observes
the pre-binding frame at the LINK boundary, including when LINKARG's
destination is also its selector. The selected destination after LINK and an
immutable entry anchor are different observations. Do not rewrite an authored
event to dispatcher scratch, capture its value with new runtime machinery or
move it after LINK. Preserve the group event count, canonical order, modes,
source-step/clause IDs and full payload; statically cloning one group per case
still delivers it once on the one executed case. A failing authored range check
delivers none of the selected group's events.

The group admits the qualified scoped REG `rN`/CONST declarations and clears;
it may carry or clear an existing stable prefix `aN` declaration. A newly opened
group-local or suffix-local `aN`/`gN` declaration does not gain re-entry support
in this slice. Preserve canonical repeated identical scoped rows and the
`metadata.scope.address.pending` conflict boundary. Apply the existing shared
site/declaration StorageId ledger and whole-state observer proof before the
binding and throughout every feasible suffix interval, including clear-before-
removal observations. A declaration over the selected destination can therefore
retain `metadata.binding.change.pending` when rebinding changes observed storage;
equal values in different cells do not satisfy this proof. Recycler/undefined,
unsupported-storage and physical-observer refusals retain their existing exact
reasons. A proved fresh null whole state can satisfy a META observation without
establishing an integer-defined arithmetic or TRACE operand; a repeated native
activation without a proved whole reset loses that freshness premise. Do not
reset the ledger or split one declaration by case.

Keep the existing non-cloning rule for callable five-field and opaque inline
three-field META: those declarations remain on the retained original definition,
not duplicated as per-case declarations. Their presence in the selected group
alone no longer forces `metadata.scope.linkarg.group.pending`; their original
relative order among retained original records is unchanged. This does not
broaden runtime META kinds or change existing legacy/suffix-only admission.

Save the source checkpoint before the staged group. If the group contains
authored `.srcstep` rows, emit those exact mapped rows once in their original
order without also replaying its last authored source row as a checkpoint.
Otherwise restore the incoming source checkpoint, including an explicitly
empty source context, through the existing synthetic source owner. Stage
`.srcfile`/`.srcline` with the group as well. Generated dispatcher/carrier
origins remain generated/contributing and add no authored source-step or TRACE
event. Caller continuation source restoration and distinct original RET source
rows retain their qualified contracts. Cleanup must protect the staged META
and source/TRACE addresses, labels and required delta carriers; conservative
expansion remains the independent baseline when cleanup lacks its own proof.

The maintained constructed lane and paired handwritten prefix-scope consumer
reuse prepared producers and one normal VM. Combine live prefix
`rN`/`a0`/`a1`/CONST descriptors with a selected-group clear, a stable group
`rN`/CONST opening, repeated identical rows, source-step/TRACE rows and alias
labels interleaved in canonical order. Include source rows with no group
source-step and an empty incoming source checkpoint. Use genuinely unknown
selection with distinct pre/post binding values, including a selector held in
the destination, and at least two ordered events at that address. Assert
independent values/status/mutations, one prefix/check/snapshot/dispatcher,
one ordered group per static case and one event batch per executed case,
pre-binding event values, exact label/binding addresses, caller source and
matched feasible closures. Read back the complete native META/source/TRACE
payloads at the binding address and its predecessor in conservative and cleaned
artifacts with native assembler optimization disabled. Reuse an unchanged
qualified legacy positive and callable/inline META non-cloning control. Legacy
`.srcfile`/`.srcline` model rows have constructed coverage; the native RXAS
consumer uses the supported `.srcstep` form.

Keep negative controls for conflicting same-name group rows, newly group-local
`aN`/`gN` scopes, a declaration whose observed storage changes at LINKARG,
recycler-dependent observations, guard bypass/alternate ingress and the existing
selector/child/profile boundaries. Pair a failing range check with absent
group events. Retention and limits preserve the exact frozen input owner and
CLI last-good output. Charge group indexing/staging, mapped operands and full
variable-length payloads, each simultaneous source/scope/origin snapshot and
constructor copy, every case's group replay and reconciliation, and emitted
labels/deltas/carriers before growth. Sealed queries borrow; no assumed COW or
uncharged repeated body scans. All added instructions use the shared
`inline.code.cells` ledger. Work/entry/byte/frame/code controls include a code
allowance one below the independently counted output total. Review coherent
source and one representative staged case early, then freeze for the small
serial constructed/paired panel; no broader QA milestone is commissioned.

### Reviewed same-module global scalar increment

The next INL-STEP04/05 slice admits private, unexposed globals in the same
frozen module as the closed main caller. It changes no ISA or linker contract.
No initializer, exposure, imported body, foreign global alias, global active
rebinding inside the selected callee (LINK/UNLINK/LINKARG with a `gN`
destination or SWAP involving `gN`), authored global ENDLIFE, or nested/
count-window call is admitted in this
first slice; each has a specific pending-proof reason rather than an inherent
infeasibility label. Start global integer and whole/status contents unknown;
the existing `cells.add()` freshness defaults do not establish them. Require a
checked full NULL or whole overwrite before reading a global; LOAD and
integer arithmetic are partial writes. Ordinary entry to `main` is after module
preparation; this slice does not rewrite initializer ingress.

The VM's physical frame order is locals, module globals, `a0`, arguments.
`lltargetfacts.frame` must model that order even when the selected fixed call
does not use an implicit counted window. One persistent StorageId per module
`gN` is shared across procedure frames, while each frame has its own active and
base bindings for that slot. Recycled frames relink their globals to those
module-owned bases. Original caller global active pointers must equal their
module/base StorageIds at each selected call, with canonical slot identities;
a caller LINK/SWAP that redirects `gN` keeps that call until a
base-binding rewrite is proved. Fixed-call actuals may borrow a canonical
global cell. The selected callee may read/write it through its original `gN`.
`RET gN` copies, never moves or ends the module cell. Per-procedure owned-base
history remains only locals and `a0`, excluding the intervening global slots.

The facts owner exposes each fixed actual's typed bank/index as well as its
storage identity; the current integer-only argument getter must not turn a
borrowed `gN` into `rN`. The expander maps selected callee `gN` operands and
metadata references to the same caller-module `gN`, before its r/a layout
logic, with no fresh local surrogate or global-count change. Per-procedure
owned-base access skips intervening globals. Preserve root local StorageIds or
guard their old ordinal assumptions explicitly. It emits no generated ENDLIFE for
a module global; authored global ENDLIFE retains the call in this slice.
Both cleanup services must check `source.globals()>0` before constructing
r-only mappings and keep the conservative expanded module until global-aware
placement proof exists;
it must not claim an invalid compaction. Extend cleanup only with a reviewed
shared storage proof later. Exact module/provider binding and private foreign
global aliases remain separate later steps.

Independent controls start with full `NULL`/integer definition of `g0=10`,
call a helper with actual `2` twice, and require returned `12`, then `14`,
and final `g0=14` through native `rxas -n`/one VM. A second global directly
borrowed as an actual checks shared storage rather than duplicated callee
state; scoped metadata/TRACE still name its original `gN`. Check global
count/index, same-module storage and COPY return structure,
conservative/default CLI output, and absence of global lifetime end.
A negative witness relinks caller `g0` to
a local `99` while its module base is `10`; an ordinary callee reading `g0`
returns `10`. Refuse this call with a specific binding reason and preserve the
original module and executable result. A counted call whose implicit window
crosses a global is a separate executable pending-proof witness. Exposure,
foreign binding and counted-cross-global cases retain precise reasons and
atomic output. Initializer input currently stops at the adapter's explicit
unsupported-metadata diagnostic, not a facts keep; do not add initializer
support merely for this fixture slice.

### Reviewed target CFG and multiple-return prerequisite

The focused-qualified INL-STEP04/05 target prerequisite makes the model's
direct branch graph truthful. Acyclic multiple-return facts and expansion are
the next separate increment. The existing generic `llir`
terminal/successor contract and `llirgraph` RPO/SCC/liveness owners suffice;
there is no target-private graph framework or new generic solver. Keep the
RXAS reader/body-draft source API and source-order writer. Version the target
schema for a typed control-record operation with the existing record fields
and declared terminal successor bounds. Structural jumps/thunks are L graph
operations, never emitted RXAS instructions.

For direct `BR`, the ordered successor is its label target; `BRT` and `BRF`
have target then lexical fallthrough; `BRTF` has its two operand targets;
`RET` has none. Preserve opcode polarity and exact source operands rather than
inferring it from generic edge order. `EXIT` is terminal, never a normal RET.
Unsupported indirect/table control and unrepresented exceptional continuations
stay transportable but mark normal CFG proof incomplete; a normal-flow opcode
with a signal contract does not by itself make its represented normal graph
incomplete. Signal success/continuation remains a separate facts proof. A
target-owned body cursor creates real blocks,
bounded forward-label entry thunks, alias-label address groups and fallthrough
edges. Zero-width source/TRACE records preceding a label stay on that label's
entry address. Finish closes only open blocks; it cannot append a sentinel
after terminal RXAS code. Validate opcode/edge/label agreement independently
of the builder, including decoded/mutated graph input and resource failure.

Native RXAS labels use one module-wide namespace. Reject duplicate label
definitions across all procedures and freshen cloned declarations against
every label in the destination module. Rewrite only declared labels and
catalogue-declared branch operands; a generic symbol/pool reference with the
same spelling is not a branch target. Retain source order, logical record IDs,
provenance and metadata addresses. Add readonly record block/address and body
label-block/derived CFG-completeness views; never accept a caller assertion of
completeness. Recognize both ordinary and control-record descriptors centrally
through module/body views, validation and facts; structural thunks are never
misclassified as empty catalogue records. Trailing zero-width metadata after
a RET has a separate body-end address group (`addressid=0`), not an operation
after the terminal RET. It stays in source order; an unreachable metadata-only
block may close structurally but must not render another RXAS RET. Existing
scalar proofs use a verified sole reachable normal RET and ignore such tail
records as execution, while publishing explicit empty access/reference rows
needed by consumers. A selected callee's detached end-anchor records stay in
its retained original declaration and are not cloned into the reachable caller
continuation; preflight/source identity work likewise skips them. Typed caller
end-anchor observations that cannot be relocated precisely keep cleanup
conservative. A branch that reaches the tail is not an
unreachable exception. A small independent constructed model checks forward/alias and
backward-lexical DAG labels, direct branch forms, multiple RET zero edges,
unreachable blocks, source/TRACE entry address, duplicate/missing labels,
wrong-but-well-formed edges, decoded interior record reordering and atomic
limits. An ordinary RXAS both-arm
control checks writer/native assembler/one VM without claiming inliner support.

The selected-callee tail exclusion must also close cloned scoped metadata.
`META_REG` bindings may be opened before a RET and closed by a detached
`META_CLEAR` at the original callee end address. The original clear stays with
the retained callee, while the expansion emits a clear for every still-open
freshened binding on each logical exit before caller continuation. Check
same-name clear/declaration order, no extra detached TRACE or source event,
native debugger metadata lookup and atomic resource behavior. Fresh scope
names must avoid collisions with caller declarations, rather than clearing a
different caller binding. This is executable metadata correctness, not an
exception to the detached-tail rule.
The current sole-RET straight-line expansion uses one executable `CNOP` after
the generated clear as its address carrier; the caller's source-order cleanup
can then still prove transport and storage compaction. A future multi-return
branching expansion uses a per-exit branch to its common continuation. The
carrier survives ordinary assembly with optimization disabled, and source
metadata is attached to its instruction address so debugger lookup from the
following instruction's `addr-1` sees the clear. A previously closed cloned
binding needs no new carrier. The independent generated caller must retain its
existing compact frame and transport cleanup after this correctness repair.

After target CFG qualification, a separate reviewed inliner slice will use
one acyclic RPO pass with RXAS-specific finite-state meets. Conditional RXAS
branches test integer zero/nonzero, not a Boolean-only 0/1 domain; an unknown
defined condition propagates both feasible edges. At a join, known
integer values survive only when equal and component/availability facts meet
conservatively; active/base pointers, slot permutation and context must match
or the call stays with a precise pending reason. Per-RET facts own COPY/MOVE,
a canonical inverse of the reached slot permutation (not a union of SWAP
histories), base restoration and owned cleanup; each expanded return reaches
one continuation after its own transfer/restoration. No branch arm is simulated
serially as if both executed, and EXIT/raising paths remain distinct. Existing
linear cleanup keeps a changed branched caller until its graph-aware mapping
and placement proofs are reviewed. Meet caller-visible and recycled owned-base
state after each feasible return transfer; a recycled cell is known reset only
when every normal exit proves it. Full
Finite padded optional presence/defaults are qualified below; dynamic varargs,
loops and exceptional unwinding remain later categories.

### Reviewed acyclic multi-return increment

INL-STEP04 next admits a direct scalar callee DAG while the authored `main`
caller remains on its already qualified linear profile. The shared target CFG
and `llirgraph` index/RPO/SCC own block identity, edges, reachability and cycle
refusal. RXAS-specific transfer state owns storage IDs, each component/status,
active/base bindings, slot permutation, context and recycled owned-base
history. Preflight path snapshots before copying; a single forward RPO pass
propagates both feasible successors of an unknown defined integer condition.
Allocate stable owned StorageIds once before path forking. Snapshot the cell
component arrays and frame map/base/identity/context directly; dereferencing
an existing state aliases it, while rebuilding a frame through its entry
factory resets `a0`, active bindings and slot identities.
`BR`, `BRT`, `BRF` and `BRTF` use their native zero/nonzero polarity and
ordered target edges. An undefined condition, cycle, unsupported normal or
exceptional edge, or incompatible join keeps the exact original with its own
reason. Do not assume 0/1 Boolean contents. No general solver or SSA owner is
introduced. A branch body containing a retained nested call stays pending
until path-local activation/cache history can also be forked and met; the
existing straight-line nested-call acceptance remains intact.

At a join, intersect availability, plain/whole/status/zeroed guarantees and
retain an integer constant only if every feasible predecessor proves the same
value. `wholeknown` means independent of recycled entry, not identical whole
bits on every path; preserve that property by conjunction, while tracking
exact status separately as known or unknown instead of inventing zero. Keep
storage identities distinct even when values coincide. Initially
require equal active/base bindings, slot identity permutation and context at a
join; precise differing-state reconciliation is later proof work. Every
feasible RET has a separate typed return action, captured-result transfer,
inverse of its reached slot permutation, base restoration and owned lifetime
end. Transfer each return first, then meet caller-visible state and recycled
owned-base reset/history over all normal exits. A base is reset only if every
feasible exit proves it. An ignored result performs no MOVE/reset. `EXIT` and
raising paths are not silently treated as normal returns.
Call reads and possible writes are unioned over feasible paths; definite kills
are intersected over normal exits, so an arm with bare `RET` cannot kill an
unchanged result. Private scalar transfer currently writes analysis-level
plain and selector rows: derive plain state from the merged poststate, seal
each LINKARG selector from that record's incoming path state, and let shared
per-call layout flags grow only monotonically. No last-visited-arm state may
leak into published facts.

Sealed per-site facts add `returns(callId)`, `returnsite(callId,ordinal)`,
`returnactionat(callId,retId)`, `restores(callId,retId)` and
`restoreleft/right(callId,retId,ordinal)` using zero-based private slot
ordinals. The old single-return getters remain valid only when there is one
proved return; they never silently select a first arm. If analysis prunes a
known branch, it also publishes `recordlive(callId,recordId)` and
`branchchoice(callId,branchId)` so emission follows exactly those feasible
paths. Otherwise every emitted arm receives a legality proof. The emitter
freshens each cloned label and one continuation against **all** destination
labels and `.const` aliases, including allocated clone names, with bounded
deterministic names safe for native RXAS. It rewrites only label declarations
and catalogue-declared branch operands; unrelated `L` symbols retain their
meaning. It preserves source/TRACE/provenance and omits only already proved
detached callee end-anchor records. Each cloned RET emits its own transfer,
restoration and owned cleanup, then branches to one shared closure block after
all cloned arms. That block closes fresh bindings still open at a normal exit,
uses one executable address carrier, and resumes the caller. An earlier arm's
generated clear must not lie lexically before a later arm: `rxdb` scans prior
addresses statically and could hide a binding inside that arm. Initially admit
only metadata scopes whose entry/exit state has a compatible proof across
feasible arms; a branch-local scope without one keeps its call with a precise
observation reason. Source-order cleanup does not assign a newly branched
caller; the separately reviewed graph-aware path below owns that proof.

Independent controls include both `0` and nonzero `2` arms of the same
reference-mutating helper (results `9`, `11`, final caller `y=99`), an unknown
condition (for example a defined `GETANDTP` result of runtime `0`/`256`) with
unequal branch values that loses its constant at join, mixed
MOVE/COPY return ownership, absent-result RET preserving a prior destination,
different exit permutations and aliased result targets, and recycled-local
history across repeated calls. Negative regressions assert precise reasons
and byte-identical retained owners for incompatible joins, cycles, undefined
conditions and unsupported edges. Tight work/frame/entry/byte limits remain
atomic. Qualify conservative expansion through native assembler `-n` and one
VM; run the existing small target/inline/IO checks only where inputs changed.

The entry-limit control also exercises construction of the target draft's
catalogue, before any cloned body exists. The target draft owner must preserve
the arena's `LL_RESOURCE_LIMIT` when its initial function/block/record cannot
be allocated; no invalid handle may replace that diagnosis with an owner
error. The inliner checks the draft error before provenance or emission and
returns the exact frozen input with status `limit`.

### Reviewed branched-caller cleanup increment

INL-STEP05 analyzes the exact frozen post-expansion caller, not a callee's
old facts. The opt-in `analysegraph(owner,fn,profile,work,bytes)` at the
existing target-facts owner returns the same sealed analysis class while
`analyse()` and the linear expansion contract remain unchanged. Reuse the
shared `llirgraph` RPO/SCC, reachability and access liveness owners. Initially
admit a complete acyclic r-only closed-root scalar CFG with compatible active,
base, identity and context bindings at every join. Snapshot each cell's
independent components/status and frame map on paths; a root retained call,
global, exceptional continuation or opaque physical-frame observer keeps this
cleanup with a precise reason until its path-local proof exists. Publish
readonly `rootlive(recordId)`, `rootbranchchoice(branchId)` (0=both, 1/2=one
edge), `rootrestored()` and `scopestorage(metaDeclarationId)` plus existing
storage/plain/access queries, all checked against the exact frozen owner.
`rootrestored()` requires active pointer==base and canonical slot identity at
every normal root RET; the existing identity-only `frame.canonical()` is not
that proof. Do not create a target-private graph or another assignment owner.

The graph transport path in `llcleanup` uses those sealed per-record incoming
StorageIds, not one lexical mutable mapping. Only with every mapping
obligation discharged may it remove LINK/UNLINK/SWAP and rewrite executable,
TRACE and other typed references to the reached base slot across the entire
region. Equal values never imply storage identity. Preserve required COPY/MOVE
snapshots, status/component effects, call windows, lifetime and numeric/signal
obligations. Changing scoped META_REG bindings keep transport until metadata
splitting is proved; stable bindings retain scope and address meaning. The
facts access rows must carry each live META_REG binding through its matching
clear, not observe only its declaration point. Use
projected CFG liveness only for proved plain dead NULL/LOAD, same-storage copy
and effect-free ENDLIFE deletion. Fold only proved branch choices, retain
necessary earlier-arm branches to shared closure, and remove fallthrough BR
only when its address has no observation. Point source/TRACE records follow
their machine group; lexical META and source-file/line state reaching surviving
addresses must remain. Build address groups once from indexed records rather
than repeated suffix scans.

After transport, reanalyze the frozen result and call the **existing**
`llrxas.assignment.finish('dense')` with its independent validator. Enumerate
explicit, implicit, metadata, TRACE and window references; drop a virtual cell
only with complete absence proof. Dense placement is injective and needs no
reset, so it is safe across opposite lexical and execution orders. The current
source-order interval reuse/reset schedule is not a DAG interference proof:
even two cells each used in one different block can execute in reverse lexical
order and expose stale status. Cross-block sharing remains open pending a
checked same-block restriction or every-arrival reset/interference extension
at the assignment owner. Preserve the current linear reuse path.

Both cleanup services retain atomic `change` results and exact-owner mapping.
Charge graph snapshots, scoped/address rows, liveness and rewrite work before
allocation; carry remaining budgets forward, check draft errors before using
handles, and validate the rebuilt target. Independent controls use the existing
known and runtime 0/256 DAG results, required MOVE/COPY, outer transport
removal, shared clear/CNOP location and a sparse DAG with an independently
counted distinct-cell frame. A backward-lexical DAG prevents unsafe reuse;
incompatible joins/cycles/observers and tight limits retain the exact original
with specific reasons. One warm affected B/L build, constructed/storage and
ordinary CLI checks on one VM are the development scope; broader modes and
platforms remain for the agreed standalone milestone. The representative
cleaned two-arm caller must have a dense frame without selected CALL, LINK,
UNLINK or SWAP glue while required MOVE/COPY remain. This structural obligation
does not establish a performance verdict.
Unknown joined values now have the bounded integer success proof below for
admitted checked arithmetic. Other components and unbounded values retain
their specific missing-proof reasons. The remaining AC05 obligations stay open.

### Reviewed bounded integer success increment

INL-STEP05 extends the same `lltargetfacts.cells` owner with a bounded integer
interval per StorageId. Exact LOAD/NULL values create singleton bounds;
COPY/MOVE carry bounds with the integer component and MOVE resets the source.
Unknown GETANDTP results, uncertain recycled content and absent definitions
have no bounds. A feasible acyclic join retains the hull only if every incoming
path has a defined bounded integer; exact knownness is retained only for a
singleton. Status, whole-cell, plain/lifetime, zeroed and storage identities
remain separate facts. Snapshot and work/byte allowances include the added
state rows. No new graph, schema, ISA or runtime arithmetic owner is added.

Before an aliased destination write, gather both operand bounds. IADD checks
the lower/lower and upper/upper sums; ISUB checks lower/upper and upper/lower;
IMULT checks all four endpoint products. Use the existing checked scalar
`arithmetic` helper at each endpoint; an unbounded operand or any failed check
retains the original with `arithmetic.write.availability.unproved`. Do not
evaluate an unchecked width, negated MIN or overflowing intermediate. Only a
proved successful write publishes integer availability, a result interval and
the existing integer-component kill. It must not assert a whole-cell kill,
freshness or known status. Branch, LINKARG and ICHKRNG selection retain their
existing exact-value rules. This deliberately loses correlations such as
`x-x` across paths until a separate identity proof exists.

The unequal-value joined-IADD control now succeeds; negative regressions retain
the exact input where one feasible endpoint can overflow or an operand has no
bounds. Subtraction/multiplication boundaries, aliased destination, status
preservation and tight resource limits have focused controls in the existing
storage/CLI development lane with native assembler optimization disabled and
one VM. Broader acceptance stays INL-STEP06.

### Integer predicate transfer increment

**Status: implemented and focused-qualified; broader initiative acceptance
open.** This INL-STEP04/05 development slice serves
INL-AC02/03/05/06/07 by admitting the six ordinary signed-integer predicates
`IEQ`, `INE`, `IGT`, `ILT`, `IGTE` and `ILTE` in the existing exact-bound scalar
direct-call profile, including explicit fixed-argument CALL1--CALL4 sites and
straight-line or acyclic bodies through their proved RETs. Their transfer
belongs to the existing `lltargetfacts` owner and is shared with the changed
caller's existing cleanup analysis. Preserve all enclosing call/body/module,
arity, frame/history, child/depth, signal and observation gates. This is an
operation-profile extension, not a new caller CFG, call form, selector proof,
import/global capability, retained-child whitelist, reference/aggregate category,
ISA, metadata annotation or solver. INL-STEP06 and the wider representable-profile
acceptance remain open. In particular, the runtime-selector CALL1 child whitelist
is a separate gate and is unchanged by the shared predicate transfer.

Use the canonical extracted opcode/effect/component/signal catalogue and the
owning VM handlers. IEQ/INE have register/register and register/immediate input
forms; the four ordered predicates additionally have immediate/register input
forms. Destination is a checked register. Do not invent immediate/immediate or
immediate-left IEQ/INE forms, infer identities from numeric opcode ranges, or
treat fused IGTBR/ILTBR as these ordinary result-producing instructions.
Keep each original predicate instruction and source/TRACE address in conservative
expansion; no compare helper, subtraction, conversion or runtime guard is added.

Resolve both input operands through the current active frame before changing
the destination, and capture all facts needed for their comparison first.
Every register input requires an independently defined integer component;
known whole-null state alone is insufficient. An input need not be an exact
constant or have finite arithmetic bounds: signed int64 comparison itself has
no overflow path. Immediate operands retain their exact signed int64 values.
Unresolved storage keeps its existing external-storage reason; an undefined or
unproved recycled integer input retains `predicate.operand.unproved`. These two
fixtures deliberately share that reason: the missing premise is the same
integer definition, with distinct fresh/recycled histories retained in their
oracles. The new predicate transfer must explicitly require
`state.ordinary(destination) = 1`, otherwise retaining
`argument.scalar.state.unproved`. REG_RETURN_INT invokes `set_int`, which can
release a reference payload or native payload before writing the integer.
Do not use `state.plain` for this gate: an ordinary string/binary side component
is permitted and must survive. This is a predicate-specific gate; it does not
change unrelated existing arithmetic transfers. A source that is the destination, a
repeated actual, or two distinct encoded slots resolving to one StorageId still
reads the complete incoming operands before the one write.

Compare signed integer values directly with the selected relation. In particular,
do not compute `left-right`, negate MIN, use absolute values or route through
decimal/string comparison. If both incoming integers are exact, publish the
independently computed exact 0 or 1. Otherwise publish a defined integer with
the existing interval `[0,1]`, including defined unbounded inputs. This slice
adds no operand-range, same-storage correlation or branch-edge refinement;
existing branch analysis may fold an exact result or carry both feasible
edges for an unknown boolean. At joins, use the existing component/interval
meet. No new value domain or per-cell state row is required.

Canonical signal classification is NONE for these forms. Under the existing
ordinary-storage proof the predicate therefore always commits its integer
result; no arithmetic-success endpoint test or terminal-signal assumption is
needed. Publish both integer reads before the write, a possible integer and
whole-observation-domain write, and a definite kill of only the destination's
old integer component. Preserve destination status, string/binary and other
components, whole-knownness, owned/base/active/slot identity and required lifetime
facts. Clear no whole-cell domain and assert no reset/freshness or known status.
Refresh the current plain/lifetime summary through its existing owner. The
remaining return-action, alias/copyback, META/TRACE and recycler proofs still
decide whether later whole observations and transfers are legal.

The smallest focused panel extends the maintained constructed scalar/storage
owner and one paired handwritten or maintained-golden-derived native RXAS
consumer. Cover all six relations, all canonical operand arrangements, equal
and unequal values, negative/zero/positive inputs and signed MIN/MAX without
overflowing oracle intermediates. Include a genuinely defined unknown/unbounded
integer producing `[0,1]`, an exact comparison that selects a DAG edge, and an
unknown comparison whose two edges reach different RETs. Cover destination equal
to either register input, repeated actuals, and a seeded string/public-status
side component which survives the integer write and result transfer. Assert
independent values and mutations, exact integer read/write/kill summaries, no
whole kill, no selected CALL, and retained original predicate instructions in
cleanup-disabled output. The cleaned result must preserve the same observations;
branch simplification may use only existing exact-result proof. A simple fixed
leaf must add no compare calls, guards, copies or frames beyond the already
required inline mapping and return ownership.

Use the maintained `classify` callee in
`compiler/tests/golden/noopt/inline_test_multi_return_assign.rxas` as the native
source pattern, retaining its branch/RET/source shape in a small fixed-call
harness. Inputs -1, 0 and 1 independently return -1, 0 and 1; the original
golden's main loop is outside this caller profile and is not selected. The
alias control starts an actual at integer -7, string `keep`, public status 512;
an aliased greater-than-zero predicate changes its integer to 0 while the
string and public status remain `keep` and 512. The original, conservatively
expanded and cleaned native artifacts must agree with these independent values.

Negative controls distinguish a fresh whole-null but integer-undefined input
and a recycled local integer without an independent definition; both retain
`predicate.operand.unproved` and the exact frozen input. Reuse existing
nonordinary/call-profile refusals for unsupported payloads; do not introduce a
new event/native-payload producer solely to exercise this predicate gate.
Earlier enclosing-profile refusals retain their existing reasons. Reuse
existing call/profile, whole-return and observation regressions;
do not widen this slice to their full matrices. Charge operand lookups/captures,
the constant-sized comparison and state/access updates through the existing
work owner before growth. Snapshot and liveness use the existing cell and graph
allowances; cloned instructions use the shared `inline.code.cells` ledger.
The new fixed work charge has a CLI limit and last-good publication control;
existing entry, byte, frame and code limit owners retain their controls and
unchanged obligations.
Review coherent source and one simple emitted predicate/branch region early,
then freeze for the coordinator's small serial constructed/paired panel with
prepared producers, native assembler optimization disabled and one normal VM.
No broad compiler, sanitizer, platform or performance campaign is selected.

### Loose string equality transfer increment

**Status: implemented and focused-qualified; broader initiative acceptance
open.** This INL-STEP04/05 slice serves INL-AC02/03/05/06/07 through
the same exact-bound scalar call/cleanup profile and resource owners as the
integer-predicate slice. Admit REQ/RNE only, each in canonical R,R,R / R,R,S /
R,S,R form, using the extracted opcode/effect/component/signal catalogue.
Preserve every enclosing call/module/arity, CFG/frame/history, child/depth and
observation gate, including the separate selector-child whitelist. Strict string
predicates, RSEQ, ordered loose relations, private ITOS, exact string folding,
new profiles and INL-STEP06/full standalone acceptance remain later work.

Capture both typed operands, input StorageIds and component premises before an
aliased output write. Each R source requires ordinary storage plus
`stringdefined` and `stringindependent`; integer availability is irrelevant.
A proved fresh/reset empty string is admitted. Unproved recycled string state
retains `string.component.unproved`; unresolved storage and nonordinary sources
retain their existing external-storage and `argument.scalar.state.unproved`
reasons after any earlier enclosing refusal. S operands retain validated exact
bytes. Require `state.ordinary(destination) = 1` for REG_RETURN_INT's possible
reference/native-payload release; do not require plain storage. Check either
input/destination overlap and repeated actuals before the one integer write.

Retain the exact original REQ/RNE instruction, order and source/TRACE address.
Its canonical signal class is NONE. The existing VM `rxvm_loose_compare_*`
owner performs locale/converter-sensitive numeric comparison or length-aware
right-space padding; long numeric candidates may allocate converter scratch,
and conversion/allocation failure uses its existing fallback. Preserve that
behavior and environment at the original opcode. Add no L parser/comparator,
normalizer, locale assumption, conversion, string copy, helper or guard.

Always publish a defined integer interval `[0,1]`, including literal-looking
or same-storage inputs. Do not inspect/parse string content, fold exact results,
introduce a string-value state/cache or refine branch operands. Existing DAG
analysis carries both proof routes. Source reads use the existing whole-
observation domain; destination writes cover integer and whole-observation
domains, with a definite integer-only kill. Preserve string/binary/public-status
components and existing whole-state, recycler, plain/lifetime, return-action,
active/base/slot and ownership obligations; assert no whole reset or freshness.

Extend the maintained constructed scalar/component lane and one compact paired
handwritten or maintained-native RXAS consumer with already-supported fixed-call
surroundings. Cover six forms and REQ/RNE inversion; independent outcomes for
`01` versus `1`, trailing-space text equality, leading-space nonnumeric mismatch
and empty versus spaces; one Unicode byte-distinct pair, nonnumeric embedded
NUL and numeric span beyond the converter's local buffer. Keep current locale/
NaN/conversion semantics at their existing owner. Cover aliases to either
input, repeated actuals and seeded integer/string/status data whose integer
changes while other fields survive. Assert unknown `[0,1]`, whole-domain reads,
integer-only kills, no selected CALL and no new parsing/copy/guard machinery.
Run original, conservative and cleaned native output with assembler optimization
disabled and one VM. Use fresh one-call helpers or existing explicit whole-state
setup for TRACE; add no product reset and remove no observations. Retain a
recycled undefined string and existing nonordinary/profile negatives with exact
input identity/reasons. Reuse unchanged integer/whole-return/TRACE regressions.

Charge the fixed operand/premise/result/access work before growth through the
existing facts owner. Proof does not scan/copy runtime strings; literal/remap/
output bytes retain existing length-sensitive charges. Reuse frame/state/
entry/byte/code owners and `inline.code.cells`. Add the new work-limit/CLI
last-good control, preserving no-partial-publication behavior; reuse unchanged
valid entry/byte/frame/code controls. Review coherent source and one simple
emitted equality/branch region early, then freeze for a small serial
constructed/native panel using prepared producers. Broader component coverage,
binding splitting, strict comparisons and initiative/milestone QA remain open.

### Concatenation and typed string TRACE transfer increment

**Status: implemented and focused-qualified; broader initiative acceptance
open.** This INL-STEP04/05 slice serves INL-AC02/03/05/06/07 within the
existing exact-bound scalar direct/fixed-call and cleanup profile. Admit the
six canonical CONCAT/SCONCAT R,R,R / R,R,S / R,S,R forms with the existing
opcode/effect/component/signal catalogue. Keep every enclosing call, arity,
CFG, frame/history, child/depth, signal, metadata-scope and observation gate;
the runtime-selector child whitelist is unchanged. Static prepared L-library
occurrences make this worth addressing, but do not establish hotness or helper
eligibility. Strict/ordered string comparisons, ITOS, aggregate/reference
values, broader destination certificate reconstruction and full standalone
acceptance remain separate work.

Before a write, capture both typed operands, their StorageIds, source premises
and destination state. Require ordinary storage and independent defined string
bytes/metadata for each register input; require ordinary destination and its
incoming string metadata certificate to be independent. A fresh/reset or
previously proved destination qualifies, including either input alias; an
unproved recycled destination retains `string.component.unproved`. Preserve
existing external-storage and `argument.scalar.state.unproved` refusals for
unresolved/nonordinary cells. The destination premise is necessary because
CONCAT may retain incoming language-normalization status bits on a non-ASCII
path even though it clears or reconstructs VM-private Unicode flags. A later
proof may track dependent certificates; this bounded slice asserts none.

Keep the exact original CONCAT/SCONCAT instruction and VM value owners. The VM
selects an alias temporary buffer or direct destination buffer, checks byte
length, allocates/reuses owned storage, copies explicit-length bytes, inserts
SCONCAT's one ASCII space, and updates its caches and Unicode certificates.
The proof never concatenates, copies, counts or normalizes runtime strings.
On the normal successor, use the existing string-component transfer only:
defined independent string, nonplain/nonzeroed destination; preserve integer
value/bounds, binary, public status, whole-state/recycler and active/base/slot
facts. Read both register sources in the whole-observation domain and write
the destination in that domain only; there is no integer or whole kill. The
canonical signal class is NONE, while VM allocation/length-overflow behavior
stays at its retained instruction; do not infer success before execution.
Retain existing owned-local MOVE, borrowed/aliased COPY, copyback, ENDLIFE
and snapshot obligations.

The existing generic TRACE gate requires an integer definition even for a
compiler-emitted string result event. Admit only ordinary register-backed
`.traceevent` with source R, supported r/a/g reference, value type S, and
flags bit 0 clear (no RexxValue override). Require independent defined string
component, not integer definition or wholeknown, because this event reads the
typed string. Keep its whole-observation access, global-definition gate and
original source/TRACE address. Leave constant, object, untyped/unknown and
other TRACE events, META scopes and integer TRACE on their existing paths.
An unproved string event retains `local.recycler.observation.unproved`.
Charge the typed decision and fixed component/access work before growth.

Extend maintained constructed proofs for all six forms, source/destination
aliases and repeated actuals, Unicode, embedded NUL, empty operands,
SCONCAT's unconditional space, and surviving integer/binary/public status.
Show a fresh string-only output whose integer remains undefined yet whose
typed S TRACE is accepted. Keep negative controls for missing source string,
recycled destination certificate, unproved typed TRACE and existing
nonordinary/profile cases with exact reasons. Pair one compiler-emitted
single-activation string helper with source/TRACE and one handwritten alias/
copyback consumer; compare original, conservative and cleaned RXAS under
native assembler optimization disabled and one normal VM. Assert selected
calls disappear, emitted CONCAT/SCONCAT and observations survive, exact text
and side fields match independent expectations, and a bounded work-limit
failure preserves last-good CLI output. Reuse unchanged resource owners and
prepared producers; no broad compiler, mode, platform, sanitizer or performance
campaign is selected for this development increment. Full initiative and
milestone QA remain open.

### Reviewed finite optional scalar prologue increment

INL-STEP04 next admits direct calls whose caller already supplies a fixed,
captured set of physical argument cells, including genuinely padded omitted
optional cells. The original arity, current active pointer for each cell and
the result target remain the call contract. A raw CALL with fewer physical
cells acquires neither padding nor default values. Count is not presence:
`REGTP_VAL` (256) and `REGTP_NOTSYM` (512) are independent ABI bits. Evaluate
each callee prologue in source order against its reached StorageId; an earlier
default or alias write may affect a later formal. Preserve default operations,
source/TRACE and scoped metadata and required copy/snapshot/status effects.

The existing BRTPANDT LRI opcode gains a truthful two-edge target CFG: one
label edge and one fallthrough edge, in draft construction and independent
decoded-graph validation. This uses the existing generic L terminal contract
and unchanged ISA/VM behavior. The scalar fact owner tracks the compiler ABI
status band separately from integer exactness, payload and whole-cell state.
SETTP replaces only requested public bands; a zero/public-empty request clears
the ABI band, while a non-ABI public write preserves it. NULL resets it,
whole COPY/MOVE and status-only ACOPY carry it, and ICOPY leaves it unchanged.
The first branch proof admits masks 256, 512 and 768, testing the actual
readable status bits at the current active source. ABI entry-independence is a
separate per-storage fact: fresh/reset or definite ABI writes establish it,
whole/status copies carry it, and feasible joins meet it. Unknown but
entry-independent ABI state explores both edges; an unreset recycled local
whose status may depend on a prior activation keeps the call. Unsupported
masks keep the call with a specific reason.
An ACOPY changes status only, never the integer component, and the proof does
not infer other status bands from ABI knowledge. Existing checked arithmetic,
slot identity, recycled-history and return obligations still apply.

Constructed target CFG controls include wrong-but-well-formed edges. Native B
noopt optional output and a small independently authored scalar prologue
provide behavioral patterns, not native correctness authority. Focused
positive cases cover omitted, supplied-zero and nonzero values, VAL/NOTSYM
combinations, sequential defaults, aliased actuals and required status copies.
Unknown count/window, missing physical cells, unsupported masks and unproved
recycled ABI history keep the exact input with stable reasons; tight budgets
remain atomic.
When an entry-independent BRTPANDT result is unknown, carry separate path
states for its two successors. For a one-bit VAL or NOTSYM mask, the taken
edge knows that bit set and the fallthrough edge knows it clear; a zero result
for the combined 768 mask clears both bits. Retain independence and all other
component facts. Meet these refined states at a join, and charge their copies
to the existing path/resource budget. This permits later presence checks to
use facts established by an earlier test without assuming a value on the
opposite edge. The combined-mask taken edge remains a disjunction unless other
facts already identify its member; it must not manufacture a particular bit.
Constructed controls probe repeated presence tests in both successors for
each one-bit mask, with distinct reached returns, so a sibling-state leak or
missing edge refinement is observable. A three-way 0/VAL/NOTSYM input probes
the union mask: false clears both, while true leaves the member unresolved.
Check conservative and cleaned RXAS with native assembler optimization disabled
and one VM, plus the affected constructed/target/CLI lanes. Wider aggregate,
dynamic-count and exception coverage remains open in INL-STEP04/05.

### Reviewed path-local retained-child DAG increment

INL-STEP04 next admits a same-module scalar child call in a reached branch of
an acyclic outer callee, under the existing direct/closed-root signal and depth
contract. The child remains an ordinary executable call in the expanded outer
body. Reuse its existing normalization and scalar transfer; do not add a
second child expansion, call ABI, CFG owner or allocator. A counted child forces
the complete ordered outer active/anchor layout even when only one branch
contains it. The emitter already remaps the retained call; prove its captured
result, current argument bindings, source/inverse swaps and normal return
restoration at every reached outer exit. Other unproved call forms, targets,
context, signals and recursion retain the exact call with specific reasons.

Before outer path forking, allocate one stable owned-base StorageId row for
each reachable admitted child procedure, charged to the fact budget. These
rows are identities, not initialized values or execution events. Carry mutable
per-procedure activation and last-exit reset history beside the cell and frame
snapshots. Each branch enters with its own history. At a join, activation is
the union of possible histories; a reset guarantee is kept only for paths that
have activated that child and prove its last exit reset the relevant base.
A never-activated path contributes the native fresh-entry case, not a fake
prior reset or an extra sequential activation. If either feasible history
could leave an observed recycled component unknown, the child proof must keep
the outer call rather than invent a NULL or copy. Preserve the active recursion
stack while simulating a synchronous child, and keep child return/selector/swap
ledgers separate from the outer expansion plan. A child's possible effects
contribute to the outer call access summary; a definite kill requires every
normal child exit to perform it.

The exact expanded root can contain retained children. `analysegraph` may use
the same path-local child transfer to publish their access and call-window
facts for cleanup. Keep the existing graph-transport implicit-storage guard
for counted calls: rewriting only the explicit count register does not prove
the required adjacent argument pointers. The graph assignment service may
place such a window densely only with its existing `assignment.window` and
fixed-slot constraints. A later transport admission requires proof that each
reached argument StorageId occupies `countStorageId + j` after mapping, or a
separately proved alias-window construction; repeated/noncontiguous aliases
cannot be erased by visible-operand rewriting alone. Fixed-argument calls have
explicit operands but still require the child/profile and access proof.

Independent constructed and RXAS controls exercise unknown 0/256 branches
with a mutating retained child in both arms, one-arm activation followed by a
later child call, repeated outer activation and child MOVE-reset/history,
aliased snapshots, and a counted r/a-crossing window with setup/reverse
swaps. Assert removal of the selected outer CALL, retention of the child CALL
and its required window, per-exit restoration, no leaked child plan rows,
precise negative keeps and atomic work/byte/frame limits. Run conservative
and cleaned output through native `rxas -n` and one VM. The broader recursive,
imported, aggregate, exceptional and general counted-window transport
categories remain open.

### Reviewed finite retained counted-call specialization

INL-STEP05 next specializes a reached retained counted child CALL in the exact
frozen expanded root's acyclic graph cleanup when its checked entry count is
1..4. This is an existing-ISA CALL1..CALL4 replacement, not a compiler AST
rewrite, new call ABI or a general counted-window erasure. Keep conservative
no-cleanup output executable with the original CALL and physical window.
The exact same-module bytecode target and current closed-root,
default-HALT/no-resume profile are required. The fixed handlers capture the
result and ordered actual value pointers before frame activation, call the
same `frame_f(N,...)` and install the same borrowed active/base argument
bindings. Preserve count/result and actual aliases, repeated actuals, mutable
copyback, public/VM status, and the original call's logical source/TRACE
attachment. Raw physical call-form/argument-base observations remain outside
the approved POC observation profile. The changed opcode spelling in a low
level instruction trace is an accepted presentation difference; authored
source/TRACE location must remain truthful.

Run `lltargetfacts.analysegraph` on the exact expanded caller and require the
existing root-restored, compatible-join and reached-call proofs. Each clone
has its own root record ID. At its reached CALL, capture the *pre-call* result
StorageId, unchanged bound procedure target, singleton count N and each ordered
`argumentstorage(callId,j)` StorageId; use neither post-call state nor equal
integer values as aliases. Require every result/actual ID in 1..original root
locals, including duplicate IDs. Before the graph transport's generic
implicit-storage refusal, make an N+2-operand CALLn row with physical r(ID-1)
operands. Charge the new operand bytes, work and encoded cells before
publication; preserve record/address/source metadata through the existing
typed graph emitter. The existing mapping/SWAP deletion then uses those
explicit operands rather than a hidden adjacent window. Never rewrite an
unknown, zero, >4, native, imported/opaque, fused or unproved target. Count
zero in particular cannot become two-operand CALL because that opcode clears
the result; a counted zero call with bare RET preserves its preloaded result.

The original access liveness still reads the count register, so one pass may
retain a dead count LOAD/NULL; a later bounded reanalysis of the rewritten
frozen module can remove it after separate proof. Do not delete it by assuming
the old count read was absent. General counted calls over four arguments
still need physical `actual[j]=count+j` adjacency after assignment or a
fresh linked pointer window with exact normal/abrupt restoration; repeated
actuals must never become value copies. This increment qualifies the finite
graph path only; linear transport and the general window remain STEP05 work.

The expanded nested caller exposes a separate shared analysis cost before
publication: backward projected liveness currently seeds reachable blocks in
block-ID order, which can revisit acyclic predecessors after each successor.
Seed the existing monotone worklist in reverse RPO, using the index's reachable
RPO membership and preserving the same transfer, queue, budget and result
semantics. The cleanup query charge must also match a safe upper bound for
`domainafter`'s actual block-suffix scan instead of charging the entire caller
for each query. Precompute inclusive operation-cost prefixes and per-block
totals in one budgeted function walk. Charge each query the containing block
total minus the prefix at the queried operation, covering every subsequent
operation's `1 + access.accesses(op)` while excluding the queried operation.
Preflight scratch space and charge the setup walk; a zero-length suffix costs
zero. Keep both changes at their Level L owners, with independent
branched/loop controls, tight-budget atomicity and the unchanged full nested
CLI fixture. Do not raise or silently bypass resource limits to qualify this
case; a bounded refusal remains valid when the actual work exceeds a budget.
If a nested CLI caller exceeds its configured work allowance after those
repairs, expose a validated positive `--work` allowance in the standalone
adapter and pass it to expansion and both cleanup stages. Retain default and
atomic limit tests and document the option. This makes the resource decision
visible to clients without changing proof or ISA semantics.

Focused constructed and handwritten RXAS controls cover a source r/a-crossing
window, both branch arms, reordered actuals and inverse swaps, count=result,
two expansions of one helper, exact aliased repeated actuals with caller
mutation and a separately retained snapshot. Assert no selected outer call,
CALLn retention for the child and removal of proved outer setup/inverse swaps
in cleaned output; conservative output keeps a valid original counted window.
Check count-zero/bare-RET result preservation, >4 and unknown-count keeps,
source/TRACE mapping and atomic work/byte/frame limits. Execute both outputs
with native `rxas -n` and one VM. Full-window, dynamic-count, recursive,
imported and exceptional shapes remain in INL-STEP04/05.

### Reviewed ordinary string-component increment

This INL-STEP04 increment admits ordinary UTF-8 string literals and `SCOPY` within the
existing closed-root, normal-return call profile: `LOAD R,S`, `SCOPY`, whole
COPY/MOVE and `RET R`/`RET S`. This is a component transfer, not a claim that
all string-bearing values are plain integers. The target and VM already own
the opcodes, UTF-8 operand validation, string constant bytes and copy/return
handlers; no ISA or generic L API change is selected. Keep reference/native
payloads and attributes absent by proof for this first slice. Binary loads,
native copy hooks, references, aggregate attributes and abrupt exits are
separate categories, not inferred safe from a string value.

At the target facts owner, retain integer availability, bounded intervals,
ABI status and `_plain` as their existing independent proofs. Add string
component availability/entry independence, covering bytes together with
VM-private UTF-8/cache flags and protected normalization certificates, and
ordinary-scalar absence of
hidden reference/native payload or attributes. Fresh/reset cells establish
an independent empty string and payload absence; unreset recycled contents
lose unproved component facts. A string LOAD establishes only the string
component and preserves integer/public-status/attribute state while following
the VM's reference/native-payload release. SCOPY copies only string bytes and
the related VM-private/normalization certificates, preserving the destination
integer, public ABI status, reference/native payload and attributes. Whole
COPY/MOVE carry the complete proved state; MOVE resets its source. Meet every
component separately across feasible paths. Do not widen `_plain` to admit
string-bearing storage into the current integer-only reuse or ENDLIFE deletion.
ACOPY may replace string certificates without changing string bytes. Their
entry-independence then follows the source status, even when its certificate
bits describe different bytes; preserve the VM's actual copied bits rather
than infer semantic validity. A recycled source with unproved status loses
that independence; ABI-bit knowledge alone cannot certify string metadata.
A recycled string LOAD does not upgrade whole-cell entry independence.
An explicit ENDLIFE on an admitted ordinary string remains executable unless
a separate lifetime proof makes it removable.

Normalize a string actual only with proved component availability and safe
ordinary ownership; a callee read of another unproved component still keeps
the call. A register RET keeps the original own-base MOVE versus borrowed or
rebound COPY rule and requires whole-state independence for its whole transfer.
An immediate `RET S` writes only the captured result's string component: emit
`LOAD capturedResult,S` at that return, exactly matching `set_const_string`.
It must preserve the destination's integer/public status and may release its
reference/native payload as the VM would. An ignored result performs no write.
Extend the per-return access summary so a string-only return does not claim an
integer possible write or definite kill; only whole COPY/MOVE claim a whole
kill. Project string accesses conservatively into the existing whole domain;
this may retain dead string stores, and adds no separate string-liveness API.
Preserve source/TRACE and exact literal bytes under the existing target
writer.

Focused constructed and handwritten RXAS controls cover borrowed string COPY,
owned local MOVE, rebound local COPY, SCOPY snapshots through aliased actuals,
string/int/status coexistence after partial writes, and `RET S` into a
preloaded result with independently checked integer and ABI flags. Include
empty and non-ASCII Unicode literals, repeated MOVE-reset versus unreset
side-component history, precise unchanged keeps for hidden payload/unknown
components, and atomic work/byte/frame limits. Inspect emitted operations and
run conservative and cleaned RXAS with native assembler optimization disabled
and one VM. String-bearing reuse and binary/reference/aggregate expansion
remain INL-STEP04/05 work.

### Reviewed ordinary binary-component increment

This INL-STEP04 increment admits `LOAD R,B`, two-register `BCOPY`, whole COPY/MOVE and
`RET R` for ordinary binary values in the checked closed-root,
default-FAILURE-HALT normal-return profile. There is no `RET B` ISA form.
The target already validates and round-trips exact paired-hex binary literals,
including empty and arbitrary bytes; no codec/ISA or generic L API change is
selected. Hook-backed native payloads, references, attributes, resumed FAILURE
handlers and range-copy forms remain separate categories.

`LOAD R,B` executes the existing `set_binary`: it can fail allocation and
raise FAILURE after releasing a reference payload. The source operation stays
executable and in its original order, with its original source origin. Only its
normal successor gains binary component facts; the checked default HALT
response has no resumed transfer. Ordinary destination proof excludes
reference/native cleanup and arbitrary hooks in this slice, but never proves
allocation succeeds. Linear and graph cleanup must retain every `LOAD R,B`,
including a dead result, because its possible FAILURE remains observable.
The canonical signal catalogue currently says UNKNOWN;
an exact local target transfer for this reviewed handler/profile may specialize
that row without making UNKNOWN generally safe. Its missing canonical
reference-release clear bit must not be mistaken for absence. If a later
profile resumes FAILURE, prove its partial state and cleanup before admission.
The existing POC observation policy preserves an occurring signal's tag,
payload and logical fault source; it does not assert identical resource
exhaustion timing after frame/storage rewriting.

`BCOPY R,R` is an exact no-op for identical storage. Otherwise it copies
ordinary binary bytes while preserving destination integer, string and public
status, but it may invoke a source native hook or abort on allocation failure.
Require source and destination ordinary-payload absence, keep BCOPY executable
and retain its fatal allocation behavior. Track binary bytes/availability and
entry independence separately from integer, string, ABI status and `_plain`.
Fresh/reset cells have independent empty binary; an unreset recycler does not.
LOAD establishes binary bytes, clears VM-private flags as the VM does and
does not invent other component independence. BCOPY transfers only binary
component proof; whole COPY/MOVE and path meet carry the separate rows. Binary
writes have no integer write/kill, and binary ENDLIFE remains executable.
Register returns retain the original own-base MOVE versus borrowed/rebound
COPY rule and require whole-state independence.

For exact VM observation in the maintained handwritten fixture, admit only
root-side `BINEQ R,R,B` with proved ordinary source and destination. Its native handler
does a non-raising byte-length/memcmp then `set_int` on the destination; this
exact local transfer may specialize its UNKNOWN catalogue row under this
profile, not authorize other UNKNOWN operations. Compare independent expected
empty and arbitrary `00 41 FF` bytes, component/status coexistence, borrowed
COPY versus owned MOVE, aliased BCOPY snapshots, repeated MOVE-reset versus
unreset side-component history and exact unchanged keeps. Check atomic work,
byte and frame caps, conservative/cleaned RXAS, native `rxas -n` and one VM.
Binary storage reuse, broad binary observers, native hooks and recoverable
allocation failure remain INL-STEP04/05 work.

### Required native bootstrap correctness repair

The rebinding consumer exposed unsound implicit constant promotion in native
B's shared optimizer: a conditional first write after a declaration was treated
as unconditional initialization. It could substitute the wrong false-path
value and delete the mandatory THEN child before emission. Repair the promotion
legality at `compiler/rxcp_opt.c`, not the downstream emitter or the consumer
assertion. Both implicit promotion arms must require a literal initializer in
the owning callable's unconditional entry sequence, before every substituted
use, with no bypass/alternate entry. Conditional, loop, protected, early-read
and ambiguous cases retain the variable when that proof is absent. Existing
alias/write/assembler-form checks and explicit language constants remain intact.

Qualify a compact maintained native B regression with independent false/true
values, branch/ELSE side effects, early reads, zero/one-iteration control and
an unconditional-fold positive, then the unchanged rebinding consumer. This
necessary producer repair is implemented and focused-qualified within the
standalone increment; it does not
commission native C frontend changes or a native optimizer replacement. Refresh
the changed producer and affected L preparation once after this repair freezes;
keep wider INL acceptance and normal milestone QA open.

### Current canonical-call flow repair

The shared transfer profile must represent a direct counted CALL's published
`windowafter` operand as a symbolic window reference rather than an unexplained
implicit selector. Point flow maps each callee-observed/returned `aN` to the
position `window+N` in the VM's r/g/a ordering through incoming active bindings.
An exact finite nonnegative count may refine presence and return facts. A
defined but unknown count makes each available position a conditional borrow;
it must not produce a known count, argument presence or return literal. The
ordinary VM maps only the supplied borrowed pointers: it does not manufacture
a defined omitted-argument cell. A callee read of a missing `aN` therefore
needs its authored guard/prologue proof or remains unknown. Positional mapping
can still overapproximate possible dirty reads and permit a specific
owned-local warning/clear without an arbitrary closed-root signal premise.
Genuinely dirty/unproved counts, missing physical slots, fused preparation and
unregistered windows remain conservative; calls still pin liveness where
effects are incomplete.
Generated B/library RXAS in the current Debug build contains many ordinary
counted and fused `SETTPCALL`/`SWAPCALL` forms alongside `CALL1`–`CALL4`.
Treat these shapes as practical coverage priorities, not rare exceptions; the
sample is a source-shape inventory, not a runtime hotness measurement.
Entry-normalization witnesses concern owned storage: an observed borrowed
argument or global with unknown contents does not by itself make the callee's
owned-local entry proof incomplete. An active binding to owned storage still
requires the ordinary dirty-read proof. Focused canonical-flow controls must
cover both the selected finite call and unsupported cases before inliner QA.
Level L normal-target projection must consume the generated catalog's normal
successor and fallthrough fields for conservative FLOW_NEXT instructions as
well as classified ones. A stale L-only classification gate can split an
inlined body at `QGLOBBIND` and leave an unterminated IR block; the constructed
CFG fixture must cover the newly admitted scoped-entry form.

## Complete initial target profile

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

All 23 current record kinds are included: STRING, BINARY, DECIMAL, FLOAT, PROC,
EXPOSE_REG, EXPOSE_PROC; META_FUNC, REG, CONST, CLEAR, CLASS, ATTR, INTERFACE,
IMPLEMENTS, MEMBER, INLINE, SOURCE_STEP, TRACE_EVENT, TASK_TARGET, PROVIDER,
INITIALIZER and AUTOLOAD. Preserve unused records, ordered chains, raw literal
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
Each supported expansion proves argument evaluation/order/presence, aN entry
baselocals and UNLINK behavior, return moves and storage ownership, local/label/
metadata relocation, per-module gN identity, module-initialization ingress and
physical-frame, exception, numeric/context and cleanup observations. Mapping
aN to fresh rN alone is unsound: active and initial bindings must both survive
UNLINK/LINK/SWAP and nested call windows. RET may move owned locals but copy
arguments/globals/linked storage; arbitrary copies can change reference identity.
Preserve EXPOSE/borrow/reference expiry and signal/CANCEL cleanup. Inlining at
initializer ingress must retain callee initialization and cycle guards. Composite
origins alone do not reproduce an inline call stack. Missing proof keeps the
call with a specific reason; later profile growth needs review. Prove expansion
with subsequent RXAS optimization disabled, fresh IDs and no coalescing first.

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
3. **TGT-AC03 — lossless semantic codec.** All six sections/23 record kinds,
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
   required reference remaps; enabled inlining is qualified under INL-AC02–07
   and TGT-AC06/07.
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

All steps are **todo**. Preserving the proposal does not complete STEP01 or
authorize implementation; detailed profile and contract approval remain open.

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
   Integrate the one shared expander developed through INL-STEP01–05; do not
   implement another algorithm. Prove local direct scalar cases and unchanged
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
