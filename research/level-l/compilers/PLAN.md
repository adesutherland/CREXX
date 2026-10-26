# Classic C research closure

## Vision and current scope

The bounded C research acceptance is complete for the stated profile, selected
panel and documented limitations. The Classic C implementation uses
Level B and Level L parsing, IR, transformation and RXAS emission. Native B
is its build seed; C requests never delegate to a native source compiler.
Completion here means the agreed research profile has meaningful instruction,
BIF, state/error and emitted-code checks, with known defects and limitations
recorded honestly. It is not universal Rexx conformance or production readiness.

The selected qualification is the existing Classic instruction/BIF panel plus
four compact dynamic interaction cells and one current-default authored-TRACE
control. Reuse the unchanged baseline of 79 Classic C cells and 21 existing
support cells. The `^llcompiler_c_` selector identifies the C panel before the
supplemental cells; shared/support cells are not full G qualification. No whole
79/100/161-cell replay is required for documentation or selector-only edits.

Only a local commit is authorized. G/B language work, self-hosting, default
replacement, release, publication, installed/platform qualification, sanitizer
campaigns and production performance verdicts are outside this closure. Existing
repository SAN findings and their worklist/closure obligations are unchanged;
this scope neither repairs nor certifies them.

Three documents own the enduring C account:

- [User guide](doc/levelc-user-guide.md): syntax, commands, APIs and examples.
- [Architecture](doc/levelc-architecture.md): components, algorithms, ownership,
  runtime boundaries and wire contracts.
- [Conformance and known limitations](doc/levelc-conformance.md): admitted
  instruction/BIF profile, independent expectations, deviations and known defects.

## C-first delivery milestone

