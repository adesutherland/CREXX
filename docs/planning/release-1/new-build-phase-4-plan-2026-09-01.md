# New-build Phase 4 plan — developer workflows and qualification

## Status and recovery point

- **Status:** approved; P4.1 complete; P4.2 next
- **Branch:** `temp/newbuild`
- **Current `develop` incorporated:**
  `292bf9e70e0dafd73b90c730942eb01902cd74f9`
- **Develop merge:** `9027c65785e99119558572891501aae28bb6b99a`
- **Phase 3 packaged dependency-hint commit:** `72d376e29`
- **Date:** 2026-09-01

This document is the restart point after an interruption. Resume at the first
unchecked work package below, inspect its named evidence, and do not repeat a
completed gate unless code, build inputs or test inputs relevant to it changed.
Each work package receives its own local commit before the next starts.

## 1. Phase 4 purpose and boundary

Phase 4 makes the new build useful to each kind of developer, makes optimized
Release artifacts routinely available for user testing, and qualifies the
single-owner build across supported platforms.

The final ownership model is deliberately simple:

- CMake/Ninja is the team and product orchestrator. It owns host-native
  compilation, the bootstrap, QA selection and preparation, install, external
  SDK integration and packaging.
- A Level B builder owns any coherent REXX dependency graph delegated to it.
  CMake may invoke that graph, but must not also own its member outputs.
- Installed REXX developers use the same Level B build engine through `crexx`
  without needing a CMake project.
- One generated artifact has one producer. Moving ownership is an explicit
  cutover, not a permanent shadow path.

Phase 3 is closed at the successful Level G/Unicode dependency-wave proof. It
is not necessary to migrate every Level C, Level L, product or optional action
out of CMake merely to satisfy the earlier provisional boundary wording.

## 2. Developer archetypes and normal contracts

| Archetype | Normal build | Normal QA | Close-out evidence |
| --- | --- | --- | --- |
| REXX user | installed optimized Release product; `crexx program` | program result | exact-SHA user-test archive or qualified package |
| REXX library/tool developer | installed Release toolchain and libraries; incremental `crexx --library` or `crexx --tool`; no CMake required | focused program/library tests | clean, no-op and changed-source closure |
| plugin developer | installed Release SDK; external CMake project | focused dynamic-plugin consumer | Release install, autoload, external consumer and sanitizer proof |
| core developer | source-tree CMake Debug; affected target and focused test or `qa-smoke` | optimizer/no-optimizer parity plus `qa-comprehensive` before close-out | relevant Debug, Release and sanitizer gates |
| release/QA maintainer | clean Release build and install/package matrix | comprehensive, qualification, stress and maintained sanitizer scopes | exact-SHA supported-platform, sanitizer and CodeQL gates |

These contracts describe the usual safe route, not mutually exclusive roles.
The developer guide must state when a broader route is required.

## 3. Build-type and REXX-optimization contract

Native build type and REXX bytecode optimization are independent axes:

- **Release** is the standard installed and user-test product. Its REXX
  library/tool builds optimize by default.
- **Debug** is the core-development default and must exercise optimized and
  non-optimized REXX equivalence where the optimizer is relevant.
- **RelWithDebInfo** is an investigation option for plugin/native faults, not
  the normal distributed user binary.
- **Debug with ASan/LSan** is qualification and diagnosis evidence, not a user
  artifact.
- **MinSizeRel** is optional size evidence. It must not silently stand in for
  the normal Release user product.

Every pull-request head and every push to `develop`, whether produced by a
merged pull request or pushed directly, must create a downloadable optimized
Release binary archive for user testing. The archive must:

- identify the exact source SHA and platform in its name and build metadata;
- contain a runnable product rather than requiring the source/build tree;
- use the normal optimized REXX product path;
- be created after the fast product/smoke gate for that platform; and
- be clearly labelled as a user-test artifact, not as a qualified release.

The PR artifact gives reviewers the exact candidate they are considering. The
`develop` artifact gives users the exact integrated development snapshot.
Formal packages, signing/notarization and release claims remain behind the
qualification gates. Debug, no-opt, sanitizer, CodeQL and deep jobs run as
assurance lanes and do not substitute for the Release user-test archive.

## 4. Installed non-CMake REXX workflow

Phase 4 extends the existing `crexx` driver instead of introducing a second
manifest or package-management language. The intended commands are:

```text
crexx --library build/mylib source1.crexx source2.crexx --jobs auto
crexx --tool build/mytool main.crexx support.crexx --jobs auto
crexx --tool build/mytool main.crexx support.crexx --native
```

The contract is:

- optimized REXX output by default, with an explicit non-optimized option;
- explicit sources and existing `-s`, `-i` and `-l` dependency roots;
- independent compile/assemble actions may run in parallel;
- linking is a visible barrier after its input wave;
- tool identity, arguments and declared input content determine incremental
  reuse; missing outputs force rebuild;
- an explicit rebuild option bypasses reuse;
- each final output is published atomically from private work; and
- packaged dependency hints and VM autoload remain the end-user dependency
  convenience mechanism.

Complex projects with native code, generators, packaging or non-trivial
configuration should continue to use CMake. Phase 4 does not add a universal
manifest, package manager, remote cache or dynamic linker-autoload mode.

## 5. QA and CI lanes

The lanes have different purposes and must remain visible:

