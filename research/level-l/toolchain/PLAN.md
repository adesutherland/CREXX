# Level L target-toolchain research

**Status: standalone inliner architecture approved; implementation in progress.**
Adrian has requested a standalone, effective RXAS inliner proof of concept and,
approved this plan for implementation. The [inliner contract](doc/inlining.md)
and INL criteria/steps below own that selected outcome within this single plan.
The wider assembler/reassembler/linker proposal remains proposed; all TGT
acceptance criteria remain open. Implementation and reviewed local commits for
the INL outcome are authorized; native replacement and publication are not.

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
they are not automatically reclassified as inherently infeasible or accepted
exclusions. Native/dynamic targets without one bound bytecode body, foreign private
globals without a binding mechanism, and physical-frame observations without an
equivalent are explicit current-boundary cases, not permanent impossibility
claims. The linker-private global alias option has been reviewed and is not
selected for this POC; review exact-site
initialization ingress alongside virtual inline provenance and useful language
effect/ownership facts. Recursion/growth policy is distinct from semantic
feasibility. API/ISA, language/platform changes and production default replacement
require separate decisions.
Inlining preserves deterministic language values, control, ownership, delivered
signals, logical source and cancellation/unwind. It need not reproduce slight
differences in resource-pressure failure paths caused solely by removed or
rearranged infrastructure, such as allocation of a call frame that no longer
exists; such hypothetical opportunities alone do not retain a call.

| Stable criterion | Observable completion and verification |
|---|---|
| **INL-AC01 — shared service and real adapter** | One B/L checked target-model inliner serves an ordinary RXAS CLI, a callable B client and an independent handwritten/DSL-style producer. No AST inline payload, copied parser/framework or native backend is required. Input/output, unknown/unsupported forms and atomic publication have explicit contracts. |
| **INL-AC02 — exact call/storage mapping** | All admitted direct, counted, fixed and fused forms preserve actual evaluation, counts/presence/defaults, active and entry argument bindings, slot identity/permutations, status components, explicit reference/value behavior, nested calls and return copy/move ownership. Independent executable and structural controls include repeated actuals, mutable aliases and abrupt exits. |
| **INL-AC03 — full representable profile** | The architecture's coverage catalogue is implemented and checked, including rebinding, varargs, multiple returns, aggregate/reference lifetimes, context/cleanup and exact-bound imported helpers. Every retained case has a specific semantic, missing-proof, body-availability or policy reason. Unimplemented representable categories keep this criterion open. |
| **INL-AC04 — module and observation integrity** | Globals keep their module/storage owner; initialization, poison/cycle ingress, dependencies, numeric/signal state, reference expiry, source/TRACE/diagnostic and declared physical-frame observations remain equivalent. Evaluate linked private aliases and narrowly scoped runtime/metadata or language additions; record the selected compatibility/observation contract before implementation. Cases lacking an approved equivalent stay calls. |
| **INL-AC05 — effective bounded cleanup** | Expansion is correct with cleanup and assembler optimization disabled. Separately prove status/transport/copy elision, constants/presence, branch/reachability cleanup and alias-aware storage reuse on changed callers. Eliminate caller setup and reverse call-window swaps made redundant by direct actual-register mapping, using storage/permutation proof across the entire call region. A simple reordered-actual leaf has no original call or outer setup/restoration swaps; an alias/snapshot case retains required copies, and a retained nested call keeps a valid window. Preserve once-only actual order, result/entry/base/slot identities, abrupt-exit restoration and swaps required by body semantics. Previously optimized modules are covered without assuming their old analysis facts remain valid. |
| **INL-AC06 — deterministic policy and resources** | Separate legality, availability and profitability decisions. Explicit recursion, depth, code/register/metadata growth and work limits terminate predictably, support diagnostics and forced-legal/disabled controls, and preserve ordinary calls on policy refusal. Malformed input or failed transformation publishes no partial output. |
| **INL-AC07 — metadata and lifecycle** | Relocate executable and observation references together; keep runtime graph/provider/task/source contracts, source/module identity and initializer dependencies. Prune only under explicit reachability/consumer rules. Invalid/repeated/interleaved/resource/cancellation/I/O cases retain owned results and last-good publication. |
| **INL-AC08 — reviewed research delivery** | Early coherent implementation and representative RXAS review, focused development checks and a final compact integration panel qualify both expanded and retained cases against independent expectations. Guides, coverage/limits and this plan are current. No production-speed, full toolchain, wider platform, SAN-clean or publication claim follows automatically. |

