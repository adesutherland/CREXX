# Level C implementation work packages

The [plan](../PLAN.md#c-first-delivery-milestone) owns acceptance/status;
[architecture](architecture.md) owns contracts and [requirements](levelc-requirements.md)
owns the crosswalk. This file assigns ownership. It does not commission product
work, tests, automation or branches. The repository
[AGENTS.md](../../../../AGENTS.md#development-testing-and-delivery-lifecycle)
is the single testing-lifecycle authority; do not add package-specific default
matrices or duplicate its publication/sanitizer rules.

## Coordination model

Adrian requested an Astra Extra High coordinator and GPT-6.1 Sol Extra High
workers (`model=gpt-6.1-sol`, `reasoning_effort=xhigh`); do not silently substitute
a requested model. The coordinator owns contract/interface decisions, review,
scheduling, acceptance validation and commit/publication authority within current
user scope. Sol workers implement, integrate, document, build and clean up.
The coordinator validates worker results rather than duplicating execution.

Use synchronized existing `develop`, preserving unrelated work. No new branches
or worktrees. Before writes, grant exclusive named paths and record baseline
HEAD/unrelated edits. One integrator owns shared schemas/CMake/plans and the
frontend/selector stream. Workers cannot commit without explicit authority,
push, stash/reset/clean, switch branches, alter automation or launch unassigned
builds. With four slots, coordinator plus at most three disjoint workers can
cover generic L, runtime and independent expectations/review. One owner schedules
shared preparation; do not overlap shared-tree builds/tests.

Review the instruction's independent expectations and exact contract before
implementation. WP-12 joins at coherent source/first representative RXAS so
material fixes can be batched. Final inspection covers scope, preservation and
changed material deltas, not an automatic full rereview. Use AGENTS to select a
proportionate development check or agreed milestone QA; a completed feature/local
commit does not close full-C acceptance. Record accepted minor deviations in the
[conformance statement](levelc-conformance.md); do not weaken semantic oracles.

## Work package catalogue

Packages define ownership, not partial-instruction completion or automatic test
campaigns. Necessary dependencies belong to the instruction batch. Acceptance
IDs below remain open until their agreed outcomes are verified; selecting fewer
checks during development does not remove later milestone obligations.

| Package / owner | Allowed implementation area | Inputs and deliverables | Decisive acceptance / dependencies |
|---|---|---|---|
| **WP-01 Contract audit** | Read-only sources; assigned worker writes requirements/plan after coordinator review | Reconcile LC-I, BIF, LC-REF and approved exceptions; inventory current test paths, APIs/platforms and C-only host/target closures. Propose minimal independent fixtures and exact changed interfaces. | CFC-AC01/07/12; CLC-STEP01. No native status copied into new completion. |
| **WP-02 L analyses and generation** | Assigned `lib/rxfnsl/rexx/`, `langtools/rexx`, stamp source-closure manifest, independent `langtools/tests` and generic docs/examples | Existing generic analyses retain their contracts. The [approved LRUSE plan](../../PLAN.md#current-action-lexer-reuse) assigns pure generation-service/private main+shield capsule and automated stamp identity/closure here, with independent non-C/G admission/equivalence proof. Coordinate build injection with WP-10. | CFC-AC07/11; STEP02 and LRUSE-STEP01/02. Approval covers the recorded reuse contract; no SDK/runtime API or algorithm change, C semantic policy or new provider is commissioned. |
| **WP-03 Classic runtime** | Assigned `lib/rxfnsc` classes/tests | Reconcile activation/pool/config/value/condition ownership, direct operations and BIF ABI; live pool binding, shared body/entry contracts, cleanup, presence and context inheritance. Keep old consumers compatible. | CFC-AC02/03/09/11; STEP03. Preserve direct B/native C/RexxScript/typed Classic contracts; select affected checks under AGENTS; public ABI approved. |
| **WP-04 Shared B Unicode** | Assigned `lib/rxfnsb/unicode`, `tools/unicode` and affected data/consumer paths | Extraction/public B API, preserved G facade, one pinned data owner and acyclic bootstrap are completed foundations. Remaining assignments integrate new C names/BIF consumers; add property/simple-map/traversal/budget interfaces only for an identified need and exact API approval. | CFC-AC07/09/12; STEP03/04. Reuse unchanged B/G/core qualification; prove changed C recognition/bindings, errors and provider/freshness closure. Coordinate CMake edits; no same-file overlap with WP-03. |
| **WP-05 C source/expressions** | Assigned `levelc` grammar/parser/frontend/IR modules and compiler tests | Complete C-SRC/C-EXPR, source/AST schemas, static validation, Unicode names, operators, numeric contracts and BF01–04 binding. Use L and runtime APIs, no duplicate algorithms. | CFC-AC01–03/06/07; STEP04 after needed WP-02/03/04 contracts. Native comparisons are controls. |
| **WP-06 One C instruction** | Named grammar/frontend/IR/selector paths and a named instruction fixture directory | One LC-I row, all forms/errors/edges, worked AST/IR/RXAS, runtime interface use and independent expectations. Required shared improvements are explicit linked dependencies. | CFC-AC01–04/06; STEP05–07. Serial instruction writer; early independent review. Full closure waits for listed acceptance, distinct from local increment checks. |
| **WP-07 BIF/host family** | Assigned runtime family/adapter and tests; frontend binding edits coordinated | One BF integration group or C-HOST contract at a time. Reuse existing BIF bodies, fixtures and valid qualification; bind names/presence/context/errors through the new compiler. Complete only named missing host contracts. Library edits require a reproduced defect, identified open contract or approved Unicode extraction. | CFC-AC01/04/09/11; STEP06–08. New public host APIs require review; no alternate ADDRESS protocol or dynamic external-call policy. |
| **WP-08 C target quality** | Assigned C selector and `compilers/common` target utility paths; optional `lib/rxfnsl/rxas` package | Target records, liveness-driven register reuse, call windows, ABI cleanup and C-OPT01–05. Explain legality and compare actual caller plus helper RXAS. | CFC-AC06/07/11; STEP09 incrementally. WP-02 generic facts; no target policy in SDK core. The optional target package has explicit ownership. No broad performance work for research. |
| **WP-09 RXAS/VM primitive** | Precisely assigned assembler/binutils/interpreter/disassembler/linker paths and direct tests | One reproduced cost or missing execution capability; compare existing sequence, library/selector fix and primitive. Propose exact operands/effects/alias/errors/source/encoding/platform contract before code. | CFC-AC10/11; STEP09 or final STEP11b. Explicit native/ABI gate; mandatory first Release verdict for production optimisation. No language parser/BIF dispatch in VM. |
| **WP-10 Delivery and independent QA** | Assigned CLI/build/test/packaging files; assigned worker owns shared root integration | Retain delivery/closure duties. Approved LRUSE-STEP03/04 owns CLI transport, output-derived REUSE_LEXER persistence/lock/checksum/publication, stamp build injection/dependencies/cleanup and initial research-C-only opt-in. Independently qualify non-C/G first, then focused C cold/warm and unchanged G controls; coordinate stamp manifest with WP-02. | CFC-AC08/11/12; STEP10/11d and [LRUSE](../../PLAN.md#current-action-lexer-reuse). Select development versus milestone checks under AGENTS; retain reuse/closure boundaries and no free shared cache path. |
| **WP-11 INTERPRET compiler/runtime** | Assigned C mode/entry/continuation and B runtime dispatch files | Final-stage mechanism proof, then same-pipeline group compilation, live caller binding, cross-owner outcomes, handler resumption and source observations. Interfaces agreed with WP-02 execution and WP-09 call/resource ownership. | CFC-AC05/11; STEP11 after static functionality and detailed implementation approval. No source compiler/evaluator fallback and no premature dynamic implementation. |
| **WP-12 Independent acceptance review** | Read-only implementation and temporary diagnostics; own test edits only if granted | Start at the early stable code/first-RXAS checkpoint: check contract/test traceability and effect/lifetime assumptions. Final inspection covers scope, preservation and changed material deltas. Findings name mechanism, affected criterion and reproducer. | Early in every increment; complete delivery review at STEP10/11d. Coordinator independently validates; worker assertions alone never close a criterion. No automatic whole-cycle restart. |

## STEP11 dynamic IR ownership

The [detailed STEP11 plan](../PLAN.md#dynamic-execution-plan) owns the selected
IR route, criteria, dependencies and todo/done status. The
[architecture](architecture.md#dynamic-execution-strategy) defines the concrete
control/call/resource/search contracts approved for this increment. Implementation
is locally qualified under the selected component profile. These assignments do not authorize
another QA campaign or close the formal compiler/platform/delivery gates.

| Owner | Dynamic increment responsibility |
|---|---|
| WP-01 / coordinator | Freeze reviewed interfaces, source-search policy, independent expectations and public/native gates before implementation. One integrator owns the optional C schema/profile, plans and shared frontend/selector files. |
| WP-02 | Precisely assigned `lib/rxfnsl/execute`, generic IR/analysis and neutral fixtures: manifests/protection/ports, conditional cells, binding leases, frame cleanup and protected abort. No Classic names/pool/search policy. |
| WP-03 / WP-07 | Shared B activation/pool/condition/BIF/host owners, compatible dynamic policy sidecars, invocation private-pool commit/restoration and numeric/observation integration. A group cannot own a committed PROCEDURE pool. |
| WP-05 / WP-08 / WP-11 | Frontend-only result/package split, program/group modes, immutable lexical snapshots, optional dynamic family, inline compiled-owner dispatcher and C runtime registry over existing shared services. Preserve static C24/body ABI. |
| WP-09 | Narrow checked native invocation boundary, owned windows/result completion and RET/unwind hooks, plus generic host-context/regular-source acquisition interfaces under their existing VM/fs owners. No new opcode, source frontend, BIF semantics or native C feature. Assign these native mechanisms separately from source-only work. |
| WP-10 | Optional dynamic build/install closure, prepared grammar/provider retention, driver/resource-view configuration and focused linked/relocated consumers. Centralize shared build/test preparation. |
| WP-12 | Review the hand-prepared IR/first emitted bridge early, verify parent/child control and persistent policies/private pools, then review affected material fixes and whole-feature criteria. Reuse maintained sources/checked expectations; formal CFC/platform QA remains later. |

The first proof needs only minimum port/abort and selector/native-call support;
full generic/C registration and parser integration follow it. Grant disjoint
named paths before writes and serialize shared integration. Native ownership
changes receive focused normal/affected-engine/maintained-sanitizer qualification;
unchanged native tools do not trigger a new sanitizer campaign for C/L edits.

## Group 2 ownership

The [Group 2 plan](../PLAN.md#group2-calls-host-plan) and
[contract](architecture.md#group2-calls-host-proposal) own the calls/host increment.
WP-05/06 own Classic name resolution, static admission and value adaptation;
WP-03/07 own activation, request/completion, callback windows and configured
resources. WP-09 owns additive embedding spans/interrupt ingress and the thin
optional metadata provider over binutils and the native loader. WP-10 owns
prepared bindings, CLI selection and component consumers; WP-12 reviews coherent
source and representative typed-call/host RXAS before final qualification.

Ordinary callees retain their ABI and capabilities. Generic Level L owns no
Classic call-search, pool or result policy. Process-backed calls and ADDRESS
share the existing child/byte-channel owners. Native C frontend, automatic
Classic source invocation and full compiler/platform QA remain outside this
increment; INTERPRET stays last.

## Group 3 ownership

The [static Group 3 plan](../PLAN.md#group3-static-language-plan) and
[contract](architecture.md#group3-static-language-contract) commission one
complete static language increment, locally qualified for its admitted static
profile. WP-05 owns source/header/classification,
expressions, dead-source diagnostics and C24 IR integration. WP-03 owns the
minimal existing `llctarget.binaryop` adaptation, Config source-inventory reset
and the C-owned `llcsrcmap` adapter. Shared B retains value, numeric, pool,
Unicode and all 95 BIF algorithms; generic L retains physical source/IR owners.
The Classic C casing boundary is implemented in B but does not change generic
B/G Unicode semantics or explicit full Unicode casing.
The integrator owns selector provenance/inventory emission, shared schema/counts,
CMake, prepared builds and the local commit.

WP-07/10 owns `tests/group3` and the maintained native-source/checked-oracle
crosswalk, including direct source-map and raw/loaded C24 admission fixtures.
WP-12/coordinator owns coherent source and first representative RXAS review
before the finite prepared Debug C-opt/IR-on/rxbvm lane. The
[BIF family crosswalk](levelc-requirements.md#group3-bif-crosswalk) distinguishes
new C transport/context proof from reused library qualification. SRCMAP has no
original SOURCELINE inventory; supplied line text belongs to provenance only.
Native C remains read-only. INTERPRET, new automatic Rexx-program invocation,
publication and full CFC/platform/delivery/formal QA remain outside this local
increment, with their existing acceptance and decision gates preserved.

## Group 4 ownership

The [Group 4 plan](../PLAN.md#group4-infrastructure-plan) and
[architecture](architecture.md#group4-infrastructure-proposal) own approved
STEP01–08, including the
[bounded execution contract](architecture.md#group4-execution-contract) and
[package/C closure contract](architecture.md#group4-package-closure-contract).
The reviewed contract does not commission new ISA work, language migrations
or INTERPRET.
WP-02 owns shared graph/analysis/proof and optional execution coordination;
WP-08 owns C selection migration and optional RXAS target/allocation integration;
WP-03 owns existing semantic runtime interfaces; WP-09 retains canonical native
opcode/effect/signal and any exact native interface gate. WP-10 owns separated
package/build/CLI closure. WP-12 independently checks the hand-derived proof/
allocation cases and early coherent source/first target records/RXAS.

One integrator coordinates schema/profile/version changes and shared CMake/docs.
Reuse existing SDK/assembler/runtime services and fixtures before introducing a
new owner. Optional cREXX/RXAS packages can own their domain knowledge; general
L core still cannot import a source frontend or hard-code its language policy.
Keep source-flow mappings separate from executable IR analyses, and runtime
execution handlers separate from compile-time constant callbacks. Exact paths
receive disjoint leases when an implementation increment is authorized; only one
owner schedules shared preparation. Component checks follow AGENTS, with formal
whole-compiler/platform QA later.

## Instruction contracts and ownership

Completed packet choreography is retired. Future edits receive fresh explicit
path assignments under the roles above. The following links preserve contract,
acceptance and ownership boundaries; current statuses belong to PLAN.

<a id="call-assignment-packet"></a>

### CALL and completion

The [CALL contract](architecture.md#call-contract) owns CCALL-AC01–07 and
CALL1–4. The serial C integrator owns grammar/frontend/IR/admission/selector/
target and shared docs/CMake; independent writers own assigned `tests/call`
fixtures, and review remains read-only. CALL2/3 use the
[CEXT contract](architecture.md#call3-static-provider-contract) and generic
classifier; preserve prepared-language reuse and existing B algorithms.
Delayed CALL4 remains open after condition/clause foundations. Existing local
[RETURN](architecture.md#return-contract) RRET-AC01–05 and
[EXIT](architecture.md#exit-contract) CEXIT-AC01–05 retain packet/result cleanup;
wider completion CEXIT-AC06 stays open. Their fixture directories have independent
owners; no incidental grammar/runtime/ISA changes are authorized by those packets.

### Queues, ADDRESS and NUMERIC

[PULL](architecture.md#pull-contract), [PUSH](architecture.md#push-contract) and
[QUEUE](architecture.md#queue-contract) have delivered local AC01–04 routes and
open shared host/observation/platform AC05. C production/schema/docs belong to
the integrator; independent fixtures use `tests/pull`, `tests/push` and neutral
`tests/queue-insertion`. Existing B owns queue/configuration algorithms.

The [ADDRESS bridge](architecture.md#address-b-binding-bridge) owns the closed
shared-B `_address.crexx`/`RexxClassicAddress.crexx` foundation. The
[C ADDRESS contract](architecture.md#address-c-command-contract) owns parser,
keyed proof and selector integration. Independent fixtures are `tests/address`
and `tests/command`; CADDR-AC08 and full LC-I-20/21 keep their shared obligations.

The [FORM correction](architecture.md#numeric-form-diagnostic-prerequisite)
owns the completed activation-validator diagnostic change. The
[NUMERIC contract](architecture.md#numeric-c-contract) assigns C source/schema/
selector to the integrator and `tests/numeric` to its independent writer;
NUM-C-AC01–06 are delivered; CCOND integrates admitted observations, while
AC07 retains wider numeric/expression, host and platform scope.
These packets do not commission another numeric or host algorithm.

### Labels, clauses and external condition scope

[CLABEL](architecture.md#label-arm-prerequisite) AC01–05/STEP01–03 are complete.
Its grammar/classifier/frontend ownership is serial; independent expectations
live in `tests/labels` and the named CALL grouped-label controls. Preserve
required-arm ownership, DO/SELECT depth, first-label resolution and natural
node landings. [Clause clocks](architecture.md#clause-clock-contract) and the
[external B condition scope](architecture.md#external-condition-b-prerequisite)
are delivered foundations. CCOND below owns delivered delayed CALL and authored
completion; broader host/platform acceptance retains its owner. Former
prerequisite source leases are closed.

<a id="signal-assignment-packet"></a>

### SIGNAL

[Whole SIGNAL](architecture.md#signal-complete-contract) CSIGNAL-AC01–07 and
STEP01–03 are complete: direct/quoted/VALUE, seven-condition ON/OFF, real
producers, visible exceptional flow, same-frame cleanup and typed payload/origin.
SIGNAL introduced C20 with 85 operations, five C types and 36 target exports;
the current family is C22 after delayed CALL/clause completion. Its generic, B runtime and SIGNALORIGIN
dependencies belong to the same instruction.

The integrator owns shared production/runtime/CMake/docs and preparation;
disjoint proof work owns explicitly assigned `llcsignalprotocol` and related
protocol paths. Independent writers own assigned `tests/signal` and
`interpreter/tests/tests_signal_origin.rxas`. Registered complementary cells
remain regressions; their existence does not require running both for each edit.
Practical TRACE is assigned below. Controlled HALT is not asynchronous host HALT;
nonresumable original-event C failure, broader host/platform acceptance, helper
raw-location presentation and INTERPRET retain the explicit
[conformance qualifications](levelc-conformance.md). They do not split SIGNAL
into unfinished micro-deliveries.

### TRACE

The [complete practical contract](architecture.md#trace-complete-contract)
owns the completed CTRACE-AC01–07/STEP01–03 and LC-I-24. One frontend/IR writer owns
`levelc.rxl`, `llcparse`, `llcfrontend` and `llcir`; one selector writer owns
`llcrxas` and the private host-only `llctraceemit`. Independent fixtures and
oracles belong in `tests/trace`. The integrator owns CMake, affected existing
schema assertions, documentation, shared preparation and the local commit.
Review coherent source and first representative RXAS early, then batch material
fixes and freeze. Shared B remains the option/command/controller owner; no native
C frontend change, target export, VM/ISA extension or INTERPRET is included.
One selected normal configuration/VM qualifies this development feature;
complete-C/platform/delivery QA remains a later milestone.

### Conditions and observations

The [whole Group 1 contract](architecture.md#conditions-observations-complete-contract)
owns CCOND-AC01–08 and STEP01–03. Sol 6.1 Extra High workers have disjoint
frontend/schema/protocol, shared-runtime/selector, and independent fixture
assignments; the integrator owns cross-boundary review, CMake, enduring docs,
shared preparation and the local commit. Native C is read-only. Start from its
maintained fixture sources and checked outputs, compiling those sources unchanged
through B/L; adapt only compiler/harness and controlled injection boundaries.
Add fixtures for genuine C22/observation gaps, not a parallel copy of native goldens.

One finite private `llccallemit` adapter consumes the existing admitted CSIG3
inventory. Shared B retains policy, pending delivery, suppression, activation,
CONDITION and pool ownership. PARSE observations extend its existing executor;
the TRACE output formatter stays shared. Preserve CPROC → SIGNAL → DO → CLAUSE
admission and conditional EXIT ownership. Review coherent source and first RXAS
early, batch material fixes, then qualify one normal Debug opt/IR-on/rxbvm lane.
No automatic mode matrix, unchanged-native sanitizer campaign, push or INTERPRET.
Formal whole-compiler/platform/delivery QA follows feature completion.

<a id="procedure-assignment-packet"></a>

### PROCEDURE and prepared-language reuse

The [PROCEDURE contract](architecture.md#proposed-procedure-contract) owns
CPROC-AC01–07; local AC01–06 are delivered and AC07/full LC-I-12 remain open.
The [current family](architecture.md#current-c-family-contract) owns version and
inventory, superseding older packet counts. The integrator owns grammar/
frontend/schema/private admission/selector and needed DO integration. Independent
fixtures use `tests/procedure` plus assigned existing consumer selectors.
Shared B algorithms, native C, generic L and VM changes require their own
explicit interface/path assignment.

[Prepared-language reuse](architecture.md#proposed-prepared-language-reuse)
CPREP-AC01–05 and STEP01–03 are delivered. Assigned generic L ownership covers
`lllanguage`, `llmunch`, `lllr`, `llcfg` and borrowed-reader controls. The C
integrator owns private initialization, parser/header/name and CLI/service/build
consumers. Preserve owning APIs, request-local limits/comments/source and the
complete ownership/copy chain; no C/Unicode policy in generic L or table-copy
fallback is authorized.

<a id="arg-assignment-packet"></a>

### ARG

The [ARG contract](architecture.md#proposed-arg-contract) retains
`CArg(templates:ParseTemplates)`, the shared UPPER packet/executor and complete
raw argument presence. Local implementation is delivered; ARG-AC05/full LC-I-11
and all CFC obligations retain their shared boundaries. The integrator owns
C grammar/parser/frontend/IR/private lowering/selector/target, current consumers
and shared CMake/docs; independent writers own assigned ARG fixtures/selectors.
Preserve source spans/contextual ELSE, empty ordinals, live/private pool order,
exact errors and complete caller/helper RXAS. Do not add a splitter, BIF surface,
ARG-specific runtime or Unicode campaign for fixture convenience. Native C's
known eager-operand limitation does not change independent expected state.

<a id="parse-integration-assignment-packet"></a>

### PARSE

The [PARSE contract](architecture.md#proposed-c-parse-integration) owns
CPARSE-AC01–08 and STEP01–05. The finite integration is delivered; complete
observations and declared delivery/platform obligations remain open. ARG/PULL
acceptance and last-stage INTERPRET are separate. Preserve the v3 cursor-only
kernel descriptor, LCP1 envelope and current family; old packet inventory counts
are not new-schema authority.

- Generic cursor ownership is explicitly assigned `interpreter/rxvmparse.c`,
  private header/guards and `tests/rxpa/parse_cursor*`. Preserve v1/v2/direct
  operations and the seven-callable built-in service; source manifests alone
  do not prove real-platform behavior.
- Shared B ownership covers `RexxClassicParse.crexx`, direct-B fixtures and named
  acquisition adapters: envelope/map admission, existing CharacterScan policy,
  live reads/writes and finite cursor cleanup/original typed relay. Do not
  invent a second matcher, cursor or generic signal ABI.
- C ownership covers named grammar/parser/frontend/IR/selector paths and private
  `llcparseprogram.crexx` construction/admission. Independent `tests/parse`
  fixtures cover forms/errors/order and failed-output preservation. Runtime is
  not linked into the compiler host; the codec is not a runtime dependency.
- One assigned integrator owns shared manifests/plans and preparation. Review
  the ownership/lifetime boundary early; evidence from native or cursor-only
  consumers never substitutes for missing new-C acceptance.

## Assignment template

Each worker prompt states:

1. Objective, complete intended outcome, CLC/CFC/LC-I/BF/LC-REF scope and owning
   contract/plan; keep unmet initiative criteria explicit.
2. Baseline HEAD/unrelated edits and exclusive write paths, exclusions and shared
   integrator. No worker reset/sync or unassigned execution.
3. Agreed API/ABI/effects, pending decisions and producer/consumer interfaces.
4. Independent expected behavior/errors/state/cleanup, the lifecycle stage and
   smallest check scope/cost under AGENTS. Broader milestone QA remains distinct;
   declare shared-build access. No default mode/VM matrix merely from a packet.
5. Relevant RXAS/helper shape and preserved observations; distinguish compiler
   from assembler modes when used.
6. Stop conditions: unapproved design/ABI/scope, unsafe dependency, conflicting
   edits, material regression or actual sanitizer finding. Do not weaken tests
   or invent fallbacks.
7. Handoff: paths/mechanism, current criterion status, checks/input identity and
   limits in conversation; update only enduring docs and delete temporary output.

For DROP, for example, specify direct/indirect forms, compound lookup order,
alias lifetime, invalid-word policy and expected pool states with `C-POOL`
effects. Do not add PROCEDURE, a pool ABI or VALUE just to make a fixture pass.

## Integration, local commits and publication

The coordinator checks path ownership/preservation and material findings, then
validates the scoped worker results. A reviewer and author agreeing is not a
substitute for coordinator inspection. Local commit authority is distinct from
publication and full-C/milestone acceptance; follow AGENTS for selecting checks,
performance verdicts, actual SAN closure and CI. Do not turn every feature commit
into a publication event or mandatory remote/CI-monitoring cycle.

Before handoff, update the existing plan's todo/done and named open criteria,
not another report. Preserve maintained fixtures, enduring contracts and known
limits; report run details in conversation and remove disposable output.