| Lane | Trigger and purpose | Contents |
| --- | --- | --- |
| fast candidate | every PR head and `develop` push | Release product, smoke QA, exact-SHA user-test archive; Linux Debug optimizer parity |
| comprehensive | `develop`, release candidates and explicit dispatch | comprehensive and qualification QA on supported platforms; clean install and external consumers |
| sanitizer | maintained PR/`develop` policy | designed Linux ASan/LSan and macOS ASan scopes through `tools/asan-run.sh` |
| static analysis | maintained PR/`develop` policy | CodeQL and graph/ownership checks |
| deep | scheduled and explicit dispatch | jobs 1/5/30 output comparison, Ninja missing-dependencies, no-op and representative change closure, repeated/fuzzed stress where appropriate |
| performance | explicit quiet-host run only | serial performance tests and reports, isolated from other build/test work |

Correctness timeouts observed only under heavy parallel load are investigated
as load/scheduling evidence. Performance tests never share that load. Hosted
and busy-host elapsed times are indicative operational data, not performance
claims.

Tests and fixtures must declare their producers before CTest starts. A test
does not build or delete the active product tree. Resource locks or serialized
waves are used only for evidenced shared state; ordinary independence is the
default.

## 6. Approved work packages

### P4.1 — contracts and saved plan

- [x] Save this plan and update the programme vision and Phase 3 closure record.
- [x] Lock the hybrid one-owner boundary, developer archetypes, build-type
  contract and exact-SHA Release artifact rule.
- [x] Commit the documentation locally before implementation begins. The
  completion commit is the commit containing this checked status.

**Acceptance:** the documents provide an unambiguous interruption/restart
point and do not require an obsolete wholesale post-bootstrap migration.

### P4.2 — developer commands and focused team QA

- [ ] Implement installed `crexx` library/tool modes using a reusable Level B
  dependency-wave/action-key/publication engine.
- [ ] Provide optimized default, explicit no-opt, automatic/bounded jobs,
  incremental no-op, forced rebuild and atomic final publication.
- [ ] Add focused tests for clean, no-op, changed-input closure, failure
  publication and packaged dependency/autoload behaviour.
- [ ] Add a named optimizer-parity QA selection and document the core and
  plugin developer routes.
- [ ] Prove the installed REXX workflow and external plugin consumer.
- [ ] Commit P4.2 locally after its focused gates pass.

**Acceptance:** an installed REXX developer can incrementally build a library
or tool without CMake, while team members retain clear CMake QA entry points.

### P4.3 — hosted workflow policy and artifacts

- [ ] Make Release, not MinSizeRel, the normal user artifact configuration.
- [ ] Upload an exact-SHA optimized Release user-test archive for every PR head
  and every direct or merged `develop` push on each supported build platform.
- [ ] Keep fast, comprehensive, sanitizer, static-analysis, deep, stress and
  performance responsibilities distinct.
- [ ] Keep performance tests out of parallel correctness workloads.
- [ ] Add explicit/scheduled deep graph and incremental checks without delaying
  ordinary user-test artifact availability.
- [ ] Commit P4.3 locally after workflow syntax and local equivalents pass.

**Acceptance:** CI policy matches the archetype contracts, and a downloadable
exact candidate is a required output rather than an occasional release side
effect.

### P4.4 — qualification and close-out

- [ ] Prove clean Release output equivalence at jobs 1, 5 and 30.
- [ ] Prove immediate no-op plus representative leaf, shared-library and
  toolchain change closures.
- [ ] Run output ownership and Ninja missing-dependency checks.
- [ ] Run installed REXX workflow, install/package and external plugin SDK
  consumers from scratch locations.
- [ ] Run optimizer parity, comprehensive, qualification, separate stress and
  maintained sanitizer scopes.
- [ ] Push the completed Phase 4 commits, run exact-SHA Build, Sanitizer and
  CodeQL gates, and retain links/results.
- [ ] Publish the developer workflow and final evidence record.
- [ ] Commit the close-out record locally before publication where practical.

**Acceptance:** the Phase 4 gate in the programme vision is met. A phase is not
described as hosted-green, sanitizer-clean or qualified before its applicable
exact-SHA jobs reach terminal success.

## 7. Interruption and drift controls

When resuming:

1. Verify branch `temp/newbuild`, worktree status and the latest local commit.
2. Fetch `origin/develop`; record and review new commits before merging only
   that branch into this branch.
3. Read the checked boxes and evidence in this document; resume the first
   incomplete package.
4. Do not repeat retained passing tests if relevant code/build/test inputs have
   not changed.
5. Keep one local commit per completed P4 package. Do not mix unrelated fixes.
6. If a first-party sanitizer finding appears, create or update its `SAN-nnn`
   worklist entry and treat it as a repository blocker under `AGENTS.md`.
7. Record host load with timing observations. Do not convert busy-host timings
   into performance claims.

## 8. Explicit non-goals

- replacing CMake for native, install, SDK or package orchestration;
- migrating artifacts solely to satisfy a textual phase boundary;
- adding a general package manager, remote cache or universal manifest;
- treating user-test archives as signed or qualified releases;
- running performance tests within ordinary parallel QA; or
- hiding optimizer, platform or sanitizer assurance inside one opaque target.

## References

- `docs/planning/release-1/new-build-system-vision-and-migration-plan-2026-08-27.md`
- `docs/planning/release-1/new-build-phase-3-progress-2026-08-31.md`
- `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- `docs/ai-context/CREXX_ASAN_TESTING.md`
- `cmake/CrexxQaTiers.cmake`
- `.github/workflows/build.yml`
- `.github/workflows/sanitizers.yml`
- `tools/newbuild/README.md`
