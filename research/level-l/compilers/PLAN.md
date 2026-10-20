# Complete Classic C compiler in B and Level L

Deliver one explicitly selected Classic C compiler and reusable compile service,
written in B using Level L parsing, IR, transformation and emission. Complete
all agreed instructions, all 95 BIF names, Unicode, diagnostics, conditions/TRACE,
host/lifecycle/platform contracts and, last, INTERPRET. Native B is the build
seed; new C requests never delegate to a native source compiler/backend.
Existing RXAS/RXLINK, VM and native host transport remain dependencies.

**Complete C remains open. Last locally delivered profile: LC-I-13 ordinary CALL.
Current: LC-I-14 RETURN source/completion.** PARSE was delivered ahead of its queue position as the shared
executor prerequisite for ARG; this does not skip PROCEDURE or close PARSE.

[Requirements](doc/levelc-requirements.md) owns the full item crosswalk;
[architecture](doc/architecture.md) owns exact contracts. The numbered
acceptance below defines complete-C closure.
[Work packages](doc/agent-work-packages.md) assigns implementation and review.
Astra reviews/coordinates; Sol 6.1 Extra High workers implement and integrate.

## Ordered instruction delivery

Proceed in LC-I order. A delivered profile is a locally qualified increment,
not whole-instruction closure. Every whole LC-I row remains open until its
full [ledger contract](doc/levelc-requirements.md#whole-instruction-work-units)
passes. Shared dependencies retain their named owners while the queue advances.

| Instruction | Status | Remaining work / owner |
|---|---|---|
| <a id="say-validation"></a>**LC-I-01 SAY / CLC-STEP05** | Bounded output/expression route; radix-literal repair qualified. | Complete expressions, configured errors and source/TRACE: STEP03/04/07; host edges: STEP08. |
| <a id="current-action-drop"></a><a id="current-drop-proof"></a>**LC-I-02 DROP / CLC-STEP05** | Ordered direct/indirect Unicode names and shared pool operations locally delivered. | Configured errors/NOVALUE/TRACE: STEP03/07; dynamic entry composition: STEP11. |
| <a id="current-action-assignment"></a><a id="current-assignment-proof"></a>**LC-I-03 assignment / CLC-STEP05** | Once-only RHS then one raw-name write; stems, aliases and exact text locally qualified. | Errors/NOVALUE/TRACE: STEP03/07; dynamic entry composition: STEP11. |
| <a id="current-action-nop"></a><a id="current-nop-proof"></a>**LC-I-04 NOP / CLC-STEP05** | Childless forms and dead-code construction repair locally delivered. | Exact diagnostics/clause/TRACE: STEP04/07. |
| <a id="current-action-options"></a><a id="current-options-proof"></a>**LC-I-05 OPTIONS / CLC-STEP05** | COptions, request-local header policy and no-word specialization locally delivered. | Full header/source/diagnostics/TRACE: STEP04/07. |
| <a id="current-action-if"></a><a id="current-if-proof"></a>**LC-I-06 IF / CLC-STEP05** | Optional-arm/contextual ELSE and minimal open joins locally delivered. | Complete expressions/recovery/conditions/TRACE: STEP03/04/07. |
| <a id="current-action-select"></a><a id="current-select-proof"></a>**LC-I-07 SELECT / CLC-STEP05** | Ordered lazy WHEN/OTHERWISE, shared truth and source-indexed errors locally delivered. | Complete expression/error/condition/TRACE owners: STEP03/04/07. |
| <a id="current-action-do"></a><a id="current-do-proof"></a>**LC-I-08 DO / CLC-STEP05** | Static counted/controlled/FOREVER/WHILE/UNTIL, once-only ordered headers and owned state locally delivered. | Complete expressions/recovery/conditions/TRACE: STEP03/04/07; dynamic invalidation: STEP11. |
| <a id="current-action-leave"></a><a id="current-leave-proof"></a>**LC-I-09 LEAVE / CLC-STEP05** | Nearest/named static targets, cleanup and default/STRICTC errors locally delivered. | Exact recovery/conditions/TRACE: STEP07; SIGNAL/INTERPRET invalidation: STEP07/11. |
| <a id="current-action-iterate"></a><a id="current-iterate-proof"></a>**LC-I-10 ITERATE / CLC-STEP05** | Nearest/named targets and selected UNTIL/advance/entry continuation locally delivered. | Same observation/dynamic owners as LEAVE; preserve final FOR advance and selected owner. |
| <a id="current-action-arg"></a>**LC-I-11 ARG / CLC-STEP06** | Complete-template `PARSE UPPER ARG` with raw presence and live shared/private pool composition locally qualified. | ARG-AC05: configured errors, resumption/clause/TRACE and delivery/platform: STEP03/07/10. |
| <a id="current-action-procedure"></a>**LC-I-12 PROCEDURE / CLC-STEP06** | Static shared entries, executed eligibility, ordered EXPOSE and normal/fatal restoration locally qualified; CPROC-AC01–06. | CPROC-AC07: complete source forms/observations, callbacks/dynamic eligibility, host/delivery/platforms; whole LC-I-12 remains open. |
| <a id="current-action-call"></a>**LC-I-13 CALL / CLC-STEP06 — last delivered profile** | Ordinary local/direct-BIF/missing calls, symbol/quoted/constant targets, omissions/results and SIGL locally qualified; CCALL-AC01–04/07. | CALL3 static signed statement providers after EXIT/provider review; CALL4 delayed traps after STEP07; whole CALL remains open. |
| **LC-I-14 RETURN / CLC-STEP06 — current** | Existing value/empty/EOF packets and shared-body cleanup; bounded source/completion contract reviewed. | Linear continuation/contextual35.1 and focused completion/status qualification next; broader observations/host/platform: STEP07/08. |
| **LC-I-15 EXIT / CLC-STEP06** | Basic termination route implemented. | Program-root propagation, optional value/EOF, once-only finalization and host completion; STEP06/08. |
| **LC-I-16 PULL / CLC-STEP06** | Not integrated as a standalone instruction. PARSE source adapter is reusable. | Acquire queue or configured input once, then shared UPPER executor; EOF/NOTREADY and nested isolation; STEP06/07/08. |
| **LC-I-17 PUSH / CLC-STEP06** | Not implemented. | Once-only optional expression and selected-queue front insertion; shared queue/host adapter, errors/ownership: STEP06/08. |
| **LC-I-18 QUEUE / CLC-STEP06** | Not implemented. | Selected-queue tail insertion, FIFO/count/nonconsumption and nested host isolation: STEP06/08. |
| <a id="current-action-parse"></a><a id="parse-cursor-local-qualification"></a>**LC-I-19 PARSE / CLC-STEP06 — prerequisite delivered ahead** | Seven sources/complete templates, one shared cursor/executor and C integration locally delivered. | CPARSE-AC07/08, engine real-platform proof and separate ARG/PULL acceptance remain open; STEP03/07/08/10. |
| **LC-I-20 ADDRESS / CLC-STEP07** | Not implemented. | Environment selection/swap, command/WITH resources, current object protocol, anchors/status/conditions/NUL policy; STEP07/08. |
| **LC-I-21 implicit command / CLC-STEP07** | Not implemented. | Once-only active-ADDRESS command, lasting connections, status/traps and source; STEP07/08. |
| **LC-I-22 NUMERIC / CLC-STEP07** | Bounded DIGITS/FUZZ/FORM and inherited/restored settings implemented. | Full defaults/validation, arithmetic/display/BIF effects and numeric fact invalidation; STEP03/04/07. |
| **LC-I-23 SIGNAL / CLC-STEP07** | Not implemented in new C. | Direct/quoted/VALUE targets, seven-condition ON/OFF, SIGL, loop clearing and source/error/handler lifetimes; STEP07/08. |
| **LC-I-24 TRACE / CLC-STEP07** | Not implemented in new C. | Agreed practical VM observations/options/query/command inhibition and disabled-cost proof; STEP07/08. |
| **LC-I-25 INTERPRET / CLC-STEP11 — last** | Not implemented; required after static acceptance. | Same compiler/group mode, live continuations/state, CALL resumption/SIGNAL discard, bounded resident ownership and every LC-87/platform gate. |

<a id="current-action-return"></a>

## Current instruction: LC-I-14 RETURN

Adrian authorized the remaining agreed C instructions/features stepwise, with
internal contract review and a local commit after each, stopping before
INTERPRET without further user approval. Scope exceptions retain their gates.
PROCEDURE1–4 remain complete for CPROC-AC01–06; CPROC-AC07 stays open.
[CALL contract](doc/architecture.md#call-contract) owns CCALL-AC01–07.

| Stable feature | Status / dependency |
|---|---|
| **CLC-STEP06.CALL1** | Complete contract review. |
| **CLC-STEP06.CALL2** | Locally qualified ordinary profile, CCALL-AC01–04/07; whole CALL stays open. |
| **CLC-STEP06.CALL3** | Open AC05: static signed statement providers after EXIT and exact provider review; fresh pool/config, inherited activation state. External expressions/late loading/native callbacks remain deferred. |
| **CLC-STEP06.CALL4** | Open AC06/07: existing B policy/checkpoints/handler lifecycle after STEP07 clause/condition support. |

No native C implementation, second runtime algorithm, push or hosted campaign
is commissioned. INTERPRET remains excluded from the standing authority.

The [RETURN contract](doc/architecture.md#return-contract) owns RRET-AC01–05.
**CLC-STEP06.RETURN1** complete: bounded source/completion contract;
**RETURN2** next: linear layout-token continuation and 35.1;
**RETURN3** queued: focused source/AST/host/RXAS qualification and local commit.
[EXIT](doc/architecture.md#exit-contract): **CEXIT-STEP01** reviewed;
**STEP02/03** queued after RETURN for program-root propagation and local AC01–05
qualification/commit; AC06 retains provider/handler/host/platform owners.

**BCONCAT-STEP01–03 complete:** intrinsic concatenation followed by target conversion;
[permanent regression](../../../compiler/tests/rexx_src/binary_concat_target.crexx).
**BCONCAT-AC01:** exact UTF8 text assignment/reassignment/blank concat;
**BCONCAT-AC02:** genuine binary bytes/BCONCAT;
**BCONCAT-AC03:** compiler/assembler modes and readable RXAS.

<a id="remaining-actions"></a>

## Shared delivery stages

Deliver the minimum shared dependency required by each instruction. These
stages retain full shared acceptance; they do not replace the ordered queue.

| Shared step | Status / remaining work |
|---|---|
| **CLC-STEP01** | Partial: finish every ledger disposition, independent expectations, ABI/platform decisions and test traceability. Exact exceptions need Adrian's approval. |
| <a id="l-construct-01"></a>**CLC-STEP02** | Partial: L exceptional/resumable edges, effects/facts/invalidation, liveness, bounded atomic failure and checked rewrites. Keep block-local values/cross-block slots; needed L APIs receive internal contract review under the standing authority. Generic lexical reuse and CMETA-STEP01–03 scalar attribute-index reading are delivered; linear llregular construction remains separate. |
| <a id="active-first-implementation-increment"></a><a id="shared-b-unicode-interface-and-c-adapter-migration"></a><a id="active-c-contract-baseline-increment"></a><a id="active-borrowed-classic-context-increment"></a>**CLC-STEP03** | Partial: borrowed context/shared B Unicode foundations delivered. Complete activation/pool/alias ownership, recursion, restoration, abrupt paths and needed direct/BIF APIs; preserve existing B/G/public metadata contracts and one algorithm/data owner. |
| **CLC-STEP04** | Partial: complete contextual source/Unicode names, exact byte/scalar origins, dead-source validation, expressions/numeric semantics and BF01–05 bindings. No native frontend dependency. |
| **CLC-STEP05** | Partial: finish shared dependencies of the delivered LC-I-01–10 profiles in the instruction table. |
| **CLC-STEP06** | Current: RETURN then EXIT after ordinary CALL, with CALL3/4 dependencies and remaining LC-I-11–19 explicit. Retain ARG/PARSE/PULL and broader completion obligations. |
| **CLC-STEP07** | Partial: LC-I-20–24, conditions/resumption, clause clocks, diagnostics/source/TRACE and BF07/09. |
| **CLC-STEP08** | Not started as a full stage: remaining BIF/host adapters, exact-length invocation/completion, callback pools, queues/streams/clocks/HALT, concurrency and declared platforms. New host APIs/exclusions require approval. |
| **CLC-STEP09** | Open throughout: C-OPT-01–06 and actual caller/helper RXAS review, truthful source/TRACE, bounded registers/call windows and cleanup. Fix construction defects; document necessary costs. Needed ISA/storage contracts receive internal review under standing authority; production performance edits retain their qualification gate. |
| <a id="step07a-validation"></a>**CLC-STEP10** | Not started: complete static ledger and CLI/service/install/cold/incremental/relocated/platform delivery, independent walkthrough/review. Static acceptance is not complete C. |
| **CLC-STEP11** | Last: same-compiler INTERPRET after static acceptance and approval of exact continuation/code-owner contracts; close every CFC criterion. |

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

- **Shared observations:** configured errors/NOVALUE, typed conditions, resumed
  handlers, clause/field/drop observations and practical TRACE remain STEP03/07
  work; ARG-AC05 and CPARSE-AC07 stay open.
- **Delivery/platforms:** CPARSE-AC08, PARSE-ENGINE-AC05 real constrained platforms,
  UNWIND-AC04 broader-platform proof and complete CLI/service/install/relocation
  remain open. Linux LSan and real 31-bit/CMS/TSO/Windows runtime need their
  declared qualification; Apple LeakSanitizer is unavailable. Local static/NUTF8
  simulations and synthetic errors do not establish those platform capabilities.
- **Dynamic execution:** approve exact continuation/code-owner/pinning/reclamation
  contracts before STEP11; preserve every LC-87 obligation and INTERPRET-last.
- **Scope:** language/API/ISA/platform exceptions, default switching, native-C
  retirement and release/publication remain explicit maintainer decisions.
