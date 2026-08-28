# New-build Phase 2 plan — deterministic imports and metadata

## Status and evidence boundary

- **Status:** approved; Wave P2.1 complete; Wave P2.2 ready
- **Architecture approval:** P2-0 approved by Adrian on 2026-08-28
- **Planning branch:** `temp/newbuild`
- **Resynchronised develop:** `origin/develop` through
  `b64f67a00b3585b130f909640a30d120095e64f6`
- **Resynchronisation merge:**
  `c3646b76f056fcc2af11d86de22f7ca5cf762717`
- **Phase 1 hosted acceptance:**
  `0a0c17d56f341692d4d735ee87238c06ef6757c0`, GitHub Actions run
  `33156840598`
- **Date:** 2026-08-28

Phase 1 is accepted on its exact pre-resynchronisation SHA: Linux x64,
Windows x64, macOS ARM64 and macOS Intel completed the hosted build workflow.
The resynchronisation then incorporated the current RexxScript string-BIF work
and POSIX child-terminal ownership work from `develop`. Those inputs have not
been requalified on the merge SHA by this planning change, so this document
does not claim that the resynchronised branch is already hosted-green.

No performance measurement belongs to Phase 2 correctness work. Durations
from a shared host or hosted runner are diagnostic build-cycle observations
only. Performance tests remain excluded from parallel correctness and stress
work and run later on an intentionally quiet worker.

### P2.1 hosted resynchronisation evidence

