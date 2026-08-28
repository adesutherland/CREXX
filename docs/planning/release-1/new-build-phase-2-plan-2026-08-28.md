# New-build Phase 2 plan — efficient CMake foundation

## Status and programme context

- **Status:** P2A locally complete; P2B active
- **Planning branch:** `temp/newbuild`
- **Current `develop` incorporated:** `origin/develop` through
  `ee8e8f8fefbdc3ee9217f2ef4dd7edf0c8d288e6`
- **Resynchronisation merge:**
  `07229d6c3e036ee515839ef99431ee2c6df5ee05`
- **Phase 1 hosted acceptance:**
  `0a0c17d56f341692d4d735ee87238c06ef6757c0`, GitHub Actions run
  `33156840598`
- **Phase 2 resolver-report qualification:**
  `f4c17459a273d7c1684f3428c7a81784560901a0`, GitHub Actions run
  `33186425039`
- **Date:** 2026-08-28

The remaining programme has three phases:

1. **Phase 2 — efficient CMake foundation:** make the current build easy to
   select, cleanly incremental and deterministic at the compiler-import
   boundary.
2. **Phase 3 — direct Level B takeover:** retain CMake/Ninja for the native and
   Level B bootstrap, then move post-bootstrap stages directly to a small Level
   B builder.
3. **Phase 4 — consolidation and qualification:** remove superseded
   post-bootstrap CMake ownership and qualify the final build, QA, install and
   packaging workflow.

This document is the implementation plan for Phase 2. Before each subphase,
work reports both its local purpose and its place in this overall programme so
that a useful diagnostic or test does not silently expand into a new build
architecture.

No performance measurement belongs to Phase 2 correctness work. Timings from
a shared host or hosted runner are diagnostic observations only. Performance
measurement remains an explicit, serial activity on a deliberately quiet host.

## 1. Why the plan was simplified

The original Phase 2 plan mixed build efficiency with a general manifest
compiler, metadata inventories for every tool, broad route-parity
qualification, a manifest-generated CMake comparison tree and platform-specific
file-access auditing. Those subjects are individually useful, but most do not
make the normal clean or incremental build faster.

Phase 1 has already established the required safety foundation:

- one owner for every generated product path;
- no cross-action cleanup;
- no CTest build of the active tree;
- reviewed product layers and execution waves;
- separate stress and performance-measurement selections; and
- resource pools for the native work proven to need them.

Phase 2 therefore concentrates on the remaining user-visible build loop:

- select a named product stage;
- build only that stage and its real prerequisites;
- repeat with no work when nothing changed;
- rebuild only the correct reverse dependency closure after a change; and
- select a QA tier without preparing unrelated fixtures.

## 2. Decisions and boundaries

### 2.1 CMake remains authoritative through Phase 2

CMake/Ninja remains the only executor in Phase 2. The observed manifest and
catalogue remain diagnostic tools; they are not promoted into a second source
of build dependency truth. Phase 2 will not generate a replacement CMake action
table or build a shadow product tree.

### 2.2 The bootstrap boundary replaces a universal low-level graph

Phase 3 will use a clear ownership split:

- CMake/Ninja owns the native toolchain and Level B bootstrap through the
  qualified Level B substrate; and
- the Level B builder owns post-bootstrap Level C, G, L, product and optional
  work.

The two executors need a small, versioned handoff contract for bootstrap tools
and artifacts. They do not need to execute the same complete low-level graph.
One human-facing stage view will describe the whole build without duplicating
action ownership.

### 2.3 Import determinism is retained, but kept proportional

The current opt-in `rxc --import-resolution-report` implementation and focused
precedence regression are retained as diagnostics. Phase 2 will not expand
them into complete symbol/provider accounting, sidecar metadata manifests or a
general cache identity unless a concrete incremental-build dependency requires
that information.

Normal build actions instead use the simpler invariant already proven for key
families: curated immutable roots, `--no-exe-import` where applicable, and one
eligible provider for a namespace or stem.

### 2.4 Race proof is structural first

Routine qualification uses:

- unique output ownership and no ancestor-overlapping owners;
- declared generated inputs and direct CMake dependencies;
- `ninja -t missingdeps`;
- before/after Ninja-log comparison around CTest;
- no-op and changed-input incremental checks; and
- repeated high-parallel execution at the phase boundary.

OS-level file tracing and broad schedule fuzzing are diagnostic tools for an
unexplained race, not mandatory infrastructure or routine gates.

