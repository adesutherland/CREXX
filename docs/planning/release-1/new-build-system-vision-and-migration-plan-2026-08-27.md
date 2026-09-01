# New Build System Vision and Migration Plan

- **Status:** historical migration record; implementation complete and approved
  for integration into `develop`
- **Current `develop` incorporated:**
  `9cc31c47d87784b79e507dc067edfef646bff0e8`
- **Planning branch:** `temp/newbuild` (to be retired after integration)
- **Last updated:** 2026-09-01

## 1. Purpose

The build should make the cREXX product structure understandable, expose real
dependencies, exploit safe parallelism, and make incremental work proportional
to the source change.

The end state has two deliberately separate interfaces with one owner for each
artifact:

1. CMake/Ninja is the team and product orchestrator. It builds the host-native
   toolchain and Level B bootstrap, invokes owned Level B dependency waves,
   prepares QA, and owns install, SDK and package integration.
2. Level B builders own coherent REXX dependency graphs beneath that
   orchestrator and provide the installed, non-CMake workflow for ordinary
   REXX program and library developers.

A simple human-facing stage view joins those interfaces. The strategic rule is
not that every post-bootstrap command must leave CMake; it is that an artifact
has one producer, the dependency boundary is explicit, and the same Level B
engine can be invoked by CMake or by an installed user command.

The practical success criteria are:

- a clear named stage can be requested directly;
- independent work within that selection runs in parallel;
- an immediate repeat does no work;
- a source change rebuilds only its real reverse dependency closure;
- essential or smoke QA does not prepare unrelated comprehensive fixtures;
- build results do not depend on stale import products or job count; and
- performance measurement remains isolated from build, correctness and stress
  workloads.

## 2. Governing principles

1. **One owner per action and artifact.** CMake owns native compilation,
   orchestration, QA, install and packaging. Level B owns each REXX dependency
   graph delegated to it; CMake may invoke that graph but must not duplicate
   its member producers.
2. **Stages explain; dependencies schedule.** A named stage is a useful product
   selection, not permission to serialize independent actions.
3. **Explicit inputs and outputs.** A generated input has a declared producer;
   a producer writes privately and publishes one complete result.
4. **Incremental behaviour is a primary contract.** Clean correctness, no-op
   behaviour and changed-input closure are tested directly.
5. **Import ambiguity is removed structurally.** Production compiler actions
   receive curated immutable roots with one eligible provider; interactive
   compiler flexibility need not be redesigned.
6. **QA preparation follows QA selection.** Tests consume already prepared
   artifacts and never build or delete the active product tree.
7. **Parallelism is bounded by evidence.** Use 30 jobs on the current macOS
   development host and 5 on portable/unknown hosts, with narrow resource pools
   only for work proven to need them.
8. **Performance is separate.** Measurement runs serially on a quiet host and
   is not inferred from shared-host or hosted-runner timing.
9. **Migration is direct on the feature branch.** Small implementation waves
   are retained for diagnosis, but permanent shadow graphs and duplicate
   production paths are not.
10. **Assurance stays proportional.** Static ownership, Ninja dependency checks,
    no-op evidence and high-parallel repetition are routine; OS tracing and
    schedule fuzzing are focused diagnostics.

## 3. Product stages

The human-facing build model is:

| Stage | Contents | Executor at completion |
| --- | --- | --- |
| C0: native foundations | platform support, generator tools, RXBIN support, native libraries | CMake/Ninja |
| C1: core C toolchain | compiler, assembler, linker, VMs, disassembler and native helpers | CMake/Ninja |
| B0: Level B bootstrap | core Level B BIF modules and `library.rxbin` | CMake/Ninja |
| X: certified exits | exit-token support and certified exits image | CMake/Ninja |
| B1: Level B substrate | class/native libraries plus filesystem, hashing, process and task facilities | CMake/Ninja |
| C: core REXX tools | Level C library, RexxScript, preprocessor and debugger | CMake orchestration; owned CMake or Level B graph per artifact |
| G: Level G library | network, Unicode and other Level G facilities | Level B graph invoked by CMake or installed command |
| L: Level L libraries | independent specialist libraries | CMake orchestration; Level B graph where dependency waves add value |
| Product | driver, runtime composition, install and package inputs | CMake/Ninja |
| Optional | examples, demonstrations, contributions and experiments | explicit CMake selection or installed Level B command |

Dependencies, not the table order alone, determine readiness. An independent
Level L action may run beside a Level G or Level C action once its declared
bootstrap inputs are ready.

## 4. Parallel and incremental execution

Each action declares at least:

- stable name and stage;
- tool and arguments;
- direct inputs and dependencies;
- complete outputs and byproducts;
- private work directory;
- publication path; and
- the small set of environment or resource settings that affect it.

