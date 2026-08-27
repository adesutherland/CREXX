# New Build Phase 0 Report

- **Status:** Phase 0 evidence capture and catalogue complete; strict graph gate not clean
- **Baseline:** `origin/develop` at `dc44d92909e706adc575932ba9ae72f2d6f05b7d`
- **Working branch:** `temp/newbuild`
- **Date:** 2026-08-27
- **Production build changes:** none

## 1. Outcome

Phase 0 has produced a repeatable observation tool, a schema and validator, four
fresh exact-baseline build captures, and a normalized catalogue of the active
CMake/Ninja/CTest graph. No production CMake target, dependency, test or
artifact has been changed.

The evidence phase is complete, but the proposed strict graph gate correctly
does not pass yet. Two generated paths have multiple candidate owners and the
provisional wave projection exposes 813 dependency inversions. Those are
current-graph findings to resolve or reclassify; the projection is explicitly
not an executable replacement build.

| Phase 0 condition | Result |
| --- | --- |
| Fresh macOS ARM64 Debug and Release captures | pass |
| Fresh Linux ARM64 Debug and Release captures | pass |
| Every observed action and test has a provisional layer/wave/QA classification | pass; zero final exporter fallbacks |
| Manifest structural validation | pass; 4,129 actions and 2,391 tests |
| Exactly one candidate owner per observed generated output | fail; two paths remain ambiguous |
| Current projected graph has no wave inversion | fail; 813 review findings |
| Production build semantics unchanged | pass |

## 2. Method and evidence boundary

The catalogue combines:

- CMake File API codemodel targets, artifacts and dependencies;
- expanded JSON trace events for active custom commands, outputs, cleanup and
  configured compiler invocations;
- `ctest --show-only=json-v1` for resolved tests, fixtures, labels, locks,
  serialization and timeouts; and
- Ninja target, command, graph and `missingdeps` views.

macOS captures used a clean detached worktree and 30 build jobs. Linux captures
used separate temporary Ubuntu 24.04 pods in the existing
`public-purpose-lab` Minikube profile, exact Git archives, five build jobs and a
5 GiB pod memory limit. Each namespace was removed after its evidence was
copied back.

CTest was inventoried but not run locally. No benchmark or performance
measurement was run. The latest successful exact-SHA GitHub build log was
downloaded as supplementary Windows, macOS and hosted Linux correctness
evidence.

## 3. Configured graph inventory

The final canonical Debug and Release catalogues agree on the configured graph:

| Observation | Count |
| --- | ---: |
| CMake targets | 1,500 |
| Custom targets | 1,277 |
| Custom commands | 1,352 |
| Project actions in manifest projection | 4,129 |
| Declared outputs and byproducts | 2,089 |
| Built artifact files hashed | 2,298 |
| Configured cleanup operations | 438 |
| Direct configured `rxc` invocations | 1,030 |
| CTest entries | 2,391 |
| Fixture setup entries | 31 |
| Fixture requirements | 937 |
| Resource locks | 110 |
| `RUN_SERIAL` tests | 139 |

The provisional action classification is complete:

| Product layer | Actions |
| --- | ---: |
| C0 host foundations | 62 |
| C1 core C toolchain | 416 |
| B0 Level B bootstrap | 703 |
| X certified exits | 207 |
| B1 Level B substrate | 324 |
| C core cREXX tools/libraries | 325 |
| G library | 102 |
| L libraries | 10 |
| Product | 2 |
| Optional/QA/examples/demos/contributions | 1,978 |

These counts include File API targets and observed custom actions, so they are
action counts rather than user-visible component counts.

## 4. Ownership and race findings

`ninja -t missingdeps all` processed 5,074 nodes in every capture and reported
no missing dependencies on generated files. That is positive but narrow: Ninja
cannot infer arbitrary reads, deletes, directory enumeration, provider
selection or two custom definitions that normalize to one path.

The static observer found:

| Finding | Count | Interpretation |
| --- | ---: | --- |
| cleanup touches output owned by another action | 251 | candidate shared read/write or delete/write coordination |
| CTest entry invokes `cmake --build` | 31 | build work can enter the test worker pool |
| multiple candidate output owners | 2 | strict graph error |
| command-bearing custom target has no `BYPRODUCTS` | 3 | needs human output review |
| action deletes its own declared output before regeneration | 2 | review for atomic publication |

The duplicate paths are the base assembler products for the jump-table
corruption matrix:

- `<BUILD>/interpreter/tests_jump_table_corruption_linear.rxbin` is declared
  by four generated fixture actions;
- `<BUILD>/interpreter/tests_jump_table_corruption_acph.rxbin` is declared by
  two generated fixture actions.

The dominant cleanup findings are the Level C and class-library patterns:

- 66 actions touch the consolidated `bin/rxfnsc.rxbin`;
- 54 touch `bin/classlib.rxbin`;
- four touch `bin/classlib_native.rxbin`;
- four touch `bin/rexxscript.rxbin`; and
- member-level cleanup accounts for the remaining classlib/rxfnsc paths.

These are conservative static conflicts, not a claim that all 251 have already
manifested as races. They identify where Phase 1 should replace shared
destructive coordination with private member output plus one single-owner
assembly/publication action.

## 5. Import resolution and metadata

The final exporter records ordered roots and flags for 1,030 direct configured
`rxc` launches. Of these, 143 bootstrap actions use both `--import-rxas` and
`--no-exe-import`; the other 887 retain executable-directory visibility. A
binary `-i` root is explicit in 1,001 actions, while 29 rely on implicit/default
resolution. No direct configured custom action supplies an additional source
root with `-s`/`--source`; test scripts that execute later are outside the
configure-time command trace.