## 3. Phase 2 subphases

### P2A — clear stage entry points and selective QA preparation

**Programme purpose:** make the already race-safe CMake graph understandable
and directly useful for short developer build/test cycles. This completes the
CMake foundation that Phase 3 will use only for bootstrap work.

#### P2A.1 — named product-stage entry points

Add non-owning aggregate CMake targets for:

1. native foundations;
2. core C toolchain;
3. Level B bootstrap library;
4. certified exits;
5. Level B class/native substrate;
6. core REXX-based tools and Level C library;
7. Level G library;
8. Level L libraries;
9. assembled product; and
10. optional examples and demonstrations.

The aggregate targets add no generated files and no duplicate dependency
tables. They name existing target selections; the underlying DAG continues to
control parallel readiness. A stage target may pull its real prerequisites,
but must not add a false barrier between otherwise independent actions.

**P2A.1 gate:** configuration succeeds; every declared stage dependency exists
on the maintained configuration; the targets appear in CMake/Ninja help; and a
focused stage plus its immediate repeat proves ordinary incremental/no-op
behaviour.

**Progress (2026-08-28): locally complete, hosted qualification deferred to
P2C.** Ten non-owning targets now expose C0, C1, B0, X, B1, C, G, L, product and
optional selections. The catalogue classifies them without fallback and all 18
lightweight catalogue tests pass. A focused `stage-c1-toolchain` build completed
after the `develop` resynchronisation. Its first repeat exposed that the `rxvm`
compatibility symlink/copy was a command-only target; it is now a declared
output that is recreated when missing or when the selected concrete VM changes.
The final immediate repeat reports `ninja: no work to do`. The reachable
`stage-product` graph passed `ninja -t missingdeps` across 1,607 nodes. No broad
CTest, stress, sanitizer, performance measurement or hosted workflow was run
for this bounded subphase.

#### P2A.2 — tier-specific QA preparation

Replace the single broad preparation dependency with explicit closures:

- `qa-prep-essential`;
- `qa-prep-smoke`;
- `qa-prep-comprehensive`;
- `qa-prep-qualification`;
- `qa-prep-stress`; and
- the existing isolated `qa-prep-measurement`.

The existing `qa-prep` target remains a compatibility aggregate for all
non-measurement correctness preparation. A tier target must not be advertised
as independently runnable from a clean tree until every executable and
generated RXBIN fixture selected by that tier has a declared target
prerequisite. Test command inspection alone is insufficient because a runtime
command may name a generated program by path or stem rather than a CMake target.

**P2A.2 gate:** each named QA tier prepares exactly its declared tests from a
clean tree; essential and smoke preparation do not pull unrelated comprehensive
or qualification-only artifacts; CTest performs no build and leaves the active
Ninja log byte-identical.

**P2A result (2026-08-28): locally complete; hosted acceptance remains in
P2C.** The ten product-stage targets and all six QA preparation targets now
select real CMake dependencies without owning outputs or starting nested
builds. Essential and smoke remain cumulative and narrow; comprehensive adds
normal correctness only; qualification and stress are independent. The old
`qa-prep` name remains a compatibility aggregate for all non-measurement
closures.

The clean-tree proof exposed and closed three classes of warm-tree assumption:
false `_prev_target` chains between independent generated fixtures, undeclared
default-provider inputs of the `crexx` driver, and hand-written tests whose
runtime command named a generated image or harness without a preparation edge.
The final fresh comprehensive preparation completed 1,788 actions at 30 jobs.
The correctness selection passed as 2,124 original passes plus a 54-test rerun
after the newly exposed fixtures were declared. Qualification passed 85/85.
Direct CTest left `.ninja_log` byte-identical.

Timing-oriented tests carrying the topical `performance` label now default to
the serial measurement tier unless they explicitly declare qualification or
stress. The current inventory is 3 essential, 147 smoke, 2,030 comprehensive,
85 qualification, 7 stress and 103 measurement tests. Stress and measurement
preparation both completed and immediately repeated with no work; their
workloads were deliberately not executed on the active host. Every tier and
the full 5,790-node graph pass `ninja -t missingdeps`. The catalogue reports no
nested active-tree build, and its manifest projection is schema-valid and
graph-clean. Elapsed times remain indicative only because the host was shared;
no sanitizer, performance measurement or hosted workflow was run in P2A.

### P2B — minimum deterministic import closure

