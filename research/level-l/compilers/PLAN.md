# Complete Classic C compiler in B and Level L

Deliver one explicitly selected Classic C compiler and reusable compile service,
written in B using Level L parsing, IR, transformation and emission. Complete
all agreed instructions, all 95 BIF names, Unicode, diagnostics, conditions/TRACE,
host/lifecycle/platform contracts and, last, INTERPRET. Native B is the build
seed; new C requests never delegate to a native source compiler/backend.
Existing RXAS/RXLINK, VM and native host transport remain dependencies.

**Complete C remains open. SIGNAL and practical TRACE, including their required
helpers/dependencies, are complete for their agreed contracts with the named
cross-cutting qualifications. Group 1 is locally complete: delayed CALL,
authored clause completion and shared field/drop observations under that scope.
Formal QA follows compiler feature completion; INTERPRET remains last.** PARSE was
delivered ahead of its queue position as the shared executor prerequisite for
ARG; this does not skip PROCEDURE or close PARSE.

[Requirements](doc/levelc-requirements.md) owns the full item crosswalk;
[architecture](doc/architecture.md) owns exact contracts. The numbered
acceptance below defines complete-C closure.
[Work packages](doc/agent-work-packages.md) assigns implementation and review.
Astra reviews/coordinates; Sol 6.1 Extra High workers implement and integrate.

## Ordered instruction delivery