This is enough to expose the policy surface, but not to prove what the compiler
actually selected. Current tools do not emit a depfile/resolution record naming
the chosen source, RXAS, RXBIN or plugin provider, rejected same-stem
candidates, or metadata digest. The manifest therefore marks metadata policy
and selected-provider evidence as `not-observed` rather than inventing it.

The B0 command pattern is directionally safer than the general build, but the
shared `bin` roots used elsewhere still allow source/binary/search-order and
in-progress-library concerns. Phase 2 remains necessary: immutable curated
roots, compiler-emitted provider evidence, and metadata-aware sidecars and
route-parity tests.

## 6. QA classification

The provisional test tiers are:

| QA tier | Tests |
| --- | ---: |
| Essential | 12 |
| Smoke | 138 |
| Comprehensive | 2,207 |
| Qualification | 3 |
| Stress | 7 |
| Measurement | 24 |

This is complete as a provisional classification, but not yet an approved
coverage model. There are 995 tests without any existing labels, so most were
placed in comprehensive by conservative default. Phase 1 must add independent
tier, component, product-layer, capability, mode and platform labels before
the selections become contractual.

All 24 `performance-measurement` tests are currently `RUN_SERIAL`, and the
hosted Build CREXX workflow explicitly excludes their label. That is a useful
existing boundary. The new workflow should go further: measurement must be a
separate, quiescent invocation, not merely a serial test encountered during a
large correctness run. Stress overload timeouts must be rerun under the normal
QA profile before being classified as functional defects; timing from a busy
host remains indicative only.

The 31 nested build tests should move into an explicit `qa-prep` graph. Tests
should then consume immutable fixtures and write only to per-test work roots.

## 7. Build-shape observations

| Platform/configuration | Jobs | Configure | Build | Peak RSS observed |
| --- | ---: | ---: | ---: | ---: |
| macOS ARM64 Debug | 30 | 6.76 s | 474.56 s | 461,160,448 B |
| macOS ARM64 Release | 30 | 7.84 s | 187.84 s | 681,721,856 B |
| Minikube Linux ARM64 Debug | 5 | 8.30 s | 502.79 s | 453,386,240 B |
| Minikube Linux ARM64 Release | 5 | 8.43 s | 636.41 s | 1,360,957,440 B |

These values are not performance baselines. Other work ran on the host and
Minikube node, load changed substantially during captures, toolchains differ,
and `/usr/bin/time` peak RSS is not aggregate whole-build memory. The values
are retained only to reveal gross shape and pressure.

The common shape was:

1. native foundations fan out;
2. generated parser work creates a visible critical section;
3. optimized Linux Release simultaneously compiles several large VM
   interpreter variants, with individual memory materially larger than Debug;
4. Level B BIF production enters a long predecessor chain; and
5. broad post-bootstrap fixture/library work fans out again.

This supports separate resource weights/job pools for optimized VM compilation
and replacing the Level B predecessor chain with semantic edges. It does not
justify a speedup claim.

## 8. GitHub and local Linux proof

The exact-SHA Build CREXX run
[33056734858](https://github.com/adesutherland/CREXX/actions/runs/33056734858)
passed Windows x64 (2,380 tests), macOS ARM64/x86_64 (2,367 each) and Linux x64
(2,367). The 24 locally inventoried measurement tests account for the Unix
difference because the workflow excludes `performance-measurement`; Windows
also configures 13 platform-specific tests.

Minikube successfully provided fresh, isolated Linux ARM64 Debug and Release
build proof without waiting for a hosted runner. It validates ordinary Linux
commands and resource profiles, but it does not emulate Windows or GitHub
service behaviour. The intended model is local Minikube proof for the common
Linux build/QA actions, followed by hosted supported-platform qualification.

## 9. Tooling delivered

`tools/newbuild/` now contains:

- a deterministic CMake/File API/CTest catalogue exporter;
- the provisional manifest schema and standard-library validator;
- a clean capture driver with compressed raw evidence;
- a resource-bounded Minikube Linux capture wrapper and pod definition; and
- ten focused unit tests covering parsing, classification, import command
  recognition, duplicate ownership, timing normalization and reproducible
  compression.

The generated manifest is marked `observed-provisional-not-executable`. Running
the validator without strict mode proves structural completeness. Strict mode
fails on the current 815 graph findings as designed.

## 10. Recommended Phase 1 order

1. Give each jump-table corruption base artifact one producer and make all
   mutation fixtures depend on it.
2. Replace classlib, rxfnsc, classlib-native and RexxScript shared deletion with
   private outputs and single-owner consolidation/publication.
3. Move all 31 nested builds into explicit `qa-prep` targets.
4. Introduce CMake/Ninja resource pools for optimized VM compilation and other
   measured heavyweight actions.
5. Replace Level B predecessor serialization with direct semantic dependencies,
   retaining a clean job-1 reference and stressing jobs 5 and 30.
6. Complete the QA label matrix and provide selections that exclude
   measurement from every correctness/stress worker pool.
7. Re-export and require zero duplicate owners, zero build-during-test entries,
   and a reviewed/justified wave graph before enabling an executable manifest.

Phase 1 should remain a separate approved implementation step. This report is
the evidence and ordering input; it does not itself authorize production graph
changes.

## Evidence

- `docs/planning/release-1/evidence/new-build-phase-0-2026-08-27/`
- `docs/planning/release-1/evidence/new-build-phase-0-2026-08-27/github-build-run-33056734858.md`
- `tools/newbuild/README.md`
