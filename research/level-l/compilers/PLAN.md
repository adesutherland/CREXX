# Complete Classic C compiler in B and Level L

**Complete C is open. CLC-STEP05.SELECT1–3 are locally complete. Next: review
the generic L checked-DFA reuse contract before further grammar changes.**

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

## Next action: generic L generation reuse contract review

Before another grammar-changing C increment, the generic L owner must propose
the checked reuse contract under L-CONSTRUCT-01. Exact cache identity, admission,
limits and build ownership need review and approval before implementation;
SELECT has not implemented that proposal.

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
| `rxlang --rxas` exact parser/AST with temporary tiny lexer; existing full C grammar recipe and `rxas` | Cheap LR admission passed in 8.239s (3 NFA / 2 DFA / 2,452 LR). The one required full-source generation/assembly passed in 653.136s: 29,709 NFA / 5,609 DFA / 2,452 LR, 7,698,798 RXAS bytes, no diagnostics. Cold construction cost only; L-CONSTRUCT-01 retains the proposed generic reuse work. |
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

Whole LC-I-05 retains full header controls/STRICTC/source maps, shared labels,
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
| **CLC-STEP05 — Partial; current SELECT** | Review LC-I-01–10 individually: SAY through ITERATE. Close every form, error, order, pool/loop effect and source observation in the ledger. Serves AC01–03/06/07. | STEP04 plus each instruction's shared dependencies. Prior bounded reviews retain shared dependencies; approved SELECT implementation is current. |
| **CLC-STEP06 — Partial** | Review LC-I-11–19 individually: ARG through PARSE. Complete one PARSE executor, shared labels/fall-through, presence/results, EXPOSE and static signed providers; integrate BF05/06/08 as needed. Serves AC01–04/06/09/11. | Needed STEP03–05 services. ARG may introduce shared PARSE infrastructure before full PARSE review. |
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

**L-CONSTRUCT-01 — later generic L construction cost (STEP02/09):** `llregular`
linearly searches prior subset keys and retains equivalent DFA states. The
generic L owner may add indexed subset lookup/equivalent-state reduction with
independent longest-match, priority, Unicode-membership/span and bounded-failure
proof. Parser-only `.rxl` changes currently repeat unchanged lexical construction
because generation emits DFA/LR together. Before the next grammar-changing
slice, that owner should propose checked DFA reuse keyed on lexical rules/order/
token IDs, Unicode/encoding and image/generator inputs, preserving current limits.
The reuse contract needs approval; it is not implemented in SELECT. The observed
cost covers generation plus assembly, not target execution or conformance retests.

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