**Programme purpose:** ensure that the CMake reference/bootstrap build does not
depend on a stale source, RXAS, RXBIN or executable-directory artifact before
post-bootstrap ownership moves to Level B.

1. Catalogue remaining production `rxc` actions that still use a broad public
   or executable-directory import root.
2. Convert only those actions whose provider choice can vary to curated private
   roots and `--no-exe-import`.
3. Retain one focused regression for simultaneous source/RXAS/RXBIN candidates
   and the known timestamp tie behaviour.
4. Use the existing resolution report for diagnosis when a route is ambiguous;
   do not require a report from every successful ordinary action.
5. Add an `rxc` depfile only where the build genuinely discovers an input that
   is not already declared in CMake.

Broad source/RXAS/RXBIN semantic parity, metadata preserve/strip inventories
and disassembler round-trip qualification remain toolchain QA work outside the
critical build migration path.

**P2B gate:** each production build family has an unambiguous declared import
root contract; seeding stale artifacts outside those roots cannot change the
result; focused import regressions pass.

### P2C — incremental and hosted acceptance

**Programme purpose:** prove that the CMake foundation is ready to become the
stable bootstrap half of the Phase 3 split.

Run a bounded acceptance matrix:

1. clean build the normal product with the portable and developer-fast job
   profiles;
2. immediately repeat and prove zero product actions;
3. change one leaf source and prove only its declared reverse dependency
   closure rebuilds;
4. change one shared provider and prove the expected wider closure rebuilds;
5. run essential and smoke QA through their tier-specific preparation targets;
6. verify CTest does not change the active Ninja log;
7. run the static ownership checks and `ninja -t missingdeps`; and
8. run the ordinary exact-SHA hosted build workflow on Linux x64, Windows x64,
   macOS ARM64 and macOS Intel once at final Phase 2 closure.

Jobs 1 is a phase-boundary determinism check, not a requirement after every
small edit. Jobs 5 and 30 represent the maintained portable and local-fast
profiles. Heavy-load stress is separate from normal correctness. Performance
measurement is not run in this phase.

**P2C gate:** the clean, no-op and changed-input contracts pass; stage and QA
selection are independently usable; the final exact SHA is hosted-green on all
four maintained platforms.

## 4. Work removed or deferred from Phase 2

| Former work | Disposition |
| --- | --- |
| full manifest compiler and generated CMake action table | removed from Phase 2 |
| former P2.6 isolated executable manifest slice | deleted; the first later slice is a real Level B-owned stage |
| complete compiler symbol-to-provider enforcement | deferred until a concrete dependency needs it |
| per-tool artifact and metadata sidecars | separate toolchain-quality work |
| broad route-parity matrix | separate toolchain qualification; retain focused known-risk regressions |
| mandatory dynamic file audit on every platform | diagnostic only when structural proof exposes a gap |
| routine schedule fuzzing and injected failure matrix | final/focused stress work, not normal build qualification |
| permanent dual-run post-bootstrap build | removed; use bounded comparison during direct migration |

The existing catalogue, schemas and resolver report remain useful diagnostic
evidence. Their presence does not require completing the former architecture.

## 5. Phase 3 handoff

Phase 2 hands Phase 3:

- a named and proven CMake bootstrap/product-stage interface;
- tier-specific QA preparation;
- a clean/no-op/incremental evidence set;
- curated deterministic imports for production build actions; and
- the exact bootstrap tools and artifacts required by Level B.

Phase 3 then implements only the smallest Level B runner needed for ordered
stages, parallel jobs within a stage, declared inputs/outputs, content-keyed
incremental skipping, fail-fast execution, private work paths and atomic
publication.

## 6. Stop points

Stop and return for direction before:

1. changing cREXX language syntax or public task APIs;
2. changing interactive `rxc` import precedence rather than isolating build
   roots;
3. promoting the observed manifest into a second executable build graph;
4. adding metadata reports or route-parity work as a build-phase requirement;
5. selecting a privileged or continuously installed file-audit backend;
6. running performance measurements on the shared host; or
7. moving the CMake/Level B ownership boundary beyond the approved bootstrap
   split.

## References

- `docs/planning/release-1/new-build-system-vision-and-migration-plan-2026-08-27.md`
- `docs/planning/release-1/new-build-phase-1-progress-2026-08-27.md`
- `tools/newbuild/README.md`
- `cmake/CrexxBuildResources.cmake`
- `cmake/CrexxQaTiers.cmake`
- `cmake/CrexxTestModes.cmake`
