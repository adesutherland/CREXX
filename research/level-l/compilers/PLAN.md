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
Group 2 static calls/host integration and Group 3 static language functionality
are locally complete. The final language increment, STEP11 INTERPRET and Rexx
source calls, is locally complete; broader delivery/platform acceptance remains
open. The current approved baseline qualification covers full ordinary QA,
Release delivery and maintained sanitizer/platform checks, followed by
publication to develop only; it does not commission a formal release.** PARSE was
delivered ahead of its queue position as the shared executor prerequisite for
ARG; this does not skip PROCEDURE or close PARSE.

[Requirements](doc/levelc-requirements.md) owns the full item crosswalk;
[architecture](doc/levelc-architecture.md) owns exact contracts. The numbered
acceptance below defines complete-C closure.
[Work packages](doc/agent-work-packages.md) assigns implementation and review.
Model allocation for coordination, design, implementation, execution and
escalation follows the current policy in the repository's canonical `AGENTS.md`.

The selected emission-repair increment is done for its local scope. The registered
research functional gate, authorized local commit and fixed six representative
RXAS review areas are done. Its G-enabled shared/consumer cells do not establish
full G qualification. Preserve the registered functional coverage and reviewed
producer/source identities without emitting every internal scenario. The bounded
post-review cost repairs below do not close broader complete-C/baseline acceptance
or authorize publication.

## Ordered instruction delivery

