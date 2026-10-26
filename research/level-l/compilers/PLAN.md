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
