# Complete Classic C compiler in B and Level L

**Complete C is open. CLC-STEP05.ITERATE1–3 are locally qualified.
Current: PARSE1 design review is complete; PARSE2's proposed generic cursor
prerequisite awaits approval. C PARSE integration has a later exact-contract
gate, followed by ARG using the same engine. No PARSE implementation is authorised.**

Deliver one explicitly selected Classic C compiler and reusable compile service,
written in B using Level L parsing, IR, transformation and emission. Complete
the agreed instructions, all 95 BIF names, Unicode, diagnostics, conditions/TRACE,
host/lifecycle/platform contracts and, last, INTERPRET. Native B is the build
seed; new C requests never delegate to a native source compiler/backend.
Existing RXAS/RXLINK, VM and native host transport remain execution dependencies.

The [requirements ledger](doc/levelc-requirements.md) owns item-by-item status:
25 instructions, 95 BIFs, 75 reference capabilities and 36 syntax contracts,
with reuse, missing integration, owners and closure checks. Native passes and
existing B bodies are controls, not new-C passes. [Architecture](doc/architecture.md)
owns contracts; [work packages](doc/agent-work-packages.md) own assignments.
Astra reviews/coordinates; Sol 6.1 Extra High workers implement, build and integrate.

<a id="current-action-parse"></a>

## Current action: PARSE engine approval