Proceed in LC-I order. A delivered partial profile does not close its whole
instruction. A row marked Complete closes its agreed
[ledger contract](doc/levelc-requirements.md#whole-instruction-work-units), with
its named cross-cutting qualifications. Shared dependencies retain their owners
while the queue advances; complete C remains a separate milestone.
The [Group 1 contract](doc/levelc-architecture.md#conditions-observations-complete-contract)
supersedes condition/observation deferrals in the earlier instruction profiles
for their admitted static source forms. Whole-instruction source, expression,
host, dynamic and delivery criteria stay open. Practical TRACE does not expand
to interactive debugging, numeric suppression or complete expression events.

| Instruction | Status | Remaining work / owner |
|---|---|---|
| <a id="say-validation"></a>**LC-I-01 SAY / CLC-STEP05** | Bounded output/expression route; radix-literal repair qualified. | Group 3 supplies admitted static expressions/source/diagnostics; host edges: STEP08. CCOND covers admitted condition/TRACE integration. |
| <a id="current-action-drop"></a><a id="current-drop-proof"></a>**LC-I-02 DROP / CLC-STEP05** | Ordered direct/indirect Unicode names and shared pool operations locally delivered. | Group 3 supplies admitted static source/diagnostics; host access: STEP08; dynamic entry composition: STEP11. Conditions/TRACE use CCOND. |
| <a id="current-action-assignment"></a><a id="current-assignment-proof"></a>**LC-I-03 assignment / CLC-STEP05** | Once-only RHS then one raw-name write; stems, aliases and exact text locally qualified. | Group 3 supplies admitted static source/expressions/diagnostics; dynamic entry composition: STEP11. Conditions/TRACE use CCOND. |
| <a id="current-action-nop"></a><a id="current-nop-proof"></a>**LC-I-04 NOP / CLC-STEP05** | Childless forms and dead-code construction repair locally delivered. | Group 3 supplies admitted static source/exact diagnostics; clause/TRACE integration uses CCOND. |
| <a id="current-action-options"></a><a id="current-options-proof"></a>**LC-I-05 OPTIONS / CLC-STEP05** | COptions, request-local header policy and no-word specialization locally delivered. | Group 3 supplies admitted static header/source/diagnostics; observations use CCOND. |
| <a id="current-action-if"></a><a id="current-if-proof"></a>**LC-I-06 IF / CLC-STEP05** | Required label-prefixed arms, contextual ELSE and minimal open joins locally delivered. | Group 3 supplies admitted static expressions/diagnostics; observations use CCOND. |
| <a id="current-action-select"></a><a id="current-select-proof"></a>**LC-I-07 SELECT / CLC-STEP05** | Ordered lazy WHEN/OTHERWISE, shared truth and source-indexed errors locally delivered. | Group 3 supplies admitted static expressions/diagnostics; observations use CCOND. |
| <a id="current-action-do"></a><a id="current-do-proof"></a>**LC-I-08 DO / CLC-STEP05** | Static counted/controlled/FOREVER/WHILE/UNTIL, once-only ordered headers and owned state locally delivered. | Group 3 supplies admitted static expressions/diagnostics; dynamic invalidation: STEP11. Observations use CCOND. |
| <a id="current-action-leave"></a><a id="current-leave-proof"></a>**LC-I-09 LEAVE / CLC-STEP05** | Nearest/named static targets, cleanup and default/STRICTC errors locally delivered. | Group 3 supplies admitted static diagnostic recovery; INTERPRET invalidation: STEP11. SIGNAL/CCOND transfer integration delivered. |
| <a id="current-action-iterate"></a><a id="current-iterate-proof"></a>**LC-I-10 ITERATE / CLC-STEP05** | Nearest/named targets and selected UNTIL/advance/entry continuation locally delivered. | Same observation/dynamic owners as LEAVE; preserve final FOR advance and selected owner. |
| <a id="current-action-arg"></a>**LC-I-11 ARG / CLC-STEP06** | Complete-template `PARSE UPPER ARG` with raw presence and live shared/private pool composition locally qualified. | ARG-AC05: wider source/host/delivery/platform closure remains; CCOND supplies errors/resumption/clause/TRACE. |
| <a id="current-action-procedure"></a>**LC-I-12 PROCEDURE / CLC-STEP06** | Static shared entries, executed eligibility, ordered EXPOSE and normal/fatal restoration locally qualified; CPROC-AC01–06. | CPROC-AC07: remaining source forms, callbacks/dynamic eligibility and host/delivery/platforms; CCOND supplies admitted handler observations. Whole LC-I-12 remains open. |
| <a id="current-action-call"></a>**LC-I-13 CALL / CLC-STEP06** | Local/BIF/Classic-provider, ordinary scalar B/G/RXPA and explicit process statement/expression calls are locally qualified with Group 2; CCOND owns delayed handlers. | Group 3 supplies admitted static source/diagnostics; dynamic source loading and declared platform/delivery acceptance remain open. |
| <a id="current-action-return"></a>**LC-I-14 RETURN / CLC-STEP06** | Linear continuation, contextual35.1, owned result presence and existing cleanup/status locally qualified; RRET-AC01–05. | Group 2 supplies host completion; full traceback/source catalog and declared platform/delivery proof remain STEP04/10; whole LC-I-14 remains open. |
| <a id="current-action-exit"></a>**LC-I-15 EXIT / CLC-STEP06** | Owned optional result, visible program-root propagation and forced cleanup locally qualified; CEXIT-AC01–05. | Group 2 supplies static provider/callback/host completion; CEXIT-AC06 retains declared platform/delivery and dynamic proof; CCOND supplies delayed-handler EXIT. Whole LC-I-15 remains open. |
| **LC-I-16 PULL / CLC-STEP06** | Shared acquisition/UPPER templates, bare consumption and live/private queue composition locally qualified; PULL-AC01–04. | PULL-AC05: wider configured host behavior and declared platforms; STEP08. CCOND supplies condition/TRACE integration. |
| **LC-I-17 PUSH / CLC-STEP06** | Optional/bare values, front insertion and operand EXIT/private restoration locally qualified; PUSH-AC01–04. | PUSH-AC05: wider host failures, selection/isolation and declared platforms; STEP08. CCOND supplies condition/TRACE integration. |
| **LC-I-18 QUEUE / CLC-STEP06** | Tail/FIFO insertion, consumed-slot/mixed-order and operand EXIT/private restoration locally qualified; QUEUE-AC01–04. | QUEUE-AC05: wider host failures/queue isolation and declared platforms; STEP08. CCOND supplies condition/TRACE integration. |
| <a id="current-action-parse"></a><a id="parse-cursor-local-qualification"></a>**LC-I-19 PARSE / CLC-STEP06 — prerequisite delivered ahead** | Seven sources/complete templates, one shared cursor/executor and C integration locally delivered. | CPARSE-AC07/08, engine real-platform proof and whole ARG/PULL acceptance remain open; STEP03/07/08/10. |
| **LC-I-20 ADDRESS / CLC-STEP07** | Explicit selection/swap, transient commands/WITH, lexical anchors and owned command lifecycle locally qualified; CADDR-AC01–05/07 explicit profile. | CADDR-AC08: native host/NUL transport and declared platforms remain; CCOND supplies conditions/resumption/TRACE. Whole LC-I-20 stays open. |
| **LC-I-21 implicit command / CLC-STEP07** | Active-ADDRESS commands, lasting connections and owned source warnings locally qualified; CADDR-AC01–07 local profile. | Host/NUL transport and declared platforms: CADDR-AC08 / STEP08. Conditions/resumption/TRACE use CCOND. |
| **LC-I-22 NUMERIC / CLC-STEP07** | Default/fixed/dynamic forms, once-only shared validation, frame installation and call restoration locally qualified; NUM-C-AC01–06. | NUM-C-AC07: broader diagnostics/host/platform scope and numeric fact invalidation; STEP02/04/08. Conditions/LOSTDIGITS/TRACE use CCOND. |
| **LC-I-23 SIGNAL / CLC-STEP07** | **Complete:** all transfers, seven-condition ON/OFF, actual producers, same-frame cleanup and typed relay; CSIGNAL-AC01–07. | Group 1 integrates delayed CALL; Group 2 supplies real host HALT ingress. Broader host/platform acceptance and INTERPRET retain their named qualifications. |
| **LC-I-24 TRACE / CLC-STEP07** | **Complete:** agreed practical options/query, command inhibition/reporting, source/live-value observations and observer restoration; CTRACE-AC01–07. | Accepted practical/presentation limitations remain explicit; formal whole-compiler/platform/delivery acceptance stays open. |
| **LC-I-25 INTERPRET / CLC-STEP11 — last** | Locally qualified through the same C frontend and neutral L IR executor. | Live caller state, CALL resumption/SIGNAL discard, source-program roots and bounded ownership use shared runtime services. Formal LC-87/CFC/platform/delivery gates remain open. |

## Delivered conditions and observations: CLC-STEP07

[Complete SIGNAL](doc/levelc-architecture.md#signal-complete-contract) includes every
agreed source form, seven-condition ON/OFF, actual producers, typed payload/origin
transport, visible exception flow and same-frame cleanup, together with its
required helpers/dependencies. CSIGNAL-AC01–07 and STEP01–03 are complete.

[Whole TRACE contract](doc/levelc-architecture.md#trace-complete-contract):
CTRACE-STEP01–03 and AC01–07 are complete for the approved native practical
ceiling. Early coherent source/representative RXAS review and the focused
Debug opt compiler, IR-on, assembler-opt rxbvm checks qualify this local
feature. C21 introduced `trace(value,context)`; shared B retains option/command
semantics and the private generated handler reuses its trace services. Formal
compiler QA follows feature completion; all CFC criteria remain open.
INTERPRET is the final language increment; its explicit STEP11 approval below
extends the earlier standing authority.

Adrian authorized the remaining agreed C instructions/features stepwise, with
internal contract review and a local commit after each, stopping before
INTERPRET without further user approval. Scope exceptions retain their gates.
PROCEDURE1–4 remain complete for CPROC-AC01–06; CPROC-AC07 stays open.
[CALL contract](doc/levelc-architecture.md#call-contract) owns CCALL-AC01–07.

| Stable feature | Status / dependency |
|---|---|
| **CLC-STEP06.CALL1** | Complete contract review. |
| **CLC-STEP06.CALL2** | Locally qualified ordinary profile, CCALL-AC01–04/07; whole CALL stays open. |
| **CLC-STEP06.CALL3** | Locally qualified [CEXT-AC01–05 / STEP01–03](doc/levelc-architecture.md#call3-static-provider-contract), with the generic classifier prerequisite. Fresh pool/config and inherited activation state. Group 2 extends packet calls to expressions, scalar typed/native providers and configured callbacks; Classic source loading remains with INTERPRET. |
| **CLC-STEP06.CALL4** | Delivered with CCOND: shared policy/checkpoints, local/BIF/signed handler lifecycle and delayed resumption. Wider CCALL-AC07 host/platform scope remains open. |
| **PULL-STEP01–03** | Locally qualified shared-parser profile, PULL-AC01–04, with CCOND condition/observation integration; AC05 retains wider host/platform work. |
| **PUSH-STEP01–03** | [Selected-queue profile](doc/levelc-architecture.md#push-contract) locally qualified for AC01–04; AC05 stays shared scope. |
| **QUEUE-STEP01–03** | [Tail-insertion profile](doc/levelc-architecture.md#queue-contract) locally qualified for AC01–04; neutral insertion controls are shared with PUSH. AC05 stays shared scope. |
| **ADDR-B-STEP01–03** | [Shared B binding bridge](doc/levelc-architecture.md#address-b-binding-bridge) locally qualified for AC01–05 as a separate prerequisite. AC06 keeps C/host/condition scope open. |
| **CADDR-STEP01–02** | [Keyed explicit ADDRESS profile](doc/levelc-architecture.md#address-c-command-contract) locally qualified for AC01–05/07 explicit scope, with the separate B prerequisite. AC08 stays open. |
| **CADDR-STEP03** | Locally qualified implicit C16 commands and owned source warnings through the same command owner; AC01–07 local profile. AC08 stays shared scope. |
| **NUM-B-STEP01–03** | Locally complete: [shared FORM diagnostic prerequisite](doc/levelc-architecture.md#numeric-form-diagnostic-prerequisite) corrects the sole validator's `33.3` identity before C source extension; no numeric algorithm change. |
| **NUM-C-STEP01–03** | [NUMERIC source-completion profile](doc/levelc-architecture.md#numeric-c-contract) locally qualified for AC01–06. AC07 shared closure remains open. |
| **COND-B-STEP01–03** | [Shared external condition scope prerequisite](doc/levelc-architecture.md#external-condition-b-prerequisite) locally qualified for AC01–05. C/provider normalization, immediate SIGNAL and delayed CALL integration are delivered; wider AC06 host integration remains open. |
| **CLABEL-STEP01–03** | [Label/arm and local-entry prerequisite](doc/levelc-architecture.md#label-arm-prerequisite) locally qualified for AC01–05; C17 schema/runtime unchanged. Its inventory supports complete SIGNAL. |
| **CLAUSE-STEP01–03** | [Clause-clock foundation](doc/levelc-architecture.md#clause-clock-contract) locally complete. CCOND adds authored completion, handler-independent samples and field/drop observations under practical TRACE. |
| **CSIGNAL-STEP01–03** | [Complete SIGNAL](doc/levelc-architecture.md#signal-complete-contract): STEP01–03 and CSIGNAL-AC01–07 complete for the agreed instruction, including all required helpers/dependencies. Named cross-cutting qualifications remain explicit. |
| **CTRACE-STEP01–03** | [Complete practical TRACE](doc/levelc-architecture.md#trace-complete-contract): STEP01–03 and CTRACE-AC01–07 complete for the agreed practical contract, including metadata/observer dependencies. Full-C and formal QA criteria remain open. |
| **CCOND-STEP01–03** | [Conditions and observations](doc/levelc-architecture.md#conditions-observations-complete-contract): complete for CCOND-AC01–08, including CALL4 and required shared-B dependencies. Full-C and formal QA criteria remain open. |

No native C frontend implementation, second runtime algorithm, push or hosted
campaign is commissioned. Needed generic exception representation and the
existing SIGNALORIGIN payload correction follow the internally reviewed whole
contract. INTERPRET remains excluded from the standing authority.

The [RETURN contract](doc/levelc-architecture.md#return-contract) owns RRET-AC01–05;
**CLC-STEP06.RETURN1–3** are complete for the local source/completion profile.
[EXIT](doc/levelc-architecture.md#exit-contract): **CEXIT-STEP01–03** complete for
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
group subcriteria make their delivery traceable without replacing those outcomes
or altering decision gates.

**Group 1 — locally complete.** The
[whole condition/observation contract](doc/levelc-architecture.md#conditions-observations-complete-contract)
and required C/L/shared-B dependencies are delivered under CCOND-AC01–08 and
STEP01–03. Qualification reuses native fixture sources/checked expectations,
independent contract supplements and prepared products in one Debug
opt/IR-on/rxbvm lane. Formal compiler QA follows feature completion. Native C
remains read-only, no push is authorized, and INTERPRET stays last.

| Group | Status / remaining work | Existing owners |
|---|---|---|
| **Conditions and observations** | **Complete local increment:** CALL ON/OFF, delayed delivery/resumption and buffered HALT; configured conditions, SIGL/CONDITION/origins, authored clause/clocks and field/drop observations, including handler pool/resource restoration. Wider host producers/platforms and dynamic execution retain the groups below. | CCOND-AC01–08 / STEP01–03; CALL4; C-OBS; condition portions of STEP03/07 and instruction criteria. |
| **Calls and host integration** | **Complete local increment:** [Group 2 contract](doc/levelc-architecture.md#group2-calls-host-proposal) and [steps](#group2-calls-host-plan). Delivered static internal/external call and activation semantics: arguments/omissions, RESULT and RETURN/EXIT completion, PROCEDURE/private/shared pools and EXPOSE aliases, inherited/restored numeric/ADDRESS/TRACE state, provider/callback entry and teardown. Preserve already-qualified ordinary recursion and call behavior. Metadata-guided ordinary scalar B/G/RXPA calls are delivered; B/G exposed globals remain inaccessible to C, true reference values require explicit adapters. COMMAND/FUNCTION/SUBROUTINE host invocation supplies exact-length/empty/NUL arguments/results, live pool access windows, initial ADDRESS/stream/queue selection, I/O adapters and NUL transport, real clock/HALT producers, resource failures and concurrent-context isolation. Declared platform/delivery proof remains open. CALL ON/OFF and event delivery remain with conditions. | C-ACT/C-HOST; call portions of C-POOL; STEP03/06/08; CPROC/CCALL/CEXT/RRET/CEXIT and host portions of queue/stream/ADDRESS criteria; BF05/06/08/09 integration dependencies. |
| **Language functionality: source, expressions and BIFs (Group 3)** | **Complete local static increment:** source/header controls, Unicode names/source inventory, operators/numeric semantics, exact diagnostics/dead-source validation, variable/stem dependencies and the 95-name BF01–09 integration over shared B bodies. Ordinary Classic casing preserves the Latin-1 input range; higher characters use Unicode codepoints. INTERPRET and automatic Rexx source/program invocation remain last, with call/lifecycle integration and compiler support supplied by their owners. | C-SRC/C-EXPR/C-NUM/C-POOL/C-UNI; STEP04–06; BF01–09 language contracts; STEP11 last. |
| **Compiler and emitter** | **Group 4 STEP01–08 locally delivered for their recorded profiles:** optional neutral L analyses/proofs, RXAS records/allocation and bounded IR execution, with C integration and four-image package closure. The [architecture](doc/levelc-architecture.md#group4-infrastructure-proposal) owns conservative lifetime/control limits. Full G4/CFC acceptance remains open; further C-OPT opportunities retain their owners. Native seed repairs are implemented under BASELINE-QA below. | G4-AC01–10 / STEP01–08; STEP01/02/09; C-ABI; C-OPT-01–06; CFC-AC07/10–12; B-SEED-ALIAS; B-SEED-INLINE-RECEIVER. |

Calls and host integration owns activation/pool/alias entry, inheritance,
arguments/results, completion and teardown, including later INTERPRET integration.
Conditions owns when handlers transfer or resume execution; compiler work owns
the supporting emitted ownership/cleanup mechanisms. Each mechanism retains one
owner, and locally qualified recursion/PROCEDURE/EXPOSE is not reopened.

### Static language baseline: Group 3 locally complete

Groups 1 and 2 and Group 4 STEP01–08 are locally delivered for their recorded
profiles. Group 3's static source/expression, retained-source/SRCMAP and BF01–09
implementation is locally qualified under the reviewed contract below. Early
coherent source/representative RXAS review and one normal Debug C-opt/IR-on/rxbvm
component lane cover values, exact errors, state, ownership, limits and recovery.
Shared B owns values, pools, Unicode and BIF algorithms; C owns source policy and
integration. Native fixture sources and
checked expectations supply independent controls, without native C frontend edits.

INTERPRET and automatic Rexx source/program invocation were excluded from Group 3
and are now locally delivered in STEP11 below under its explicit approval.
Existing signed/prepared Classic and generic external calls stay
with Group 2. Full CFC/formal/platform/delivery acceptance remains open; the
separate DFA construction backlog is not a prerequisite.

INTERPRET retains its final position and existing contract/approval gates.
Formal whole-compiler QA follows feature completion. STEP10 delivery checks
(CLI/service, installation, cold/incremental/relocated builds and declared
platform proof), the complete ledger crosswalk and CFC-AC01–12 remain explicit
acceptance work across these groups; they are not additional language features.

<a id="dynamic-execution-plan"></a>

### Current action: STEP11 dynamic execution implementation

**IR route implemented and locally qualified.** The
[architecture](doc/levelc-architecture.md#dynamic-execution-strategy) selects the neutral
L executor, shared Classic services, an inline compiled-owner dispatcher and a
checked VM invocation service. Runtime source requests use no assembler or native
source compiler. Static compilation retains RXAS; the Rexx assembler is separate
programme work. Preserve LC-87 and CFC-AC05/10/11/12. Implementation uses early coherent source/RXAS review and focused component
checks. The selected component profile is complete; the wider registered
inventory and formal compiler QA remain the later named gate.

The detailed STEP11a–d sequence below replaces the earlier assembly/image-group
implementation proposal. Approval covers its native invocation/resource contracts
and concrete source-search policy. Begin with the minimal mechanism proof before
full integration, preserve unrelated work and commit locally without pushing.
Formal compiler/platform/delivery criteria remain open.

The selector's ordinary helper-call windows reserve every fixed formal ABI slot,
including implicit receivers and omitted optional arguments. Missing optionals
are cleared with the native B absent-argument flags; physical counts and window
sizing include them. This repairs the reproduced `scope.close()` teardown fault
and the same `resume()` boundary without changing the native VM or source frontend.

Empty `Sequence` bootstrap operations derive their provenance from the complete
owned frontend input, including comments and whitespace. This preserves strict
SIGNAL admission without manufacturing authored clauses. The dynamic PARSE
adapter passes caller metadata only for SOURCE, in SOURCEINFO form, leaving
source mode/path normalization with the shared configuration owner. A fresh
external source child detaches its inherited host-completion association before
its own host Scope enters; the parent remains live and Scope retains ownership
of live bindings. These narrow integration repairs reuse the prepared applications
and receive affected runtime checks only.

RexxScript is the other intended execution consumer: an untyped, simplified G
syntax/operator profile with simplified classes in its vision, retaining shared
Classic BIFs. Its language/class/consumer migration design belongs to the
[Level L/RexxScript roadmap](../PLAN.md#accepted-first-product-direction--rexxscript-10-october).
It is not a second Classic evaluator or a prerequisite to implementing every
Classic instruction. Existing implementations are reuse and migration evidence;
their organic language details do not define the new product.

<a id="group3-static-language-plan"></a>

### Group 3: complete static language increment

**Locally complete as one static implementation increment.** Deliver the remaining
agreed static C source, expression/numeric, variable/instruction and BF01–09
contracts through the existing B/L compiler and shared runtime. Reuse native
fixtures and checked expectations first; investigate disagreements independently.
INTERPRET, automatic Rexx source/program calling and new platform/host policy are
excluded. Existing Group 2 signed/prepared calls remain supported. Formal QA and
CFC/platform/delivery acceptance remain open; the DFA backlog is independent.
Adrian confirmed the ordinary Classic Latin-1 compatibility rule: casing of
codepoints 0–255 stays within that range; higher characters retain Unicode
codepoint behavior, and LENGTH/positions count codepoints. Explicit Unicode
functions retain their full contracts. The shared Classic table generator owns the
case boundary for native C and the B-implemented Classic runtime, qualified by
independent all-256 and higher-character controls. This is a C compatibility
policy, intended for later O reuse, and changes no B/G language or generic
Unicode semantics. Lexer categories and the generated grammar are unchanged.

| Stable criterion / status | Observable local closure |
|---|---|
| **G3-AC01 — locally qualified** | Complete admitted static source/header/token forms, contextual words, Unicode names/constants, strings/radix, comments/continuations, exact labels and dead-source validation. Preserve original bytes/scalar columns; authored 6.*, 7–21, 31.*, 35–38, 46.*, 50.1/51.1 and existing instruction errors have located independent controls. |
| **G3-AC02 — locally qualified** | Complete prefix/power/arithmetic/concatenation/normal+strict comparison/logical families use shared contextual values, Classic precedence/left-associative power, eager once-only operands, numeric guards/rounding/LOSTDIGITS and original failures. No second arithmetic/string algorithm or native C frontend call. |
| **G3-AC03 — locally qualified** | Remaining static variable/stem/reserved/alias and instruction dependencies use the shared live pool and retain Group 1/2 state, observations, control and cleanup; source inventory, SOURCELINE and mapped diagnostic/TRACE metadata never reread files or invent original text. |
| **G3-AC04 — locally qualified** | All 95 existing BIF bindings have a checked new-C value/context/error crosswalk using BF01–09 and maintained sources/oracles. Omitted/empty/NUL, numeric/text/ordinal/configuration, pools, source, queues/streams, clock/RNG and nested/root isolation use the existing owners. Unchanged library proof is reused; shared bodies change only for a demonstrated defect/unfinished contract. |
| **G3-AC05 — locally qualified** | C24 appends the reviewed operations without changing existing identities; raw/loaded admission, schema/target boundaries and source-map declarations are checked and bounded. Prior output survives source/import/limit failure; no target dependency on compiler or L. |
| **G3-AC06 — locally qualified** | Early coherent source/representative simple and effectful RXAS review, one prepared normal Debug/C-opt/rxbvm component lane, meaningful negative/state/cleanup/limit controls, enduring docs and one local commit qualify the increment. No push/full compiler QA/mode matrix/sanitizer campaign. |

| Stable step / status | Work, dependencies and decision gate |
|---|---|
| **G3-STEP01 — done** | Record and internally review the complete [static contract](doc/levelc-architecture.md#group3-static-language-contract), independent expectations, exact interfaces and disjoint ownership before product edits; AC01–06. Preserve existing public language/API/ISA/platform gates. |
| **G3-STEP02 — done** | Batch grammar/token policy, static diagnostics/validation and C24 expression/schema/selector adaptation. Integrate the reviewed C-owned source-map adapter; regenerate the Unicode grammar once after the coherent source review; AC01/02/05. |
| **G3-STEP03 — done** | Complete source inventory and BIF/pool/configuration integration over existing runtime owners; no blanket BIF rewrite. Review representative RXAS early and batch material fixes; AC02–05. |
| **G3-STEP04 — done** | Qualify the complete finite source/operator/pool/BF01–09 crosswalk with prepared tools and independently checked expectations; reuse valid unaffected Group 1/2/4 results and rerun only affected fixes; AC01–06. |
| **G3-STEP05 — done** | Update enduring architecture/conformance/crosswalk and simple status, preserve unrelated work, delete scratch and commit locally. Full CFC/formal/platform/delivery QA and final INTERPRET stay open; AC06. |

<a id="group2-calls-host-plan"></a>

### Group 2: calls and host integration

**Locally complete as one static calls/host increment, including expression calls.** The
[architecture contract](doc/levelc-architecture.md#group2-calls-host-proposal) separates
callee-owned ABI/behavior, C value adaptation, shared Classic runtime and
generic metadata/VM/platform capabilities. It deliberately extends the current
statement-only signed activation-provider profile to ordinary scalar typed and
RXPA procedures and external expressions. B/G exposed globals are inaccessible
to C; true reference values use explicit adapters, as confirmed by Adrian.
Executable CALL uses approved explicit process bindings. Adrian approved
exit status as the result, with stdout/stderr inheriting the current host streams
and no new CALL redirection syntax. He requires exactly the same low-level
process/byte-channel capabilities as ADDRESS; no second launcher. Automatic
Classic source calls stay with
STEP11/INTERPRET. No new native frontend service, loader/search policy,
unspecified native API/ISA or platform exclusion is implicitly approved.

The outcome is one complete static calls/host increment, preserving qualified
recursion and CCOND while completing invocation, callback pools and configured
host services. These subcriteria route existing C-ACT/C-HOST/C-POOL and
CFC-AC03/04/09–12 obligations. The selected development profile is qualified;
full criteria retain their wider compiler, delivery and declared-platform proof.
Coherent source and representative RXAS review, metadata/ABI and native span/clock/
interrupt controls, maintained native call fixtures and focused host/process
consumers qualify one prepared normal Debug, C opt/rxbvm lane. Relevant valid
results were reused; affected fixes received only their own checks. Formal QA
remains open, with no mode matrix, sanitizer campaign or publication.

| Criterion / status | Observable pass condition and proof |
|---|---|
| **G2-AC01 — open** | Local/BIF/Classic-provider/ordinary-provider/explicit-process resolution preserves quoting, precedence, ambiguity, missing-versus-unavailable errors, once-only source-order actuals and dead-source admission. External bindings share one alias namespace; no implicit PATH/source fallback. |
| **G2-AC02 — open** | One checked metadata owner supports admitted scalar B/G/RXPA signatures, build-time arity/presence refusal, callee defaults and scalar varargs, runtime value conversion, exposed temporary cells without copyback, result/void distinction and provider identity. Reject implicit arrays/objects/true references and direct B/G global access. Independent metadata/conversion and RXPA controls prove the ordinary ABI. |
| **G2-AC03 — open** | Internal/external roots, pool/PROCEDURE/EXPOSE, RESULT/no-result, RETURN/EXIT and numeric/ADDRESS/TRACE/condition inheritance/restoration follow the explicit state table. Retain recursion; reject foreign handler ordinals and expired borrows on every completion. |
| **G2-AC04 — open** | COMMAND/FUNCTION/SUBROUTINE requests preserve exact-length/empty/NUL values and omissions, initial state/resources/traps, and distinct present/absent/condition/resource/unable-to-continue outcomes. Owned completions outlive the invocation; transport failure remains separate. |
| **G2-AC05 — open** | Enabled synchronous callback windows delegate substituted/direct pool set/value/drop, other-pool access and visible iteration/reset to the existing pool owner. Nested restoration and disabled/stale/foreign/teardown refusal are exercised without granting ordinary typed calls implicit pool access. |
| **G2-AC06 — open** | Selected ADDRESS, queues and stream/I/O adapters preserve byte/text/codec distinctions, positions, EOF/close, FIFO/LIFO, RC/.RC/.RS, command conditions, exact callback spans and process argv. Executable CALL reuses exactly ADDRESS's child/byte-channel owners with approved argument/result policy. OS NUL refusal, channel errors, failure/cancellation and cleanup use shared owners and specified mappings; no accidental stdout/status conflation. |
| **G2-AC07 — open** | Real clock and HALT ingress reuse existing platform samples and execution-local interrupts, preserving frozen clause samples, current UTC TIME(E/R), CCOND delivery/buffering and worker isolation. Configuration hooks prove replacement, null/fallthrough, trap overrides and resource/unavailable behavior. |
| **G2-AC08 — open** | Context reuse, nested callbacks, concurrent independent contexts, bounded limits and failure/cancel/teardown retain owners and release all windows/resources without stale state, global defaults or cross-context VM values. |
| **G2-AC09 — open** | Early coherent source and representative simple typed-call RXAS review, focused positive/negative/state/cleanup fixtures and actual compile/assemble/link/run consumers qualify changed boundaries in one normal Debug/rxbvm lane. Reuse maintained native source/oracles and directly affected B/G consumers; no automatic mode matrix/full compiler QA. |
| **G2-AC10 — open** | Public interface/capability/platform rows name exact ownership, failures and delivery. New native API/ISA contracts receive explicit approval if demonstrated necessary. Declared macOS/Linux/Windows/CMS/TSO/embedded proof and broader delivery/formal QA remain open until exercised; no unavailable facility becomes an implicit exclusion. |

| Step / status | Scope, dependency and decision gate |
|---|---|
| **G2-STEP01 — complete** | Approved complete architecture, scalar/void/default/vararg rules and explicit process-binding/argument/failure policy, including expression calls and executable exit-status result/inherited streams. Record/review exact component interfaces before product edits; new native API/ISA gates retain their contract scope. AC01–10. |
| **G2-STEP02 — complete locally** | Implement bounded generic metadata/manifest projection, explicit process binding admission and ordinary scalar typed/RXPA CALL/expression adaptation, reusing native ABI/runtime services; AC01/02. Exact B-facing projection/binding contract first. |
| **G2-STEP03 — complete locally** | Complete activation and runtime-owned host request/result/export entry, modes/presence/completion and inherited/restored state; AC03/04/08. Prefer existing checked RXVML calls; demonstrate any missing outcome transport before new API approval. |
| **G2-STEP04 — complete locally** | Integrate live callback pool windows and configured hook/trap/resource selection over existing owners; AC05/07/08. Exact window/reentry/expiry contract first. |
| **G2-STEP05 — complete locally** | Complete shared I/O/queue/stream/ADDRESS adapters, current channel condition mapping, real clock/HALT ingress and context isolation; AC06–08/10. Public span/interrupt/clock interfaces retain exact approval gates if required. |
| **G2-STEP06 — complete locally** | Integrate all routes, review coherent source and first representative RXAS early, batch material fixes, then freeze the increment. AC01–09. Preserve CCOND and existing ordinary/private recursion rather than reopening their whole suites. |
| **G2-STEP07 — complete locally** | Run selected component and boundary checks once using prepared Debug/rxbvm products, review affected fixes only, update enduring contracts/simple status and commit authorized paths locally. AC09/10; no push. Formal compiler/platform/delivery QA remains later; INTERPRET stays last. |

<a id="group4-infrastructure-plan"></a>

### Group 4: enabling architecture and delivery

**Approved implementation: G4-STEP01–08.** Adrian selected Group 4 next and clarified that Level L may supply
specific RXAS/instruction and cREXX IR support alongside generally reusable
infrastructure. Keep a neutral core plus optional target/semantic packages,
with one algorithm/data owner and no reverse dependency on a source frontend.
Serve the current C compiler first, preserve existing typed consumers, and make
the contracts usable by later B/G, Classic/O, assembler and RexxScript work.
Those future product migrations are not silently added to C-first delivery.
Use Sol 6.1 Extra High workers, proportionate component checks and a local
commit without pushing. STEP07 is a separate bounded execution feasibility
increment; STEP08 completes package/C integration under the
[recorded closure contract](doc/levelc-architecture.md#group4-package-closure-contract).
Formal compiler QA and INTERPRET remain later.

STEP01–08 are locally delivered for their recorded profiles. Open G4 criterion
labels retain wider capability/optimization outcomes and later assurance, not
another pending package/C migration. Accepted conservative profiles, named
C-OPT/native owners and full CFC criteria are not closed by this development
qualification.

The outcome is an explicit semantic-IR and target-IR pipeline: shared flow and
variable/lifetime facts support checked transformations; target liveness and
RXAS constraints determine register reuse; the same semantic IR can optionally
run through a bounded execution engine with language-owned runtime services.
The [architecture](doc/levelc-architecture.md#group4-infrastructure-proposal)
distinguishes present capabilities from missing services and exact decision
gates. This does not approve new opcodes, wire formats or INTERPRET implementation.

| Criterion / status | Observable pass condition and proof |
|---|---|
| **G4-AC01 — open** | Core, optional cREXX/RXAS packages, source frontends, runtime and native ISA owners have acyclic source/build/install closures. Independent non-cREXX consumers work without either optional package; existing G/C signatures and bootstrap remain valid. |
| **G4-AC02 — open** | One indexed IR graph/analysis service supplies def/use, reachability, predecessor/successor queries, dominance, demanded loop structure and value/slot liveness, including irreducible cycles and declared exceptional/resumable execution. Hand-derived independent graphs establish facts, limits, stale-owner rejection and atomic failure. |
| **G4-AC03 — open** | Versioned structured effects and checked proof results distinguish core slots, runtime pool bindings, aliases, borrows/escapes, ownership, result availability and observations. Actual mutation/handler/callback boundaries invalidate the right facts; unknown premises prohibit the affected optimisation. A fact-map insertion is not accepted as a proof. |
| **G4-AC04 — open** | Typed RXAS records carry all explicit/implicit uses, conditional definitions, register components, calls/windows, reference/cleanup and debug/TRACE observations before allocation. Opcode/effect/signal data has one canonical binutils owner, with checked SDK views and maintained native instruction/flow fixture crosswalks. |
| **G4-AC05 — open** | Deterministic allocation reuses dead temporary and eligible slot storage while preserving aliases, storage-cell lifetime, argument presence, numeric state, conditional results and cleanup. Independent interference/assignment checks and representative RXAS show frame demand follows live constraints; budget failure preserves prior output. |
| **G4-AC06 — open** | C-OPT-01–06 have explicit ownership and closure checks. Remove direct construction defects with optimisation off; use proved effects/availability for optional rewrites. C retains language premises; optional RXAS owns machine legality; unchanged runtime algorithms remain single-owned. |
| **G4-AC07 — complete for the bounded prototype** | A bounded optional IR runner executes admitted core/registered operations through trusted runtime handlers, with exact values/presence, calls, failures, observations, cancellation and owned suspended continuations. Unsupported capabilities fail before effects. An independent non-cREXX case and a small effectful/resumable case establish feasibility; constant-fold callbacks are not used as a runtime evaluator. |
| **G4-AC08 — prototype decision complete; IR direction now approved** | The bounded feasibility assessment remains qualified for its recorded profile. Adrian selects semantic IR for final Classic INTERPRET and as the reusable engine direction for future G-derived RexxScript. Detailed integration/mechanism proof remains STEP11 below; current products are not yet replaced and all LC-87/CFC-AC05 outcomes remain binding. |
| **G4-AC09 — open** | C consumes the shared pipeline with early source/representative target review and proportionate component checks. Reuse native source/fixtures and independent L expectations; preserve Group 1 conditions/observations and an affected existing typed consumer. No new B/G/O coverage, assembler replacement or native C frontend edit is claimed. |
| **G4-AC10 — open** | Enduring API/architecture/build guidance, simple status and capability boundaries match delivered products. Static targets contain no compiler, allocator or IR runner unless selected. Full compiler/platform/delivery QA and all CFC criteria stay open for their agreed milestones. |

| Step / status | Work, dependencies and gates |
|---|---|
| **G4-STEP01 — complete** | Approved layers, exact effect/control/proof and target-record contracts, canonical instruction ownership, migration/version rules and independent expectations are recorded and internally reviewed; AC01–10. Public API/wire/ISA decisions outside this contract retain their explicit gates. |
| **G4-STEP02 — complete** | Optional `llirgraph` supplies indexed function/CFG/def-use/dominance and demanded SCC/loops/liveness. Protected/incomplete functions expose an explicit all-live/pinned profile; no reuse proof crosses undeclared exception/resumption lifetimes. Core `llirservices` and source-role `llflow` remain compatible; AC02/03. |
| **G4-STEP03 — complete** | `llireffects` supplies versioned sealed premises and checked owner-bound proofs; core effects cannot be overridden. `llceffects` retains the C context-binding proof and pool/handler/callback barriers, with conditional C22 results. Existing protocol owners/order remain intact; AC03/06. |
| **G4-STEP04 — complete** | Optional `llrxas`/`llrxascatalog` admit typed records before allocation, including C/G body, helper, call/window, cleanup and TRACE work. A build-time view reuses canonical native effects/components/signals; quoting/metadata and ISA remain single-owned; AC01/04/09. |
| **G4-STEP05 — complete** | Deterministic whole-cell assignment and an independent constraint/interference validator support same-block plain scalar reuse and proved C context-slot coalescing across blocks. References, owned values, protected/cross-label/loop storage, presence, windows and release points retain conservative identities. C and the existing typed consumer use one target service; no packing/spilling; AC02–05/09. |
| **G4-STEP06 — complete** | C context transport removes redundant cells/copies without an optimization flag; function/owner indices remove repeated membership filtering. Structural checks retain real caller arguments and helper/cleanup costs. C-OPT-01–06 retain the architecture's ownership/closure checks for further setup, numeric, binding, call and machine opportunities; no fully optimized or speed claim. B-SEED-ALIAS stays separate; AC06/09. |
| **G4-STEP07 — complete; bounded feasibility increment** | Optional `llirexecute` implements the [recorded execution contract](doc/levelc-architecture.md#group4-execution-contract). Independent control/presence/bytes and owned Classic VALUE/LENGTH/numeric fixtures cover calls, failure, suspension/resume/discard, cleanup and limits; one frozen graph also emits RXAS against independent expectations. The [suitability decision](doc/levelc-architecture.md#group4-execution-assessment) retains current RexxScript and Classic routes pending full integration proof; AC07/08. No INTERPRET source feature or language migration. |
| **G4-STEP08 — locally qualified** | Four-image SDK stage/inventory, actual package/import/source controls and independent public core/RXAS clients establish the [package closure contract](doc/levelc-architecture.md#group4-package-closure-contract); AC01/09/10. Focused C contracts and maintained condition/SAY/numeric/private-recursion/TRACE sources plus an affected typed consumer preserve the delivered pipeline. STEP02–07 conservative profiles remain explicit. Formal QA follows feature completion; INTERPRET stays last. |

STEP01–06 are one locally qualified enabling increment. Generic graph/effect and
RXAS fixtures, C protocol/limit cases, maintained native C fixtures and an
affected typed control own the component expectations. Public package guides
describe independent use and static runtime separation. STEP07 has a separate
focused Debug/rxbvm prototype qualification and public package contract; its
admitted profile does not execute complete C22 or establish live compiled-caller
ownership. STEP08 locally qualifies package/C integration with exact public
selection boundaries and representative caller/helper RXAS, reusing unchanged
earlier proof. Full G4 criterion closure, the named capability/optimization
outcomes, all CFC criteria and formal compiler QA remain open. These do not
change the order of Groups 2/3 and final INTERPRET.

Each implementation increment uses early coherent source/first-target review,
batched material fixes and the smallest meaningful normal configuration/VM.
Additional engines/platforms/native assurance follow a changed mechanism or an
agreed milestone, not this increment. Ordinary research resource/code-quality
observations do not create a formal performance gate.

<a id="shared-delivery-stages"></a>

### Stable delivery steps

Deliver the minimum shared dependency required by each instruction. These
stages retain full shared acceptance; they do not replace the ordered queue.

| Shared step | Status / remaining work |
|---|---|
| **CLC-STEP01** | Partial: finish every ledger disposition, independent expectations, ABI/platform decisions and test traceability. Exact exceptions need Adrian's approval. |
| <a id="l-construct-01"></a>**CLC-STEP02** | Partial: L exceptional/resumable edges, effects/facts/invalidation, liveness, bounded atomic failure and checked rewrites. Keep block-local values/cross-block slots; needed L APIs receive internal contract review under the standing authority. Generic lexical reuse and CMETA-STEP01–03 scalar attribute-index reading are delivered; possible DFA construction improvements remain in the separate [Level L backlog](../PLAN.md#dfa-construction-backlog), with no prerequisite on Group 3. |
| <a id="active-first-implementation-increment"></a><a id="shared-b-unicode-interface-and-c-adapter-migration"></a><a id="active-c-contract-baseline-increment"></a><a id="active-borrowed-classic-context-increment"></a>**CLC-STEP03** | Shared B Unicode/borrowed context and Group 2 static activation, recursion, pool/alias entry, restoration and host ownership are locally delivered. Group 3 supplies the admitted static language/pool/BIF contracts; dynamic lifetime and declared delivery/platform proof remain open. Preserve existing B/G/public metadata and one algorithm/data owner. |
| **CLC-STEP04** | Group 3 locally supplies admitted static source/Unicode names, exact byte/scalar origins, dead-source validation, expressions/numeric semantics and BF01–05 integration. Configured-source/dynamic, platform/delivery and formal acceptance remain open; no native frontend dependency. |
| **CLC-STEP05** | Group 3 locally completes admitted static source/expression dependencies of LC-I-01–10; the instruction table retains dynamic/platform/delivery qualifications. |
| **CLC-STEP06** | Local LC-I-11–19 profiles, CCOND delayed handlers and Group 2 static call/host completion are delivered. Group 3 supplies admitted static source/expression/BIF integration; dynamic and ARG/PARSE/PULL delivery/platform obligations stay explicit. |
| **CLC-STEP07** | SIGNAL, practical TRACE and Group 1 conditions/observations locally qualified. Group 3 locally supplies source inventory/diagnostics and BF07/09 integration; host/platform/delivery acceptance retains its owners. |
| **CLC-STEP08** | Group 2 static host implementation is locally delivered: exact-length invocation/completion, callback pools, queues/streams, clocks/HALT and lifecycle. Group 3 supplies static source inventory/BIF integration; declared platform, delivery and formal acceptance proof stay open. New host API/platform exceptions retain their gates. |
| **CLC-STEP09** | Open throughout: C-OPT-01–06 and actual caller/helper RXAS review, truthful source/TRACE, bounded registers/call windows and cleanup. Fix construction defects; document necessary costs. Needed ISA/storage contracts receive internal review under standing authority; production performance edits retain their qualification gate. |
| <a id="step07a-validation"></a>**CLC-STEP10** | Not started: complete static ledger and CLI/service/install/cold/incremental/relocated/platform delivery, independent walkthrough/review. Static acceptance is not complete C. |
| **CLC-STEP11** | Last language increment: same-frontend IR INTERPRET and Classic source calls after static functionality, with detailed design approval and early bridge proof. Formal STEP10/CFC/platform/delivery QA follows feature completion; a local increment does not close those criteria. |

**L-VIEW-01 — locally qualified, CLC-STEP09:** the
[retained operation-view contract](doc/levelc-architecture.md#retained-c-operation-views)
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

### STEP11: same-frontend dynamic IR execution

**Implemented and locally qualified as the final language increment.**
The [architecture](doc/levelc-architecture.md#dynamic-execution-strategy) is the single
contract owner. This replaces the earlier assembly/load/ephemeral-image mechanism,
not the LC-87 outcomes. Local component and boundary checks qualify this increment;
they do not close formal whole-compiler, installed or platform acceptance.
Existing Group 1–4 local results are reused where unchanged.

| Stable criterion | Observable pass condition / selected proof |
|---|---|
| **DYN-AC01** | A frontend-only checked result prepares complete `program` or label-free `interpret-group` IR through the same C grammar/lowering and shared preparation. Source is evaluated once, exact Unicode/NUL and implied delimiter rules are enforced, empty/nested/complete groups work, malformed/dead source is diagnosed before effects of the newly compiled source. Prior source-expression/actual effects are preserved. Raw/loaded/rewritten execution manifests are checked; no native source/assembler fallback. |
| **DYN-AC02** | Groups borrow the authoritative context/ARG/pool binding. First generated PROCEDURE, empty/nested eligibility and existing placement/errors have independent controls. Its committed private pool is invocation-owned and survives group completion/SIGNAL, then restores on actual invocation exit. EXPOSE aliases and failed setup leave no dangling bindings. |
| **DYN-AC03** | Existing compiled body ABI and static-only profile remain valid. Inline owner dispatch executes local/compiled/IR calls and handlers, preserves omissions/results and resumes once. RETURN/EXIT/SIGNAL reach the correct owner; child SIGNAL leaves the parent group suspended, outer loops are inactive, nested requests and recursion retain actual invocation identity. |
| **DYN-AC04** | Dynamic ON/OFF policies survive group completion, plan eviction and internal inheritance into later static clauses. Protection/checkpoints exist without authored static handlers; delayed CALL preserves captures/RESULT/CONDITION/clocks. SIGNAL discards only crossed owners. Native faults/CANCEL preserve raw payload/origin and release sessions/windows before relay and same-context reuse. |
| **DYN-AC05** | Neutral L protection, declared ports, conditional absence/gates, owned faults, leases, per-frame cleanup and aggregate admission have an independent non-C consumer. Stale/foreign/consumed handles, malformed completion, unsupported effects and absent loads fail without partial publication. No C policy enters generic L. |
| **DYN-AC06** | Checked bytecode/RXPA invocation preserves full metadata/provider identity, original presence, callee defaults/checks, scalar conversions and owned vararg windows. Exposed temporary writes are discarded; true references/objects/tasks/global access remain behind explicit adapters. Pre-entry failure changes no target counter; nonzero numeric data and void are distinct from failure. Shared BIF numeric/context/value behavior remains single-owned. |
| **DYN-AC07** | Local/BIF/prepared binding/hook/source/43.1 order and quotation are verified for CALL and expressions. Directory-major trusted-base/context-cwd/configured-root policy, suffixes, exact paths/case, duplicate roots and found-invalid stop rules match the detailed contract. No implicit PATH, executable/shebang dispatch or native frontend rescue. |
| **DYN-AC08** | Source acquisition has exact bounded reads, typed absence/error/limit outcomes, regular-file/symlink and close semantics. Context cwd/environment changes and resource views remain isolated. Full-program source calls own fresh roots/pools/configurations, approved inheritance, child-local EXIT, result presence and restored caller resources. |
| **DYN-AC09** | One aggregate ledger covers acquisition/preparation/cache/IR/source/frame/value/policy/reader/call-window owners across nested/concurrent work. Cache keys contain exact source, logical identity/mapping, captured resource base/configuration, immutable lexical/binding definitions and ABI/dependency identity; same-path/same-mtime changes and created-after-miss files are visible. All-pinned exhaustion preserves outer runs; stale handles, rollback, cancellation and teardown release ownership. |
| **DYN-AC10** | Numeric state reaches actual operation/call frames and the resumed caller. Authored source, SIGL/CONDITION, retained SOURCELINE, practical TRACE/value events and raw faults remain truthful across compiled/IR boundaries. Coordinator locations never replace interpreted origins. |
| **DYN-AC11** | Static applications without dynamic capability have no compiler/L-executor dependency. Optional installed dynamic closure contains no RXAS selector/assembler/linker/native source frontend; linked/relocated provider retention includes real symbols, full contracts and native identities. Existing B/G/Classic consumers remain regressions; script language/classes and assembler work remain separate. |
| **DYN-AC12** | Early coherent source/representative RXAS and bridge review, batched material repairs and proportionate component qualification establish the whole local feature. Changed native call/frame ownership receives focused normal checks, a representative affected second-engine check and the matching maintained sanitizer check at mechanism qualification. Full compiler/mode/platform/install/sanitizer assurance remains the later named CFC/STEP10 gate, not a default per-step campaign. |

| Stable step / status | Work and dependency / owner |
|---|---|
| **CLC-STEP11a.1 — done** | Detailed packets, profiles/ABI, persistent policy/private-pool ownership, invocation/resource service and source-search contracts are recorded, internally reviewed and approved for implementation. Serves AC01–12; WP-01/03/09/11/12. Mechanism and whole-feature proof remain below. |
| **CLC-STEP11a.2 — done locally** | Build the smallest maintained call/control mechanism: hand-prepared IR and minimal selector-generated inline dispatcher, owned bytecode/RXPA windows, VM boundary/RET/unwind hooks, exact event transport and minimal abort/port support. Prove live-pool mutation, compiled call/resume, owner RETURN/SIGNAL, child isolation, native error/CANCEL and same-context reuse before registering the full C inventory. Qualify changed native frame/window ownership with focused normal, representative second-engine and matching maintained sanitizer checks. Depends on 11a.1; WP-09/11 with one integrator for minimal L/selector edits. |
| **CLC-STEP11a.3 — done locally** | Extend that mechanism proof to nested groups, persistent dynamic policies/checkpoints, interpreted PROCEDURE eligibility and invocation-owned private pool, numeric/TRACE synchronization, aggregate all-pinned failure and retained/relocated binding metadata. Inspect simple and effectful RXAS; batch material fixes. Depends on 11a.2; WP-03/08/11/12. |
| **CLC-STEP11b.1 — done locally** | Complete generic executable manifests, protection/ports, lease checks, conditional cell states, frame lifecycle, event/abort and resource contracts with an independent non-C consumer. Finish the minimal mechanisms proven in 11a without adding C algorithms to L. Depends on 11a.2; WP-02. |
| **CLC-STEP11b.2 — done locally** | Complete shared C preparation and all admitted C runtime handlers over existing B owners, including persistent policy/private-pool sidecars, numeric/observation adapters and aggregate resident accounting/cache. Preserve C24/static-only ABI. Depends on 11a.3/11b.1; WP-03/07/08/11. |
| **CLC-STEP11b.3 — done locally** | Add the narrow context resource-view and exact bounded regular-source reader under existing host/fs owners, with typed failures and cleanup. No search policy in generic/native code. Depends on 11a.1; may proceed alongside disjoint L/runtime work, centrally scheduled; WP-09/10. |
| **CLC-STEP11c.1 — done locally** | Split frontend-only request/package closure, integrate group mode/optional dynamic IR family, exact quote/source keys, invocation catalogue and inline dispatcher into the C frontend/selector. Admit all INTERPRET expressions/instructions and empty/nested/control cases; no lexer regeneration until coherent source is reviewed. Depends on 11b.1/11b.2 and bridge proof; WP-05/08/11. |
| **CLC-STEP11c.2 — done locally** | Integrate the specified source resolver after an unhandled hook; full-program root/results/host guards, trusted locators/context snapshots, deterministic candidate order and no negative-cache staleness. Depends on 11b.3/11c.1; WP-07/10/11. |
| **CLC-STEP11d.1 — selected local qualification done** | Qualify complete LC-87/DYN-AC01–12 local behavior with maintained fixture sources/checked expectations first, independent dynamic controls and meaningful negative/state/cleanup/limits. Adapt harness boundaries, not whole labelled fixtures into illegal groups; native C does not implement INTERPRET and is not its execution oracle. Depends on 11c.1/11c.2; WP-12 with centrally owned preparation. Full LC-87 coverage and wider registered inventory retain 11d.3. |
| **CLC-STEP11d.2 — done locally** | Update enduring architecture/conformance/ledger and simple status, remove scratch, preserve unrelated work and commit authorized implementation paths locally without pushing. This closes only the selected increment. Depends on 11d.1; integrator. |
| **CLC-STEP11d.3 — later formal gate** | Complete STEP10 and every LC-87/CFC/declared platform/delivery criterion under separately selected formal QA after feature completion. Both-engine/platform/installed evidence and actual native sanitizer findings retain their owning requirements. No automatic broad campaign is commissioned by the design or local commit. |

Development starts with one prepared normal Debug configuration and rxbvm; reuse
valid producers and fixtures. The new shared native call/frame ownership mechanism
also needs a focused second-engine control and maintained native sanitizer check
after the normal check, because that mechanism changes, not because B/L executes
on native tools. Source-only stages do not trigger another native campaign.
Register wider regressions for later QA without running a compiler matrix at
every step. Native resource I/O receives its own affected ownership/error checks.

RexxScript sharing is proved at the service/IR boundary with selected G-style
scalar operations and Classic BIF presence/context/value controls. A full new
script parser/class product and B/G-wide compatibility changes are not silently
added to STEP11. Their owning roadmap retains exact language/migration decisions.

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
| **CFC-AC05** | INTERPRET satisfies LC-87-01–05 using the same B/L compiler, live caller state, real resumption/transfers, nested groups and bounded resident IR/source/frame ownership and reclamation. Tests include trapped failures, all-pinned exhaustion, stale handles and teardown; no native/RexxScript fallback. |
| **CFC-AC06** | Representative full RXAS and helper paths have checked structural expectations for simple expressions, loops, calls, errors and dynamic execution. Compiler/RXAS modes are distinct. Direct defects are fixed; necessary cost and later opportunities have owners. Preserve ABI, source, TRACE and numeric observations; static counts are not speed claims. |
| **CFC-AC07** | Generic L services have independent non-C/G consumers, checked structure/effects/facts, bounded failure and documented algorithms. C/BIF/ISA policy stays in owning extensions; source/import/install audits prove no native or typed frontend in the C host closure. |
| **CFC-AC08** | Every required C row closes on qualified inputs, with CLI/service/installed/platform proof, documentation and runnable walkthroughs. Distinguish local, hosted, guest/SDK, sanitizer and release results. Complete C is separate from default replacement and B/G delivery. |
| **CFC-AC09** | B Classic runtime has one implementation of values, pool aliases, activation/context, BIFs and host adapters. C target closure uses the shared B Unicode foundation independently of G aggregation; native C, typed Classic mode and RexxScript retain their contracts. Public metadata/doc tags and consumer regressions prove compatibility. |
| **CFC-AC10** | Any RXAS/VM change has an approved operation/storage contract, exact error/alias/lifetime/source semantics, encoding/disassembly/loader tests and both-engine/platform proof. Alternatives are compared; production performance edits pass the first Release verdict gate. No native source-language implementation is introduced. |
| **CFC-AC11** | Compile/request/result and runtime activation/code lifetimes are isolated, reusable and bounded. Verify exact lengths, atomic failure, stale/foreign references, interruption, repeated/concurrent contexts, declared budgets and recovery without partially published output. |
| **CFC-AC12** | Delivery/build manifests separate native seed, L tools/SDK, C host, C runtime and optional dynamic-compile support. Cold/incremental/installed/relocated builds have one producer and correct freshness; static applications have no compiler dependency. Platform capability rows and source/user/API documentation match actual products. |

## Emission simplifications and conformance choices

Instruction structure is the first performance measure for this increment:
count executable work separately from source metadata and literal data, include
called helper work, and distinguish hot paths from cold paths. Report actual
measurements in conversation; keep this ledger to the intended simplification,
its semantic obligations and decision. Fewer instructions do not by themselves
establish equivalent behavior or a runtime-speed claim. INTERPRET invalidates
affected mutable facts at its boundary; it does not prohibit unrelated proofs
or require dynamic redispatch of every ordinary statement.

| Stable opportunity | Simpler instruction structure | Conformance obligation / decision |
|---|---|---|
| **CEP-P01 — idle completion** | A bounded scalar predicate and local continuation; no idle boundary/argument objects, numeric/TRACE reinstall or all-site search. Share cold dispatch and use bounded continuation routing. | Preserve the authored completion point, pending CALL/HALT, EXIT, captured decisions, source origins and cleanup. Conformance-preserving repair; CDDS owns implementation and proof. |
| **CEP-P02 — fresh argument chains** | One mutating append per actual or omission, without copying every growing packet prefix. | Consume only a proved fresh, unpublished single-owner chain; shared values retain value semantics. Preserve left-to-right evaluation, omissions, failed capture and abrupt cleanup. Conformance-preserving repair; CCPK owns proof. |
| **CEP-P03 — immutable catalogue facts** | Prepare immutable labels/signatures/definition identity once under a stable module owner; avoid repeated invocation construction. | The first slice is locally implemented with selected checks complete; the full functional panel, fixed representative review and broader CFC qualification retain their scope. Keep policies, pools, generations, capabilities and tickets invocation-owned. Preserve exact foreign/stale-definition rejection, recursion, source identity and resource bounds. Encoding and cache changes remain separate. |
| **CEP-P04 — reachable capabilities** | Keep stable language-profile admission separate from surviving operations and runtime need; omit machinery only when every admitted entry proves it unnecessary. | First repair post-pruning clause admission under the contract below. Validate all source, including dead source; account for label entries, host callbacks, numeric state, TRACE and INTERPRET effects. A missing proof retains the general path. No new reachability filter is selected. |
| **CEP-P07 — guarded specialization and fallback** | Preserve unaffected facts at effect boundaries; use a cheap guard and a general compiled path where a small specialization removes demonstrated work. A later one-way IR fallback might remove larger compiled slow paths. | Precise invalidation and selective compiled fast/slow paths are the preferred design direction, with each implementation requiring its own proof. IR fallback remains unselected: current sessions cannot adopt compiled state, and a general semantic definition, checked continuation map, live owners/observations and explicit resource envelope are required. No JIT, state-adoption API, epoch scheme or fallback prototype is authorized by the design review. |
| **CEP-P05 — reduced observations** | An explicit reduced-observation profile could omit selected TRACE/source/error-origin machinery. | This would lose named observations and diagnostic detail. No default change or such profile is selected; any concrete proposal must identify the lost contract and obtain Adrian's decision. |
| **CEP-P06 — restricted dynamic language** | An explicit static-only profile could reject INTERPRET and source-resolved calls and omit their support. | This is a reduced language contract, not complete C. It remains an unselected tradeoff; do not introduce it as a silent optimization or weaken the default test oracles. |

**CEP-P03 first slice — locally implemented; selected checks complete.** Use the
existing per-worker module globals and initializer, with immutable declaration
rows shared by function catalogues. No process cache or new VM owner is needed.
**CEP-P03-AC01:** one initializer constructs complete pure data, checks readiness
and signals on failure; no public entry can observe partial publication. Stable
globals outlive recursive scopes and remain unchanged through late module loads.
Its private META_FUNC and META_INITIALIZER records use the same qualified symbol,
local procedure, void result and empty argument list; no callable is exported.
**CEP-P03-AC02:** prepared scopes borrow labels and the exact legacy fingerprint;
separate owners still compare exact fingerprint bytes. Function identity remains
in that fingerprint; generation is a separate invocation check. Existing owned
getters, handler snapshots, foreign/stale guards and cache keys retain their
semantics. **CEP-P03-AC03:** pure provider projections may be prepared once, but
live rxinvoke catalogues/bindings, policies, pools, budgets and tickets remain
invocation-owned. Binding happens after scope publication so existing fatal
cleanup can release partial reservations. Charge shared declarations once plus
each function catalogue against the existing compilation resource envelope,
using checked dimensions; this is not an aggregate VM/RSS bound.
Eager preparation intentionally retains unused function data until module
teardown and can move allocation failure from first call to module initialization;
that timing/resource tradeoff is accepted for this first slice. Encoding changes,
fingerprint shortening, cache namespacing and reachability analysis are excluded.
The selected source/RXAS, identity, binding, failure/cleanup and initializer
checks are complete. The full research functional panel is authorized before
the remaining fixed representative reviews; broader CFC qualification remains open.

**CEP-P04 first repair — locally implemented; selected checks complete.** A
legal source with `EXIT 0` followed by unreachable INTERPRET must remain accepted
when generic IR optimization prunes the unreachable operation. The source-owned
clause checkpoints do not become illegal when their last dynamic-operation
witness disappears. The existing canonical `crexx.c.dynamic` versioned family is
a stable semantic profile, not evidence that an invocation needs dynamic state.
**CEP-P04-AC01:** required clause admission uses that admitted profile separately
from the unchanged all-record dynamic-operation fact; a transformed graph gets
fresh facts bound to its own identity. No new IR marker, hidden proof flag,
schema version, runtime service or reused pre-transform proof is introduced.
The declaration owner supplies the complete canonical dynamic contract through
`llcir.dynamiccontract()`. SIGNAL compares full frozen manifests, including
versions, types, operations, ordered fields and effects; family/version alone
cannot admit a partial or forged profile. Cache unknown/yes/no only in the
request-owned proof, query only when CALL inventory and the surviving dynamic
witness are both absent, and charge its existing required work/workspace
budgets before manifest construction. No global cache or new lifetime is added.
**CEP-P04-AC02:** preserve complete schema admission and the canonical table's
ownership/once-written source, exact authored clause ID/origin and conditional
EXIT guard checks. Missing/foreign tables, malformed profiles and forged guards
remain refusals; failed compilation preserves prior published output. Full
source/binding validation, including unreachable authored source, still precedes
optional pruning. **CEP-P04-AC03:** the dead-only source compiles with both IR
settings, while a positive admitted label below the root's EXIT retains its
reachable INTERPRET implementation through the entry dispatch. A root-only walk
cannot remove label, handler, host/public or exceptional entry behavior.
**CEP-P04-STEP01 — done:** implement and source-review the stable-profile
admission repair and its owner-local positive/refusal controls.
**CEP-P04-STEP02 — done:** inspect corrected IR-on dead-only and admitted-label
RXAS and qualify the affected contract/error paths. Existing generic pruning
removes unnecessary dynamic construction for the dead-only IR-on case; IR-off
may retain unoptimized dead work. No further construction analysis is selected.
**CEP-P04-STEP03 — todo:** retain this coverage in the representative review map;
any later construction proof must cover all admitted entries, bound optional
work/workspace and fall back to general compiled behavior on an unproved case.

1. **CEP-P-STEP01 — in progress:** select and review the conformance-preserving
   repair batches before source edits, with independent structural expectations.
2. **CEP-P-STEP02 — todo:** report simpler/fewer-instruction evidence and any
   remaining unavoidable cost, keeping each proposed conformance compromise
   explicitly unselected until Adrian decides it.
3. **CEP-P-STEP03 — done for the selected research scope:** retain the registered
   functional gate, authorized local commit and fixed six representative
   review areas under CEP-STEP03.
   Broader CFC, BQA, platform, sanitizer and publication gates retain their scope.

## BASELINE-QA regression plan

This is the test plan for the approved `develop` publication gate, not a claim
that complete-C acceptance or a formal release is finished. The ordinary
automatic pack is CMake's `qa-comprehensive` target: it prepares its declared
inputs and selects CTest labels `essential`, `smoke` and `comprehensive`.
Qualification, stress, measurements, installed delivery and sanitizer assurance
have separate owners and must not be silently counted as ordinary pack passes.
For this agreed baseline milestone, the existing maintained functional,
sanitizer and performance suites all run after their prerequisites are green;
add C/L cases to the owning suites where the coverage inventory finds a gap.
Report CTest entries passed, failed, timed out and not run separately; nested
tool commands and fixture examples are not additional CTest passes.

| Stable criterion | Required proof before this gate closes |
|---|---|
| **BQA-AC01 — complete automatic inventory** | Map the [C requirements crosswalk](doc/levelc-requirements.md), including LC-I-01–25 and all 95 BIF names, and the approved Level L service families to named registered tests. Record unique maintained fixtures as registered, covered by a named owner, or an explicit open gap. Identify every uncovered approved baseline contract: none may be dropped merely to reduce cost, and the gate stays open until each has proof. Manual research panels outside the approved baseline retain an explicit later owner. Native C fixture values may be reused as independent expectations; native C test results are not new-C passes. |
| **BQA-AC02 — meaningful oracles** | New C and L checks cover observable values, errors, state, cleanup, limits and atomic failed output where relevant, plus representative full RXAS/source/TRACE structure. A name appearing in a source or a command exiting zero does not alone prove its contract. Reuse existing neutral L, B runtime and native controls without duplicating their algorithms or weakening their assertions. |
| **BQA-AC03 — independent execution axes** | The final ordinary pack runs all its registered lanes; new work defaults to one prepared normal Debug/rxbvm lane. Before freeze, run only newly added and failed selected cases. Use a compact interaction panel to vary C compiler build mode, semantic IR optimization, assembler optimization and concrete VM independently where they can affect that feature. Existing opt/on/rxbvm versus noopt/off/rxtvm pairs do not by themselves isolate those four axes. Do not multiply every fixture into an automatic Cartesian matrix. |
| **BQA-AC04 — reviewable test cost** | Retain prepared aggregates where they avoid repeated compilation, but expose named per-case outcomes and focused selectors. Separate costly source/provider/link preparation where freshness-checked reuse is practical. Keep long lifecycle, process, timeout and integrated compile checks explicit and budgeted; nested command counts are not test counts. No check is shortened by removing diagnostics, cleanup, modes or failure assertions. |
| **BQA-AC05 — final functional result** | Repair known failures in a coherent batch, rerun only affected cases, then freeze source and prepared-product inputs. On that identity, complete the full ordinary automatic pack once with zero failed, timed-out or unexpectedly skipped entries. Product changes after the freeze require affected replay and a final pack result on the new identity; documentation-only changes do not. |
| **BQA-AC06 — delivery result** | After functional green, build ordinary Release and prove cold/incremental freshness, installed and relocated C CLI/service/static and optional dynamic consumers, package closure and native-compiler absence from the C request path. Keep unsupported or unrun platform rows open. |
| **BQA-AC07 — memory and hosted result** | After functional green, run the existing maintained `daily` component-memory panel and the explicit `full` sanitizer suite on the required macOS and Linux platforms, adding reviewed C/L native-memory boundary regressions to the owning panel. Run the focused, original-trigger and platform checks expressly required by open SAN worklist items. Commit and publish the qualified `develop` identity only after its local gates, then link normal hosted CI for that exact head. Do not call an open SAN item or unrun hosted platform proof clean. |
| **BQA-AC08 — performance result** | After functional green, execute the existing serial `qa-measurement` suite and the maintained language-benchmark runner's default portfolio, including RexxCPS, on an ordinary profiling-off Release product. Add correctness-gated C/L compiler and execution/lifecycle cells where the suite lacks them. Check workload/image identity, output equivalence and resource/code-size observations separately from elapsed time. Apply the performance governance sampling and host rules to any formal comparison; label a bounded research screen as diagnostic and make no product-speed or replacement claim from it. |

The automatic coverage additions for BQA-AC01–03 are bounded. Generic L already
registers core AST/source/IR/flow/analysis/optimisation fixtures across its
consumer compiler/assembler modes and VMs; this does not imply a no-opt SDK
rebuild. One normal lane covers indexed graph facts, typed RXAS and IR execution.
Add only identified effect/manifest/catalogue trust-boundary negatives inside
those owners. STEP01 selects essential uncovered generator, budget, Unicode,
diagnostic, source-emission and authoring/isolation cases from the manual
harnesses; it does not register their entire nested matrices. C already
registers instruction families, Group 1–3, Group 4 and
STEP11; retain their positive/error/state/cleanup oracles. Add independent
recent-feature axis checks, live nested INTERPRET exhaustion/recovery and
compiled-to-IR interruption/reuse, a per-name BIF evidence crosswalk identifying
existing value/error/presence/context checks and bounded missing integration
cases, and an installed/relocated compiler-service consumer. Full BIF-CHECK and
declared-platform acceptance remain open under CFC. `cleanup-c.crexx` and
`cleanup-g.crexx` are owned by `check-optimise.py`; retain their existing
IR-off/on execution and cleanup oracles. Build-tree package-closure checks do
not establish installed delivery.

| Stable step | Current status and dependency |
|---|---|
| **BQA-STEP01 — todo** | Finish the registered-test/requirements map and freeze the finite list of every missing approved baseline case and independent mode axis. Name later manual research gaps and their owners explicitly; BQA-AC01–03. |
| **BQA-STEP02 — done for the selected research scope** | Preserve the reviewed emission repairs, completed registered research functional gate, authorized local commit and fixed six representative RXAS areas. Keep every registered lane and its oracles; this bounded local sequence does not close the broader BQA-AC01–04 inventory, independent-axis or delivery requirements. |
| **BQA-STEP03 — todo** | Freeze the candidate, run `qa-prep-comprehensive` through its declared build graph and complete `qa-comprehensive` once. Report exact selected/passed/failed/timeout/not-run counts; BQA-AC05. |
| **BQA-STEP04 — todo** | Complete Release build and installed/relocated consumers after functional green; BQA-AC06. |
| **BQA-STEP05 — todo** | Run the existing maintained daily and full sanitizer suites, close or explicitly retain each SAN finding under its original-trigger/platform rule, then run `qa-measurement` with agreed C/L additions and the maintained language-benchmark portfolio serially on a quiet Release host; BQA-AC07/08. |
| **BQA-STEP06 — todo** | Commit only authorized paths, verify that the commit contains exactly the tested source inputs, publish `develop`, and qualify that published head through normal hosted CI. Preserve separate CFC, SAN and release gates; BQA-AC07/08. |

## Outstanding decisions and dependencies

- **BASELINE-QA — open; approved native seed repairs and qualification:** repair
  B-SEED-ALIAS, B-SEED-INLINE-RECEIVER and B-SEED-CLASSIC-CATCHALL in their owning
  native B backend paths, retaining supported inline shapes and the native C
  frontend unchanged. Establish isolated permanent reproducers and inspect
  representative RXAS before broader checks. Replay failing functional cells to
  green, then freeze the candidate and run the complete ordinary
  correctness/optimizer/VM regression suite once. After that functional gate is
  green, run Release build/install checks, the existing maintained `daily` and
  `full` sanitizer suites on both maintained platforms, and the existing
  performance suite with the C/L coverage additions identified above. Level L
  research has no automatic formal performance verdict, but this agreed
  milestone requires real serial performance-suite results with clear claim
  boundaries. Actual SAN findings retain their separate full closure
  requirements; an open item is reported explicitly, never called sanitizer-clean.
  Applicable hosted platform gates follow publication of the qualified commit.
  Preserve unrelated work; commit and publish develop after local gates pass,
  then qualify that exact published head through the hosted gates before
  reporting a green baseline. No master promotion or formal release/tag is commissioned.
  Baseline qualification does not close unverified CFC, release or
  constrained-platform criteria by implication.
- **PROVIDER-LIMIT-CLASSIFICATION — open; baseline C adapter repair:** preserve
  the documented native `rxcontract` status at the C binding boundary; no native
  provider, parser, allowance or signature-grammar change.
  **PLIMIT-AC01 — locally qualified:** after charging returned acquisition/projection
  counters, status `limit` produces `LL_RESOURCE_LIMIT`; invalid/other failures
  retain `LL_COMPILER_IMPORT` and publish no usable signature or lookup.
  **PLIMIT-AC02 — locally qualified:** unchanged independent byte/work boundary and aggregate
  group checks remain green; native acquisition preflight limits retain their
  diagnostic/classification and existing invalid-signature controls retain
  their separate import-error classification.
  **PLIMIT-STEP01 — done:** coordinator reviewed this bounded classification
  contract before product edits. **PLIMIT-STEP02 — done:** source and
  representative RXAS review cover the B adapter classification and independent
  native preflight/no-publication probe; its signature local has procedure scope.
  **PLIMIT-STEP03 — done locally:** affected bindings and contract image are
  rebuilt; focused normal Debug/rxbvm contracts and the full external-call
  consumer pass. Broader BASELINE-QA stays open.
- **RXAS-LABEL-LOOKUP — open; baseline generic target repair:** selected branch
  targets repeatedly scan every label. Prior record-admission exhaustion or
  search exhaustion can then be replaced by an unresolved-label error.
  Owner: generic `llrxas` target finalization; retain C selection, native
  frontends, register/lifetime rules and existing budgets.
  **RXLABEL-AC01 — locally qualified:** build one exact first-match label-to-record index;
  retain every label location for conservative storage checks. Dense forward
  and backward targets remain bounded by indexed key work, with no budget raise
  or instruction/source changes. **RXLABEL-AC02 — locally qualified:** pre-existing admission
  errors and finish-time work exhaustion retain their original resource-limit
  identity and publish neither text nor assignments; a truly absent target
  retains its separate record error. **RXLABEL-AC03 — locally qualified:** maintained generic
  target controls and the independent Group 2 host compile/assemble/link/run
  consumer qualify this repair in the prepared normal Debug/rxbvm lane. Wider
  G4/CFC-AC06/07/11 and baseline/platform qualification remain open.
  **RXLABEL-STEP01 — done:** record and obtain coordinator review of this exact
  indexed-lookup/error contract before product edits. **RXLABEL-STEP02 —
  done:** the generic index, atomic error handling, focused controls and owning
  RXAS guide are updated and source/representative RXAS reviewed.
  **RXLABEL-STEP03 — done locally:** focused normal Debug/rxbvm generic controls
  and the Group 2 host compile/assemble/link/run consumer pass. Keep broader
  BASELINE-QA open.
- **C-TARGET-WORK — open; baseline C adapter repair:** expanded dynamic dispatch
  exceeds a fixed target-work allowance during record admission even though it
  remains within the requested output-byte cap. The bounded admission repair
  retained conservative emission; BASELINE-QA has since reopened its repeated
  dynamic-dispatch cost under C-DYNAMIC-DISPATCH-SIZE.
  **CTWORK-AC01 — locally qualified:** each C target receives one checked combined allowance
  of `output.bytes + 1000000`. Record admission costs less than its already
  bounded selected-text bytes; the existing million-work allowance remains for
  finalization. Generic L defaults, C output/register caps and atomic errors
  remain unchanged; dense allocator work can still return a bounded refusal.
  **CTWORK-AC02 — locally qualified:** the maximum representable sum passes budget admission;
  larger output limits return the existing C LIMIT family before arithmetic,
  with no output or changed graph. **CTWORK-AC03 — locally qualified:** warning-free affected
  leaf preparation, representative RXAS and focused boundary controls precede
  the independent Group 2 host compile/assemble/link/run consumer. Full baseline,
  CFC and platform acceptance remain open.
  **CTWORK-STEP01 — done:** record and obtain coordinator review of the text-plus-
  existing-finish decomposition and overflow contract before C product edits.
  **CTWORK-STEP02 — done:** add one checked C-owned helper/allowance field, use it
  at every target constructor, update the backend guide and existing focused
  contract controls; source and representative helper/host RXAS are reviewed.
  **CTWORK-STEP03 — done locally:** warning-free affected leaves, focused
  boundary controls and the Group 2 compile/assemble/link/run consumer pass.
  BASELINE-QA remains open.
- **C-INTERPRET-DIAGNOSTIC — open; baseline frontend repair:** classified
  INTERPRET expression failures use the existing instruction-expression
  diagnostic owner before the general DO/SELECT fallback. A dangling operand
  inside a group with a matching END is `35.1` at its operator, including dead
  bodies; it is not a missing-END `14.1`/`14.2`. Preserve lexical/PARSE priority,
  contextual INTERPRET names, source bytes and generated LR expectations.
  **CID-AC01:** bare and dangling INTERPRET expressions have independent located
  `35.1` controls; actual incomplete DO/SELECT retain `14.1`/`14.2`.
  **CID-AC02:** existing accepted INTERPRET forms and name/label/function
  classification remain unchanged. **CID-STEP01 — done:** coordinator review
  of this contract and the bounded decorator/test diff before product edits.
  **CID-STEP02 — done:** add INTERPRET to the existing classified expression
  decorator and strengthen the maintained source-error controls; no grammar,
  generated tables or native frontend change. **CID-STEP03 — done locally:**
  affected frontend images and helper RXAS are reviewed; focused normal
  Debug/rxbvm grammar and syntax checks pass with the prepared grammar.
  Full baseline/platform acceptance remains with BASELINE-QA.
- **B-SEED-CALL-WINDOW — open; baseline caller repair:** two independent
  native B caller defects share the counted-call boundary. A repeated actual
  used by value and once by EXPOSE must retain its live exposed destination;
  choose the exposed occurrence for swap/restore, marshal each shared source
  once, then snapshot its by-value occurrences from the primary call slot and
  apply their argument status before entry. This includes either formal order
  and an occurrence already in its destination slot. Duplicate-bearing calls
  materialize preparation before those snapshots; ordinary calls retain fused
  preparation. Dynamic interface method/factory
  selection must use an allocated result register outside its still-live
  argc/argument window, including calls nested in inlined arguments; do not
  propagate a deferred parent destination into selector storage. Preserve
  ordinary call ABI, source/TRACE observations and supported inline shapes;
  native C frontend and VM algorithms remain unchanged.
  **BCALL-AC01:** exact text and exposed mutation survive mixed by-value/EXPOSE
  aliases in compiler opt/no-opt, with input snapshots and normal restoration.
  **BCALL-AC02:** representative method/factory selectors and counted calls
  preserve argc and receiver/argument values, including inlined consumers;
  structural assertions compare operand roles rather than fixed registers.
  **BCALL-AC03:** repeated-by-value/fixed/local calls retain their semantics;
  calls without repeated sources retain their existing marshalling costs.
  **BCALL-AC04:** failure while reserving skipped call-result registers is a
  terminal compiler allocation diagnostic, never a result inside the forbidden
  live window. Inject first/later allocation failures into the owning helper;
  healthy allocation must choose an outside register and restore every skipped
  register in its original reuse order. Structural checks require ordered
  same-call alias staging and a fresh, non-overwritten argc producer for every
  dynamic call; procedure/image/call boundaries cannot lend stale evidence.
  **BCALL-STEP01 — done:** coordinator review of coherent caller allocation/
  emission and this contract before product edits. **BCALL-STEP02 — todo:**
  narrow native B repair plus permanent source/structural regressions; inspect
  the first representative RXAS. **BCALL-STEP03 — todo:** centrally owned
  affected normal checks, existing RXPA strings/Classic host regressions and
  matching maintained checks within BASELINE-QA; reuse unchanged prepared
  inputs and keep full baseline/platform acceptance open.
- **INLINING-STRATEGY — separate design review; implementation decision open:**
  [the existing inlining design](../../../compiler/docs/inlining_design.md#future-inlining-boundary-review)
  compares native AST, semantic IR and RXAS/bytecode expansion. The proposed
  direction is producer-independent inlining of RXAS instruction bodies through
  checked target IR, using the bound caller context and existing machine proofs.
  This is the shared boundary for B/G/C and DSL producers; semantic IR may still
  specialize earlier but is not a required parallel library-body format.
  First specify call/context, lifetime, module-body and source contracts, then
  prove local expansion and real imported-helper integration before migration.
  Cross-module bodies can be imported during compilation or collected by the
  linker; a separate pre-link stage is not required. Two separately reusable
  services, target expansion and target optimization, can be orchestrated by
  compiler, assembler or linker adapters. Compiler-driven conservative expansion
  followed by RXAS cleanup is a proposed first experiment, not approved work.
  Post-inlining optimization scheduling, analysis invalidation/reuse, phase
  ordering, work/growth budgets and measured optimizer/program performance remain
  unresolved. Previously optimized bodies do not guarantee cheaper subsequent
  optimization or equivalent final code. This review
  authorizes no replacement inliner, new body format, B/G expansion or default
  switch. Repair the recorded native seed defects independently of that choice;
  no existing supported inlining shape is withdrawn as a workaround.
- **BUILD-IMPORT-ISOLATION — open; baseline preparation repair:** parallel
  ordinary preparation exposed normal compiler consumers scanning unrelated
  RXBIN outputs while sibling producers rewrote them, producing real import
  read/truncated-header diagnostics. Reuse the existing exact import-root
  staging service for the affected Level L, example, host-contract and RXPA
  producers, with completed declared inputs and real file dependencies.
  Preserve compiler/runtime semantics; qualify diagnostic-free parallel
  preparation and affected consumers before the full baseline gate.
- **PACKAGED-STREAM-PROVIDER — open; baseline preparation repair:** RXPP's
  embedded Classic runtime requires the existing `rxcstream` provider, but its
  standalone executable omitted that static provider and its Unicode bytecode
  dependency. Use the existing static provider binding and bytecode link/build
  dependency services, preserve loader/search policy and
  qualify real preprocessing plus an isolated relocated executable consumer.
  Audit directly equivalent packaged Classic consumers for the same omission.
- **BUILD-MAKE-OWNERSHIP — open; SAN-QA-026:** full maintained preparation
  executes duplicate Make recipes for the same C grammar and canonical Level L
  module outputs. Give each selected grammar/module one explicit producer
  target, order import staging/aggregate consumers through those targets and
  retain file freshness dependencies. Reuse the existing generated-grammar
  ownership pattern; change neither generator nor runtime algorithms. A small
  parallel Make regression using the actual owner declarations must prove one
  production, warm no-op and single missing-output recovery before reusing
  valid prepared artifacts and completing the baseline gates.
- **PREPARATION-WARNINGS — open; SAN-QA-027:** the full Apple preparation
  exposes deprecated native formatting calls outside the daily memory panel
  and implicit ODBC fixture bindings. Replace only the encountered formatting
  calls with checked, bounded writes using their actual allocated capacities;
  preserve exact output, ownership and frontend semantics. Declare intended
  fixture-local bindings explicitly. Review coherent changes and select affected
  normal/maintained checks before completing warning-free full preparation;
  do not suppress diagnostics or launch an unrelated cleanup.
- **RECV-DIRECTORY-BOUND — repaired; full baseline qualification open:** SAN-014
  records the actual directory formatter overflow. Its display capacity now
  derives from the admitted eight-byte member, TTR, indicator, 62 user-data bytes
  and NUL. The maintained legal-entry regression checks complete output and
  parser state in normal Debug and Apple ASan. Preserve that exact contract;
  full platform closure remains with BASELINE-QA and SAN-014.
- **RESEARCH-INSTALL-CLOSURE — open; baseline delivery repair:** the documented
  research stage omits the installed dynamic-runtime image and contract-provider
  package. Select their existing producer targets in that stage, preserving
  optional language selection and the base product boundary. Qualify the stage
  dependency contract and retry installation plus relocated CLI/service consumers
  with the already prepared Release artifacts; no compiler algorithm changes.
- **C-DIAGNOSTIC-ORIGIN — repaired; baseline qualification open:** generic VM
  transport retains original authored callers with the raw event across helper
  unwinding, copies/rethrows, nested same-tag faults and teardown/reuse. Module
  bounds exclude imported procedures' foreign offsets. The retained-source
  `divide-error-c` oracle and focused source/ownership/callback/initializer controls
  retain exact payload and raw origin. Only guarded terminal reporters render;
  captured initializer failures also avoid a duplicate loader summary. Continue
  the matching maintained checks and broader baseline qualification under
  DIAG-AC01–06; no stale global last-source inference or public ABI change.
  A generated `crexx.c.program.exiting` query whose admitted program-call fact
  proves the same source range as its invoking call must present that call's
  authored source location to the VM diagnostic renderer. The C RXAS selector
  may classify only that query's source record as derived while preserving its
  frozen IR origin, query identity and existing anchor. Because the origin is
  also used by no-source transport, the query presentation needs a distinct
  checked source-step ID; one ID must never describe two different anchors.
  Other synthetic transport remains without a source location.
  Query and ordinary source mapping use distinct, purpose-named IR-store borrows
  in their respective current scopes. Preserve each guarded acquisition and
  scope-exit unlink, source-step range check and authored source selection.
  **DIAG-STEP03.CQUERY — todo:**
  inspect the query's active RXAS source record and replay the affected EXIT
  failure and cleanup controls before wider BASELINE-QA. This serves
  DIAG-AC06 and CFC-AC03/06 without a public API or ISA change.
- **C-SERVICE-EARLY-FAILURE — repaired; baseline qualification open:** a
  `llccompiler.request` refused before frontend construction must retain its
  original error through every public diagnostic/output accessor. The factory
  initializes its existing typed absent frontend before any early return;
  guarded accessors preserve the refusal. Owner: C compile-service construction;
  VM semantics, native frontends and public interfaces remain unchanged.
  **CSERVICE-AC01 — locally qualified:** missing selected providers and conflicting
  routine/host exports return their original import refusal. Repeated warning
  and diagnostic access is safe, warnings are empty, diagnostics retain the
  logical filename/error, output has zero bytes and previous owned output is
  unchanged. Cover status-first, warning-first and diagnostic-first access.
  **CSERVICE-AC02 — locally qualified:** a valid request after each refusal succeeds in the
  same worker without stale error, frontend or output state. Representative
  RXAS confirms typed absence before any early preparation return.
  **CSERVICE-STEP01 — done:** source and prepared RXAS identify the absent
  frontend slot and the early-return accessor boundary; AC01/02.
  **CSERVICE-STEP02 — done:** the narrow typed-absence initialization and
  maintained refusal/accessor/recovery controls are reviewed in source and RXAS.
  **CSERVICE-STEP03 — done locally:** the affected normal Debug opt/rxbvm
  contract and Group 2 consumers pass on the prepared images. Full BASELINE-QA,
  CFC and platform qualification remain open.
- **CORE-PROVIDER-PACKAGING — open; baseline qualification repair:** installed
  native packaging requests nonexistent external archives for mandatory VM-core
  `rxparse` bindings; `rxinvoke` has the same metadata shape. Preserve native
  provider identities and exact callable contracts for the invocation bridge.
  Share the existing declaration-only tables through binutils; the interpreter
  binding definitions remain their sole signature/provider-id owners. An
  internal query using the existing signature parser classifies exact mandatory
  core bindings, external bindings and conflicts. The linker validates these
  records and omits only exact core bindings from its external-archive
  requirements projection, retaining linked provider metadata and map records.
  Reject core identity/signature conflicts and preserve missing-external-provider
  errors; no driver exception list, duplicate archive, optional downgrade,
  loader-policy or binary-format change. Review before implementation, then
  qualify real native-packaged cursor/Classic consumers, both retained core
  identities, malformed/unknown contracts and external requirements.
  Reuse the VM's provider-signature equality policy through the shared signature
  owner, preserving parameter names, whitespace handling and `[]`/`[*]`
  equivalence; the looser call-contract matcher is not an identity check.
- **B-SEED-CANCEL — locally complete; required Group 2 dependency:** typed B
  admission, catch-all emission and the shared signal mapping now admit the
  existing maskable VM CANCEL tag. Shared process/host cleanup selectors release
  resources before relaying it with the original payload and origin.
  Native Classic parsing/lowering, ISA and public host interfaces stay unchanged.
  Focused typed-handler and real process/host cancellation controls qualify
  resources/windows released before same-context reuse.
  Strong KILL/shutdown retain terminal native teardown; no general finally claim.
- **Shared observations:** CCOND supplies configured errors/NOVALUE/LOSTDIGITS,
  typed conditions, resumed handlers and authored clause/field/drop observations
  for the admitted static profile. ARG-AC05/CPARSE-AC07 retain their wider source,
  host/dynamic and delivery/platform obligations; formal QA remains open.
- **Delivery/platforms:** CPARSE-AC08, PARSE-ENGINE-AC05 real constrained platforms,
  UNWIND-AC04 broader-platform proof and complete CLI/service/install/relocation
  remain open. Linux LSan and real 31-bit/CMS/TSO/Windows runtime need their
  declared qualification; Apple LeakSanitizer is unavailable. Local static/NUTF8
  simulations and synthetic errors do not establish those platform capabilities.
- **Dynamic execution:** IR implementation is present; finish the selected local
  qualification under the
  [detailed STEP11 contract](doc/levelc-architecture.md#dynamic-execution-strategy) and
  implement the approved concrete call/resource/search interfaces.
  Preserve every LC-87 obligation and INTERPRET-last; bounded IR/source/frame
  ownership replaces generated-image ownership without waiving mixed-frame proof.
- **L-IR-DESCRIPTOR-OWNER — fixture repaired; qualification open:** the reported
  `LL_IR_OWNER: operation descriptor expired` does not establish lifetime expiry:
  `operation.valid()` also rejects a nonpositive operation ID or changed schema
  identity. Renaming both `numeric.form` inputs to `wrongcontext` sets the first
  schema error, `duplicate operation field wrongcontext`; a later `addop` then
  returns zero because that schema is already invalid. The reviewed fixture
  repair restricts mode 2 to its intended second/context input, preserving a
  generic-valid target-contract negative. Runtime qualification and an unchanged
  isolated replay remain open; sibling timeouts stay separately unclassified.
  No generic L, native B or VM lifetime defect is established, and their product
  validity/diagnostic behavior remains unchanged.
  **LIR-OWNER-AC01:** a minimal independent registration control distinguishes a
  duplicate field's original schema diagnostic and subsequent zero operation ID
  from a truly expired reference. A live local `schema_data` owner retains its
  checked borrow and identity while valid operations/fields/effects are appended;
  `.llir..operation(reference data,data.addop(name))` preserves the same target
  and once-only ID. Use the existing generic duplicate-field/frozen/foreign
  controls; do not remove validity checks or copy/extend an owner to mask failure.
  **LIR-OWNER-AC02:** forged mode 2 changes only the intended context-input name,
  preserving the other ordered input and unique generic field names. Every
  forged profile remains a valid generic schema/graph with exactly its intended
  C target-contract violation; the C selector supplies the expected precise
  refusal and preserves graph bytes and prior output atomically.
  **LIR-OWNER-AC03:** early source and representative RXAS inspection plus the
  reduced control identify the first failure before a repair is assigned.
  Preserve supported compound actuals and inline/call shapes. Change product
  ownership logic only if a valid live-owner reproducer independently fails,
  then record/review that owning component's exact contract first. A proven
  shared BCALL cause stays with BCALL; native C remains read-only.
  **LIR-OWNER-AC04:** the original C context fixture completes without owner panic
  and retains its negative-schema/atomic-refusal coverage. Reuse prepared inputs
  and unaffected generic results. Isolated timeout-only outcomes receive their
  existing scheduling treatment; full baseline/platform/CFC acceptance stays open.
  **LIR-OWNER-STEP01 — done:** record and independently review the validity
  distinction, duplicate-field cause and independent expectations before edits.
  Active full normal CTest owns all preparation/execution.
  **LIR-OWNER-STEP02 — done:** restrict the reviewed mode-2 fixture mutation to
  its second/context input; retain the existing generic duplicate-field negative
  and product diagnostic/probe semantics; AC01–03. Runtime proof remains below.
  **LIR-OWNER-STEP03 — todo:** after the active run, centrally replay the unchanged
  material failure, isolate the mode-2 first-error/zero-ID cascade and qualify the
  corrected fixture and original C consumer in the affected normal mode/VM.
  Reuse prepared products and preserved generic lifecycle results; AC01–04.
  Update enduring simple status after proof. Wider qualification remains the
  existing BASELINE-QA gate; this fixture repair does not commission native or
  whole-solution assurance again.
- **C-DYNAMIC-FRESH — open; baseline qualification repair:** INTERPRET may
  preserve an incoming Fresh eligibility window, consume it, or commit the
  invocation's Private pool; it cannot recreate Fresh after an ordinary DO,
  SELECT or other instruction consumed it. In the existing three-state proof,
  expand an incoming Fresh bit to Fresh/Consumed/Private and otherwise retain
  the incoming Consumed/Private union. Preserve the explicit expansion proof:
  only its immediately adjacent, same-source/same-span clause begin and canonical
  context-load prefix, its admitted source-expression region and its paired
  completion/EXIT transport may run while Fresh. Do not admit arbitrary clause
  begins, unrelated checkpoints, stores, loop work or source-span matches as an
  eligibility exemption. Owner: C procedure admission; no native frontend,
  shared runtime algorithm, schema or ABI change.
  **CFRESH-AC01:** valid live and dead DO/SELECT INTERPRET source is accepted,
  retains exact authored AST/expression spans and emits a checked explicit
  expansion when retained; malformed dead operands still report their own
  source syntax error. Reuse the maintained frontend cases and add a named
  fresh-entry source control.
  **CFRESH-AC02:** raw positive controls distinguish an incoming consumed path
  from a Fresh-capable entry/group path. INTERPRET preserves all possible
  incoming eligibility outcomes without inventing Fresh, and empty/nested
  generated groups retain first-PROCEDURE eligibility and existing 17.1 errors.
  **CFRESH-AC03:** tiny generic-valid raw negatives reject an unpaired or
  different-origin clause begin, intervening semantic work before the expansion,
  a noncanonical context, unrelated post-expansion administration while Fresh
  and an ordinary write after a Fresh-capable INTERPRET without consume. Exact
  same-origin canonical administration remains admitted. Test fresh and loaded
  graphs; reuse a valid transformed positive and existing atomic refusal and
  budget controls rather than multiplying the full raw matrix.
  **CFRESH-STEP01 — done:** exact contract and independent coordinator/source
  review precede product edits; AC01–03.
  **CFRESH-STEP02 — done:** the demonstrated C-owned transfer is repaired and
  focused source/raw controls are maintained; coherent source and the first
  affected raw graph are reviewed. Qualification remains STEP03.
  **CFRESH-STEP03 — todo:** include affected frontend/procedure images in the
  coordinated material-fix build, inspect representative RXAS, then run the
  selected normal Debug opt/rxbvm frontend DO/SELECT and raw admission checks.
  Reuse unchanged dynamic eligibility/runtime results. Wider modes, platforms
  and maintained baseline assurance retain BASELINE-QA; no second broad
  campaign is created here.
- **PULL-EOF — open; shared runtime baseline repair:** queue-empty PULL and
  PARSE PULL reuse `RexxClassicConfig.pullText()->lineinText()` and the existing
  `RexxClassicStream.readLine` owner. An ordinary exhausted configured input
  supplies empty text, including after PARSE LINEIN has reached EOF; it must
  not escape as raw NOTREADY. Keep the LINEIN BIF's existing caller-owned
  condition-policy transport unchanged. This narrow repair serves LC-81-04,
  PULL-AC01/05, G2-AC06 and CFC-AC02/04/09 without closing their broader scope.
  Reviewed shared interface: `RexxClassicStream.readEof()->boolean` reports
  only the most recent `readCharacters` or `readLine` attempt. Initialize it
  false, reset it before each such read's validation, set it only at the four
  existing ordinary EOF outcomes only when requested count is positive and
  the native transport is ready, and clear it on close. Classify the existing
  result without an early readiness guard: an OS-status failure must not block
  a later retry that the existing reader can recover. It is an observed
  read outcome, not a prediction of availability or a message-text comparison.
  In `lineinText`, sample the flag and existing error while the authoritative
  stream is linked, unlink it, then raise the existing NOTREADY only for a
  nonempty error without EOF. Preserve error text and stream/BIF state;
  no decoder, queue, read algorithm, C frontend, VM or condition ABI changes.
  **PULL-EOF-AC01 — open:** direct stream/configuration controls establish
  one line, a real empty line, an unterminated final line and repeated exhausted
  reads, including LINEIN-BIF-then-PULL on the same configured default. Queue
  entries, including present empty and NUL text, still take precedence and
  do not read or clear the configured stream's input position/outcome.
  **PULL-EOF-AC02 — open:** the typed flag is false before a read and after
  successful reads, explicit reposition/count-zero reads and close, including
  count-zero reads after `processInput` exhaustion. A closed,
  failed-open, unreadable or codec-failed input does not become ordinary EOF,
  including an alias closing the shared native handle after an earlier EOF.
  `lineinText` retains its raw NOTREADY for
  these non-EOF failures; the BIF retains its existing codec/SYNTAX precedence
  and OFF/CALL/SIGNAL transport. Reuse maintained LINEIN policy controls.
  **PULL-EOF-AC03 — open:** configured PULL and LINEIN hook replacements and
  genuine bound NOTREADY failures preserve precedence, exact payload/origin
  and callback cleanup. Host failures must not be swallowed by the default
  stream's EOF policy, even when their message is `End of stream`.
  **PULL-EOF-AC04 — open:** the maintained whole-PARSE source/oracle reaches
  `pull-eof=0` and its later output after `eof=0`; direct and compiled controls
  use the same shared runtime owner. Review coherent source and representative
  caller/helper RXAS, retain once-only acquisition and linked-stream cleanup,
  then qualify the affected normal Debug opt/rxbvm lane. Reuse prepared inputs;
  wider modes, platforms and maintained assurance remain BASELINE-QA.
  **PULL-EOF-STEP01 — done:** record this contract and obtain independent
  coordinator/source review before product edits; AC01–04. Active full normal
  CTest retains exclusive preparation/execution ownership.
  **PULL-EOF-STEP02 — implemented; review open:** add only the typed read-outcome flag/accessor,
  narrow Config EOF handling and focused maintained direct controls; update
  the existing stream/host guidance for this default-input contract. Inspect
  representative RXAS before freezing; AC01–04.
  **PULL-EOF-STEP03 — todo:** after the active run, centrally rebuild affected
  shared runtime images and consumers, run the direct fixture and whole-PARSE
  control plus the unchanged LINEIN policy boundary once, and update simple
  status from observed proof. Keep full baseline/CFC/platform acceptance open;
  no native memory mechanism or new sanitizer campaign is introduced.
- **C-DYNAMIC-DISPATCH-SIZE — local repair implemented:** the earlier selector
  repeated a full same-frame boundary loop at every resumable site, including
  cold call, transfer, signal and failure arms. The repair addresses this
  pathological caller growth and assembler work.
  Owner: C selector/RXAS lowering. Share the cold dispatch while retaining exact
  site successors, once-only child calls, source/TRACE and traceback locations,
  RETURN/EXIT/SIGNAL behavior and cleanup. Close with structural sharing
  assertions, independent control/state/error cases and representative
  code-size/resource checks. Selected mechanism checks qualify the local repair;
  the full research functional panel is authorized with broader acceptance open.
  **CDDS-AC01:** one same-frame cold dispatcher per owning compiled routine,
  with stable checked continuation IDs and bounded source-mapped child/provider
  call stubs. Unknown IDs refuse; the dispatcher adds no private helper frame,
  public ABI/ISA or second runtime algorithm. One site adds only bounded site work, not another
  copy of the whole boundary loop or signal/label table.
  **CDDS-AC02:** normal/static completion, nested child calls, child failure,
  RETURN/EXIT and lexical/grouped SIGNAL preserve exactly-once resumption,
  result ownership, numeric/handler/pool state, balanced signal guards and child
  ticket cleanup. Terminal paths invalidate continuation state; site-specific
  live DO, command and private-pool owners are not merged by assumption.
  **CDDS-AC03:** authored child/provider calls retain their exact source and
  traceback position; shared protocol code has truthful generated provenance.
  TRACE source/value count and order, including recursion and caught failures,
  do not gain a second authored clause observation.
  **CDDS-AC04:** the maintained small CALL source has one shared boundary loop
  and one lexical SIGNAL landing table, with no repeated terminal epilogue for
  equivalent obligations. Its optimized assembly and execution complete within
  a proportionate component-check budget; compare instruction classes, source
  records, bytes and assembler phase costs with the same input before broad QA.
  **CDDS-AC05:** an idle authored clause takes a direct local continuation,
  without constructing a dynamic boundary/argument packet, reinstalling
  unchanged numeric/TRACE state or searching all continuation sites. Actual
  pending CALL/HALT and dynamic work retain their exact clause boundary,
  captured decision/result, source and cleanup semantics. Cold continuation
  routing has a stated bound and rejects invalid IDs. Batch 1 uses the private
  target adapter `clausepending(context)`: check program EXIT/host failure first,
  then pending CALL without consuming it. Unlike repeated inline getter windows,
  one scalar call keeps the caller bounded; proven static no-handler bodies
  retain their existing omission. Run the predicate before `bpoff`, using the
  existing target-helper TRACE exclusion; an idle result leaves TRACE unchanged.
  Its caller takes at most eight instructions: one context-reference COPY,
  counted-call setup/call/argument clear, a false branch, then boundary and
  completion-carrier clears. This excludes surrounding source/EXIT observations
  and ordinary helper-frame work, not an assertion of allocation-free execution.
  Clear any preceding static boundary at this completion's former replacement
  point. A true result uses the unchanged checkpoint and same-boundary draining.
  Native HALT ingress and the pending-read/return decision boundary stay intact.
  Every cold route uses existing integer BLT splits over flat BEQ leaves of at
  most four checked integer IDs. For N sites and `G=ceil(N/4)` leaves, route
  code has `N+2*G-1` branch instructions; successful routing takes at most
  `ceil(log2(G))+4` conditional checks. Small maps retain their flat shape.
  Invalid negative, zero and out-of-range IDs refuse; suffix-separated labels
  cannot route to foreign stubs. This retains O(N) code: a flat map has fewer
  records but linear worst-case checks, while per-ID median trees add more
  records/labels. The bounded hybrid is an algorithmic tradeoff, not a language
  compromise or speed verdict. Recursion keeps
  continuation state frame-local. INTERPRET invalidates mutable facts whose
  premises it can change; it does not globally disable optimisation.
  **CDDS-STEP01 — done:** establish the independent source/RXAS attribution,
  no-opt versus optimized assembler diagnostic and same-frame design review;
  AC01–04. **CDDS-STEP02 — done:** implement the
  shared dispatcher and source-mapped call/continuation stubs together with a
  bounded idle fast path; share equivalent terminal tails while retaining
  site-specific cleanup; AC01–03/05. Implement the bounded predicate/cleanup and
  all cold-route families together; keep packet and catalogue repairs separate.
  **CDDS-STEP03 — done for selected mechanism scope:** inspect small and CALL
  RXAS and qualify the bounded idle/route, state/error/TRACE and source behavior
  through focused normal checks. The full functional panel and remaining fixed
  representative reviews retain their separate scope; this is no speed verdict.
- **C-EMISSION-PROPORTION — bounded representative review:** each selected
  area needs a meaningful RXAS sanity disposition against its source purpose
  and independent semantic obligations. Separate
  instructions from metadata and literal payloads, authored work from helpers,
  and hot-path cost from shared cold machinery. Review calls, copies,
  allocations, frames, branches, cleanup and source/TRACE records; line counts
  alone establish neither quality nor speed.
  Bind each disposition to the source/request, prepared producer and dependency
  identity, including compiler build mode and semantic IR options. Distinguish
  current output from older artifacts. Assembler optimisation and target-VM
  consumption remain separate axes; consuming identical RXAS again is not
  another emission review. Compile-refusal cases explicitly require no
  successful new RXAS and retain failed-output preservation checks.
  Selected runtime-error cases still require valid emitted code. Source-purpose
  review alone is not an emitted-shape pass; missing or stale selected output
  remains unverified. Representative coverage does not remove any registered
  functional scenario or require a new capture of every internal variant.
  Owner: C frontend/IR selection and RXAS lowering. Preserve immediate SIGNAL,
  delayed CALL at authored clause completion, EXIT/HALT, recursion, captured
  decisions, TRACE observations, once-only values and same-frame cleanup.
  Dead-source validation does not itself require runtime dynamic capability on
  a proved unreachable path. Idle NOP, assignment and SAY need bounded work
  independent of source-site count. Immutable catalogue facts may be shared
  under proved existing lifetimes; mutable policies, pools and invocation state
  remain separately owned. Prove lifetimes before consuming packets, shortening
  cleanup or reusing storage. No public ABI/ISA change, provenance loss or
  nonconformance is an implicit cost repair.
  **CEP-STEP01 — in progress:** map all registered research groups to functional
  and representative emitted-code coverage. Bind selected reviews to actual
  producer/source identities and independent expectations; distinguish current,
  older, missing and refused output. Further capture-harness expansion is held.
  The audited CALL, PROCEDURE, EXIT, label, TRACE, SIGNAL and condition-observation
  checkers, C-context and frontend-grammar VM cells, and PARSE-core checkers use
  private work roots or read-only prepared products. Under the configured
  ordinary QA job budget, admit at
  most two by charging each half that budget, rounded up; retain their modes,
  workloads, timeouts and oracles. Initialize the existing central job setting
  before registration, without another policy default. Native-audit scheduling
  and all other locks remain unchanged. This is admission control, not a
  measured memory bound or a guarantee for a larger manual CTest job count.
  The retained inspection interfaces below are available tooling contracts,
  not a requirement to collect every variant before functional regression.
  Group 2/3 owners provide an explicit emit-only worker mode: retain their
  distinct request schemas, compile/refusal diagnostics and prior-output
  checks, then skip application assembly, linking and execution. Emit-only
  results are EMISSION, never functional qualification. Group 2 may use only
  an explicitly declaration-only fixed routine ABI for dependent imports;
  compare the complete declaration with the actual provider RXAS before
  accepting dependent emission. Normal regression uses the real provider.
  Source-only collection reuses the Group 3 worker's reserved tenth field for
  semantic IR selection: empty retains the existing on default, and only
  explicit zero or one is admitted. The adapter exposes this option only in
  emit mode and records the actual selection with the fixed optimized B
  compiler closure and rxbvm producer identity. Group 2 keeps its distinct
  routine/binding schema; raw, loaded and transformed IR requests retain
  their exact existing graph constructors and require owned output export,
  rather than source approximations. No assembly/VM consumer axis is
  relabeled as a different compile request.
  The first raw-output export family is the existing CALL, PROCEDURE and
  EXIT contract owners. An optional trailing emit-only directory preserves
  their normal arguments, graph constructors, required checks and refusal
  oracles. Stable owner/case/variant identities distinguish fresh, loaded,
  transformed, packet, forged-schema and bounded requests. Successful
  requests export their exact frozen graph and owned RXAS; refused requests
  retain zero output and prior-owned-output checks. Protocol-only cases
  are explicitly identified without inventing an emission. Emit-only
  records are distinct from functional qualification and do not assemble
  or execute target C code. Import-root variants keep their exact owners.
  The following export family is SIGNAL, protected SIGNAL conditions, TRACE
  and label-entry contracts. Capture their actual source-selected, fresh,
  loaded, rewritten and prechecked outputs, including prior-output,
  bounded and replaced-owner refusals. P04 profile-only proofs and positive
  frontend/AST-only checks retain explicit no-emission dispositions; frontend
  compile refusals retain checked diagnostics and no RXAS. Preserve exact
  source bytes when no graph survives a request that is overwritten in a loop.
  Do not introduce extra selection merely to produce a review artifact.
  Forged schemas remain embedded in their exact temporary graph images;
  these owners introduce no overwritten external import input. Preserve
  normal arguments/oracles and the same START/disposition/completeness
  contract before target assembly or execution.
  Assignment, DROP and selected PUSH/QUEUE insertion form the next state
  export family. Retain the existing supplied-context provider outputs,
  exact raw-name and context/origin negatives, loaded controls and bounded
  insertion refusal. Emit-only provider destinations belong to the owned
  temporary request directory. Frontend-only RHS/span checks remain
  no-emission cases with exact source bytes; do not add target selection.
  PUSH and QUEUE retain distinct request identities and instruction
  arguments. No external input is overwritten by these owners.
  Named AST/schema-only groups supply separate no-emission dispositions;
  they add neither a selector request nor an invented frontend refusal.
  Context admission, prepared-language ownership and the Group 3 source-map
  IR owner form the following foundational export family. Keep every
  existing context selector profile and its actual output/register budgets,
  ABI/schema/slot/source-origin controls, transfer and DO variants. Preserve
  prepared-language valid and failed-initialization producer images as
  distinct identities; the failed grammar lane generates no C RXAS.
  Source-map fresh, loaded, transformed and bounded requests retain their
  exact frozen graphs. Protocol-only ownership/work checks remain explicit
  no-emission cases. No external input is overwritten in this family.
  Request context records carry the actual import root and decimal output and
  register budgets; serialize int64 allowances as strings in transient JSON.
  Command warnings/views and conditional CALL observation form a bounded
  export pair. Capture their actual raw/reloaded/rewritten selections,
  prechecked reuse and local limits. Warning access through compiler
  graph/warnings APIs remains frontend-only: it must not force lazy
  selection. Preserve exact source/logical-name bytes, request budgets
  and the deliberately missing import root for the existing full-service
  refusal. Views and proof-only ownership checks remain no-emission
  cases. External-provider/native metadata controls retain a separate
  owner batch and their actual import variants.
  Observe lazy service requests through their completed graph state, never a
  new full-service status/output call. Store logical names as data files, not
  filesystem names; the full-service refusal still requires zero returned bytes.
  The first source-CLI emit family is clause clocks, commands, NUMERIC,
  labels, EXIT, RETURN, PULL, PUSH and QUEUE. Keep each owner's actual
  compiler boundary, generated/physical source bytes and existing source,
  warning, refusal, diagnostic and previous-output checks. An explicit
  emit phase requires its existing inspection destination and captures the
  complete owner inventory before skipping C target assembly/link/execution.
  Retain the configured B host mode, producer VM, semantic IR option, exact
  compiler/import/provider identities, bindings, logical names and limits;
  a fixed optimized worker does not stand for other producer lanes. Capture
  existing test-only entry-export rewrites as derived outputs of their exact
  base RXAS, not extra compiler requests. Independent request/disposition and
  derived-output counts must agree with each owner recipe. Results are
  EMISSION, not functional passes. Preserve normal arguments and markers;
  do not run B raw contracts in this source mode, invent a refusal for clause
  clocks, add a fake assembler or introduce a shared inspection framework.
  PUSH and QUEUE retain their existing shared orchestration and distinct IDs.
  NUMERIC raw admission and PARSE packet admission form the next export
  pair. Preserve NUMERIC's fresh/loaded controls, actual typed-spill rewrite,
  stale-family refusals and every existing bounded-output request. Its parser
  and descriptor-only groups remain explicit no-emission cases. PARSE keeps
  each fresh/loaded packet mode, including retained-origin and dead malformed
  inputs, without inventing transforms or limits. A trailing emit-only directory
  preserves normal arguments and checks; export exact frozen graphs and owned
  RXAS, with a separately checked complete disposition inventory. These owners
  overwrite no external import input and execute no target application.
  External-provider admission retains its original default, typed/process and
  host-failure profiles as distinct export inventories. Preserve exact ordered
  input bindings, roots, source/logical names, exports, first-accessor order and
  limits; capture no extra budget-charging prepared-owner observations. Export
  completed service output while its graph owner is alive, then return that same
  owned output for the existing expiry check. Empty-inventory graph access stays
  frontend-only; stale/expired snapshots and early import refusals retain their
  actual argument state and zero-output oracles. Metadata/task-only groups have
  explicit no-emission dispositions, with original metadata input and driver
  identities; in-memory mutations do not become external file writes.
  Clause-clock and ADDRESS owners extend their partial inspection seams to
  complete emit-only inventories. Preserve each original optional inspection
  argument and use a separate trailing emit-only directory. Capture fresh,
  loaded and genuinely transformed graphs, early entry-owner refusals and
  borrowed prechecked selections with their actual measured output caps.
  Retain every existing bounded request, prior-owned output and immutable-input
  check; schema, parser and proof-budget-only groups remain explicit
  no-emission dispositions. Existing partial output writes keep their original
  meaning. Add no selector, transform, limit or target execution to collect them.
  **CEP-STEP02 — done:** retain the reviewed idle/cold-route, fresh-packet,
  immutable-catalogue and stable-profile repairs, selected checks and completed
  registered research functional gate and authorized local commit; no lane is dropped.
  **CEP-STEP03 — done:** retain exactly six representative
  areas: typed/process external bindings; authored PARSE frontend source;
  nested dynamic/condition resumption; source-loaded expression/CALL arguments;
  actual noopt/IR-off C RETURN cleanup/calls; and existing G cleanup/control.
  Use the maintained selected cases and exact producer axes, reusing valid
  outputs and checks. No new helper, capture seam or mode matrix is required.
  Full CFC/BQA, platform, sanitizer and publication acceptance remains open.
  **CEP-STEP04 — done for selected A/B cost repairs:** retain the existing
  structural and focused checks; broader acceptance remains open.
  - **TRACE helper rejection:** keep the exact mode/breakpoint/module gates,
    lazy context owner and case-sensitive `llctarget.` helper set. Read its
    procedure through the existing linked getter and apply the same prefix
    rejection before `_trace_should_trace_context` prepares source/content.
    Preserve nonhelper controller filters, source/value observations, interrupted
    frame reads, receiver/context cleanup and `ret 0`. No new suppression,
    runtime API or VM behavior is selected. The emitted ordering and unchanged
    maintained TRACE check qualify this selected repair; no whole-panel restart.
  - **Provenance wrapper:** select the existing `provenance()` group through
    `backend-tests`' existing third-argument dispatch. Keep the full default
    aggregate in that backend owner and retain every provenance source,
    runtime and error case. The focused wrapper reuses the existing fixtures and oracles;
    existing wrapper and unknown-selector controls qualify this selected repair.
  - **G21 unread scalar writes — open opportunity only:** generic/G forwarding may
    remove unread scalar transports only with proved liveness, all implicit
    uses and source/TRACE observation obligations. No G code change is selected.
- **C-CALL-PACKET-COST — separate C ownership repair:** ordinary CALL,
  expression/BIF/provider and source-resolver arguments currently append to a
  persistent whole packet by copying each growing prefix. Owner: C frontend,
  L ownership proof and existing activation packet service. Permit a consuming
  append only for a fresh, unpublished, single-owner chain; shared IR packet
  values retain value semantics. **CCPK-AC01:** actuals and omissions preserve
  left-to-right once-only evaluation, typed errors, failed-capture cleanup and
  EXIT/SIGNAL unwinding. **CCPK-AC02:** emitted construction has no whole-prefix
  COPY per appended argument and refuses the consuming route if an earlier
  packet value remains live. The optional proof belongs to the existing C
  entry-protocol owner and binds to the frozen graph/index identity. Reuse the
  shared all-record use/read/write index, including dead and handler records.
  The selector explicitly requests this optional pass; frontend/IR execution
  keeps required validation unchanged. Stage generic analysis metadata at this
  first compiler-host consumer, without pulling RXAS selection into the frontend.
  Resolve packet-slot readers to exact ordered writer versions; an unresolved
  reader prevents dead-store claims for that slot. Ignore a store as transport
  only after proving its version unread. This admits ordinary IR-on forwarded
  SSA beside unread stores as well as direct SSA and private-slot chains.
  Each fresh-seed chain has one semantic consumer per version and one final
  admitted call/BIF. Reject the whole candidate on forks, foreign writers,
  escapes or unsupported control; keep the persistent helper for unproved
  shared inputs. Memoized linear regions of unique checked program-false edges
  permit nested-call continuation without per-argument CFG walks; joins,
  cycles and exceptional reads do not gain that privilege.
  Optional work/workspace exhaustion declines optimisation without changing
  required C admission failures. A live shared index may belong to a distinct
  module value-copy of the same frozen revision as a borrowed frontend proof.
  At this C boundary, use the proof's existing belongs check on index.owner()
  after index.valid(); retain generic index.matches as exact storage identity.
  Reloaded/transformed revisions and stale or expired owners still decline.
  The existing packet controls retain exact-owner paths and add a same-revision
  copy positive plus foreign/reloaded and expired-index negatives, without new
  target selections or weakening the fresh-source no-prefix-copy oracle.
  Coalesce only proved packet versions into one
  fixed private owner register, outside call windows. Existing append owns the
  mutation; retain actual-value/presence copies, the existing presence conversion
  and final call marshalling.
  Elided transport instructions retain source/TRACE records. Release private
  prefixes on abrupt cleanup, preserving completed results and outer prefixes
  across ordinary nested calls. No MOVE, new schema, VM operation or packet
  runtime algorithm is introduced.
  **CCPK-STEP01 — done:** identify the independent
  source/RXAS ownership cost. **CCPK-STEP02 — done for selected mechanism scope:**
  implement the consuming path, inspect representative emitted RXAS and qualify
  the fresh/shared proof, nested call, failure and cleanup paths through focused
  checks. The full functional panel and fixed representative reviews remain
  separate from this local mechanism qualification.
- **B-SEED-ARRAY-TRACE — open; baseline source-observation repair:** the native
  B array emitter treats a `NOVAL` length selector as a resolved compound tail,
  recording its unwritten temporary as a register-backed `C` event. Enabled
  Intermediates TRACE consequently observes unrelated register state. Owner:
  native B/shared backend compound-event construction in `compiler/rxcpemit.c`;
  native C frontend, TRACE runtime, register allocation and ISA stay unchanged.
  Omit the event for the absent tail rather than manufacture a name or value;
  retain meaningful index/stem events and existing length-value observations.
  **BATRACE-AC01 — open:** fixed/open array length forms, including first and
  later empty dimensions and the zero-index length spelling, emit no
  register-backed compound event for `NOVAL`; their value/assignment events,
  authored source anchors, index evaluation and linked-owner cleanup remain
  correct. Structural checks follow operand roles, not register numbers.
  **BATRACE-AC02 — open:** an enabled Intermediates runtime control reports the
  independently expected lengths without a stale `>C>` value; a real variable
  index and existing stem-sugar controls retain their live compound events.
  Qualify the affected compiler opt/no-opt RXAS and one normal Debug/rxbvm
  consumer, then reuse the matching maintained baseline gate. The original
  `val_magic_01` compiler-only fixture does not establish runtime TRACE proof.
  **BATRACE-STEP01 — done:** record this contract and obtain coordinator/source
  review before product edits. **BATRACE-STEP02 — implemented/reviewed; RXAS check open:**
  add the narrow `NOVAL` event guard and maintained structural/runtime controls;
  inspect representative array-length and live-index RXAS before freezing. Update only
  affected goldens after semantic review. **BATRACE-STEP03 — todo:** centrally
  execute the selected checks after the active ordinary QA run, preserving
  valid prepared inputs and open BASELINE-QA/CFC/platform acceptance.
- **B-SEED-ALIAS — repaired; baseline qualification open:** native B register
  allocation reserves indexed attribute receivers and multidimensional linked
  results before descendant temporaries/call windows. Maintained class/local
  array regressions check nested method indices, reads/writes, once-only effects
  and register interference in both compiler modes. Retain the C ADDRESS
  selector's scalar indices and owner/clause shape controls.
- **B-SEED-INLINE-RECEIVER — repaired; baseline qualification open:** native B
  inline binding follows the caller's explicit callable provenance through
  nested lexical scopes before considering a foreign generated callee scope.
  The isolated `current.advance(_plan.next(op))` regression checks caller and
  factory fields, nested/skipped paths, once-only effects and retained expansion
  across compiler/assembler modes and available VMs. The L runner's neutral
  next-PC slot/control consumer remains. No native frontend change is needed.
- **B-SEED-CLASSIC-CATCHALL — repaired; baseline qualification open:** typed B
  catch-all emission includes admitted CLASSIC_SYNTAX/CLASSIC_CONDITION in its
  existing balanced handler list. Maintained explicit/catch-all regressions
  preserve exact binary/object payloads, original module/address/authored source
  and normal/LEAVE/RETURN restoration. Source-observation tests deliberately
  retain linked source; stripped-image tests retain their separate profile.
- **VM-REF-OWNER-COST — open under BASELINE-QA:** the warm dynamic compiler's
  second of 43 sequential requests reached `START` but no `COMPILED` marker;
  a bounded sample was dominated by `MKREF_REG_REG` owner discovery recursively
  scanning `unlinked_attributes` up to backing capacity. Repeated traversal of
  a large owned tree is the source-based hypothesis; a cycle is unproved and
  would also violate recursive lifetime cleanup. Owner: generic VM reference/
  lifetime service, with Level L construction callers.
  **VREF-AC01:** bounded reference acquisition preserves exact local, borrowed
  argument, nested physical attribute, linked external/global and expired
  reference lifetimes across normal return, unwind and frame reuse.
  **VREF-STEP01 — in progress:** derive one small RXAS control from
  `tests_perf2_reference_guards.rxas`: grow an unrelated acyclic backing tree
  before a later linked external target, repeat fallback `MKREF`, and check
  value/validity before and after each owner ends. Use bounded visited-node and
  active-path diagnostics on the original trigger to distinguish finite scans
  from a cycle; preserve the first representative source/RXAS review.
  **VREF-STEP02 — implemented; coordinator review open:** review a narrow VM-owned repair against frame/attribute
  ownership, especially caller-borrowed arguments and inactive backing slots;
  do not mask a cycle only in owner discovery or duplicate ownership in L.
  Private VM contract: a fallback `MKREF` marks every active frame
  from the current frame through its ancestors for reference-lifetime cleanup.
  Each frame's exit still invalidates only its own physical locals and `a0`
  trees, never borrowed `a1+` or linked external/global storage. This bounds
  acquisition by call depth without changing reference identity, API or ISA;
  the direct owned-local/attribute guards remain. Confirm acyclic backing
  storage and the caller/linked-target controls before accepting this repair.
  **VREF-STEP03 — todo:** run the focused normal Debug RXAS reference/lifetime/
  unwind checks on rxbvm and a representative rxtvm control, then the matching
  maintained sanitizer checks through `tools/asan-run.sh`; replay the original
  dynamic check before resuming BASELINE-QA. Compare observed work without an
  unmeasured speed claim or a separate production performance campaign.
- **Scope:** language/API/ISA/platform exceptions, default switching, native-C
  retirement and release/publication remain explicit maintainer decisions.

## Baseline terminal-diagnostic repair

**DIAG-STEP02 — implemented and reviewed; qualification open.** This repairs
C-DIAGNOSTIC-ORIGIN under BASELINE-QA and CFC-AC03/10/11. The generic VM owns
diagnostic transport and terminal rendering; C keeps its source maps, raw
conditions and existing fatal policy. No public API, ISA, binary format, raw
five-slot layout, wrapper layout or value ABI changes are approved or needed.

The following closure contracts remain open until verified:

- **DIAG-AC01:** snapshot the complete original ordinary caller chain before
  first unwind, including suspended same-context native/initializer callouts.
  Validate return addresses and canonical coordinates using module-owned
  procedure bounds; reuse existing source resolution and escaped rendering.
  Linked imports carry foreign
  procedure offsets and must not truncate a local caller's range; a real
  cross-module collision control and the authored divide oracle qualify this.
  Skip signal-entry pseudo-call edges. Detach names and
  length-bearing source data; retain no live frame, module, worker, value or
  metadata pointer. Diagnostic scopes are independent of invocation authority.
- **DIAG-AC02:** one immutable resource belongs to the raw container's existing
  native-payload sidecar, separately from its unchanged payload and origin.
  Existing copy/move/zero/finalizer services carry ownership. Use checked atomic
  retain/release and process-heap storage; never mark it bit-copy-safe. Explicit
  captured-empty and unavailable states prevent recapturing a later rethrow.
- **DIAG-AC03:** execution-local pending owners and alias-safe SIGNALORIGIN
  preserve the particular event across stored/copied/reordered same-tag faults.
  Fresh signals replace ownership; ignored/handled delivery and teardown release
  pending references. Checked events/initializer sinks retain their existing
  full-event semantics. Private native callout/callback cells restore on every
  exit and repeated/nested use without a global last-failure cache.
- **DIAG-AC04:** preserve CALLMETHOD's existing code, generic message and outer
  origin. Its immediate private adapter may transfer only this callback's
  diagnostic owner after existing setters. Public code-only callback APIs gain
  no inferred event. Normal provider signal setters clear diagnostic state.
- **DIAG-AC05:** render once only inside existing HALT/fail-action terminal
  guards after callback/initializer capture declines. The captured chain replaces
  live-caller fallback, preserving one PANIC and the original raw-origin report.
  Local handlers, owned host completions and recovered callbacks/initializers
  remain silent. Overflow/allocation/retain failure preserves the original
  signal and carries an allocation-free unavailable witness; no arbitrary
  truncation or diagnostic-only failure may replace the event.
- **DIAG-AC06:** keep installed divide-error and numeric-fatal authored-location
  oracles. Independent stored-event, alias/NUL/object, nested helper/callout,
  callback/initializer, context-reuse/worker and allocation-failure controls
  verify ownership and silence. Review coherent source and a representative
  emitted case, batch material repairs, then freeze. Run affected normal checks
  before matching maintained native checks and resume full baseline gates.

| Stable step / status | Work and dependency |
|---|---|
| **DIAG-STEP01 — done** | Record and independently review the private carrier, copy/failure/reentry rules and unchanged public contracts before product edits; AC01–06. |
| **DIAG-STEP02 — done** | Implement and independently review generic VM transport/rendering and affected focused regressions; AC01–05. No C-specific logic enters the VM. |
| **DIAG-STEP03 — done** | Review coherent source and retained RXAS; qualify the selected normal component profile, actual authored divide error and matching maintained checks. Wider AC06 delivery/source obligations remain with BASELINE-QA. |
| **DIAG-STEP04 — todo** | Update owning guides/simple status, delete scratch and complete BASELINE-QA before commit/publication. Full-platform closure remains explicit. |