| Stable step | Status, dependency and decision gate |
|---|---|
| **INL-STEP01 — architecture and contract** | Architecture approved; AC01–08. The preservation policy and shared target expansion/cleanup boundary are accepted; foreign private globals retain calls under the selected current-format profile. Record and internally review concrete APIs/schema/profile details for each coherent increment before product edits; unverified implementation criteria remain open. |
| **INL-STEP02 — shared model and conservative expansion** | first constructed-model slice done; after 01; AC01/02/04/06/07. Optional `rxfnsl-target`, L-owned frozen records/descriptors, exact binding and conservative direct scalar-return expansion under the checked closed-root signal profile are implemented and focused-qualified, using fresh identities and no copy elimination. Broader model/call coverage and cleanup remain open; this does not complete the full INL outcome. |
| **INL-STEP03 — standalone RXAS client** | initial adapter done; after the first 02 slice; AC01/02/04/06/07. Generated-L RXAS reader/writer, callable client and single-output CLI are focused-qualified with a real generated caller, independent handwritten expansion, optimized input and atomic-failure controls. Broader exact-bound imported helpers and profile/qualification remain open. Full assembler/codec/linker delivery is unnecessary. |
| **INL-STEP04 — complete representable expansion** | in progress; after 03; AC02–04/06/07. Scalar/metadata/batch, straight-line callee LINK/UNLINK/SWAP, one retained same-module scalar nested leaf in straight-line and acyclic branch bodies, finite LINKARG/known-passing ICHKRNG, checked runtime LINKARG with at most three choices and a straight-line scalar or bounded acyclic suffix through multiple reached RETs including a conditional retained same-module scalar CALL1 under its leaf gate, private same-module scalar globals, acyclic direct-branch scalar multiple returns, finite padded optional scalar prologues, ordinary UTF-8 string LOAD/SCOPY/whole-return and immediate-string return, ordinary binary LOAD/BCOPY/whole register return, exact global-free straight-line and acyclic branch imported scalar bodies with proved private locals plus one retained already-imported exported scalar child, existing public-global mapping for imported bodies, and bounded caller-owned unbound G-style signal scopes through the full 29-name catch-all, bound event capture and one guarded post-join owner projection around a checked range leaf are implemented and focused-qualified. Direct target CFG transport/validation, detached-scope closure and stable-register scoped META re-entry on direct acyclic calls and the checked runtime-selector suffix are focused-qualified. Complete broader imported, aliasing, dynamic varargs, prefix/LINKARG-group scoped META re-entry, changing-storage metadata, recursive nesting, aggregate/reference and state/cleanup categories. Classify each hard boundary; do not leave feasible categories silently outside the finish line. |
| **INL-STEP05 — proof and cleanup** | in progress; after each coherent conservative category, with final qualification after 04; AC02–07. Shared access/liveness, canonical fact extraction, scalar transport, typed assignment/checking, per-path scalar status/storage/activation meets, bounded integer success across acyclic joins, ABI-bit presence and branch refinement, ordinary string/binary component and recycler proof with retained nonplain ENDLIFE and failure-capable binary LOAD, and the fresh-root private-scalar reuse slice are focused-qualified. The acyclic closed-root r-only graph path removes proved mapping and plain-lifetime glue, simplifies proved branches and assigns distinct reached cells densely; linear cleanup remains separately qualified. Retained branch-child calls have root graph access summaries; reached same-module counted children with one to four proved actuals now use explicit CALL1–CALL4 pointers, removing proved caller setup/inverse swaps. Reached exact same-module counts above four now use a fresh linked pointer window under the checked profile; zero, unknown and unproved implicit forms remain open. Shared backward liveness uses reverse RPO and transport query charges match bounded block suffixes; larger CLI requests can select an explicit per-stage work allowance. Private same-module globals retain a conservative expanded caller under explicit cleanup guards. Typed end-anchor observations, changing scoped bindings, broader control/component, alias and lifetime reuse, and checked cross-block sharing remain open. Retain independent no-cleanup results; inspect simple and alias-heavy output, not only execution. |
| **INL-STEP06 — standalone research acceptance** | todo; after 03–05; AC01–08. Freeze and run the compact agreed integration panel through the matching assembler/linker/VM profile (current tools for the unchanged contract; explicitly approved owner extensions when selected), plus CLI/service/DSL, negative/lifecycle and structural cases. Use one normal VM; broaden only for specific unresolved risks. Commit reviewed authorized paths locally; no push/default switch. Wider TGT criteria remain open. |