Deliver PARSE's agreed source/template behavior through one shared executor,
then use that executor for ARG. Earlier completed writes must be visible to
later dynamic operand reads without reading future operands early or replaying
template prefixes. The [machinery review](doc/architecture.md#parse-machinery-review)
records existing investment and the native live-pool ordering limitation;
native output remains a read-only control, not the new compiler's oracle.

The **unapproved** [generic cursor proposal](doc/architecture.md#proposed-parse-cursor-contract)
owns the exact API, events, errors and lifecycle, with the single acceptance
matrix **PARSE-ENGINE-AC01–06**: event/order semantics; validation/errors;
ownership/lifecycle; finite resources/HALT; one kernel with mandatory core/static
delivery; and independent consumers, existing-consumer compatibility and actual
RXAS proof. Its **PARSE-ENGINE-STEP01–04** own approval/boundary freeze,
kernel/provider implementation, independent tests/build delivery and final
qualification/review/cleanup. All engine implementation and proof remain open;
these IDs are not a second C integration acceptance matrix.

**CLC-STEP06.PARSE1 — Design review complete:** existing kernel/exits,
independent ordering expectations and core/static build feasibility are reviewed.
Record the exact proposed prerequisite before requesting Adrian's approval.
**CLC-STEP06.PARSE2 — Pending approval:** implement and independently qualify
only the approved generic engine under PARSE-ENGINE-AC01–06 / STEP01–04;
no generic API, ISA, native C feature or product edit is authorised yet.
**CLC-STEP06.PARSE3 — Later exact-contract gate:** after that prerequisite,
review C source acquisition, templates, AST/IR/target operations, live pool
effects, diagnostics/conditions and code shape before C integration. Record
its own checkable criteria and obtain approval for exact new contracts first.
ARG subsequently consumes the same executor rather than introducing another.

The seven agreed PARSE sources and current EXTERNAL/NUMERIC exclusions retain
their ledger dispositions. Shared labels/EXPOSE, host/condition/TRACE and all
twelve CFC criteria remain open; INTERPRET remains last. This action is
documentation and approval preparation only, with no PARSE implementation or
implementation qualification.

<a id="current-action-iterate"></a>

## Completed named ITERATE increment

Complete LC-I-10's admitted static nearest/named transfers without changing
DO evaluation, selected-loop state or end-work order. Resolve raw names within
the current routine using the qualified shared Classic casing rule; close only
crossed inner state, retain the selected owner and enter its existing UNTIL /
advance / entry path. Adrian approved the
[exact transfer contract](doc/architecture.md#approved-leave-iterate-contract)
on 20 October 2026, conditional on the now-completed LEAVE commit. Reuse its
optional-name AST, request-local STRICTC, C v6/52 operations, 33 exports and
shared invalid-transfer bridge. No grammar/schema/API/Unicode algorithm change
is proposed. Native output remains a read-only control; INTERPRET remains last.

| ID / status | Approved checkable closure |
|---|---|
| **ITERATE-AC01 — Locally qualified: source/targets** | Independent nearest/named/shadowed/raw compound targets and original spans use the existing optional Name AST and shared casing. Groups/IF/SELECT do not become targets; state-free wrappers and private routine boundaries preserve lookup. Existing nearest-ELSE authority remains, including bare inline ITERATE plus ELSE versus ordinary/grouped ELSE names. Reuse unchanged source/name/Unicode proof and retain malformed/dead syntax refusals. |
| **ITERATE-AC02 — Locally qualified: policy/errors** | Default C refuses source-provable invalid ITERATE even dead/uncalled: 28.2 without a repetitive loop, even named, and 28.4 for a missing name inside loops. STRICTC signals only reached invalid instructions with authored name, visible C source and unchanged bound raw helper event; dormant invalid paths do not signal. Reuse qualified header/request isolation and four-code admission/bridge proof; distinct CLI refusals preserve prior output. |
| **ITERATE-AC03 — Locally qualified: execution/lifetime** | Independent markers/control values prove only crossed inner owners close, selected state survives and the jump reaches normal UNTIL before advance/entry. True UNTIL skips advance; false UNTIL retains step/count/entry order and the final FOR-limited advance. Same-name shadowing, raw compound identity despite tail changes, stateful/state-free crossings and private-call/reentry boundaries preserve state without source EXPOSE/shared-pool inference. |
| **ITERATE-AC04 — Locally qualified: quality/qualification** | Actual IR-off/on caller RXAS shows required inner-to-outer closes and one selected end-point jump, no target pool search/helper or selected-owner close, and unchanged fatal/RETURN/source obligations. Existing focused selectors, small independent corpus and representative paired B/IR/VM Debug cells qualify the slice; reuse v6 metadata/admission, shared B/Unicode, native and current grammar proof. Retain default coverage, generous serial scheduling, concise docs and task-only cleanup; no new aggregate, broad QA or per-slice ASan. |

**CLC-STEP05.ITERATE1 — Approved/complete (AC01–04):** exact contract,
independent expected state/order and reused dependencies are recorded before
product edits. Coordinator review/dispatch is complete on disjoint ownership.
**CLC-STEP05.ITERATE2 — Complete (AC01–04):** named ITERATE and focused
source/runtime/shape controls reuse the existing LEAVE foundation; only
`llcfrontend` changes in the product.
**CLC-STEP05.ITERATE3 — Locally qualified/complete (AC01–04):** focused
Debug proof and independent actual RXAS review pass; owning docs and scheduling
are current. Whole-instruction dependencies remain open.

Shared labels/EXPOSE, SIGNAL/INTERPRET loop invalidation, complete expressions,
exact recovery, resumable conditions/SIGL and clause/TRACE retain their owners.
All twelve CFC criteria remain open; this bounded increment is not whole C.

<a id="current-iterate-proof"></a>

### Current ITERATE proof

Normal Debug qualifies `d61ac16c3` plus frozen frontend `b56a02b9` and the
reviewed focused fixtures (corrected positive source `19f949fa`). Affected-only
preparation passes 15 commands in 15.922s, warning-free. Grammar/parser, v6/52/33
schema/target, header policy, protocol/admission, Unicode and shared B runtime
are unchanged; their [LEAVE proof](#current-leave-proof) and the existing native/
core `stage-product` proof are reused. No grammar or native rebuild was needed.

| Command / inspection | Observed result and boundary |
|---|---|
| `frontend-tests … iterate` | 52/0 in 18.274s: optional-name/spans, nearest ELSE, static targets, default dead/private refusals and STRICTC dormant invalid source. Existing header/Unicode/name controls are reused. |
| `backend-tests … iterate` | 104/0 in each paired opt/rxbvm and noopt/rxtvm cell (52.547/43.678s), internally both IR modes. Five cases follow selected UNTIL/advance/entry successors, retain selected owners, skip state-free wrappers, preserve shadowing and include the shared LEAVE inclusive-close regression. |
| `check-backend.py` selected `iterate-…` cases | All 40 commands pass: opt both IR modes run four cases (16 commands each, 35.009/35.973s); noopt/rxtvm runs the positive case in both modes (4 each, 20.194/18.778s). Independent UNTIL 2/10, FOR final 5/10, shadow 10/11→12→13, raw-tail 1/90/10, inline ELSE and private 5/5 states pass. Reached 28.2/28.4 locations and dormant STRICTC pass. Explicit x/y selfbindings corrected the initial case-sensitive pool setup; its 41.6 was consistent with the missing lowercase element, not a product defect. |
| CLI refusal sentinels; one native C read-only control | Three distinct refusals preserve prior output/no staging residue: dead no-loop 28.2, missing-target 28.4 and malformed STRICTC syntax. Native compiler `-n`/assembler `-n` prints `for 1 10`, `for 3 10`, `for-final 5 10` from the independently expected outer-ITERATE control. Seven commands pass in 5.507s; native output is not the oracle. |
| Complete caller RXAS, root/peer review | Clear selected UNTIL→advance→entry, state-free/SELECT crossings, shadowing, private reentry and fatal paths. Only crossed inner owners close; selected state survives to normal end work, child references release before requirevalue, and logical source/raw names remain truthful. No runtime target search, transfer helper or per-transfer numeric setup is added. Twelve same-IR capture pairs are byte-identical across B hosts; ordinary context/frame/packet costs remain admitted C-OPT baseline costs. |
| Existing coverage/scheduling | Default grammar/backend selectors retain new assertions and four corpus cases. Backend 58 cases/421 commands retains serial lock/1800s; frontend grammar retains serial lock/900s. Measured new runtime/shape components support these hang guards, with no full-aggregate replay. Maintained examples contain no LEAVE/ITERATE, so unchanged relevant lowering and prior current-v6 view proof are reused without regeneration. |

Shared v6 metadata/admission/header/direct bridge/Unicode proof is reused;
no second shared matrix, source-slice ASan, installed/hosted/platform or whole-C
qualification is claimed. Shared labels/EXPOSE, dynamic loop invalidation,
resumable conditions, exact recovery and clause/TRACE remain open; all twelve
CFC criteria remain open and INTERPRET remains last. PARSE machinery review is
next, with ARG using that engine after exact-contract approval.

<a id="current-action-leave"></a>

## Completed named LEAVE increment

Complete LC-I-09's admitted static nearest/named transfers while preserving
control values, source, ownership and failures. Resolve authored names within
the current source routine using the shared Classic symbol-casing rule, never
pool lookup or compound-tail substitution. Reuse the qualified DO protocol and
shared invalid-transfer helper. Adrian approved the
[exact shared transfer contract](doc/architecture.md#approved-leave-iterate-contract)
on 20 October 2026; DO is qualified and locally committed. This increment adds
the shared optional-name AST/STRICTC/v6 foundation and implements named LEAVE.
It retains four C types, advances C to v6/52 operations and the target to 33
exports, and refuses v5. At that LEAVE checkpoint, named ITERATE retained its
profile refusal; the separately qualified increment above now integrates it. No SDK, shared-runtime algorithm, native or VM API changes
are commissioned.

| ID / status | Approved checkable closure |
|---|---|
| **LEAVE-AC01 — Locally qualified: source/targets** | Independent optional-Name AST and original instruction/name byte spans cover nearest, named/shadowed/raw compound and Unicode controls through group/IF/SELECT wrappers and private routines. Eligible inline IF keeps bare `leave else` as LEAVE plus ELSE; ordinary/grouped ELSE names and `leave i else` retain nearest-ELSE ownership. Malformed/surplus/dead syntax refuses with locations; named ITERATE explicitly refused at that checkpoint. |
| **LEAVE-AC02 — Locally qualified: policy/errors** | Ordinary requests reject all source-provable invalid LEAVE targets, including dead/uncalled source: 28.1 without a repetitive loop, even named, and 28.3 for a missing name inside loops. STRICTC checks syntax everywhere but invokes the shared unchanged error only when reached, with authored name and visible C location/raw helper event. Same-process enable/default/failure, duplicate/quoted/later/call-shaped header controls prove request-local static policy and failed parsed.strictc()=0; prior output survives refusal. |
| **LEAVE-AC03 — Locally qualified: execution/lifetime** | Independent markers/control states prove LEAVE skips crossed UNTIL/advance work, preserves current values, selects the innermost matching controlled loop and closes crossed owners inner-to-outer including its target. State-free crossed loops add no owner, group/SELECT wrappers are skipped, unrelated outer state survives and repeated private calls remain isolated; no source EXPOSE/shared-pool claim is inferred. |
| **LEAVE-AC04 — Locally qualified: ABI/admission/quality** | Exact v6/52-op/33-export metadata and all-record code/name controls admit only the approved schema/signature, including dead/loaded records. Fresh and post-transform invalid graphs refuse atomically without graph/prior-output mutation. Direct ERROR-origin bridge checks and IR-off/on caller/helper RXAS show only required closes/jumps for valid transfers, no runtime name search, and invalid cold closure/common fatal cleanup. Focused existing Debug selectors and representative paired B/IR/VM cases, reviewed docs and task-only cleanup qualify the bounded slice; reuse unchanged DO/B/L/native proof and checked warm grammar. |

**CLC-STEP05.LEAVE1 — Approved/complete (AC01–04):** exact architecture,
independent expectations and dependencies recorded before product edits.
**CLC-STEP05.LEAVE2 — Complete (AC01–04):** approved shared AST/header/schema/
bridge and named LEAVE are implemented, retaining all source validation and
existing nearest-ELSE authority.
**CLC-STEP05.LEAVE3 — Locally qualified/complete (AC01–04):** the focused
Debug proof and independent actual RXAS review below pass. Owning docs and
scheduling are current; whole-instruction dependencies remain open.

<a id="current-leave-proof"></a>

### Current LEAVE proof

Normal Debug qualifies the frozen LEAVE source/fixtures on `579a8377`, with
separately qualified B seed repair `63184ae42`. Merge `2b536f6df` reconciles
unrelated upstream GLOBALV additions without changing these inputs; no GLOBALV
qualification is claimed. Affected modules/hosts/fixtures were prepared
warning-free through direct recipes; final frontend/backend fixtures are
`b776d7bf` / `f5bba6ab`. The seed's required core `stage-product`
proof below is reused; no merge-only rebuild or research grammar refresh.

| Command / inspection | Observed result and boundary |
|---|---|
| Checked warm C grammar recipe and `rxas` | Pass in 10.951/3.872s: 29,709 NFA / 5,609 DFA / 2,707 LR, 7,778,155 RXAS bytes. Capsule/manifest bytes and mtimes unchanged. This pre-seed-refresh table remains byte-identical and qualified; changed native/stamp identities are not rewritten to force reuse. |
| `frontend-tests … leave`; focused `leave-policy` and `options-refusals` | Reuse 132 passing assertions from the initial 136-check run, plus repaired policy replay 21/0 and reduced OPTIONS refusals 40/0. This is composed proof after B-SEED-01, not a wholly green initial run. Raw spans/targets, nearest ELSE, default/dead refusals and request-local STRICTC are covered. |
| `backend-tests … leave`; focused `leave-invalid` | Opt uses 90 passing positive assertions plus 18/0 focused invalid replay; noopt/rxtvm passes 108/0 in 42.137s, internally both IR modes. Disposable-link preparation corrections changed no product/oracle. Default coverage retains all six cases. |
| `target-context … leave`; `check-target-context.py` | Direct bridge 14/0 in each paired B/VM cell; helper shape 238/0 opt and 241/0 noopt. Exact 33 exports, four unchanged shared errors and bound raw ERROR event/source forwarding pass. Source-bearing direct fixtures are linked unstripped. |
| `c-context-contracts … leave`; DROP count maintenance | 166/0 in 30.532s: exact v6/52 schema, fresh/loaded/selected graphs, all-record code/name/dead checks, atomic limits/input/prior-output controls. DROP maintenance 47/0. Unchanged DO 313, B pool/Unicode and shared-runtime proof are reused. |
| `check-backend.py` four selected `leave-…` cases, IR on/off | All four paired opt/rxbvm and noopt/rxtvm cells pass 16 commands each in 34.330/35.526/35.452/35.897s. Independent state/markers, reentry, dormant STRICTC and visible 28.1/28.3 locations pass; assembler `-n` isolates compiler IR. |
| `check-step07.py --c-only --leave-metadata`; CLI refusal sentinels | Metadata 10/0 commands in 16.093s; four one-field new-signature mutations preserve prior output. Three distinct CLI refusals retain caller output/no staging residue: default dead invalid target and malformed default/STRICTC syntax across both IR modes. |
| One native C read-only control; complete caller/helper RXAS | Native compiler `-n`/assembler `-n` prints independently expected `outer 1 10` in four commands. Root and peer review clear required closes/jumps, skipped end work, stable owners, reentry and fatal cleanup/raw source; 20 same-IR capture pairs are byte-identical across B hosts. No runtime target search is added. Ordinary context/packet/frame costs remain admitted C-OPT baseline costs, not speed or minimum-frame claims. |
| Maintained view consumers and CTest properties | Two current-consumer relinks and three probes reproduce four pipeline C AST/IR/RXAS views byte-for-byte under v6; no asset rewrite or walkthrough runtime replay. Default backend 54 cases/393 commands and representative C calls 145/others 68 retain serial locks and generous 900/1800s hang guards. The new 16-command runtime components (34–36s), shape component (42s) and metadata component support scheduling; no new full-aggregate measurement is claimed. |

No source EXPOSE/shared-pool inference, resumed-condition/TRACE, installed,
hosted, cross-platform, fresh sanitizer or whole-C qualification is claimed.
That LEAVE checkpoint retained named ITERATE's profile refusal; its separate
qualified increment is recorded above.
All twelve CFC criteria remain open; INTERPRET remains last.

### Completed B-SEED-01 prerequisite: preserve linked-output stores

**Locally qualified; separate seed correctness repair.** The minimal B procedure links an integer
reference, then assigns a scalar argument through it. Qualified native `rxc`
with default optimisation drops that observable store: output 0/status 1;
`rxc -n` retains `icopy` and gives output 1/status 0. Both use `rxas -n`, the
same library and `rxbvm --no-autoload` (eight commands, 1.093s). Actual RXAS
confirms the link remains but only the optimised store disappears. STRICTC's
linked policy output is the reproduced real consumer, not a C syntax defect.

Preserve observable stores through reference-linked aliases while retaining
valid ordinary-local optimisation. This is a seed correctness repair, with no
new API or performance initiative; keep it separate from LEAVE implementation.

| ID / status | Checkable closure |
|---|---|
| **B-SEED-AC01 — Locally qualified** | Permanent minimal scalar/reference store regressions and an ordinary-local control distinguish the failing optimisation from supported copy propagation; optimised/unoptimised results agree with independent expected values. |
| **B-SEED-AC02 — Locally qualified** | Actual RXAS retains the appropriate alias store and truthful TRACE/source observations, while the ordinary-local control retains its valid optimisation. Focused existing reference/flow regressions pass; no unrelated matrix or benchmark campaign. |
| **B-SEED-AC03 — Locally qualified** | Warning-free core product preparation and the original STRICTC consumer prove the qualified seed/affected-input boundary. Preserve passed evidence only where relevant inputs remain unchanged, update concise proof, clean diagnostics and create a separately reviewed local prerequisite commit. |

**B-SEED-STEP01 — Reviewed/complete (AC01–03):** source/RXAS isolates the
seed optimiser. Approved repair scope is only `compiler/rxcp_flow.c`, retaining
the existing aliased fact during collection; independent permanent fixtures
plus nine focused reference/flow controls and `nr26_flow_contract` own closure.
**B-SEED-STEP02 — Complete (AC01–02):** existing alias facts are
retained before flow transforms; permanent reference controls extend the
existing fixture. The minimal fixed opt/noopt probe now gives 1/status 0 in
both modes (eight commands, 1.539s), retaining linkref then icopy and correct
assignment TRACE. No broad optimisation fallback or source workaround.
**B-SEED-STEP03 — Locally qualified/complete (AC01–03):** ten selected native
reference/flow/NR26 controls pass in 8.948s; root and independent actual RXAS
review preserve linked int/bool/float stores, alias snapshot semantics, TRACE
and ordinary-local copy/dead-store optimisation. Required `stage-product`
passes warning-free (591 steps, 193.892s), following the focused rxc refresh.
The post-core audit retains 113/129 recorded images byte-identical, including
B/L/runtime foundations, common services and C grammar; changed native tools
and stamped generator images are recorded as new identities, with no forced
capsule admission or research grammar regeneration. Recompiled parser/focused
consumer passes `frontend-tests … leave-policy` 21/0 (seven commands, 18.545s).
The original 136-check LEAVE run had 132 passes and four failures; this focused
policy replay is composed proof, not a wholly green initial run. Qualified
inputs are `579a8377` plus the reviewed flow/reference patch and the existing
pending LEAVE consumer; no LEAVE feature is included in this prerequisite.
No ASan, broad matrix, API or performance claim is added.

Then CLC-STEP05.ITERATE1–3 implements named ITERATE using the same approved
foundation. Shared labels/EXPOSE, SIGNAL/INTERPRET loop invalidation, resumable
conditions/SIGL, complete expressions/exact recovery and clause/TRACE retain
their owners. All twelve CFC criteria stay open; INTERPRET remains last.

<a id="current-action-do"></a>

## Completed static DO increment

Complete LC-I-08's static DO forms and unnamed LEAVE/ITERATE integration within
the admitted source/expression profile. Preserve header evaluation, numeric and
live control-variable state, nested targets, original source and failures; reuse
existing B DO/value/pool/numeric/Unicode bodies rather than duplicating them.
The [approved exact contract](doc/architecture.md#approved-do-contract) owns the
AST, eight operations, seven exports and finite protocol; the
[instruction review](doc/LEVELC-INSTRUCTION-REVIEW.md#do-contract)
records its bounded language scope. The approved revision uses C v5 with
four existing types/51 operations and 32 target exports, refusing v4. Static text
keys scoped to each function select hidden owned state; no first-class DO state type,
SDK/ISA/native API or algorithm change is commissioned. **Exact-contract approval
was granted on 20 October 2026; the bounded local proof below passes.**

| ID / status | Approved checkable closure |
|---|---|
| **DO-AC01 — Locally qualified: source/AST** | Independent AST/span and syntax controls cover grouping, counted/controlled/FOREVER and standalone/combined WHILE/UNTIL, authored TO/BY/FOR order, scalar/compound controls, null clauses and admitted main/nested/private/IF/SELECT placements. END names compare authored symbols without tail substitution using shared Classic Unicode casing; missing/wrong/noncontrol names, malformed/duplicate/incompatible headers and dead source refuse with locations and prior-output preservation. |
| **DO-AC02 — Locally qualified: runtime/order/errors** | Independent markers/state/error expectations prove once-only count capture and controlled start capture before once-only modifiers in all six written orders, immediate modifier validation before control start/assignment (including FOR 0), count/TO entry before WHILE, UNTIL before advance after body/ITERATE, LEAVE skipping both, and the final count-limited advance. Cover zero/large/exponent counts, signed/zero BY, spelling/scale, live compound-tail/body mutation, nested/private ownership and unnamed transfers skipping groups/SELECT; retain shared 26.2/26.3, 41.4–41.6 and distinct 34.3/34.4 with visible C locations and bound raw helper-origin forwarding under the current fatal profile. |
| **DO-AC03 — Locally qualified: exact ABI/protocol admission** | Exact four-type v5/51-op and 32-export metadata controls admit only the architecture's descriptors/signatures. Bounded C-owned all-record validation checks key/name/part declarations and one constructor per function-local key, including dead records; reachable-only Collect/Ready/Body/FalseExit stack/phase/mask and merge/backedge proof rejects invalid order, non-top use, re-entry and unmatched cleanup. Fresh, loaded and post-transform graphs receive the same checks before publication, preserving input graphs/prior output on refusal. Generic LLIR retains record/type/terminator integrity; unreachable components receive no invented execution facts. |
| **DO-AC04 — Locally qualified: ownership/RXAS/qualification** | IR-off/on caller/helper inspection proves stable hidden strong owners and synchronous borrowed helper references, no escaping/copyable IR state, correct normal/LEAVE/RETURN/EXIT/fatal reverse-depth cleanup and retained outer state for state-free inner exits. State-free forms allocate no DO state; count-mode start emits no helper call; captures/configuration and joins contain no avoidable repetition. Focused existing Debug selectors/runtime/admission controls, representative paired B/IR/VM cells and independent RXAS review qualify the change with unchanged shared B/L/native proof reused, checked grammar reuse where eligible, current docs and task-only cleanup; no new aggregate, per-slice ASan or broad campaign. |

**CLC-STEP05.DO1 — Approved/complete (AC01–04):** vision, exact architecture,
independent expectations and dependencies reviewed; Adrian approved the recorded
contract on 20 October 2026.
**CLC-STEP05.DO2 — Complete (AC01–04):** approved
grammar/parser/frontend, v5 admission/selector, seven delegating target shims
and focused independent fixtures are implemented. Qualified tooling/shared
runtime and checked lexer reuse are retained; no algorithms are duplicated.
**CLC-STEP05.DO3 — Locally qualified/complete (AC01–04):** focused source,
order/error, ABI/protocol and ownership proof passes, including independent
actual caller/helper RXAS review. Owning docs and scheduling are current;
whole-instruction dependencies remain open.

All twelve CFC criteria remain open. Named LEAVE/ITERATE and invalid-target
delivery remain LC-I-09/10 work. Full expressions, shared labels/EXPOSE, exact
recovery, resumable conditions/SIGL and clause/TRACE retain their owners;
INTERPRET remains last. Named transfers are delivered separately under the
approved v6 contract and current action above.

<a id="current-do-proof"></a>

### Current DO proof

**DO-F01 — Repaired/qualified, DO-AC03 exact capture parts:** word-list
membership admitted padded ` repeat ` and controlled `to by` attributes in
forged IR; source lowering already emits canonical parts. Exact equality with
the selected token now rejects both. Against retained `a33f65b7` v5 tools,
`c-context-contracts … do-exact` reproduces 6 checks / 2 refusal failures
(4 commands, 5.158s); replacing only the repaired protocol passes 6/0
(4 commands, 2.334s), warning-free. Both fixtures remain generic-valid and
input-preserving. Default coverage retains the two controls; unchanged 313-check
admission/source/runtime proof below is reused, with no v6, native or grammar
build in this prerequisite.

Normal Debug qualifies `46593174` plus the reviewed frozen DO source/fixture
patch. Affected B C modules, hosts and fixtures were prepared warning-free
with existing native tools. Native/core product, generic L, shared B runtime
and Unicode inputs are unchanged; their prior stage-product and focused proof
are reused. No native refresh, source-slice ASan, installed/hosted/platform or
whole-C qualification is claimed.

| Command / inspection | Observed result and scope |
|---|---|
| Exact parser/tiny-lexer preflight; checked warm grammar recipe; `rxas` | Pass in 8.884 / 10.894 / 3.776s, no diagnostics. Actual grammar: 29,709 NFA / 5,609 DFA / 2,683 LR, 7,774,129 RXAS bytes. Generator `4e1b2a23…700a5`, capsule/manifest bytes and mtimes unchanged; no cold lexical construction. |
| `frontend-tests-linked -a do`; focused `do-literals` | Reuse 217/0 source/AST/span/header/END/dead-error checks; final closed-body malformed UNTIL assertion passes 5/0 after four fixture preparation/execution commands (6.918s). Default grammar/DO coverage retains it. |
| `backend-tests-linked -a … do`; focused `do-final` | Opt reuses 130 passing original assertions plus corrected 26/0 focused replay (29.348s); noopt/rxtvm passes 146/0 (63.651s), internally both IR modes. Ten endpoint cases and once-only marker CFG assertions include closed UNTIL, nested RETURN/EXIT and SELECT/loop transfers. This is composed opt proof, not one wholly green initial run. |
| `target-context-linked -a do`; `check-target-context.py` | 16/0 in each opt/rxbvm and noopt/rxtvm cell; actual helper shape 234/0 and 237/0. Seven exact borrowed-state shims, repeated mutation, shared errors/message/source and bound-event forwarding pass. |
| `c-context-contracts-linked -a … do`; DROP maintenance | 313/0 focused protocol checks: exact v5/51, fresh/loaded/selected graphs, dead declarations versus reachable phases, keys/merges/backedges/LIFO, bounded refusal and immutable/atomic output. Opt DROP maintenance passes 47/0; unchanged B algorithms are reused. |
| `check-backend.py` selected ten `do-…` cases, `--ir on/off` | All ten independent runtime cases qualify across paired opt/rxbvm and noopt/rxtvm, both IR modes. Opt/on is composed proof after the exact ARG-uppercase oracle correction and a three-command assembly/link/execution replay; passed unchanged cases are reused. Other cells each pass 40 commands in 100.106 / 100.749 / 100.154s. Assembler `-n` isolates compiler IR. |
| `check-step07.py --c-only --do-metadata`; CLI refusal sentinels | Metadata 16/0 commands in 30.579s: valid catalogue plus seven one-field new-signature mutations preserve prior output. Three CLI sentinels preserve caller output/no staging residue: duplicate modifier and malformed dead UNTIL in both IR modes. Six maintenance commands plus sentinels pass in 20.283s. |
| Complete readable caller/helper RXAS | Root and independent review clear count/control/once/nested/SELECT/reentry and 26.2/41.6/34.4 fatal paths in sampled IR modes. Owners initialize before fallible setup; synchronous aliases clear before reverse-depth owners; RETURN packet precedes cleanup; fatal preserves result/diagnostic origin. State-free forms allocate no owner, count start calls no helper, and closed paths add no spare join/advance. Ordinary context/packet/frame work remains an admitted C-OPT baseline, not a speed claim. |
| Existing pipeline view consumers; CTest registration | Four commands (9.019s) refresh only step03 pipeline C AST/IR and step04 pipeline C IR/RXAS: CDo/DoWhile and contextual while.truth with truthful spans. No new walkthrough assembly/link/runtime proof; unchanged G and other C views are reused. Backend 50 cases/365 commands and representative C calls 136/others 68 retain serial locks and generous 900/1800s hang guards. Isolated new 40-command runtime component 100.106s plus prior 295-command aggregate 89s support scheduling; no new full-aggregate timing is claimed. |

The exact LEAVE/ITERATE v6 contract is approved for the next increment after
this DO local commit. Its named-transfer delivery does not change the frozen
v5 proof or close source EXPOSE/shared labels, full expressions, exact recovery,
resumable conditions/SIGL or clause/TRACE. All twelve CFC criteria remain open.

## Completed prerequisite: generic L lexical construction reuse

The [generic Level L owner](../PLAN.md#current-action-lexer-reuse)
records locally qualified LRUSE-AC01–06 / STEP01–04 and the approved generation
contract. L-CONSTRUCT-01 is this dependency, not a C-specific cache/API; SELECT
has not implemented it. All CFC rows and INTERPRET-last sequencing remain.

<a id="current-action-select"></a>

## Completed SELECT increment

Implement LC-I-07's approved ordered SELECT/WHEN/OTHERWISE subset in the B/L C
compiler, using the [approved exact contract](doc/architecture.md#approved-select-contract).
Users receive lazy first-match selection, ordinary admitted arms, contextual
34.2 errors and visible source-indexed 7.3 when no arm matches. Preserve source,
borrowed ownership, loop targets and failed-output atomicity. The approved
C-owned change adds CSelect/CWhen/COtherwise, advances C to v4/43 operations and
llctarget to 25 exports; existing 41 operations/four types/context ABI, generic L
format/core and B/L/VM APIs stay unchanged. Version 3 refuses; no compatibility
adapter or duplicate shared-runtime logical/selection body is commissioned.
Reuse shared B truth/select-missing bodies. Native LC-AC-68 is a read-only reference, never an
oracle or fallback; INTERPRET remains last.

| ID / status | Checkable closure |
|---|---|
| **SELECT-AC01 — Locally qualified** | Independent CSelect ordered CWhen/COtherwise/CLabel AST and original spans cover at least one WHEN, one optional final OTHERWISE (empty/list/inline), nested/nearest ELSE, keyword variables, main/group/WHILE/private and admitted arms. Missing/stray/duplicate/out-of-order clauses, missing expressions/arms/END, named END and malformed dead source refuse with locations/prior-output preservation. Retained labels give the existing shared-label profile refusal. |
| **SELECT-AC02 — Locally qualified** | Marker/state/error expectations prove one evaluation/truth per reached WHEN, first exact `1` wins, later tests/arms dormant, exact `0` continues, full OTHERWISE list/empty branch, loop LEAVE/ITERATE targets and private-pool boundaries. Invalid WHEN gives 34.2 before body/later output. No match gives visible 7.3 at SELECT. Direct shims preserve existing shared-helper raw origin/message via ERROR→CLASSIC_SYNTAX signalorigin; only the generated fatal profile claims nonresumption. |
| **SELECT-AC03 — Locally qualified** | Exact current v4/43-op family and 25-export metadata admit the two approved descriptors/signatures; v3/forged/dead schema/import records refuse atomically. Every fresh/loaded/dead select.missing has an owned exact source-backed nonempty valid indexed span/location; absent/synthetic/recovered/derived/composite/invalid origins refuse without output/graph mutation. No serialized line attribute or generic L/VM API is added. |
| **SELECT-AC04 — Locally qualified** | Both IR modes and actual caller/helper RXAS show ordered contextual WHEN checks, required source/owners/cleanup and joins only for open endpoints, with no SELECT loop frame or spare default/join. The missing path retains only approved cold return.absent/core.return closure. Existing focused Debug selectors/runtime/atomicity controls and resource limits qualify paired representative B/IR/VM cells; scheduling is measured as needed. Reuse unchanged native/B/L proof, one necessary frozen-grammar refresh, no new aggregate/ASan/broad campaign. |

**CLC-STEP05.SELECT1 — Complete (AC01–04):** approved vision, exact contract,
criteria and independent expectations recorded before product edits.
**CLC-STEP05.SELECT2 — Complete (AC01–04):** ordered AST/parser/lowering, v4
admission and two shared-runtime shims implemented; source and independent
fixtures reviewed on disjoint ownership.
**CLC-STEP05.SELECT3 — Complete (AC01–04):** the [focused Debug proof](#current-select-proof)
and independent actual caller/helper RXAS review pass. Owning docs/scheduling
are current, unrelated work is preserved and the bounded local increment is
reviewed complete; whole SELECT and CFC closure remain open.

Whole SELECT closure also requires shared labels/fall-through/EXPOSE, complete
expressions/full DO/named transfers, exact diagnostics/recovery and condition,
SIGL, clause/TRACE observations under STEP03–07. Current fatal-profile behavior
does not implement resumable conditions. No CFC criterion closes in this slice.


## Current SELECT proof

Normal Debug qualifies `23aebe427adbb9bf4c277cdd5d327645d5712fd6` plus the
reviewed SELECT source/fixture changes. Grammar SHA256 is
`b7aba43f36e86f5d071a96a7911582c919be43d70d699d69af06bf741f2e9691`.
Affected B modules/hosts/fixtures were compiled, assembled and linked through
existing direct recipes; final fixture refresh is warning-free. Existing native
tools, generic L and B runtime products are unchanged and reused.

| Command / inspection | Observed result and scope |
|---|---|
| `rxlang --rxas` exact parser/AST with temporary tiny lexer; existing full C grammar recipe and `rxas` | Cheap LR admission passed in 8.239s (3 NFA / 2 DFA / 2,452 LR). The one required full-source generation/assembly passed in 653.136s: 29,709 NFA / 5,609 DFA / 2,452 LR, 7,698,798 RXAS bytes, no diagnostics. Cold construction cost only; generic reuse is qualified separately under L-CONSTRUCT-01. |
| `frontend-tests-linked -a select <runtime> <examples>` | 190/0 checks, opt/rxbvm, 49.628s: AST/order/spans, Unicode/CRLF, optional arms/nearest ELSE, syntax/dead/profile refusals. |
| `backend-tests-linked -a <runtime> <capture> select` | 90/0 twice, opt/rxbvm and noopt/rxtvm, internally both IR modes; 37.845/38.644s. Open/closed/nested/loop/missing endpoints, once-only truth, literal pruning, anonymous source and actual shape. |
| `target-context-linked -a select`; `check-target-context.py` | 6/0 in each paired B/VM cell; shape 181/0 opt and 184/0 noopt. Shared 34.2/7.3 messages/source, valid raw event IDs and bound-event→signalorigin forwarding. Distinct inline sites need not share physical PCs. |
| `c-context-contracts-linked -a <runtime>`; opt `drop-contracts-linked` | 105/0 admission checks in 4.512s; DROP maintenance 47/0. Exact v4/43 descriptors, versions 3/5/unused forgeries, fresh/loaded/dead origin refusal and `<source>` positive; prior generic invalid-origin controls reused. No full DROP rerun. |
| `check-backend.py --case select-c --case select-when-error-c --case select-missing-c --ir on/off` | 48 tool commands pass across opt/rxbvm and noopt/rxtvm, both IR modes. Four isolated components 13.801/14.166/19.977/15.131s. Independent lazy/order/OTHERWISE/loop/private output; visible 34.2 at WHEN line 4 and 7.3 at SELECT line 3. Assembler `-n` isolates compiler IR. |
| `check-step07.py --c-only --select-metadata` | 12/0 commands, 13.011s: valid authored catalogue plus five one-field new-export mutations, each preserving prior output. Default coverage retains these controls. |
| CLI SELECT refusals; `check-backend.py --native-controls --case select-c` | Three prior-output sentinels pass (6.517s): missing WHEN once and malformed dead arm in both IR modes, no staging residue. One independent native control passes 4 commands; native output is not the oracle. |
| Complete readable caller/helper RXAS, both IR modes | Root and independent review clear. Ordered call/requirevalue/WHEN truth, no spare closed join/default, enclosing WHILE targets, source-indexed missing line, fatal diversion, numeric setup, owner/ref cleanup and source mappings remain. Required cold missing absent/return closure and ordinary context/window costs remain explicit. |
| Existing PoC C view consumer; CTest show-only | One relink + one view invocation passes 13 checks; all five maintained walk-c AST/IR/RXAS views are byte-identical, so no asset rewrite or repeated runtime walkthrough. Registration inspected without executing aggregates: backend 40 cases/295 commands, representative C-calls 121/others 68; existing serial locks and generous 900/1800s hang guards retained. Component timings are not a newly measured full aggregate. |

No fresh sanitizer/native rebuild, installed/hosted/cross-platform or whole-C
qualification is claimed. Existing unchanged B/L/runtime proof is reused; no
new generic L, VM or runtime algorithm is introduced. Current fatal delivery
is nonresumable. All twelve CFC criteria and the whole-instruction dependencies
above remain open; the next instruction review is separate.

<a id="current-action-if"></a>

## Completed IF increment

Local commits `e6601b13842c0d3ce124398f038be2f271401ac1` (optional-arm parser)
and `23aebe427adbb9bf4c277cdd5d327645d5712fd6` (closed-arm joins) qualify the
[bounded IF contract](doc/LEVELC-INSTRUCTION-REVIEW.md#if-one-condition-and-only-the-selected-arm)
with [normal Debug proof](#current-if-proof). IF-F01/F02 are closed; whole IF
retains SELECT/full DO/shared labels/EXPOSE, full expressions/exact static
errors, resumed conditions/NOVALUE and clause/TRACE dependencies.

| Stable ID / status | Qualified closure |
|---|---|
| **IF-AC01 — Qualified** | Independent AST/spans/nearest ELSE, supported placements/optional-default arm minima and located/dead-source/prior-output refusal. |
| **IF-AC02 — Qualified** | Once condition/truth, lazy selected/no-ELSE paths, exact `0`/`1` and fatal 34.1 prefix; private calls retain fresh pools. |
| **IF-AC03 — Qualified** | Truth survives literal pruning; closed endpoints add no spare join/epilogue, and dead validation/source/owner/fatal work remains. |
| **IF-AC04 — Qualified** | Existing paired B/IR/VM Debug selectors, actual RXAS, sentinels and measured scheduling; unchanged grammar/native/runtime/L reused. |
| **CLC-STEP05.IF1–3 — Complete** | Approved plan, two reproduced reviewed repairs, independent fixtures/local qualification and scoped local commits completed before SELECT1. |

<a id="current-action-options"></a>

## Completed OPTIONS increment

Local commit `690e930d772c9814b5c10db9284a9572afe3ea3f` qualified the
[approved COptions/current-v3 specialization](doc/architecture.md#approved-options-contract)
with independent source/RXAS and [normal Debug proof](#current-options-proof).

| Stable ID / status | Qualified closure |
|---|---|
| **OPTIONS-AC01 — Qualified** | Raw optional expressions/header words/spans, complete first-clause static admission and fresh request-local comment rules; quoted/operator/call/later forms stay dynamic. |
| **OPTIONS-AC02 — Qualified** | Once/reached operands, unknown/NUL/DBCS no-word behavior, effectful Name/arithmetic/call/required-result paths and discarded-literal/dead validation. |
| **OPTIONS-AC03 — Qualified** | Reached/post-terminator blank pairs add no OPTIONS processor/value/helper/default/frame work; operand/source/owner/fatal paths remain. |
| **OPTIONS-AC04 — Qualified** | Focused paired B/IR/VM Debug, prior-output and independent RXAS review; one grammar refresh and existing source-only scheduling, with unchanged native/B/L proof reused. |
| **CLC-STEP05.OPTIONS1–3 — Complete** | Approved contract, implementation and independent local qualification completed before IF1. |

Whole LC-I-05 retains full header controls/source maps, shared labels,
exact diagnostics and conditions/TRACE. Adding a runtime word/effect to B
`applyOptions` requires C review, qualification and recompilation.

<a id="current-action-nop"></a>

## Completed NOP increment

Local commit `ca679a39d55660981e3805ccc94c2d973ad788f6` qualified the
[childless/non-emitting path](doc/LEVELC-INSTRUCTION-REVIEW.md#nop-childless-statement-without-runtime-work)
and repaired NOP-F01 with independent source/RXAS and [normal Debug proof](#current-nop-proof).
Whole LC-I-04 retains SELECT/shared-label placements, exact 21.1 and
clause/TRACE/condition dependencies; no full-instruction closure is claimed.

| Stable ID / status | Qualified closure |
|---|---|
| **NOP-AC01 — Qualified** | Childless AST/spans, case/contextual variable behavior and ordered main/IF/group/while/private execution. |
| **NOP-AC02 — Qualified** | Variable/quoted/numeric/bracket/keyword tails, inline/dead source and first-extra-token refusal with located errors/prior-output preservation. |
| **NOP-AC03 — Qualified** | Valid reached/dead blank-clause pairs add no NOP operation/helper/default/dead-return/frame work; surrounding owners/source remain truthful. |
| **NOP-AC04 — Qualified** | Existing narrow frontend/backend selectors, paired B/IR/VM Debug and independent source/RXAS review, with unchanged grammar/tools/runtime reused. |
| **CLC-STEP05.NOP1–3 — Complete** | Contract review, focused repair/fixtures and local qualification completed before OPTIONS1. |

<a id="current-action-assignment"></a>

## Completed assignment increment

Local commit `b42cf6fc7da7e487a45a646aa4e0ea7bfda74857` qualified the
[existing one-write contract](doc/LEVELC-INSTRUCTION-REVIEW.md#assignment-one-write-after-rhs-evaluation)
without product changes, with independent source/RXAS and [Debug proof](#current-assignment-proof).
Source-call ordering and supplied-context mutation/storage remain complementary;
private source calls have fresh pools. Whole LC-I-03 retains EXPOSE/shared-label,
exact diagnostic, resumed condition/NOVALUE and clause/TRACE dependencies.

| Stable ID / status | Qualified closure |
|---|---|
| **ASSIGN-AC01 — Qualified** | Scalar/keyword/empty-RHS/stem/arbitrary compound forms, numeric/unset/empty tails, substituted case, main/IF/group/private execution, Unicode/NUL/high ordinals and explicit/dropped-tail reset/reassignment. |
| **ASSIGN-AC02 — Qualified** | Once-only source RHS call/read before the sole raw-name write, no tail pre-read; complementary supplied-context mutation then resolution, without claiming source EXPOSE. |
| **ASSIGN-AC03 — Qualified** | Live scalar/stem/exact/nested aliases, private reset isolation, unrelated case-distinct tails and exact text/cache preservation without conversion. |
| **ASSIGN-AC04 — Qualified** | Explicit/empty/dead-source refusals preserve prior output; fatal RHS cannot reach write/later output; one-write/no-default/splitter/conversion RXAS with owners/source and focused mode/VM proof. |
| **CLC-STEP05.ASSIGN1–3 — Complete** | Contract review, independent fixtures/structural guards and local Debug/RXAS qualification completed before NOP1. |

## Completed DROP increment

Local commit `fccb95b46dc4cfd75a453c0573fba27a78d5ff3b` qualified the
[approved C v3 DROP/name contract](doc/architecture.md#proposed-next-instruction-drop)
with independent source/RXAS review and [normal Debug proof](#current-drop-proof).
No remote publication or whole LC-I-02 closure is claimed; source EXPOSE,
conditions/full diagnostics and TRACE keep their later owners.

| Stable ID / status | Qualified closure |
|---|---|
| **DROP-AC01 — Qualified** | Raw Unicode symbols, ordered CDrop spans/items and exact v3/name admission; dead-source, reserved-dot-prefix and routine-profile refusals. |
| **DROP-AC02 — Qualified** | Direct/indirect order, one captured list, scalar/stem/compound states, defaults/tombstones and self-carrier cases. |
| **DROP-AC03 — Qualified** | Unicode/configured blanks, micro-sign/Greek identity, ARG names, aliases/private pools and invalid-word continuation. |
| **DROP-AC04 — Qualified** | Existing pool calls and caller/helper owners/source in actual RXAS; focused compiler/IR/assembler/VM Debug proof. |
| **CLC-STEP05.DROP1–3 — Complete** | Shared symbols/AST, ordered lowering and independent local qualification completed before ASSIGN1. |

<a id="active-first-implementation-increment"></a>
<a id="shared-b-unicode-interface-and-c-adapter-migration"></a>
<a id="active-c-contract-baseline-increment"></a>
<a id="active-borrowed-classic-context-increment"></a>

## Completed foundations and qualified C ABI

Bounded independent C expressions/control/private calls and LENGTH/ARG/DIGITS,
generic L services and shared B Unicode are foundations, not whole-instruction
closure. **CTX-AC01–06 / CTX-STEP01–04 are locally complete**, including borrowed
B owners and two reproduced B seed repairs; baseline commits are
`ff63527ae94b1d91acb7ba01025f2e197cf522bc` and
`eeca738af1c2f3b6dc826f087d398f6400b941ba`.

STEP03d now has the [reviewed clean C v2/context ABI](doc/architecture.md#approved-c-family-revision-and-generated-procedure-abi)
and normal Debug proof below. Adrian removed research-C backward compatibility:
one current C family/target, no old-image bridge or copied-context adapters.
Shared B/L contracts remain intact. Local commit
`62d6dd44df528d56f4ca191bdc28d73b960c9432` closed this slice before DROP began.

| Stable ID / status | Checkable closure and owning contract |
|---|---|
| **CABI-AC01 — Qualified** | Exact v2/40-op schema, required context reference, save/load, canonical payloads and entry-bound context slots; malformed/dead records refuse with immutable graph/empty output ([C admission](tests/c-context-contracts.crexx)). |
| **CABI-AC02 — Qualified** | One root config, stable borrowed root/child owners, live pool, call inheritance/presence/results; direct `target-context` plus STEP07 argv/ARG/call/recursion controls. |
| **CABI-AC03 — Qualified** | Same-frame Classic DIGITS/FUZZ/FORM install/restore; invalid NUMERIC has fatal diversion without installs, composed with direct adapter atomicity and numeric/fatal RXAS checks. |
| **CABI-AC04 — Qualified** | One 23-export `llctarget`; obsolete namespace/bodies/compatibility tests removed, BIF bodies/shared contracts/doc tags preserved ([target shape](tests/check-target-context.py)). |
| **CABI-AC05 — Qualified** | Exact runtime/class/receiver/optional signature admission, owner-hit/fresh-request metadata refusal and sentinel preservation; no native compiler fallback or new public L/VM API ([C matrix](tests/check-step07.py)). |
| **CABI-AC06 — Qualified** | Caller/helper source/owner/ref/numeric/RXAS guards, both compiler/IR modes and VMs, restricted C-only roots and measured serial normal Debug scheduling. Sanitizer boundary is stated once below. |
| **CLC-STEP03d.1–3 — Complete** | Approved contract freeze, implementation and independent fixtures/build/Debug measurement; architecture and maintained regression sources own the exact checks. |
| **CLC-STEP03d.4 — Complete** | Independent source/RXAS review, final normal Debug proof and coordinator-reviewed local commit completed before DROP started. |
| **META-AC01 — Complete** | Real linked constructor and independent merged definition/prototype fixture select only definition-linked metadata ([services](../../../langtools/tests/services.crexx)). |
| **META-AC02 — Complete** | Duplicate definitions remain ambiguous; private/missing/malformed links/envelopes refuse. Selected references stay in their pool/1,000,000-ID bound; ignored prototype signature fields stay opaque. |
| **META-AC03 / CLC-STEP03d.P — Complete** | Reader API/format/type ownership unchanged; separate reviewed six-path prerequisite commit `9c632093f6ab97e819ad9e48d3a6b379d88afba0`, independent services and real C consumer pass. |

<a id="remaining-actions"></a>

## Remaining delivery steps

The steps below describe remaining work, not a requirement to finish a universal
framework before its first consumer. Deliver the minimum shared service needed
by each reviewed instruction; complete its wider contract before closing that
step. Numeric, error and source foundations are needed early even though full
NUMERIC, SIGNAL and TRACE instruction reviews occur later.

| Step / current status | Remaining action and closure check | Dependencies / approval gate |
|---|---|---|
| **CLC-STEP01 — Partial** | Finish contract-to-test traceability, exact ABI/dependency/platform decisions and independent expectations. Every required ledger row has an explicit disposition and closure check. Serves CFC-AC01/06–08/12. | Accepted C direction; exact language/platform exceptions need Adrian's decision. |
| **CLC-STEP02 — Partial** | Supply registered exceptional/resumable edges, observations, facts/invalidation, liveness and checked rewrites as consumers require. Independent non-C/G tests prove correctness, limits and atomic failure. Serves AC07/10/11. | STEP01 contracts; approve new public L APIs/schemas. Keep block-local values/cross-block slots; no required SSA rewrite. |
| **CLC-STEP03 — Partial** | Complete remaining live activation/pool/context, direct operations/BIF ABI and one-body labels, recursion, EXPOSE, restoration and abrupt paths. Close remaining property/simple-map/traversal/budget contracts only where needed. Serves AC02/03/09/11/12. | Existing Unicode/CTX pieces are complete. Each later extension needs the relevant STEP01/02 contract and exact ABI approval; direct B and new-C proof are distinct. |
| **CLC-STEP04 — Partial** | Complete contextual source/Unicode names, AST/source maps, static errors even in unreachable text, all expressions/numeric semantics and BF01–05 bindings. Independent values, effects, diagnostics and RXAS checks close C-SRC/C-EXPR. Serves AC01–03/06/07/09/11. | Required L/runtime services from STEP02/03; no native frontend dependency. |
| **CLC-STEP05 — Partial; bounded reviews through ITERATE qualified** | Review LC-I-01–10 individually: SAY through ITERATE. Close every form, error, order, pool/loop effect and source observation in the ledger. Serves AC01–03/06/07. | STEP04 plus each instruction's shared dependencies. Prior bounded reviews, static DO and named LEAVE/ITERATE are locally qualified; their whole-instruction dependencies remain open. |
| **CLC-STEP06 — Partial; current PARSE engine approval** | Review LC-I-11–19 individually: ARG through PARSE. Complete one PARSE executor, shared labels/fall-through, presence/results, EXPOSE and static signed providers; integrate BF05/06/08 as needed. Serves AC01–04/06/09/11. | Needed STEP03–05 services. [PARSE1–3](#current-action-parse) separates completed design review, the unapproved generic engine prerequisite and later exact C integration; ARG uses the same engine. |
| **CLC-STEP07 — Partial** | Review LC-I-20–24 individually: ADDRESS, commands, NUMERIC, SIGNAL, TRACE. Complete conditions, clause clocks, source/error/TRACE observations and BF07/09. Serves AC01–04/06/09. | Required STEP06 control/activation contracts and host adapters; early foundations do not close these instructions. |
| **CLC-STEP08 — Not started** | Complete remaining BIF/host contracts: exact-length invocation/completion, callback-window pools, queues/streams, clocks/HALT and declared platform adapters. Qualify all static reference capabilities. Serves AC01–04/09/11/12. | Deliver needed adapters alongside earlier consumers; full closure follows STEP04–07. New host APIs and capability exclusions need approval. Reuse existing B bodies. |
| **CLC-STEP09 — Open throughout** | Inspect caller/helper RXAS every increment and fix direct construction/emission defects immediately. Close C-OPT-01–06, register/call-window/allocation obligations and named remaining costs. Serves AC06/07/10/11. | Uses STEP02 facts and explicit ABI/lifetime rules; final closure follows STEP08. New ISA/storage contracts need approval; production performance edits retain their verdict gate. |
| **CLC-STEP10 — Not started** | Close all static ledger rows, source/runtime/host tests, installed/cold/incremental/relocated delivery, declared platforms and independent walkthrough/review. Serves static AC01–04/06–12. | STEP01–09. Static acceptance is not complete C. |
| **CLC-STEP11 — Not started; last** | Implement INTERPRET through the same compiler, then close all CFC criteria and complete installed/platform proof. | STEP10 and exact continuation/code-owner mechanism approval. |

**CLC-STEP11a–d remain separate checkable parts:** 11a proves nested execution,
SYNTAX, CALL resumption and SIGNAL with live continuations and cleanup/limits;
11b implements approved ephemeral code ownership, pins/reclamation, stale handles,
all-pinned failure, rollback and both VMs; 11c connects group compilation,
assembly/loading, live pools/arguments/config, caller dispatch and spills;
11d closes LC-87-01–05, PROCEDURE/outer-loop rules, RETURN/EXIT/SIGNAL,
delayed handlers, source/TRACE, resource limits and installed/platform acceptance.
A file compiler or bounded lookup cache alone does not meet this contract.

## C-first delivery milestone

All **CFC-AC01–12 remain open for complete C**. An item closes only on its full
observable contract; bounded results below do not close whole instructions.

| ID | Observable pass condition and proof |
|---|---|
| **CFC-AC01** | Complete crosswalk of every requirements-ledger row: LC-I-01–25, all 95 BIF names, LC-REF-001–075, 36 syntax contracts, detailed native criteria/gap routing, extensions and approved exceptions. Each links new implementation, independent positive/error expectations and qualification; no omitted or inherited native passes. |
| **CFC-AC02** | Expressions, numeric/string/Unicode values, symbols/stems/pools/EXPOSE, PARSE/ARG, control and calls implement the agreed contract. Test state, order, errors and limits, including defects exposed by native comparisons; each instruction has its own disposition. |
| **CFC-AC03** | Activation/labels, SIGNAL/CALL conditions, RETURN/EXIT, clause clocks, source/TRACE and diagnostic origins survive normal/abrupt/nested paths, Unicode/CRLF/NUL and optimisation. Prior output survives failed compilation. Source identities are logical and stable. |
| **CFC-AC04** | Host invocation/completion, callback-window pool access, ADDRESS/commands, streams/queues, clocks/HALT and signed external calls have positive/error/ownership proof on their declared platforms. Configuration and missing host facilities stay open unless explicitly excluded. |
| **CFC-AC05** | INTERPRET satisfies LC-87-01–05 using the same B/L compiler, live caller state, real resumption/transfers, nested groups and bounded resident-code reclamation. Tests include trapped failures, all-pinned exhaustion, stale handles and teardown; no native/RexxScript fallback. |
| **CFC-AC06** | Representative full RXAS and helper paths have checked structural expectations for simple expressions, loops, calls, errors and dynamic execution. Compiler/RXAS modes are distinct. Direct defects are fixed; necessary cost and later opportunities have owners. Preserve ABI, source, TRACE and numeric observations; static counts are not speed claims. |
| **CFC-AC07** | Generic L services have independent non-C/G consumers, checked structure/effects/facts, bounded failure and documented algorithms. C/BIF/ISA policy stays in owning extensions; source/import/install audits prove no native or typed frontend in the C host closure. |
| **CFC-AC08** | Every required C row closes on qualified inputs, with CLI/service/installed/platform proof, documentation and runnable walkthroughs. Distinguish local, hosted, guest/SDK, sanitizer and release results. Complete C is separate from default replacement and B/G delivery. |
| **CFC-AC09** | B Classic runtime has one implementation of values, pool aliases, activation/context, BIFs and host adapters. C target closure uses the shared B Unicode foundation independently of G aggregation; native C, typed Classic mode and RexxScript retain their contracts. Public metadata/doc tags and consumer regressions prove compatibility. |
| **CFC-AC10** | Any RXAS/VM change has an approved operation/storage contract, exact error/alias/lifetime/source semantics, encoding/disassembly/loader tests and both-engine/platform proof. Alternatives are compared; production performance edits pass the first Release verdict gate. No native source-language implementation is introduced. |
| **CFC-AC11** | Compile/request/result and runtime activation/code lifetimes are isolated, reusable and bounded. Verify exact lengths, atomic failure, stale/foreign references, interruption, repeated/concurrent contexts, declared budgets and recovery without partially published output. |
| **CFC-AC12** | Delivery/build manifests separate native seed, L tools/SDK, C host, C runtime and optional dynamic-compile support. Cold/incremental/installed/relocated builds have one producer and correct freshness; static applications have no compiler dependency. Platform capability rows and source/user/API documentation match actual products. |


New B/G features, proper B self-hosting (LLCOMP-STEP07B) and combined replacement are outside this
delivery, with their unverified outcomes retained in the requirements ledger.
Existing consumers remain regressions. Default switching, native-C retirement,
release approval and resumed CI monitoring require separate direction. Native
Classic source is read-only; demonstrated B seed defects may be repaired with
focused regressions. No native Classic feature implementation is authorised.

<a id="step07a-validation"></a>
<a id="say-validation"></a>

<a id="current-if-proof"></a>

## Completed IF proof

Local commits `e6601b13842c0d3ce124398f038be2f271401ac1` and
`23aebe427adbb9bf4c277cdd5d327645d5712fd6` record the qualified source below.

Inputs are `690e930d772c9814b5c10db9284a9572afe3ea3f` plus frozen IF-parser-1,
IF-frontend-1 and the focused fixtures. The corrected frontend fixture SHA-256
is `60eecef856bf96ef2ac03f7449f7948eae4536a9ffa6339284e652d04bb61d67`.
Qualified grammar/native/runtime/L inputs are unchanged; direct existing B
recipes refreshed only affected modules/hosts/fixtures (44 commands, 20.9072s;
six fixture-only commands, 5.1679s), with no compiler warnings.

**IF-F01 closed:** the pre-edit two-command source/IR/RXAS probe found two
predecessorless joins with extra absent helpers/returns. Lazy first-open joins
remove them directly: source absent operations 4→2; IR-off caller bodies each
lose four instructions, one helper call and one local (root 30→29, private
32→31). Both arm visits, dead validation, contextual truth, origins and
owner/numeric/fatal setup remain. Per-arm EXIT fallback packets are separate
admitted costs; these counts are not a speed claim.
**IF-F02 closed:** bare RETURN swallowed its ELSE into an operand. Actual-owner,
condition/completion admission now covers existing optional/default minima,
while preserving ELSE condition/operand/CALL-target names and incomplete-head
refusal. Grammar and arm instruction contracts are unchanged.

| Focused normal Debug command/component | Observed result |
|---|---|
| Existing `frontend-tests ... -a if` and `if-errors` selectors | Reuse 127 unchanged positive checks from the first 173-check command (43.517s); five empty-substring matcher errors were fixture-only. Corrected negative-only replay passes 46/0 in 12.4414s. No wholly green 173-check run is claimed. |
| Existing `backend-tests ... -a runtime prefix if` | 74/0 each in opt/rxbvm (31.6382s) and noopt/rxtvm (31.8275s), both IR modes internally; closed/open/nested/WHILE endpoints, once/truth and literal pruning. |
| `check-backend.py --case if-c --case if-error-c --ir on/off` | 32/0 tool commands across paired opt/rxbvm and noopt/rxtvm cells, 38.8549s isolated total; independent ordered/lazy/bare-arm and 34.1 before/condition-only output. |
| Existing `--case if-c --native-controls`; CLI prior-output controls | Four read-only native commands agree with the independent expectation; three CLI refusals pass in 3.0485s, including dead malformed source in both IR modes. |
| Independent full caller/fatal RXAS and registration inspection | Before/after, endpoint, once, lazy/bare-arm and invalid-truth paths clear. Four grammar lanes use serial/backend lock and 900s; four backend lanes retain serial/lock/1800s with 37 cases/274 commands. No full aggregate rerun. |

Maintained C view inputs have only unaffected open, nonnested arms and no
corrected bare-arm boundary; no view regeneration was needed. This is local
source-only qualification, not a native/core refresh, sanitizer, hosted,
installed/platform or whole-IF claim. The approved SELECT increment is separate from this IF qualification.

<a id="current-options-proof"></a>

## Completed OPTIONS proof

Baseline `ca679a39d55660981e3805ccc94c2d973ad788f6` plus the frozen three-file
OPTIONS patch passes the focused normal Debug packet below. Only C grammar
and affected B modules/linked hosts/fixtures were refreshed through existing
recipes; final preparation is warning-free. Qualified native tools, shared
B runtime, generic L comment/budget and core-product proof are reused on
unchanged inputs. No native/core refresh, ASan, full aggregate, installed or
hosted qualification is claimed.

| Command / inspection | Observed result and scope |
|---|---|
| Existing C grammar `generate.cmake` recipe, `dfa.states=16384`, then `rxas` | One required generation/assembly: 643.937s, 29,709 NFA / 5,609 DFA / 1,780 LR states, 7,555,017 RXAS bytes, no diagnostics. This is cold construction cost, not a speed claim; L-CONSTRUCT-01 retains generic ownership. |
| Existing affected B `rxc`/`rxas`/`rxlink` recipes | Both modes pass 54 commands in 24.7256s with zero warnings; the final pair-construction-only fixture refresh uses six commands. `llcomments`/`llsource` remain in the existing `rxfnsl` closure. |
| `frontend-tests-linked.rxbin -a options <imports> <examples>` | 153 checks/0 in 19.7506s, opt/rxbvm: raw COptions AST/spans; static versus dynamic complete-clause admission; real retained Name/operator IR; request-local comments, shielding and Unicode/CRLF; bare IF ownership; malformed/dead/UTF8 and early source-limit refusal. |
| `backend-tests-linked.rxbin -a <runtime> <inspection-prefix> options` | 55 checks/0 each: opt/rxbvm 38.9912s, noopt/rxtvm 43.0447s. Five identical-header/byte-width blank pairs in both IR modes preserve IR operations/values/blocks/slots and executable/frame shape; reached/postEXIT/privateRETURN bare/simple literals add no work. Name/unary/binary/call/required-result operations remain. |
| `check-backend.py --case options-c --case options-error-c --case options-result-error-c --ir on/off` in those paired B/VM cells | Four cells, three cases/12 commands each, 48 commands in 46.7497s total. Once/reached/private/inline-arm marker order passes; 42.3 and 45.1 preserve only the expected before-prefix. Assembler `-n` isolates compiler output. |
| Focused CLI prior-output controls | Three commands pass in 1.7505s: conflicting static options once and a dead malformed literal in both IR modes preserve the sentinel destination and leave no stage artifact. |
| Independent actual RXAS review | Both B-mode pair captures agree. Owning fatal handlers/source/child cleanup and required-result checks remain; no continuation reaches later SAY after failure. No OPTIONS processor or processor conversion is added. Existing private-call config inheritance and EXIT fallback remain admitted costs, not OPTIONS work. |

Component timings are isolated normal Debug observations, not a new full
aggregate or formal performance claim. Both selectors run in existing maintained
defaults. Generated registration confirms 35 cases/260 commands and the retained
1800s/serial/backend-lock properties in all four lanes without replaying the
aggregate. Fourteen existing-consumer commands refresh 23 maintained C views
(4 AST, 11 IR, 8 RXAS) in 25.875s; C count tables/owning text are reconciled.
These are view-generation/consumer checks, not new walkthrough assembly/link/VM
runs. The new header read is covered by the 48 runtime commands; unchanged body
and G proof is reused. Source-only sanitizer scheduling follows AGENTS.md.
Product inputs and grammar match their frozen qualification identities. Whole
instruction/shared dependencies remain open.

**OPTIONS-F01 — repaired (AC01/02; OPTIONS2):** actual IF owners now gate a
following ELSE after bare OPTIONS. Inline/separated true/false and nested arms
pass while standalone/stale-IF/completed-ELSE operands remain Names. The private
header procedure uses synchronous diagnostic references without an unsupported
factory receiver or escaping links; two tiny compile-only controls isolated that
source-authoring boundary. No native seed defect or new public API is claimed.

<a id="current-nop-proof"></a>

## Completed NOP proof

NOP commit `ca679a39d55660981e3805ccc94c2d973ad788f6` passes focused
normal Debug with independent source/actual RXAS review. Only affected B
frontend/dependent modules, linked hosts and fixtures were refreshed directly
through the existing `rxc`/`rxas`/`rxlink` recipes. Qualified native tools,
grammar, shared libraries and runtime inputs were reused; no expensive grammar,
native/core-product, sanitizer or full aggregate run was performed. Final
fixture preparation is warning-free. Installed/hosted/whole-C proof is not claimed.

| Normal Debug command / inspection | Observed result and scope |
|---|---|
| Existing `frontend-tests-linked.rxbin -a nop <imports> <examples>` | 124 checks/0 in 20.2367s, B opt/rxbvm: childless spans, supported contexts, contextual variable, first-extra-token refusals and continued dead semantic validation. |
| Existing `backend-tests-linked.rxbin -a <contracts> <inspection-prefix> nop` | 50 checks/0 each: opt/rxbvm 29.955s; noopt/rxtvm 29.3s. Five valid equal-width NOP/blank pairs in both IR modes compare source/selected IR operations, values, blocks, slots, executable instructions and frames while retaining truthful metadata. |
| `check-backend.py --case nop-c --ir on/off` in those paired B-mode/VM cells | Four research runs/16 commands pass the independent four-line main/IF/group/while/private oracle. Selected native control: four commands, same expected output; native C remains a read-only comparator. Assembler optimisation is disabled; unchanged assembler proof is reused. |
| Focused failure controls | Empty/unknown case filters refuse before creating work; two dead-NOP CLI refusals in both IR modes locate the first extra token and preserve previous output. |
| Existing registration inspection | Default backend corpus becomes 32 cases/239 commands. Generated pass expression and retained 1800s/serial/backend-lock properties verified without rerunning the aggregate. The added shape component is measured at about 30s Debug; source-only sanitizer policy follows AGENTS.md. |

The three selectors took 79.4923s in total; the bounded execution/failure and
two actual-RXAS capture controls took 24.7319s. These are component timings,
not a new full-aggregate or formal performance claim. Both new selectors also
run through the existing maintained defaults; no orchestrator was added.

**NOP-F01 — repaired (AC03; NOP2):** post-EXIT/private-RETURN NOP previously
reopened a dead block, adding an absent-result helper and frame slot (21→22).
Preparation now occurs in `statement` after the normal visit/budget check;
structural traversal and non-emitting statements leave termination intact.
The extra path is gone, affected frames match blank controls at 21 and following
dead semantic errors still refuse. The EXIT pair differs only in truthful
source text; the private RETURN pair is byte-identical to its blank control.
Existing EXIT fallback work remains; no optimizer/schema/runtime workaround or
zero-cost claim for unrelated machinery follows.

<a id="current-assignment-proof"></a>

## Completed assignment proof

Assignment commit `b42cf6fc7da7e487a45a646aa4e0ea7bfda74857` qualifies the
unchanged C v3 store path with independent source/actual RXAS review. Six direct
`rxc`/`rxas`/`rxlink` commands prepared both B-mode contract images warning-free
in 1.9524s using qualified DROP imports/grammar; no product or grammar rebuild.

| Normal Debug command / inspection | Observed result and scope |
|---|---|
| `check-assignment.py --build cmake-build-debug --work <temporary> --vm rxbvm --vm rxtvm` completed components | 36 positive commands: 24 source/AST/IR/provider and 20 B-host alias/cache/live-binding assertions per B-mode/VM cell; 23-line oracle in both B/IR modes and VMs. Five source refusals include dead RHS/both IR modes and preserved prior output. |
| Same checker with `--phase fatal` | Ten commands/5.6418s; both IR modes/VMs produce only prefix + RXC-LC-42.3; own-frame diversion cannot enter store. B-opt host/B-noopt target, assembler optimisation off. |
| Caller/helper and CTest inspection | Provider: three reads/eight ordered raw writes, no conversion/splitter/owner copies; same-IR RXAS byte-identical across B modes. Existing helpers unchanged. `llcompiler_c_assignment`: 51 commands, available VMs, 1800s/serial/backend lock; registration inspected without repeated execution. |

The completed positive/refusal components plus one fatal compile took at most
77s; remaining fatal work took 5.6418s. No single successful full-workload timing
is claimed; unchanged positive/refusal results were reused. No fresh sanitizer,
installed, hosted or whole-instruction/C claim follows.

<a id="current-drop-proof"></a>

## Completed DROP proof

DROP commit `fccb95b46dc4cfd75a453c0573fba27a78d5ff3b` has a
warning-free focused Debug host/fixture build and independent source/actual RXAS
review. Positive execution passed 47 source/IR/name/schema assertions and nine
prebound alias/configuration assertions in each B-mode/VM cell. The independent
36-line source oracle passed both B modes, both C IR modes and both target VMs,
plus one assembler-on control. Unchanged native/runtime/core-product proof is
reused from the completed CABI baseline below; optional packaged/installed C
products, hosted, cross-platform and complete-C acceptance are not claimed.

| Normal Debug check / command | Observed result and scope |
|---|---|
| `check-drop.py --build cmake-build-debug --work <temporary> --vm rxbvm --vm rxtvm` positive component | All 40 positive commands passed; 47 contract assertions and nine B-root alias/config assertions per host-mode/VM cell, 36-line source oracle in both IR modes/VMs and assembler-on control. Observed elapsed upper bound: 125s, including one source refusal. |
| `check-drop.py --build cmake-build-debug --work <temporary> --refusals-only` | All 15 source refusals passed in 19.4944s: located syntax/profile diagnostics, dead malformed source, reserved-dot prefixes and previous-output preservation. Positive inputs/results were reused; no single successful full-workload timing is claimed. |
| Focused CTest selection below | Seven tests passed in 42.43s: C admission 54 checks in four cells; frontend syntax 205, grammar 82 and mapping 81. |
| Maintained views through existing frontend/CLI/service probes | Four commands refreshed seven C IR/RXAS assets plus the verified inline pipeline snippet. Only authored raw-name casing changed; instruction counts, frames, source records and byte sizes are unchanged. Prior walkthrough execution remains valid. |
| CTest registration inspection | Default `llcompiler_c_drop` composes the 55 commands, both available VMs; generated command and 1800s/serial/resource-lock properties verified without rerunning the positive workload solely for registration. |

```sh
ctest --test-dir cmake-build-debug -R '^(llcompiler_c_context_contracts_(opt|noopt)_(rxbvm|rxtvm)|llcompiler_frontend_(syntax|grammar|mapping)_opt_rxbvm)$' --parallel 4 --verbose --output-on-failure
```

**DROP-F01 — generation repaired (AC01/04):** C generation forwards the existing
`rxlang --limit dfa.states=16384`; global defaults remain 4096. The generated
Unicode 17 artifact has NFA 29,709 / DFA 5,609 / LR 1,785 and 7,550,816 RXAS bytes;
normal Debug generation plus assembly took 651.355s. The three-command
`check-generator.py ... --forwarding-only` control proves named budget refusal,
previous-output preservation/tmp cleanup and two-limit forwarding with a
byte-identical successful artifact. No language/default-policy change or formal
performance claim follows.

<a id="l-construct-01"></a>

**L-CONSTRUCT-01 — locally qualified generic L reuse (STEP02/09):** opted-in
parser-only `.rxl` builds can reuse unchanged lexical construction. The
[generic LRUSE plan](../PLAN.md#current-action-lexer-reuse)
and [approved contract](../../../langtools/doc/generation-design.md) own checked
main/all-shield reuse, bounded service/stamp and output-derived persistence.
LRUSE-STEP01–04 are locally complete, including one real C cold fill and warm
output equivalence. Reuse requires unchanged generator/stamp artifacts; version
relinks can intentionally miss. C/B semantics and SDK/runtime APIs/algorithms
retain their owners. Separately, `llregular`'s linear
subset lookup/equivalent-state construction remains a later generic algorithm
opportunity requiring independent longest-match/priority/Unicode/span/budget
proof. Reuse does not claim to solve that algorithmic cost or establish a formal
performance result.

## Completed CABI proof and limits

The following results qualify CABI commit
`62d6dd44df528d56f4ca191bdc28d73b960c9432`; later changed inputs need their own
focused qualification.
`cmake --build cmake-build-debug --target stage-product`
and focused hosts/fixtures passed warning-free; the optional `llctarget` product
copy byte-matches the qualified opt target. No other optional packaged/installed C product,
hosted, cross-platform, release or complete-C claim follows.

| Normal Debug command / check | Observed result |
|---|---|
| `check-step07.py --c-only`, four host-mode/VM cells, both IR modes | Representative opt/rxbvm: 110 commands, 96.4892s; other cells: 68 each, opt/rxtvm 60.29s, noopt/rxbvm 59.20s, noopt/rxtvm 60.34s; zero failures. Expanded aggregate uses 1800s/serial/resource-lock hang protection. |
| Focused CTest direct target, C admission, shape and independent IR selections | Target: 142 current checks in each of four cells; admission: 54 each/four cells; shapes: 168 opt / 171 noopt. Independent IR: four cells plus frontend mapping/calls controls passed. |
| Independent IR placement and existing optimiser/PoC consumers | Four standalone IR cells passed in 0.53s; fixture RXAS/RXBIN byte-identical across placement. Reuse optimiser 33 commands/24.69s and PoC 54/39.69s passes on unchanged inputs. |
| Maintained walkthroughs and direct backend control | Eight referenced C RXAS and eleven IR views regenerated; 37 commands assembled/linked/executed required views/errors. Scale RXAS byte-identical to the qualified sample; direct backend 58 checks passed. [Walkthrough counts](examples/step06/README.md) include wrappers/cold handlers and exclude helper bodies/metadata. |
| Same-source literal compile and code-shape review | Initialized catalog/owner admission cache: 8.113s to 1.174s, byte-identical RXAS. Explicit EXIT/NUMERIC and present args construct no unused defaults. Literal branch pruning retains effectful truth with derived provenance. |
| Shared metadata prerequisite services | Four Debug cells passed in 22.81s; prior maintained Apple ASan four cells in 34.35s, LSan unavailable. Real C numeric 12-output and IR54 consumer proof passed. |

Per Adrian's source-only policy, source slices claim no fresh sanitizer pass;
valid unchanged-native evidence is reused and any warranted new sanitizer work
is consolidated at final delivery under [AGENTS.md](../../../AGENTS.md).
Actual SAN findings remain blockers. Retained owner/packet/getter copies,
contextual truth work, numeric setup, conservative registers and seed cleanup
remain named C-OPT-01–03/05/06 work; static counts and the local timing comparison
are not speed or minimal-frame claims. No direct construction defect remains
open from this slice. All complete-C CFC criteria above remain open.