Phase 2 expressed those facts in CMake. Phase 3 proved that a Level B builder
can own a substantial dependency graph. Phase 4 reuses that engine where it
improves the REXX workflow without creating a second product graph.

For Level B actions, a successful action key is derived from the tool identity,
arguments and content of declared inputs. An action is skipped only when that
key matches and all declared outputs exist. Remote caching, distributed
execution and a general cache protocol are not part of the initial runner.

The first scheduler has ordered stages and a bounded parallel job pool within
each stage. Direct dependencies may further limit readiness. Weighted pools or
exclusive locks are added only for an observed resource problem; they are not
part of the minimum API.

## 5. Import boundary

`rxc` supports ordered source and binary roots, source/RXAS/RXBIN/plugin
candidates, executable-directory imports and timestamp-sensitive RXAS/RXBIN
selection. That flexibility is useful interactively but must not make a build
depend on a previous or concurrent publication.

The production rule is simpler than a universal provider database:

- construct a private import root for an action or coherent family;
- stage only declared immutable providers;
- use `--no-exe-import` where an action must not see installed/build-tree
  products;
- ensure one eligible provider per namespace or stem; and
- declare the staged providers as build inputs.

The existing opt-in resolution report remains a diagnostic for ambiguous or
route-sensitive cases. A depfile is added only where `rxc` genuinely discovers
an input that the build action cannot declare directly.

Broad route parity and metadata transformation remain important compiler,
assembler and linker qualification subjects, but they are not prerequisites
for an efficient build system.

## 6. Output isolation and race prevention

Every action writes beneath an action-private work directory. Consumers read
immutable staged outputs. Assembly has one owner, and completed products are
published by temporary-file-and-rename rather than shared append/delete
coordination.

Routine race proof consists of:

- one normalized owner for each output;
- no unordered write/write, delete/read or write/read overlap;
- declared producer reachability for generated inputs;
- `ninja -t missingdeps` for the CMake graph;
- unchanged Ninja logs during CTest;
- repeated clean and incremental builds at maintained job profiles; and
- deliberate stale-import checks at the compiler boundary.

Platform file tracing, randomized schedules and injected delays are available
when an unexplained race remains. They are not required in every normal build
or on every platform.

## 7. QA model

| Tier | Purpose | Scheduling rule |
| --- | --- | --- |
| essential | minimum toolchain and bootstrap correctness | every build/commit loop; smallest preparation closure |
| smoke | representative end-to-end confidence | normal developer and PR loop; essential plus smoke preparation |
| comprehensive | maintained broad correctness | phase checkpoints and broad CI |
| qualification | install, package, platform and focused route contracts | explicit selection |
| stress | concurrency, capacity and failure work | separate from normal correctness |
| measurement | governed performance evidence | serial, isolated and quiescent |

Each tier has a matching preparation target. A test reads immutable prepared
inputs, writes only its own temporary paths, and never invokes a build in the
active tree. A shared facility receives a named resource lock only when the
facility is genuinely exclusive.

A timeout under deliberate heavy load is retained as a capacity observation
and rerun under the normal profile. It is not hidden by globally increasing
timeouts. Performance tests never share the general parallel build or QA pool.

## 8. Migration programme

### Phase 0 — observe the current graph: complete

Captured the configured CMake, Ninja and CTest graph; assigned provisional
owners, stages and QA tiers; and retained local/macOS and Linux/Minikube
evidence. The catalogue remains diagnostic rather than executable.

### Phase 1 — make CMake race-safe: complete

Removed duplicate owners and shared cleanup, isolated the major library
families, moved active-tree builds out of CTest, separated optional material and
measurement, introduced named native resource pools, and qualified the exact
implementation SHA across the hosted matrix.

### Phase 2 — efficient CMake foundation: complete

#### P2A — stage and QA selection

1. Add named non-owning product-stage targets.
2. Map each QA tier to its exact executable and generated-artifact preparation
   closure.
3. Preserve `qa-prep` as a compatibility aggregate while making essential and
   smoke preparation genuinely narrow.

#### P2B — minimum deterministic imports

1. Find remaining production compiler actions with broad or ambiguous roots.
2. Isolate only those actions with curated inputs and `--no-exe-import`.
3. Retain focused stale/source/RXAS/RXBIN precedence regressions.
4. Use the resolver report or add a depfile only when a real dependency gap
   requires it.

#### P2C — incremental and hosted acceptance

1. Prove clean, immediate no-op, leaf-change and shared-provider rebuild
   behaviour.
2. Prove essential and smoke QA do not build unrelated tiers.
3. Run ownership, missing-dependency and unchanged-Ninja-log checks.
4. Qualify the exact final SHA once on Linux x64, Windows x64, macOS ARM64 and
   macOS Intel.

**Phase 2 gate:** CMake provides a clear, deterministic and incrementally
efficient reference/bootstrap build and selective QA interface.