Per-feature checks and execution allocation follow the repository instructions;
INL-STEP06 is the named standalone integration milestone.

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

### Reviewed private-global boundary and pending logical-activation decision

Same-module private globals are representable and already have a bounded
inlining path. Foreign private globals retain calls with
`import.global.private.pending`: current `gN` operands address the executing
module's bank and private type metadata does not bind foreign storage. Public
`gN .expose=qualified.name` on both modules already maps different physical
indices to one worker-local value. Merely adding that declaration to a
compiler-private global changes source import visibility because the compiler
imports matching exposed `META_REG` names. Complete typed-global metadata
already exists; adding more type data does not solve this storage/privacy
boundary. Adrian accepts this retained-call case for the POC, noting that
globals are expected to be relatively uncommon and unlikely to be hot; this
is a prioritization judgment, not measured workload frequency.

A reserved private link key with compiler filtering or a separate typed
linker-private alias record remains a possible later compatibility design,
not a selected POC feature. If adopted later, a handwritten producer must be
able to express a stable symbol without naming the provider's allocated `gN`;
the linker/loader must preserve exact provider identity, worker-local ownership
and initialization ingress. No source hash, attestation or inliner-side META
payload is selected.

The approved resource rule permits modest variation in infrastructure-pressure
error paths, including elimination of `FAILURE` from removed call-frame
allocation. Full catch-all still needs exact state and unwind semantics for
`CANCEL`, inherited `SIGRET`, partial setup and atomic
return; an `INLINE_ENTER`/`INLINE_LEAVE` ISA/VM fallback remains an explicit
decision if existing instructions cannot express them. Neither follows from
private-global metadata.

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

**Status: direct acyclic slice implemented and focused-qualified; runtime
selector reuse and milestone QA todo.** This INL-STEP04/05 increment replaces the
conservative scoped-META dominance/early-layout gates only where the original
lexical address state can be reproduced with stable observed storage. First
qualify the current direct acyclic scalar call path. Then admit the checked
runtime-selector DAG through the same projection/reconciler if its case-local
publication satisfies the identical contract. Retain all existing caller,
arity, operation, successful-arithmetic, child/depth, module and terminal-signal
gates. There is no new source-language rule, RXAS syntax, ISA, wire record,
linker/VM algorithm, producer proof META, graph solver or register-reuse slice.
The original `.meta "name"="b" "type" rN` declaration and
`.meta "name"` clear express the contract for handwritten RXAS too.
New re-entry admits REG `rN` only. Preserve the already-qualified common-entry
`aN` path and its shared closure; branch-local/changing `aN` metadata retains
its existing unsupported boundary rather than gaining re-entry support.

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

**Status: suffix-only increment implemented and focused-qualified; prefix and
LINKARG-group metadata plus milestone QA todo.** Extend the qualified original-address
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

**Later todo in this same increment family:** prove nonempty prefix scoped
checkpoints and transitions on the selected LINKARG address group. That follow-on
must distinguish incoming descriptors from the authored group's ordered records,
keep prefix/check/dispatcher effects single, and place META/source/TRACE/alias
records at each case's original logical binding address without attaching them
to dispatcher scratch or duplicating observations. Review its exact group
staging, prefix restoration and compatibility controls before product edits;
the suffix-only proof does not discharge that boundary.

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
If the full nested CLI caller still exceeds the default one-million work
allowance after those repairs, expose a validated positive `--work` allowance
in the standalone adapter and pass it to expansion and both cleanup stages.
Use the explicit allowance only for this larger integration fixture, retain
the default and atomic limit tests, and document the option. This makes the
resource decision visible to clients without changing proof or ISA semantics.

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
archive requirements. Linking changes references/encoding where necessary but
does not optimize, expand or otherwise rewrite instruction sequences.

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
   Instruction sequences remain unchanged apart from required reference remaps.
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