Proceed in LC-I order. A delivered partial profile does not close its whole
instruction. A row marked Complete closes its agreed
[ledger contract](doc/levelc-requirements.md#whole-instruction-work-units), with
its named cross-cutting qualifications. Shared dependencies retain their owners
while the queue advances; complete C remains a separate milestone.
The [Group 1 contract](doc/architecture.md#conditions-observations-complete-contract)
supersedes condition/observation deferrals in the earlier instruction profiles
for their admitted static source forms. Whole-instruction source, expression,
host, dynamic and delivery criteria stay open. Practical TRACE does not expand
to interactive debugging, numeric suppression or complete expression events.

| Instruction | Status | Remaining work / owner |
|---|---|---|
| <a id="say-validation"></a>**LC-I-01 SAY / CLC-STEP05** | Bounded output/expression route; radix-literal repair qualified. | Complete expressions/source/diagnostics: STEP04; host edges: STEP08. CCOND covers admitted condition/TRACE integration. |
| <a id="current-action-drop"></a><a id="current-drop-proof"></a>**LC-I-02 DROP / CLC-STEP05** | Ordered direct/indirect Unicode names and shared pool operations locally delivered. | Full source/diagnostics: STEP04; host access: STEP08; dynamic entry composition: STEP11. Conditions/TRACE use CCOND. |
| <a id="current-action-assignment"></a><a id="current-assignment-proof"></a>**LC-I-03 assignment / CLC-STEP05** | Once-only RHS then one raw-name write; stems, aliases and exact text locally qualified. | Full source/expressions/diagnostics: STEP04; dynamic entry composition: STEP11. Conditions/TRACE use CCOND. |
| <a id="current-action-nop"></a><a id="current-nop-proof"></a>**LC-I-04 NOP / CLC-STEP05** | Childless forms and dead-code construction repair locally delivered. | Full source/exact diagnostics: STEP04; clause/TRACE integration uses CCOND. |
| <a id="current-action-options"></a><a id="current-options-proof"></a>**LC-I-05 OPTIONS / CLC-STEP05** | COptions, request-local header policy and no-word specialization locally delivered. | Full header/source/diagnostics: STEP04; observations use CCOND. |
| <a id="current-action-if"></a><a id="current-if-proof"></a>**LC-I-06 IF / CLC-STEP05** | Required label-prefixed arms, contextual ELSE and minimal open joins locally delivered. | Complete expressions/static diagnostics: STEP04; observations use CCOND. |
| <a id="current-action-select"></a><a id="current-select-proof"></a>**LC-I-07 SELECT / CLC-STEP05** | Ordered lazy WHEN/OTHERWISE, shared truth and source-indexed errors locally delivered. | Complete expressions/static diagnostics: STEP04; observations use CCOND. |
| <a id="current-action-do"></a><a id="current-do-proof"></a>**LC-I-08 DO / CLC-STEP05** | Static counted/controlled/FOREVER/WHILE/UNTIL, once-only ordered headers and owned state locally delivered. | Complete expressions/static diagnostics: STEP04; dynamic invalidation: STEP11. Observations use CCOND. |
| <a id="current-action-leave"></a><a id="current-leave-proof"></a>**LC-I-09 LEAVE / CLC-STEP05** | Nearest/named static targets, cleanup and default/STRICTC errors locally delivered. | Full static diagnostic recovery: STEP04; INTERPRET invalidation: STEP11. SIGNAL/CCOND transfer integration delivered. |
| <a id="current-action-iterate"></a><a id="current-iterate-proof"></a>**LC-I-10 ITERATE / CLC-STEP05** | Nearest/named targets and selected UNTIL/advance/entry continuation locally delivered. | Same observation/dynamic owners as LEAVE; preserve final FOR advance and selected owner. |
| <a id="current-action-arg"></a>**LC-I-11 ARG / CLC-STEP06** | Complete-template `PARSE UPPER ARG` with raw presence and live shared/private pool composition locally qualified. | ARG-AC05: wider source/host/delivery/platform closure remains; CCOND supplies errors/resumption/clause/TRACE. |
| <a id="current-action-procedure"></a>**LC-I-12 PROCEDURE / CLC-STEP06** | Static shared entries, executed eligibility, ordered EXPOSE and normal/fatal restoration locally qualified; CPROC-AC01–06. | CPROC-AC07: remaining source forms, callbacks/dynamic eligibility and host/delivery/platforms; CCOND supplies admitted handler observations. Whole LC-I-12 remains open. |
| <a id="current-action-call"></a>**LC-I-13 CALL / CLC-STEP06** | Ordinary local/direct-BIF/missing and static signed statement calls, omissions/results and SIGL locally qualified; CALL4 adds delayed handlers under CCOND. | Complete external-function/host/platform and whole CALL remain open. |
| <a id="current-action-return"></a>**LC-I-14 RETURN / CLC-STEP06** | Linear continuation, contextual35.1, owned result presence and existing cleanup/status locally qualified; RRET-AC01–05. | Full traceback/source catalog, broader host completion and platforms: STEP04/08; whole LC-I-14 remains open. |
| <a id="current-action-exit"></a>**LC-I-15 EXIT / CLC-STEP06** | Owned optional result, visible program-root propagation and forced cleanup locally qualified; CEXIT-AC01–05. | CEXIT-AC06: broader provider/callback lifecycle, host completion and platforms; CCOND supplies delayed-handler EXIT. Whole LC-I-15 remains open. |
| **LC-I-16 PULL / CLC-STEP06** | Shared acquisition/UPPER templates, bare consumption and live/private queue composition locally qualified; PULL-AC01–04. | PULL-AC05: wider configured host behavior and declared platforms; STEP08. CCOND supplies condition/TRACE integration. |
| **LC-I-17 PUSH / CLC-STEP06** | Optional/bare values, front insertion and operand EXIT/private restoration locally qualified; PUSH-AC01–04. | PUSH-AC05: wider host failures, selection/isolation and declared platforms; STEP08. CCOND supplies condition/TRACE integration. |
| **LC-I-18 QUEUE / CLC-STEP06** | Tail/FIFO insertion, consumed-slot/mixed-order and operand EXIT/private restoration locally qualified; QUEUE-AC01–04. | QUEUE-AC05: wider host failures/queue isolation and declared platforms; STEP08. CCOND supplies condition/TRACE integration. |
| <a id="current-action-parse"></a><a id="parse-cursor-local-qualification"></a>**LC-I-19 PARSE / CLC-STEP06 — prerequisite delivered ahead** | Seven sources/complete templates, one shared cursor/executor and C integration locally delivered. | CPARSE-AC07/08, engine real-platform proof and whole ARG/PULL acceptance remain open; STEP03/07/08/10. |
| **LC-I-20 ADDRESS / CLC-STEP07** | Explicit selection/swap, transient commands/WITH, lexical anchors and owned command lifecycle locally qualified; CADDR-AC01–05/07 explicit profile. | CADDR-AC08: native host/NUL transport and declared platforms remain; CCOND supplies conditions/resumption/TRACE. Whole LC-I-20 stays open. |
| **LC-I-21 implicit command / CLC-STEP07** | Active-ADDRESS commands, lasting connections and owned source warnings locally qualified; CADDR-AC01–07 local profile. | Host/NUL transport and declared platforms: CADDR-AC08 / STEP08. Conditions/resumption/TRACE use CCOND. |
| **LC-I-22 NUMERIC / CLC-STEP07** | Default/fixed/dynamic forms, once-only shared validation, frame installation and call restoration locally qualified; NUM-C-AC01–06. | NUM-C-AC07: broader diagnostics/host/platform scope and numeric fact invalidation; STEP02/04/08. Conditions/LOSTDIGITS/TRACE use CCOND. |
| **LC-I-23 SIGNAL / CLC-STEP07** | **Complete:** all transfers, seven-condition ON/OFF, actual producers, same-frame cleanup and typed relay; CSIGNAL-AC01–07. | Group 1 integrates delayed CALL; real host HALT, broader host/platform acceptance and INTERPRET retain their named qualifications. |
| **LC-I-24 TRACE / CLC-STEP07** | **Complete:** agreed practical options/query, command inhibition/reporting, source/live-value observations and observer restoration; CTRACE-AC01–07. | Accepted practical/presentation limitations remain explicit; formal whole-compiler/platform/delivery acceptance stays open. |
| **LC-I-25 INTERPRET / CLC-STEP11 — last** | Not implemented; required after static acceptance. | Same compiler/group mode, live continuations/state, CALL resumption/SIGNAL discard, bounded resident ownership and every LC-87/platform gate. |

## Current instruction status: CLC-STEP07

[Complete SIGNAL](doc/architecture.md#signal-complete-contract) includes every
agreed source form, seven-condition ON/OFF, actual producers, typed payload/origin
transport, visible exception flow and same-frame cleanup, together with its
required helpers/dependencies. CSIGNAL-AC01–07 and STEP01–03 are complete.

[Whole TRACE contract](doc/architecture.md#trace-complete-contract):
CTRACE-STEP01–03 and AC01–07 are complete for the approved native practical
ceiling. Early coherent source/representative RXAS review and the focused
Debug opt compiler, IR-on, assembler-opt rxbvm checks qualify this local
feature. C21 introduced `trace(value,context)`; shared B retains option/command
semantics and the private generated handler reuses its trace services. Formal
compiler QA follows feature completion; all CFC criteria remain open.
INTERPRET remains last and outside the standing implementation authority.

Adrian authorized the remaining agreed C instructions/features stepwise, with
internal contract review and a local commit after each, stopping before
INTERPRET without further user approval. Scope exceptions retain their gates.
PROCEDURE1–4 remain complete for CPROC-AC01–06; CPROC-AC07 stays open.
[CALL contract](doc/architecture.md#call-contract) owns CCALL-AC01–07.

| Stable feature | Status / dependency |
|---|---|
| **CLC-STEP06.CALL1** | Complete contract review. |
| **CLC-STEP06.CALL2** | Locally qualified ordinary profile, CCALL-AC01–04/07; whole CALL stays open. |
| **CLC-STEP06.CALL3** | Locally qualified [CEXT-AC01–05 / STEP01–03](doc/architecture.md#call3-static-provider-contract), with the generic classifier prerequisite. Fresh pool/config, inherited activation state; external expressions/late loading/native callbacks remain deferred. |
| **CLC-STEP06.CALL4** | Delivered with CCOND: shared policy/checkpoints, local/BIF/signed handler lifecycle and delayed resumption. Wider CCALL-AC07 host/platform scope remains open. |
| **PULL-STEP01–03** | Locally qualified shared-parser profile, PULL-AC01–04, with CCOND condition/observation integration; AC05 retains wider host/platform work. |
| **PUSH-STEP01–03** | [Selected-queue profile](doc/architecture.md#push-contract) locally qualified for AC01–04; AC05 stays shared scope. |
| **QUEUE-STEP01–03** | [Tail-insertion profile](doc/architecture.md#queue-contract) locally qualified for AC01–04; neutral insertion controls are shared with PUSH. AC05 stays shared scope. |
| **ADDR-B-STEP01–03** | [Shared B binding bridge](doc/architecture.md#address-b-binding-bridge) locally qualified for AC01–05 as a separate prerequisite. AC06 keeps C/host/condition scope open. |
| **CADDR-STEP01–02** | [Keyed explicit ADDRESS profile](doc/architecture.md#address-c-command-contract) locally qualified for AC01–05/07 explicit scope, with the separate B prerequisite. AC08 stays open. |
| **CADDR-STEP03** | Locally qualified implicit C16 commands and owned source warnings through the same command owner; AC01–07 local profile. AC08 stays shared scope. |
| **NUM-B-STEP01–03** | Locally complete: [shared FORM diagnostic prerequisite](doc/architecture.md#numeric-form-diagnostic-prerequisite) corrects the sole validator's `33.3` identity before C source extension; no numeric algorithm change. |
| **NUM-C-STEP01–03** | [NUMERIC source-completion profile](doc/architecture.md#numeric-c-contract) locally qualified for AC01–06. AC07 shared closure remains open. |
| **COND-B-STEP01–03** | [Shared external condition scope prerequisite](doc/architecture.md#external-condition-b-prerequisite) locally qualified for AC01–05. C/provider normalization, immediate SIGNAL and delayed CALL integration are delivered; wider AC06 host integration remains open. |
| **CLABEL-STEP01–03** | [Label/arm and local-entry prerequisite](doc/architecture.md#label-arm-prerequisite) locally qualified for AC01–05; C17 schema/runtime unchanged. Its inventory supports complete SIGNAL. |
| **CLAUSE-STEP01–03** | [Clause-clock foundation](doc/architecture.md#clause-clock-contract) locally complete. CCOND adds authored completion, handler-independent samples and field/drop observations under practical TRACE. |
| **CSIGNAL-STEP01–03** | [Complete SIGNAL](doc/architecture.md#signal-complete-contract): STEP01–03 and CSIGNAL-AC01–07 complete for the agreed instruction, including all required helpers/dependencies. Named cross-cutting qualifications remain explicit. |
| **CTRACE-STEP01–03** | [Complete practical TRACE](doc/architecture.md#trace-complete-contract): STEP01–03 and CTRACE-AC01–07 complete for the agreed practical contract, including metadata/observer dependencies. Full-C and formal QA criteria remain open. |
| **CCOND-STEP01–03** | [Conditions and observations](doc/architecture.md#conditions-observations-complete-contract): complete for CCOND-AC01–08, including CALL4 and required shared-B dependencies. Full-C and formal QA criteria remain open. |

No native C frontend implementation, second runtime algorithm, push or hosted
campaign is commissioned. Needed generic exception representation and the
existing SIGNALORIGIN payload correction follow the internally reviewed whole
contract. INTERPRET remains excluded from the standing authority.

The [RETURN contract](doc/architecture.md#return-contract) owns RRET-AC01–05;
**CLC-STEP06.RETURN1–3** are complete for the local source/completion profile.
[EXIT](doc/architecture.md#exit-contract): **CEXIT-STEP01–03** complete for
local AC01–05. AC06 retains provider/handler/host/platform owners. CALL3 static
statement integration, standalone PULL, PUSH and QUEUE are locally delivered.
Explicit ADDRESS and implicit commands/warnings are locally delivered.
NUMERIC's source profile and the shared B condition prerequisite are locally
delivered. CLABEL's required label-prefixed arms and shared local entries are locally delivered.
CLAUSE1 supplies lazy activation clocks. Complete SIGNAL includes its required
condition/handler dependencies; CCOND supplies CALL4 and authored completion.
The whole exact contract is internally reviewed before implementation.

**BCONCAT-STEP01–03 complete:** intrinsic concatenation followed by target conversion;
[permanent regression](../../../compiler/tests/rexx_src/binary_concat_target.crexx).
**BCONCAT-AC01:** exact UTF8 text assignment/reassignment/blank concat;
**BCONCAT-AC02:** genuine binary bytes/BCONCAT;
**BCONCAT-AC03:** compiler/assembler modes and readable RXAS.

<a id="remaining-actions"></a>

## Remaining work by area

Use these four groups to organize the remaining implementation and integration
work. The existing instruction, step and acceptance IDs retain their contracts;
the groups do not introduce new acceptance criteria or alter decision gates.

**Group 1 — locally complete.** The
[whole condition/observation contract](doc/architecture.md#conditions-observations-complete-contract)
and required C/L/shared-B dependencies are delivered under CCOND-AC01–08 and
STEP01–03. Qualification reuses native fixture sources/checked expectations,
independent contract supplements and prepared products in one Debug
opt/IR-on/rxbvm lane. Formal compiler QA follows feature completion. Native C
remains read-only, no push is authorized, and INTERPRET stays last.

| Group | Status / remaining work | Existing owners |
|---|---|---|
| **Conditions and observations** | **Complete local increment:** CALL ON/OFF, delayed delivery/resumption and buffered HALT; configured conditions, SIGL/CONDITION/origins, authored clause/clocks and field/drop observations, including handler pool/resource restoration. Wider host producers/platforms and dynamic execution retain the groups below. | CCOND-AC01–08 / STEP01–03; CALL4; C-OBS; condition portions of STEP03/07 and instruction criteria. |
| **Calls and host integration** | Remaining internal/external call and activation semantics: arguments/omissions, RESULT and RETURN/EXIT completion, PROCEDURE/private/shared pools and EXPOSE aliases, inherited/restored numeric/ADDRESS/TRACE state, provider/callback entry and teardown. Preserve already-qualified ordinary recursion and call behavior. Complete COMMAND/FUNCTION/SUBROUTINE host invocation with exact-length/empty/NUL arguments/results, live pool access windows, initial ADDRESS/stream/queue selection, I/O adapters and NUL transport, real clock/HALT producers, resource failures, concurrent-context isolation and declared platform adapters. CALL ON/OFF and event delivery remain with conditions. | C-ACT/C-HOST; call portions of C-POOL; STEP03/06/08; CPROC/CCALL/CEXT/RRET/CEXIT and host portions of queue/stream/ADDRESS criteria; BF05/06/08/09 integration dependencies. |
| **Language functionality: source, expressions and BIFs** | Remaining syntax/header controls, Unicode names/source inventory, operators/numeric semantics, exact diagnostics and dead-source validation; outstanding variable/stem and instruction semantics outside the condition/call/host groups; complete C context and behavior contracts for the 95 already-bound BIF names, reusing the shared B bodies. INTERPRET is the last feature in this group, with call/lifecycle integration and compiler support supplied by their owners. | C-SRC/C-EXPR/C-NUM/C-POOL/C-UNI; STEP04–06; BF01–09 language contracts; STEP11 last. |
| **Compiler and emitter** | Generic L exceptional/resumable control-flow representation, effects/facts/invalidation, liveness and checked rewrites; register assignment/reuse, call windows, copies/context/helper costs and cleanup; truthful source/TRACE metadata, import/ABI ownership, bounded compilation and atomic failure; compiler/runtime build separation. Keep the native B seed alias defect as a named supporting dependency with its existing owner and repair authority. | STEP01/02/09; C-ABI; C-OPT-01–06; CFC-AC07/10–12; B-SEED-ALIAS. |

Calls and host integration owns activation/pool/alias entry, inheritance,
arguments/results, completion and teardown, including later INTERPRET integration.
Conditions owns when handlers transfer or resume execution; compiler work owns
the supporting emitted ownership/cleanup mechanisms. Each mechanism retains one
owner, and locally qualified recursion/PROCEDURE/EXPOSE is not reopened.

INTERPRET retains its final position and existing contract/approval gates.
Formal whole-compiler QA follows feature completion. STEP10 delivery checks
(CLI/service, installation, cold/incremental/relocated builds and declared
platform proof), the complete ledger crosswalk and CFC-AC01–12 remain explicit
acceptance work across these groups; they are not additional language features.

<a id="shared-delivery-stages"></a>

### Stable delivery steps

Deliver the minimum shared dependency required by each instruction. These
stages retain full shared acceptance; they do not replace the ordered queue.

| Shared step | Status / remaining work |
|---|---|
| **CLC-STEP01** | Partial: finish every ledger disposition, independent expectations, ABI/platform decisions and test traceability. Exact exceptions need Adrian's approval. |
| <a id="l-construct-01"></a>**CLC-STEP02** | Partial: L exceptional/resumable edges, effects/facts/invalidation, liveness, bounded atomic failure and checked rewrites. Keep block-local values/cross-block slots; needed L APIs receive internal contract review under the standing authority. Generic lexical reuse and CMETA-STEP01–03 scalar attribute-index reading are delivered; linear llregular construction remains separate. |
| <a id="active-first-implementation-increment"></a><a id="shared-b-unicode-interface-and-c-adapter-migration"></a><a id="active-c-contract-baseline-increment"></a><a id="active-borrowed-classic-context-increment"></a>**CLC-STEP03** | Partial: borrowed context/shared B Unicode foundations delivered. Complete activation/pool/alias ownership, recursion, restoration, abrupt paths and needed direct/BIF APIs; preserve existing B/G/public metadata contracts and one algorithm/data owner. |
| **CLC-STEP04** | Partial: complete contextual source/Unicode names, exact byte/scalar origins, dead-source validation, expressions/numeric semantics and BF01–05 bindings. No native frontend dependency. |
| **CLC-STEP05** | Partial: finish shared dependencies of the delivered LC-I-01–10 profiles in the instruction table. |
| **CLC-STEP06** | Local LC-I-11–19 profiles and CALL4 condition integration delivered; remaining full scope stays explicit. Retain ARG/PARSE/PULL delivery/platform and broader call/host completion obligations. |
| **CLC-STEP07** | SIGNAL, practical TRACE and Group 1 conditions/observations locally qualified. Complete source inventory/diagnostics, remaining BF07/09 contracts and host/platform/delivery acceptance retain their owners. |
| **CLC-STEP08** | Not started as a full stage: remaining BIF/host adapters, exact-length invocation/completion, callback pools, queues/streams/clocks/HALT, concurrency and declared platforms. New host APIs/exclusions require approval. |
| **CLC-STEP09** | Open throughout: C-OPT-01–06 and actual caller/helper RXAS review, truthful source/TRACE, bounded registers/call windows and cleanup. Fix construction defects; document necessary costs. Needed ISA/storage contracts receive internal review under standing authority; production performance edits retain their qualification gate. |
| <a id="step07a-validation"></a>**CLC-STEP10** | Not started: complete static ledger and CLI/service/install/cold/incremental/relocated/platform delivery, independent walkthrough/review. Static acceptance is not complete C. |
| **CLC-STEP11** | Last: same-compiler INTERPRET after static acceptance and approval of exact continuation/code-owner contracts; close every CFC criterion. |

**L-VIEW-01 — locally qualified, CLC-STEP09:** the
[retained operation-view contract](doc/architecture.md#retained-c-operation-views)
limits this increment to the six post-admission lookups in
`llccommandprotocol`. Reuse its existing arena reference and the existing checked
`opview(reference arena,id)` constructor; preserve initial graph/error/belongs
admission, bounded IDs and view identity/lifetime checks. Cold failure-offset
lookups retain `module.operation`. L-VIEW-STEP01–03 are complete: the focused
consumer probe passes with compiler/assembler opt on rxbvm and noopt on rxtvm;
owning RXAS forwards the retained reference with no arena MKREF at the six sites.
Unchanged generic view controls are reused. Generic accessor
internals remain with CLC-STEP02; no generic L/native/API/cache change or full
source matrix is commissioned. This does not establish replacement readiness.

**CLC-STEP11a–d:** 11a: nested execution, SYNTAX, CALL resumption/SIGNAL, live
continuations and cleanup/limits. 11b: ephemeral ownership, pins/reclamation,
stale handles, all-pinned exhaustion, rollback and both VMs. 11c: group compiler,
assembly/load, live pools/arguments/config, caller dispatch/spills. 11d: every
LC-87 obligation, PROCEDURE/outer-loop rules, RETURN/EXIT/SIGNAL, delayed
handlers, source/TRACE, resources and installed/platform delivery.

## C-first delivery milestone

All **CFC-AC01–12 remain open**. Complete C is separate from default replacement,
new B/G features, proper B self-hosting (LLCOMP-STEP07B), combined replacement,
native-C retirement and release approval. Their unverified outcomes remain in
the requirements ledger; existing consumers remain regressions. Exact public
scope/API/ISA/platform exceptions outside the standing authority require Adrian's decision. Native Classic
source is read-only; a demonstrated B seed defect may receive a separate repair.

| Stable criterion | Observable pass condition and proof |
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

## Outstanding decisions and dependencies

- **Shared observations:** CCOND supplies configured errors/NOVALUE/LOSTDIGITS,
  typed conditions, resumed handlers and authored clause/field/drop observations
  for the admitted static profile. ARG-AC05/CPARSE-AC07 retain their wider source,
  host/dynamic and delivery/platform obligations; formal QA remains open.
- **Delivery/platforms:** CPARSE-AC08, PARSE-ENGINE-AC05 real constrained platforms,
  UNWIND-AC04 broader-platform proof and complete CLI/service/install/relocation
  remain open. Linux LSan and real 31-bit/CMS/TSO/Windows runtime need their
  declared qualification; Apple LeakSanitizer is unavailable. Local static/NUTF8
  simulations and synthetic errors do not establish those platform capabilities.
- **Dynamic execution:** approve exact continuation/code-owner/pinning/reclamation
  contracts before STEP11; preserve every LC-87 obligation and INTERPRET-last.
- **B-SEED-ALIAS — open:** native B noopt can overwrite a live array receiver
  while evaluating a nested method-call index. The C ADDRESS selector uses
  sequential scalar indices. Native seed register allocation owns a later
  focused repair and a class-array/method-index regression in both compiler
  modes; keep the C owner/clause shape controls.
- **Scope:** language/API/ISA/platform exceptions, default switching, native-C
  retirement and release/publication remain explicit maintainer decisions.