The useful CFC identifiers below now describe this bounded research acceptance.
Broader original delivery/replacement outcomes remain unqualified under
[future work](#outside-current-c-closure); they are not hidden prerequisites.

| Stable criterion | Observable research acceptance |
|---|---|
| **CFC-AC01** | Conformance accounts for LC-I-01–25 and all 95 BIF names, linking their existing independent cases and exact bindings. Unsupported, unverified and defective behavior has an explicit disposition; native passes are not new-C proof. |
| **CFC-AC02** | The selected Classic panel covers admitted expressions, numeric/string/Unicode values, symbols/pools/EXPOSE, PARSE/ARG, control and calls, including state, order, errors and limits. Known limitations remain visible rather than weakening existing oracles. |
| **CFC-AC03** | Selected activation, SIGNAL/CALL conditions, RETURN/EXIT, clause/source/TRACE and diagnostic checks preserve normal, abrupt and nested behavior. The compact interactions distinguish compiler build, semantic IR, assembler and target-VM axes. |
| **CFC-AC04** | Existing configured host/provider/ADDRESS/queue/stream controls retain their independent values, failures and ownership checks. This establishes their exercised research environment, not declared-platform coverage. |
| **CFC-AC05** | INTERPRET and Rexx source calls use the same C frontend/IR execution, live state and checked resumption/transfer ownership. Existing lifecycle/limit cases and the selected nested/source-call interactions retain their expected outcomes. |
| **CFC-AC06** | The six representative RXAS areas have a meaningful disposition. Separate instructions, literals/metadata, caller/helper and hot/cold work; preserve ABI, numeric, source/error and TRACE observations. Structural counts are not speed claims. |
| **CFC-AC09** | Shared B owners retain values, pools, activation, BIF and host behavior; C does not introduce a second runtime algorithm or depend on G aggregation. |
| **CFC-AC11** | Existing request/result, frozen graph and runtime owners retain bounded failure, stale/foreign refusal, cleanup and prior-output preservation. No native-memory/SAN qualification is inferred. |

## Remaining actions

**BQA-AC01–04** retain the useful rules for finite case coverage, meaningful
oracles, independent axes and proportionate test cost. The former comprehensive,
Release, installation, platform, SAN and measurement commissions are not part of
this revised C research closure. Report selected outcomes in conversation;
retain no timing tables, hashes, run receipts or completed chronology here.

| Stable step | Current status and dependency |
|---|---|
| **BQA-STEP01 — done** | The three C documents, known-limit inventory and links are consolidated. Unique wider obligations survive removal of obsolete C-only planning ledgers; CFC-AC01/02/04. |
| **BQA-STEP02 — done** | Reuse the unchanged Classic/support functional baseline and its repaired failure cells. Existing instruction/BIF/source/error/cleanup oracles remain intact; no G qualification claim. |
| **BQA-STEP03 — done** | The four compact dynamic contrasts and current-default authored-TRACE control retain their independent expected outcomes; CFC-AC03/05. Preserve their selected scope and unchanged default workload. |
| **CEP-STEP03 — done** | Six representative areas: typed/process bindings, authored PARSE, nested dynamic/condition resumption, source-loaded expression/CALL arguments, noopt/IR-off RETURN cleanup/calls and existing G cleanup/control. The last is retained prior consumer evidence, not new G scope. |
| **CEP-STEP04 — done** | Selected TRACE early-helper rejection and provenance-wrapper dispatch preserve their exact contracts and passed their existing structural/focused checks. |
| **BQA-STEP04 — done** | The selected checks, documentation and scoped local closure are complete. Publication and additional QA campaigns require their own authority. |

### Compact dynamic interactions

Extend only the existing dynamic checker and CMake registration. Preserve its
unchanged 43-case default, package/no-native-fallback guards, inputs, masks,
cleanup and independent expected outcomes. Existing `--only` selects named cases;
no new fixture, generic framework or runtime API is needed.

- `--mode opt|noopt` chooses the built C compiler, metadata and target adapter.
  The compiler host remains `rxbvm`.
- `--ir on|off` controls the existing worker's tenth request field; explicit `0`
  disables semantic IR optimization. The default remains on.
- `--vm rxbvm|rxtvm` chooses the application VM only.
- `--assembler opt|noopt` chooses ordinary RXAS or `rxas -n`; the default stays opt.
  The noopt contrast uses the existing emit-only worker, then assembles its exact
  published RXAS in the driver; default worker assembly and markers stay unchanged.
- Shared `llcdynamic` and its package-shape control remain the opt-built product,
  including when the selected C compiler is noopt. Bind identities to the actual
  selected images and options, never to a label alone.

Three one-axis contrasts select existing `live-once`,
`child-signal-resumes-group` and `source-call-expression-arguments`: noopt compiler
with IR on; opt compiler with IR off; and opt/IR-on with target `rxtvm`.
The fourth uses opt/IR-on/rxbvm with assembler noopt, selecting `live-once` and
`authored-trace`. Run one existing current-default `authored-trace` selection as
its control. A compact label may identify the four new C-only cells; preserve
existing test names, default workloads, locks, timeouts and preparation owners.
QA alone owns builds/execution; reuse prepared stages and unchanged valid results.

## Emission simplifications and conformance choices

| Stable opportunity | Current disposition and proof boundary |
|---|---|
| **CEP-P01 / CDDS — done for selected scope** | Bounded idle predicate and same-frame shared cold dispatch/hybrid routing. Preserve pending CALL/HALT/EXIT, captured decisions, source origins and cleanup. |
| **CEP-P02 / CCPK — done for selected scope** | Mutate only fresh unpublished single-owner packet chains, including proven SSA/slot transport. Shared prefixes retain value semantics; actual copies, omissions and abrupt cleanup remain required. |
| **CEP-P03 — first slice done** | Module-owned immutable catalogues retain exact identities; policies, pools, capabilities and invocation state remain mutable/local. Encoding and cache changes are future work. |
| **CEP-P04 — first repair done** | Canonical language-profile admission survives dead-op pruning. Runtime capability remains tied to admitted operations/entries; missing optional proof retains general behavior. |
| **CEP-P07 — future** | Selective guarded specialization/general compiled fallback requires exact invalidation, liveness, source/observations and resource proof. One-way IR state adoption is unselected; no JIT, epoch service or new VM/API is authorized. |
| **CEP-P05 / CEP-P06 — unselected** | Reduced observations or a static-only language would lose contracts. Neither is a default optimization; a concrete change requires Adrian's decision. |

INTERPRET invalidates affected mutable facts, not all immutable facts or local
freshness proofs. Further simplification must preserve errors, source/TRACE and
ownership. No broad optimization or performance campaign is commissioned.

## COMMAND interpreter and compiler run mode

This authorized development increment adds a standalone `llrexx.rxbin`, its
normal packed `llrexx` launcher, and `llrxc-c --run`. It does not expand language,
platform, VM/ISA or native-provider contracts or replace another interpreter.
Program input is a file initially; source-from-stdin and interactive input are
future work. Ordinary program PULL/LINEIN keep stdin unchanged.

| Stable criterion | Exact entry contract |
|---|---|
| **RCMD-AC01** | `llrexx [--] file [argument ...]` and `llrxc-c --run [--] file [argument ...]` use one shared source CLI implementation. `--run` is first-switch dispatch; existing compilation is unchanged. Run mode rejects compile-only metadata/output/provider/export/optimization switches. After the filename every token is program data. |
| **RCMD-AC02** | Match native Classic command argv: each trailing OS argument becomes one present ARG position, preserving empty/Unicode text; no arguments means zero positions. Do not reconstruct a shell tail. The owned host request retains its existing length-bearing/NUL API, while OS argv cannot carry embedded NUL. Read exact source bytes without adding an OPTIONS header. |
| **RCMD-AC03** | A source-only `llcdynamic.runprogram(source, COMMAND request, resourceBase)` facade returns an owned `RexxClassicHostResult`. Anchor request activation/pool/config/context, HostScope and root dynamic scope; use full-program `llcgraph`, setIR, inventory, registry/manifest, start and advance. Never masquerade as SUBROUTINE or interpret-group. |
| **RCMD-AC04** | Pump admitted SIGNAL landings and full IR internal calls until normal/RETURN/EXIT/failure. Preserve numeric/TRACE/source and existing cleanup, close all owners before returning, and retain primary plus cleanup conditions. Unsupported static/invoke boundaries fail explicitly; never replace them with an empty success. |
| **RCMD-AC05** | Usage errors return 2; direct source-file input errors return 1. Normal completion uses HostResult.commandStatus(), including existing absent/nonnumeric/whole-integer rules. After cleanup, typed frontend/runtime failures use existing signalrelay and VM diagnostics/status rather than becoming status zero or a new textual classifier. |
| **RCMD-AC06** | Default COMMAND configuration remains UTF8, NUMERIC 9/0/SCIENTIFIC, TRACE N and ADDRESS CREXX with current streams/selected queue through public request settings. Prepared provider bindings are empty in this first CLI; BIFs, existing native facilities and configured source resolution retain their owners. Runtime source search gets the explicit program directory and existing policy, without PATH discovery. |
| **RCMD-AC07** | The interpreter RXBIN contains only bootstrap, frontend/IR executor and ordinary runtime closure. It contains no RXAS selector, assembler, linker or native source compiler. The compiler command retains its compilation package, but --run calls the same facade and produces no RXAS/RXBIN or temporary compiled program. |

| Stable step | Current status and dependency |
|---|---|
| **RCMD-STEP01 — done** | Coordinator and independent source review of this whole entry/CLI/package contract before product edits. |
| **RCMD-STEP02 — done** | The source facade, shared CLI, standalone image/launcher and compiler dispatch use one runtime algorithm. Exact CMake/product dependencies and the owned cleanup projection are source-reviewed. |
| **RCMD-STEP03 — done** | The maintained COMMAND entry and owned-result controls cover SOURCE/ARG, empty/Unicode arguments, plain source/labels/recursion, INTERPRET/source-child resumption, status, ADDRESS CREXX, typed/input failures and cleanup/recovery. Both entry spellings and the runtime-only package retain their checked contracts. |
| **RCMD-STEP04 — done** | Representative entry RXAS, the agreed complete C panel and compact interactions, and enduring documentation are ready for the scoped local commit. Commit only authorized paths locally; no push or unrelated broad campaign. |

Keep the existing compiler-only `llrxc-c-linked.rxbin` and `imports-llrxc-c`
lean for internal compilation/worker consumers. `llccompilecli.run(argv)` owns
one unchanged compilation parser/body; the lean main delegates only to it.
A distinct `llccommand` main dispatches `--run` to `llcruncli` or compilation
to that same service. Only its separate rich import root/image feeds the public
packed `llrxc-c`. No ordinary compiler host acquires the COMMAND runtime or new
native-provider requirements. The standalone interpreter has its own
frontend/IR/ordinary-runtime closure.

Keep cleanup projection private: `scope.closeoutcome(?primary)` returns an owned
absent object or first cleanup event, including an active child's outcome.
Existing `close(?primary)` retains its no-primary relay/primary suppression
contract. The COMMAND facade records a returned cleanup event without erasing
an existing primary condition; no event list, native ownership service or wire
contract is added.

Batch reviewed slow-runner repairs before that freeze. The explicitly audited
44 private Python C cells, 24 existing weighted owner cells, four compact
contrasts and the COMMAND entry use one ordinary Unix admission of `floor(CREXX_QA_CTEST_JOBS / 5) + 1` processors per heavy cell,
bounding concurrency to four at the agreed 30-job setting. Their mutable
artifacts remain private and prepared inputs read-only. Keep the four direct
prepared-language VM cells serial. Preserve Windows and actual instrumented/
native-audit serial/lock policy, other G registrations, and every workload,
oracle, name, timeout and preparation edge; do not match future tests by regex.

The ARG shape check combines main, IF and post-EXIT contexts into one source/
blank pair and retains the original isolated CALL/RETURN pair. This reduces
eight C compilations to four per lane while keeping exact executable-shape
equality, including private labels and frame sizes, and the absence of parse-
source/execute helpers. Equal-width blanks replace `arg ,,,` with seven spaces
and `arg` with three spaces. Explicit clause delimiters prevent a comma template
from continuing into another context; source and control retain the same
positions. Keep all simple, refusal, fatal and runtime assertions and
registrations unchanged.

## Selected interpreter cost follow-up

The selected startup repair (**RCOST-01**) and removal of unused emitted location
state (**RCOST-02**) are complete for their focused research checks. The completed
C research closure and its documented limits remain unchanged; no G, VM/ISA or
broad QA work is added.

- **RCOST-AC01:** Reduce actual standalone `llrexx`/compiler `--run` preparation
  before the first program result. DFA/LR automata are already generated at
  build time; runtime materializes the definition and validates/indexes loaded
  data. Lazy preparation for first parse, INTERPRET or a source call can avoid
  unused initialization, but `llrexx` needs parsing immediately: merely moving
  the same cost there is insufficient; reduce the remaining first-parse work.
- **RCOST-AC02:** Preserve the one grammar/parser owner, exact source mapping,
  state, diagnostics, resource limits, initialization failures and ownership/
  cleanup. Reuse existing prepared products; the concrete startup mechanism
  requires source review and coordinator clearance before edits.
  RCOST-01/02 use existing formats and native VM/ISA mechanisms; RCOST-05 below
  owns the separate generic prepared representation.
- **RCOST-AC03:** Remove location-string loads only after all-use proof shows
  their values are overwritten before observation. Preserve source/control-entry
  metadata, TRACE, caught CONDITION/SIGL, IR source identities and the original
  typed payload/event through cleanup and outward relay; no replacement VM API.

| Stable step | Current status and dependency |
|---|---|
| **RCOST-STEP01 — done** | Identify remaining first-parse preparation and compare lazy, existing build-time prepared and narrower forms; peer the bounded owner/failure contract; AC01–02. |
| **RCOST-STEP02 — done** | The selected startup repair and focused real first-result, syntax, lifecycle and source-call checks are complete; AC01–02. |
| **RCOST-STEP03 — done** | Proved-dead location state is removed; representative RXAS and existing fatal-relay/source/payload checks qualify the selected scope; AC03. |

RCOST-01 reuses the single `llcheck` structural validator for root, grammar
limits, clause kinds, child bounds, alphabet and semantic-profile flags. Its
existing `run()` still computes the complete nullable/FIRST/FOLLOW, reachability
and admission facts. CFG production layout needs only structural admission;
nullable/FIRST demand the complete facts once, guarded by a retained readiness
flag rather than an extra analysis-method call on each query. Packed LR/DFA
identity, record validation and indexes remain unchanged.

The private C owner initializes only its lifecycle state eagerly. First use
constructs and checks a local definition, publishes it once into anchored module
storage, and retains ready/failed/reentry states. Invalid definitions publish no
borrow; an original typed preparation failure remains owned and is relayed on
later requests. Mutable requests and task-worker overlays keep their existing
ownership. Retain the existing scanner/parser readiness checks.

Eager complete analysis repeats facts not needed by a loaded table. Laziness
alone merely moves that cost to `llrexx`'s immediate first parse. The narrower
checked layout removes unnecessary first-parse analysis while preserving the
existing build-time LR construction's demand for full facts. RCOST-05 separately
owns the generic prepared representation. Existing shared structural/full-fact
controls, packed-image invalids and C cold/reuse/failure
controls guard the contract; QA also checks the actual first result and syntax,
source-call and interpreter lifecycle behavior without a new broad panel.

The existing absent-grammar control traps a second construction attempt. One
additional opt/rxbvm-only prepared-owner control forces actual reentry while
preparing, retains that error's provenance in a typed NUL-bearing payload, and
checks the cached event again after constructor locals have unwound. Compare
exact tag, physical binary component/bytes and original module/address; a counter
trap detects reconstruction. Separate fresh module instances cover absent and
typed failures; no reset/debug API or failure matrix is introduced.

RCOST-02 retains a private cold fatal-payload carrier in its existing role.
Only obsolete eager location loads, diagnostic cache/formatting and the
saved-location slot are removed. Source anchors, `.srcstep` flags/query identities,
faultline, captureline and public `failuremessage` ABI remain unchanged. Existing PROCEDURE
and SIGNAL structural checks retain original typed relay and full authored
span/clause/flag checks without requiring discarded diagnostic strings.

**RCOST-03 — future only:** Entry, numeric setup and transport simplification
requires context/liveness/alias proof, exact numeric inheritance, argument
presence/value ownership and preserved source/TRACE/error observations.

**RCOST-04 — future only:** Signal-guard simplification requires transitive
failure/effect proof, nested policy restoration, handler/resumption and cleanup
equivalence. Neither opportunity authorizes implementation or a new panel.

**RCOST-05 — implemented and locally qualified for the selected generic/C profile:** The
[generic Level L plan](../PLAN.md#current-action-prepared-definitions) owns the
qualified LDP1 checked transport contract. Its scanner/nonregional LR1 profile reuses
packed DFA/LR data and complete portable grammar/CFG/policy/schema metadata, preserving
existing language()/builder/reader mutation, ownership, error and resource
contracts. The shared serializer/hydrator excludes VM pointers and request state.
RCOST-01/02 remain complete; RCOST-03/04 remain future only. No G feature or wider
qualification is selected. The selected Debug checks establish no general
startup-speed or peak-memory improvement, formal performance verdict, Release,
platform or SAN qualification.

The approved [LFP1 follow-on](../PLAN.md#current-action-frozen-definitions) makes
generated definitions trusted, frozen packed data behind the existing read API.
Validation and indexing happen at build time; `lllanguage..compiled()` performs
only a short header check and binds owned packed sections, with no hydration.
The checked `prepared()` loader remains for external LDP1 images, and mutable
builders remain the construction path. Per-input AST result storage is unchanged.
The generic plan owns the selected focused checks and local delivery; this adds
no G feature or wider qualification.

## Outside current C closure

**G21 unread scalar writes** remains a generic/G forwarding opportunity only:
prove liveness, every implicit use and source/TRACE observations before removing
transports. No G implementation or test campaign is selected.

Original **CFC-AC07/08/10/12** wider generic-service, delivery, public RXAS/VM,
installed and declared-platform outcomes remain future/unqualified. Production
budgets, default switching, retirement/rollback, full reference conformance and
release approval are not established by this local closure.

Unverified wider **LLCOMP-AC01–16** outcomes remain: **LLCOMP-STEP07B** proper B and B1/B2/B3
self-hosting from the retained seed, with deterministic closure/mode/semantic/dependency
comparisons; complete typed B/G frontends, metadata
conformance and replacement coverage; common graph/codec/transform/emission
quality and bounded reusable services; full C/G/platform/installed/constrained
qualification; representative production replacement latency/storage/code-size
budgets; current schemas/algorithms/walkthroughs; and an explicitly approved
default switch with seed-retention/retirement and rollback proof. C portions
are covered by the current CFC criteria. New B/G/self-hosting/combined replacement
is not scheduled by this C plan and has not been completed or cancelled.

## Compatibility anchors

These retained anchors lead existing plan links to the current governing scope;
exact language/runtime contracts live in the three owning C documents.

<a id="say-validation"></a><a id="current-action-drop"></a><a id="current-drop-proof"></a><a id="current-action-assignment"></a>
<a id="current-assignment-proof"></a><a id="current-action-nop"></a><a id="current-nop-proof"></a><a id="current-action-options"></a>
<a id="current-options-proof"></a><a id="current-action-if"></a><a id="current-if-proof"></a><a id="current-action-select"></a>
<a id="current-select-proof"></a><a id="current-action-do"></a><a id="current-do-proof"></a><a id="current-action-leave"></a>
<a id="current-leave-proof"></a><a id="current-action-iterate"></a><a id="current-iterate-proof"></a><a id="current-action-arg"></a>
<a id="current-action-procedure"></a><a id="current-action-call"></a><a id="current-action-return"></a><a id="current-action-exit"></a>
<a id="current-action-parse"></a><a id="parse-cursor-local-qualification"></a><a id="dynamic-execution-plan"></a>
<a id="group3-static-language-plan"></a><a id="group2-calls-host-plan"></a><a id="group4-infrastructure-plan"></a><a id="shared-delivery-stages"></a>
<a id="l-construct-01"></a><a id="active-first-implementation-increment"></a><a id="shared-b-unicode-interface-and-c-adapter-migration"></a><a id="active-c-contract-baseline-increment"></a>
<a id="active-borrowed-classic-context-increment"></a><a id="step07a-validation"></a>