GitHub Actions run
[`33176811762`](https://github.com/adesutherland/CREXX/actions/runs/33176811762)
qualified exact commit
`c8ce9ed4f42c976a5501ac7ac4e2bc3e48b0dcf1`. Every platform completed its
clean MinSizeRel build, explicit `qa-prep`, correctness CTest and applicable
workflow closeout successfully:

| Platform | Correctness tests | CTest duration | Result |
| --- | ---: | ---: | --- |
| Linux x64 | 2,339 | 787.54 s | passed |
| Windows x64 | 2,267 | 662.18 s | passed |
| macOS ARM64 | 2,339 | 577.25 s | passed |
| macOS Intel | 2,339 | 1,592.65 s | passed |

The exact logs show
`--label-exclude "^(stress|performance-measurement)$"`, so neither stress nor
measurement ran in the correctness worker. The durations are diagnostic only:
the hosted runners are shared and this was not a controlled performance
experiment. The workflow-dispatch release and dev-snapshot jobs were correctly
skipped.

The incorporated `develop` changes add two platform-applicable CTest entries:
the POSIX launch diagnostic and pseudo-terminal `linein` regression. Both are
comprehensive correctness tests, not measurement tests. RexxScript string-BIF
coverage extends its existing functional suites.

## 1. Outcome

Phase 2 makes dependency selection and metadata flow explicit enough that a
human and a build engine can answer the same questions:

1. What action is ready, and what is it waiting for?
2. Which ordered roots and artifact kinds may satisfy each import?
3. Which source, RXAS or RXBIN provider did `rxc` actually select, and why?
4. Which metadata did each tool generate, preserve, transform or strip?
5. Which exact action owns and atomically publishes each artifact?
6. What must rebuild after one input changes, and what must remain untouched?

The phase promotes the current observed catalogue into a validated manifest
contract and proves one bounded post-bootstrap family through that contract.
CMake/Ninja remains the executor. The self-hosted Level B scheduler, task API
and normal post-bootstrap takeover remain Phase 3 work.

## 2. Scope and non-goals

Phase 2 includes:

- versioning and hardening the action/artifact/test manifest;
- observe-only instrumentation at the real `rxc` resolution decision point;
- artifact and metadata reports from the compiler/assembler/linker pipeline;
- strict expected-provider checking after current behaviour is captured;
- source/RXAS/RXBIN/plugin/linked route-parity qualification where applicable;
- curated immutable import roots driven from the manifest for one proving
  family;
- incremental, reproducibility and race-oriented graph qualification; and
- exact-SHA hosted qualification on all four maintained build platforms.

Phase 2 does not include:

- a new cREXX language feature or public Level B task API;
- the Level B scheduler or removal of the CMake reference path;
- a general build-system rewrite;
- a broad ASan redesign or a sanitizer-clean claim;
- performance optimisation or performance benchmark execution;
- making every generated binary byte-identical across platforms; or
- silently changing current import precedence before its behaviour and users
  have been captured in permanent tests.

## 3. Approval gate P2-0

The following choices affect the build architecture and must be approved
before implementation begins.

### 3.1 Manifest contract

Use canonical JSON and evolve the existing `crexx.build-manifest/v1` schema.
JSON is already emitted and validated by Phase 0 tooling, can be consumed by
CMake adapters now and by Level B later, and permits deterministic machine
comparison. Human comprehension comes from stable `plan` and `explain`
renderings; the JSON is not expected to be hand-authored unaided.

The schema version identifies semantics, not merely file shape. An additive
field may remain in v1 only when old consumers can safely ignore it. A change
to action identity, provider ordering, metadata meaning or scheduling
semantics requires a new schema version.

### 3.2 Reproducibility contract

Require byte identity for canonical reports, manifests, depfiles and other
textual evidence. Require semantic identity for platform-native executables
and for generated artifacts known to carry platform/debug/signature entropy.
RXAS and RXBIN should become byte-identical where their supported format makes
that possible, but route parity must be defined by semantic behaviour rather
than assuming that different compilation routes have identical debug labels
or layout.

### 3.3 Execution boundary

Keep CMake/Ninja as the only Phase 2 executor. The first executable manifest
slice is translated into ordinary CMake custom commands in an isolated output
tree. It must not create a second owner for any current artifact. Phase 3 will
decide and prove the Level B execution boundary.

### 3.4 Dynamic file-access audit boundary

Build the portable static conflict model in Phase 2. Retain a bounded dynamic
audit proof on supported local platforms, with backend selection treated as a
separate implementation decision: for example `fs_usage`/Endpoint Security on
macOS, `strace`/fanotify/eBPF on Linux, and ETW/Process Monitor evidence on
Windows. No one backend should become part of the manifest semantics.

## 4. Implementation waves

Each wave has a narrow changed family, focused preparation and focused tests.
Unchanged families are not rebuilt merely to mark progress. A broad build or
test gate occurs only at the stated checkpoints or when a compiler, assembler,
linker or shared build input invalidates the retained evidence.

### Wave P2.1 — refresh and freeze the accepted baseline

**Progress (2026-08-28): complete.** The exact resynchronised SHA passed all
four maintained hosted platforms in run `33176811762`. Correctness QA excluded
stress and performance measurement. No local build, CTest, sanitizer,
Minikube or timing workload was run for this gate.

1. Catalogue the resynchronised graph and classify the new RexxScript and
   terminal tests without executing performance measurements.
2. Compare the current graph and test inventory with retained Phase 1
   evidence; explain every new action, artifact, label or dependency.
3. Run only focused preparation/tests needed to validate the incoming areas.
4. Use the hosted build workflow as the cross-platform resynchronisation gate
   before Phase 2 compiler/tool instrumentation is layered on top.

**Gate:** the resynchronised exact SHA is green on Linux x64, Windows x64,
macOS ARM64 and macOS Intel, or any platform exception is explicitly diagnosed
and approved. This is correctness qualification, not a timing baseline.

### Wave P2.2 — harden manifest v1 and its validator

**Progress (2026-08-28): first contract slice complete.** The v1 schema and
standard-library validator now retain each observed `rxc` invocation separately
with ordered roots, permitted kinds, RXAS scanning and executable-directory
visibility. Declared import actions must name their resolution report and
content-digested expected providers. The canonical
`crexx.import-resolution-report/v1` schema separates candidate discovery
events from final provider bindings and can record missing/read-failed
candidates without losing the report. Existing observed v1 projections remain
valid when they lack the new optional invocation detail; the current exporter
always emits it. Seventeen lightweight catalogue tests pass. Cycle, full path
conflict, `plan` and `explain` work remains in P2.2.

Extend the manifest so that an action can declare:

- normalized source, work, staging and publication paths;
- inputs, outputs, byproducts, deletion scope and atomic-publication owner;
- product layer, execution wave, direct dependencies and resource class;
- ordered source and binary import roots;
- permitted import kinds and executable-directory visibility;
- source/RXAS preference and `--import-rxas`/`--no-exe-import` state;
- expected provider identity and the path of its resolution report;
- required, generated, preserved and stripped metadata classes;
- platform predicates, QA tier, labels, locks and measurement exclusion; and
- tool identity, environment inputs and content digests used in action
  identity.

Extend validation to reject:

- cycles, missing producers and wave inversions;
- duplicate, ancestor-overlapping and platform case-folded output owners;
- unordered write/write, write/read and delete/read path conflicts;
- undeclared public-root or executable-directory import visibility;
- ambiguous expected providers and impossible allowed-kind combinations;
- test actions that build or mutate the active product tree; and
- measurement actions scheduled with correctness or stress work.

Add deterministic `plan` and `explain` views. The manifest remains marked
`observed` or `draft` until an action family is actually generated from it.

**Gate:** canonical round-trip output is byte-identical, all Phase 1 graph
rules remain represented, and deliberately invalid fixtures cover every new
validator rule.

### Wave P2.3 — record `rxc` import resolution at the decision point

**Progress (2026-08-28): first observe-only resolver slice complete.** `rxc`
now accepts `--import-resolution-report <path>` and atomically emits canonical
v1 JSON from the existing resolver. Candidate events distinguish admission,
ordered-root rejection, same-root replacement, older-mtime rejection and the
RXBIN-on-equal-mtime tie-break. The post-collapse candidate set is recorded
with SHA-256 digest status, while logical root identifiers keep the report
independent of the checkout path and preserve executable-directory provenance.
The focused fixture captures a newer RXAS replacing RXBIN in one root, an
earlier ordered root defeating a newer executable-root artifact, and an exact
mtime tie retaining RXBIN. A CLI fixture separately proves atomic publication,
`--no-exe-import`, schema identity and the observe-only empty
`provider_bindings` boundary. Five focused import/report tests pass, including
the existing concurrent directory-snapshot test. No broad build, sanitizer,
performance measurement or timing comparison was run locally.

This slice does not yet record final namespace/symbol-to-provider bindings,
emit a compiler depfile, enforce expected providers, or replace the current
timestamp-sensitive policy. Those remain the next reviewed P2.3 work; the
wave gate below is therefore not yet complete.

**Implementation note (2026-08-28):** read-only source inspection confirms
that discovery currently scans the primary/source roots before binary roots,
keeps the first same-stage module found across ordered roots, and, when
`--import-rxas` exposes both RXAS and RXBIN for one module in the same binary
root, chooses the newer mtime before using artifact kind as the tie-break. The
observe-only fixture must capture that timestamp-sensitive behaviour before a
separate reviewed change replaces it with the approved deterministic policy.
Candidate admission is not equivalent to final namespace/symbol binding, so
both event classes remain distinct in the report.

Instrument the existing resolver before changing its precedence. The report
must be emitted by the code that chooses the provider, not reconstructed from
CMake arguments after the fact. For each requested namespace or symbol it
records:

- requested identity and importing source;
- ordered source and binary roots exactly as searched;
- candidate path, artifact kind and root index;
- chosen path, kind, content digest and selection reason;
- rejected candidates and the reason each lost;
- executable-directory visibility;
- automatic RXAS/source generation state; and
- the import flags that materially affect resolution.

Paths in the canonical report are workspace-relative or use declared logical
root identifiers so that two checkout locations remain comparable. A
compiler depfile records actual selected file dependencies for Ninja.

First add observe-only focused tests for the known precedence cases,
especially simultaneous source and RXBIN candidates, RXAS/RXBIN ties,
`--import-rxas`, and `--no-exe-import`. Only after those pass should an
expected-provider input make a mismatch fail the action.

**Gate:** current selection behaviour is permanently captured; a stale public
or executable-directory artifact cannot silently replace a declared provider;
and the selected provider and digest match the manifest.

### Wave P2.4 — record artifact and metadata flow

Give each tool the report appropriate to its role:

- `rxc`: selected imports plus generated RXAS and compiler metadata inventory;
- `rxas`: input RXAS digest, output RXBIN digest and assembled metadata
  inventory;
- `rxlink`: selected modules, provider requirements, initializer retention,
  strip/preserve options, output digest and linked metadata inventory; and
- `rxdas`: disassembled metadata inventory used for round-trip comparison.

`rxas` should not be described as resolving REXX imports if it does not perform
that search. It reports transformation and metadata evidence. Likewise,
`rxlink` records its actual module/provider decisions rather than inheriting a
compiler report by assumption.

Reports are written privately and published atomically with their artifact.
A failed producer must leave the previous public artifact and report paired,
or publish neither.

**Gate:** each published artifact has a matching action identity, input/output
digest record and metadata policy; preserve/strip and disassembler round-trip
fixtures pass.

### Wave P2.5 — qualify alternate provider routes

Build a permanent matrix for each route that the product genuinely supports:

| Route | Required comparison |
| --- | --- |
| source | provider identity, compile contract, diagnostics and runtime result |
| RXAS | assembled contract, metadata inventory and runtime result |
| RXBIN | imported namespace/symbol contract, metadata and runtime result |
| native plugin | provider/ABI binding and runtime result |
| linked image | module requirements, initializers, metadata and runtime result |

Progress from a small Level B fixture to the route-sensitive library families:
native class-library adapters including `KeyDB`, `rxfnsc`, RexxScript, and the
main class library. Compare namespaces, overloads, classes/interfaces, inline
behaviour, initializers, diagnostics and runtime results. Do not equate route
parity with byte identity when a supported route legitimately changes debug
labels or layout.

**Gate:** every declared route either passes its stated semantic contract or
is removed from the manifest as unsupported; selection reports prove which
route each test exercised.

### Wave P2.6 — make one bounded manifest slice executable

Use the small native-backed class-library adapter family as the first proving
slice. It is bounded, already has explicit providers and atomic publication,
and exercises the source/native boundary and the known `KeyDB` route risk.

1. Generate an isolated CMake action table from the approved manifest.
2. Compare its actions, dependencies, commands, roots and reports with the
   retained hand-written table.
3. Build into an isolated comparison tree so it cannot compete for existing
   output ownership.
4. After parity is proved, switch that family to one manifest-generated owner
   and remove only its superseded duplicate dependency table.
5. Retain a readable `plan`/`explain` rendering beside the machine manifest.

**Gate:** the family has one output owner, the same supported runtime and
metadata behaviour through both proofs, and no undeclared fallback to the
ordinary `bin` or executable directory. No Level B scheduler is introduced.

### Wave P2.7 — incremental, reproducibility and race proof

For the executable slice and then the route-parity families:

1. Build clean at jobs 1, 5 and 30 and compare canonical reports/artifacts.
2. Change one source and prove only its declared reverse dependency closure
   rebuilds.
3. Change timestamps and ready-queue order without changing content and prove
   provider selection is unchanged.
4. Seed stale source/RXAS/RXBIN candidates in forbidden roots and prove they
   are ignored or cause a strict, explained failure.
5. Inject producer failure before publication and prove no partial or
   artifact/report-mismatched public state appears.
6. Run static conflict validation, `ninja -t missingdeps`, action-log
   comparison and bounded schedule fuzzing.
7. Compare observed dynamic file reads/writes/deletes with declared paths on
   the approved audit backend.
8. Prove CTest consumes already prepared artifacts and leaves the active Ninja
   log unchanged.

Heavy-load stress is a separate QA invocation. Performance measurements are
not run in this wave and receive no timing interpretation from it.

**Gate:** clean and incremental results do not depend on job count, scheduling
seed, timestamp or stale candidate files; observed file access agrees with the
manifest or produces an actionable exception.

### Wave P2.8 — final hosted qualification

Run the ordinary hosted build workflow at the exact final Phase 2 code SHA on
Linux x64, Windows x64, macOS ARM64 and macOS Intel. Windows specifically
qualifies path case-folding, rename/publication behaviour and the absence of
overlapping build/test processes in one tree. Hosted durations are retained as
diagnostic build-cycle data only.

Sanitizer work remains a separate gate. If Phase 2 exposes a first-party
sanitizer finding, record and handle it under `docs/SANITIZER-WORKLIST.md`; do
not suppress it or describe the phase as sanitizer-clean.

**Gate:** all maintained hosted build platforms pass at the exact final SHA,
with no performance tests mixed into correctness/stress workers and no claim
beyond the sanitizer scope actually run.

## 5. QA classes used by Phase 2

| Class | Purpose | Scheduling rule |
| --- | --- | --- |
| essential | graph/schema/tool sanity needed for any build | prepared first; short and deterministic |
| smoke | representative end-to-end product confidence | parallel after preparation |
| comprehensive | maintained broad correctness | parallel at checkpoint only |
| qualification | route, metadata, install and platform contracts | explicit family/platform selection |
| stress | concurrency, failure and schedule-fuzz behaviour | separate invocation from normal QA |
| measurement | governed performance evidence | quiet isolated worker only; never Phase 2 parallel QA |

Tests declare their prepared artifacts at configuration time. CTest never owns
a build fixture. Tests sharing an exclusive external facility use a named
resource lock; ordinary generated fixtures use unique paths and direct DAG
dependencies rather than global serialization.

## 6. Evidence retained per wave

Each completed wave records:

- exact source SHA and manifest schema version;
- selected action family and invalidated prior evidence;
- canonical `plan`/`explain` and resolution/metadata reports;
- focused preparation and test commands;
- jobs/profile used, without treating elapsed time as performance evidence;
- action/Ninja log before and after CTest;
- static graph, missing-dependency and path-conflict results;
- output/report digests and stated semantic comparisons;
- hosted run URL and platform conclusions where applicable; and
- any capability limit, open `SAN-nnn`, or decision deferred to Phase 3.

## 7. Completion criteria

Phase 2 is complete only when:

- the approved manifest contract and version policy are documented and
  schema-validated;
- one bounded family is generated from the manifest with exactly one owner;
- every import selected for that family is recorded at the real resolver and
  matches an expected provider;
- executable-directory contents, stale public images and timestamps cannot
  change that selection;
- compiler, assembler, linker and disassembler metadata responsibilities are
  recorded and qualified without assigning a tool behaviour it does not have;
- applicable source/RXAS/RXBIN/plugin/linked routes pass their declared
  semantic contract;
- clean and incremental proofs pass at jobs 1, 5 and 30;
- CTest performs no active-tree build and performance measurements remain
  isolated;
- static and approved dynamic race checks find no undeclared access or
  unordered conflict in the proving scope; and
- the exact final code SHA passes Linux x64, Windows x64, macOS ARM64 and
  macOS Intel hosted build gates.

## 8. Stop points

Stop and return for direction before:

1. changing current import precedence rather than reporting or enforcing an
   approved expected provider;
2. adding a public compiler/linker option whose contract was not approved;
3. introducing a Level B task/scheduler API or moving the execution boundary;
4. treating semantic route parity as byte identity, or relaxing an approved
   byte-identity requirement;
5. selecting a privileged or continuously installed file-audit backend; or
6. removing the reference CMake path for any family beyond the proved slice.

## 9. First implementation slice after approval

Start with Wave P2.1, then implement only the schema/validator additions needed
for an observe-only `rxc` resolution report and one precedence fixture. This
first slice must not change provider selection, introduce the Level B runner,
or rebuild unrelated libraries. Review its report shape and focused evidence
before enabling strict expected-provider enforcement or metadata reports in
the other tools.

## References

- `docs/planning/release-1/new-build-system-vision-and-migration-plan-2026-08-27.md`
- `docs/planning/release-1/new-build-phase-1-progress-2026-08-27.md`
- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/CREXX_LIBS.md`
- `docs/ai-context/RXAS_ASSEMBLER.md`
- `docs/ai-context/RXLINK_LINKER.md`
- `tools/newbuild/build-manifest.schema.json`
- `tools/newbuild/README.md`
- `compiler/rxcpmain.c`