### Phase 3 — Level B dependency-wave proof: complete

1. Define the small bootstrap handoff contract.
2. Implement a Level B runner with readable stages, parallel jobs, explicit
   inputs/outputs, content-keyed skipping, fail-fast execution and atomic
   publication.
3. Migrate one complete independent post-bootstrap lane as the first real
   cutover, not as a shadow manifest-to-CMake experiment.
4. Prove dynamic packaged RXBIN dependency hints through the compiler, linker,
   RXBIN metadata and VM autoload path.
5. Remove superseded member producers for the migrated graph so that each
   generated artifact has one owner.

**Phase 3 gate:** the Level B engine owns a complete real dependency graph,
passes focused clean/no-op/incremental QA, and publishes packaged dependency
hints without a permanent dual-run path.

The first real lane is now cut over: a Level B controller owns the complete
Level G/Unicode graph, using the public Level B task classes for parallel
waves. CMake only bootstraps and invokes it, and the superseded Level G CMake
producers have been removed. See
`new-build-phase-3-progress-2026-08-31.md` for the ownership and acceptance
evidence.

The accepted Phase 3 boundary deliberately does not require wholesale
migration of Level C, Level L, product assembly or packaging. Those remain
under the single-owner hybrid model above.

### Phase 4 — developer workflows, CI policy and qualification: complete

1. Publish clear build and QA contracts for REXX users, REXX program/library
   developers, plugin developers, core developers and release maintainers.
2. Add the installed non-CMake REXX program/library workflow and a focused
   optimizer-parity team target without introducing a new manifest system.
3. Make optimized Release the standard user product. Every pull-request head
   and every direct or merged push to `develop` must produce a downloadable,
   exact-SHA Release user-test binary. Debug, no-opt, sanitizer and deep lanes
   remain assurance evidence and do not replace that artifact.
4. Separate fast merge evidence, comprehensive qualification, scheduled deep
   checks, sanitizer scope, stress and quiet-host performance measurement.
5. Run clean/no-op/change-closure checks at jobs 1, 5 and 30, exact-SHA hosted
   platform gates, install/package consumers and publish the developer guide.

**Phase 4 gate:** a fresh checkout builds, tests, installs and packages the
supported product without stale inputs, manual sequencing, hidden network
requirements or build-during-test behaviour; its exact-SHA optimized Release
user-test artifact is available for every PR head and `develop` push.

The approved, interruption-resistant work packages and resume state are in
`new-build-phase-4-plan-2026-09-01.md`.

## 9. Human-facing interface

The exact spelling may evolve, but the concepts remain small:

```text
build stage c-toolchain --jobs 30
build stage level-b-bootstrap --jobs 5
build product --jobs 30
build optional examples,demos
build qa essential --jobs 30
build qa smoke --jobs 30
build qa comprehensive
build qa stress
build qa measurement       # serial; quiet host only
```

For team members these map to named CMake targets. Installed REXX users can
invoke the same owned Level B dependency engine through `crexx` for a library
or tool without adopting CMake. Complex generated, native or packaged projects
remain CMake projects.

## 10. Completion criteria

The programme is complete when:

- the stage ownership boundary is documented and mechanically true;
- each generated output has one owner and every generated input has a producer;
- immediate rebuilds are no-ops;
- representative leaf and shared-input changes rebuild only the correct
  closures;
- production compiler actions cannot see undeclared stale providers;
- essential and smoke QA have narrow, clean-tree preparation paths;
- normal CTest never builds or mutates the active tree;
- clean output is independent of jobs 1, 5 and 30;
- performance measurement is isolated from build, correctness and stress work;
- Level B runners complete their owned REXX dependency graphs without partial
  publication;
- every PR head and `develop` push provides an exact-SHA optimized Release
  user-test artifact; and
- the final exact SHA passes the supported hosted matrix and designed sanitizer
  scope.

## 11. Decisions requiring separate approval

Only these remaining architectural choices require a new approval:

1. the public Level B build/task API, if any new public surface is required;
2. the exact serialized shape of the bootstrap handoff contract;
3. a change to interactive compiler import precedence;
4. a requirement for platform OS-level file-audit infrastructure; or
5. moving artifact ownership between CMake and a Level B dependency graph.

## References

- `docs/planning/release-1/new-build-phase-2-plan-2026-08-28.md`
- `docs/planning/release-1/new-build-phase-3-progress-2026-08-31.md`
- `docs/planning/release-1/new-build-phase-4-plan-2026-09-01.md`
- `docs/planning/release-1/new-build-phase-1-progress-2026-08-27.md`
- `docs/planning/release-1/new-build-phase-0-report-2026-08-27.md`
- `cmake/CrexxBuildResources.cmake`
- `cmake/CrexxQaTiers.cmake`
- `cmake/CrexxTestModes.cmake`
- `tools/build-system/README.md`
