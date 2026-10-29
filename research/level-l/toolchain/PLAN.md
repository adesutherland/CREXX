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
claims. In particular, review linker-private global aliases and exact-site
initialization ingress alongside virtual inline provenance and useful language
effect/ownership facts. Recursion/growth policy is distinct from semantic
feasibility. API/ISA, language/platform changes and production default replacement
require separate decisions.

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
| **INL-STEP01 — architecture and contract** | Architecture approved; AC01–08. The recommended preservation policy, shared target expansion/cleanup boundary and private-global binding direction are accepted. Record and internally review concrete APIs/schema/profile details for each coherent increment before product edits; unverified implementation criteria remain open. |
| **INL-STEP02 — shared model and conservative expansion** | first constructed-model slice done; after 01; AC01/02/04/06/07. Optional `rxfnsl-target`, L-owned frozen records/descriptors, exact binding and conservative direct scalar-return expansion under the checked closed-root signal profile are implemented and focused-qualified, using fresh identities and no copy elimination. Broader model/call coverage and cleanup remain open; this does not complete the full INL outcome. |
| **INL-STEP03 — standalone RXAS client** | initial adapter done; after the first 02 slice; AC01/02/04/06/07. Generated-L RXAS reader/writer, callable client and single-output CLI are focused-qualified with a real generated caller, independent handwritten expansion, optimized input and atomic-failure controls. Exact-bound imported helpers and broader profile/qualification remain open. Full assembler/codec/linker delivery is unnecessary. |
| **INL-STEP04 — complete representable expansion** | in progress; after 03; AC02–04/06/07. Scalar/metadata/batch, straight-line callee LINK/UNLINK/SWAP, one retained same-module scalar nested leaf in straight-line and acyclic branch bodies, finite LINKARG/known-passing ICHKRNG, private same-module scalar globals, acyclic direct-branch scalar multiple returns, finite padded optional scalar prologues, ordinary UTF-8 string LOAD/SCOPY/whole-return and immediate-string return, and ordinary binary LOAD/BCOPY/whole register return are implemented and focused-qualified. Direct target CFG transport/validation and detached-scope closure are focused-qualified. Complete broader aliasing, dynamic varargs, branch-local metadata, recursive nesting, aggregate/reference and state/cleanup categories. Classify each hard boundary; do not leave feasible categories silently outside the finish line. |
| **INL-STEP05 — proof and cleanup** | in progress; after each coherent conservative category, with final qualification after 04; AC02–07. Shared access/liveness, canonical fact extraction, scalar transport, typed assignment/checking, per-path scalar status/storage/activation meets, bounded integer success across acyclic joins, ABI-bit presence and branch refinement, ordinary string/binary component and recycler proof with retained nonplain ENDLIFE and failure-capable binary LOAD, and the fresh-root private-scalar reuse slice are focused-qualified. The acyclic closed-root r-only graph path removes proved mapping and plain-lifetime glue, simplifies proved branches and assigns distinct reached cells densely; linear cleanup remains separately qualified. Retained branch-child calls have root graph access summaries; reached same-module counted children with one to four proved actuals now use explicit CALL1–CALL4 pointers, removing proved caller setup/inverse swaps. Other counted implicit windows still need adjacency or equivalent alias-window mapping. Shared backward liveness uses reverse RPO and transport query charges match bounded block suffixes; larger CLI requests can select an explicit per-stage work allowance. Private same-module globals retain a conservative expanded caller under explicit cleanup guards. Typed end-anchor observations, changing scoped bindings, broader control/component, alias and lifetime reuse, and checked cross-block sharing remain open. Retain independent no-cleanup results; inspect simple and alias-heavy output, not only execution. |
| **INL-STEP06 — standalone research acceptance** | todo; after 03–05; AC01–08. Freeze and run the compact agreed integration panel through the matching assembler/linker/VM profile (current tools for the unchanged contract; explicitly approved owner extensions when selected), plus CLI/service/DSL, negative/lifecycle and structural cases. Use one normal VM; broaden only for specific unresolved risks. Commit reviewed authorized paths locally; no push/default switch. Wider TGT criteria remain open. |

Per-feature checks and execution allocation follow the repository instructions;
INL-STEP06 is the named standalone integration milestone.

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
