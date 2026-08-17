# CREXX Roadmap

Status: project direction. This document is not a release contract.

The current release documentation describes what CREXX implements today. This
roadmap is the repository home for future direction, research themes, and
discussion clean-up decisions that should not be mistaken for current language
or toolchain behaviour.

Use this rule of thumb:

* GitHub issues track concrete work that the project is willing to carry.
* Roadmap entries track direction, research, and possible future work.
* Release notes describe what has actually landed.

## Release 1 Plan

The fixed-date path to Release 1 is tracked in
[`release-1-plan.md`](release-1-plan.md). That plan carries the current
Release 1 gates, scope tiers, provisional owners, and issue candidates. This
roadmap remains the home for broader future direction and research themes.

## Beta 3 Milestone

Beta 3 is the next Release 1 foundation milestone after the tagged beta 2
baseline. It is work in progress on `develop` until `v1.0.0-beta.3` exists.
The beta 3 planning note is
[`releases/v1.0.0-beta.3.md`](releases/v1.0.0-beta.3.md).
Candidate issues and working team guidance are tracked in
[`planning/beta-3/issue-candidates.md`](planning/beta-3/issue-candidates.md)
until GitHub issues are created.

| Date | Milestone | Direction |
|------|-----------|-----------|
| 2026-06-17 | Beta 3 opens | Move live branch docs and version strings to beta 3 WIP while preserving beta 2 release notes as the latest completed beta baseline. |
| 2026-07-03 | Design lock | Approve Level B/G split, plugin policy, UTF ownership, Level C MVP, GPU/threading scope, and issue ownership. |
| 2026-07-31 | Beta 3 foundation target | Land or defer high-risk VM/compiler foundations, decide beta 3 package shape, and make the release note match actual tag assets before publication. |
| 2026-08-14 | Feature complete after beta 3 | Freeze user-facing Release 1 surface except for release defects, documentation, QA, examples, packaging, and measured performance work. |

## Packaging Roadmap

Beta 2 shipped the package formats that were already in the release close-out
path: portable ZIPs, signed Windows ZIPs through the maintainer signing flow,
macOS `.pkg` installers when Apple signing and notarization are configured, and
prototype Linux `.deb` packaging through the moving dev snapshot.

For beta 3, improve the end-user install experience while keeping portable ZIP
assets available for CI, testing, and users who do not want a system install:

| Platform | Direction |
|----------|-----------|
| macOS | Treat the signed, notarized, stapled `.pkg` as the preferred user install. Keep ZIPs as portable developer/CI archives. |
| Linux | Keep the `.deb` as adequate for Debian/Ubuntu-style users, but harden it before promoting it from prototype: install/uninstall smoke tests, dependency review, and later `.rpm` packaging. |
| Windows | Add a signed NSIS `setup.exe` for beta 3 if the signing and upload flow is reliable. It should be built from the signed Windows payload, install CREXX into a normal Windows location, add the tools to PATH, register an uninstaller, and keep the signed ZIP available for portable use. |

Longer-term Windows packaging may include WiX/MSI and `winget` publication, but
the simple click-through NSIS installer is the next practical user-experience
step.

## Release 1 / Level B Quality Issues

These items are specific enough to track as GitHub issues because they affect
the Release 1 or Level B quality bar.

| Area | Issue | Source discussions | Notes |
|------|-------|--------------------|-------|
| Unicode and text semantics | [#583](https://github.com/adesutherland/CREXX/issues/583) | #155, #162, #194, #231, #470 | Define and verify Level B behaviour for case conversion, `TRANSLATE`, comparison, and byte/codepoint conversion decisions. |

Closed Release 1 quality issues remain useful as evidence rather than active
roadmap work:

| Area | Issue | Outcome |
|------|-------|---------|
| Tool output paths | [#584](https://github.com/adesutherland/CREXX/issues/584) | Closed with normalized `-o` behavior and regression coverage. |
| RXAS float literal precision | [#585](https://github.com/adesutherland/CREXX/issues/585) | Closed with stored binary64 precision coverage separated from display formatting. |
| RXAS instruction coverage | [#586](https://github.com/adesutherland/CREXX/issues/586) | Closed after the instruction inventory and regression surface were extended. |

## Platform Roadmap

Mainframe support is a long-term platform direction, not a Release 1 desktop
release gate.

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| Legacy 32-bit platform validation | Release 1 `.int` contract | Keep `.int` signed 64-bit even when revisiting a 32-bit host ABI. Audit pointer-sized handles, RXBIN compatibility, compiler/toolchain availability, memory limits, and performance as separate platform work; do not restore a host-sized `.int` typedef. |
| z/VM CMS support | #278 | Keep the CMS interest and contact history as roadmap context. Current CMS work is best expressed as deterministic demos and ADDRESS environment compatibility rather than a full platform promise. |
| VM/370 build recovery | #294, #322 | Investigate cross-compilation and source-structure constraints after the Level B desktop release line is stable. |
| MVS/370 porting | #379 | Treat as a future platform project. Likely needs a dedicated maintainer, toolchain notes, and a clear cross-build strategy. |

## Language And Compatibility Roadmap

These items are useful direction but should not become Release 1 commitments
until they are narrowed.

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| Classic compatibility BIFs | #129 | `BITAND`, `BITOR`, and `BITXOR` belong with broader Classic/Level C compatibility planning, not as isolated Level B promises. |
| RexxScript positioning | Beta 3 planning | Treat RexxScript as a modern interpreted-only Rexx-family surface that primarily uses strings. It is not the Level C compiler path, though it may later migrate toward a light Classic Rexx subset and should share BIF implementations where that is clean. |
| Level C canonical AST lowering | Beta 3 planning, `compiler/docs/levelc_working_architecture.md` | Level C is compiled Classic Rexx. The intended implementation path is to transform the Level C parse/AST shape into the canonical compiler AST shape, then use the normal validation, optimization, and emission pipeline where possible. |
| Source provenance through generated code | Beta 3 planning | Preprocessor output, RexxScript integration, parser-mode diagnostics, Level C lowering, TRACE, and debug metadata need a shared source-line/provenance strategy so errors map to the user source rather than generated text. |
| Classic Rexx value model | Beta 3 planning | Create a dedicated Classic Rexx value class with canonical string storage and cached derived forms such as integer or numeric values. This supports Level C and shared BIFs without leaking Level B typed semantics. |
| Classic Rexx variable pool | Beta 3 planning, #424 | Create a variable-pool abstraction for Classic Rexx lookup, assignment, stems, host-variable access, SAA, and ADDRESS integration. BIFs and Level C work should use this instead of ad hoc maps. |
| Shared Rexx BIF surface | Beta 3 planning, #129 | Curate BIFs that can serve RexxScript string-first use and later Level C Classic value/pool use. Prefer shared implementation strategy over per-BIF conversion islands. |
| Rexx-style loose comparison | #233 and closed issue #150 | Keep under future compatibility review. Current Level B comparison policy should remain explicit in the language reference. |
| Argument count and optional parameters | #219 | Current `arg()`, `arg[]`, `...`, and `?name` behaviour is documented. Reopen only if a concrete unsupported case appears. |
| StringIterable loop sugar and callbacks | #591 | Level B should keep the minimal collection contract as `StringIterable.iterator()` returning a `StringIterator` with `hasNext()` / `next()`. Object-valued collections use the same explicit tagging style through `ObjectIterator` / `ObjectIterable`; string-key object maps use `StringObject...` names. A no-argument `forEach()` method is not useful without callable/reference support, so richer Java-style callback iteration belongs with future Level G facilities. Possible Level B syntax sugar such as `loop item over collection` should lower to the current iterator loop and validate against the relevant iterator interface contract. Bare collection names remain reserved for future Level G generic or generic-like surfaces. Object-key collections stay deferred until object equality/hash/ordering semantics are defined. |
| Compile-time build metadata | #454 | A `_build_date()`-style virtual function is a possible convenience, but it needs a naming and semantics decision before issue tracking. |
| Unused imports | #467 and closed issue #441 | Already tracked previously. Reopen only if the compiler policy is still wanted and not implemented. |

## Runtime, Backend, And Performance Roadmap

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| JIT / MIR / LLVM-style backend research | #331 | Research only. Keep separate from the interpreter and bytecode release contract. |
| RXAS instruction rationalisation | #288, #338, #357 | Review after instruction coverage is better understood. Preserve assembler-user value unless there is a measured maintenance cost. |
| Optimizer and loop super-instructions | #339 | `BCTP` / `IGTBR` optimizer work has landed. Future optimizer work should be benchmark-driven and covered by RXAS optimizer tests. |
| Performance benchmark portfolio expansion | [`POST-PERF3-WORKLIST.md`](../performance/POST-PERF3-WORKLIST.md) | Full AWFY Json, DeltaBlue and CD are qualified reserve lanes and change no aggregate. DeltaBlue's 4.25x optimized RXAS expansion/16.81x-18.32x bounded process gap and CD's 2.87x RXAS expansion/5.16x-5.41x bounded process gap are retained inputs to generic scalar-access and late-inlining work, not hidden by benchmark rewrites. Active next: qualify Havlak. Each addition needs pinned provenance, semantic equivalence, deterministic correctness, opt/no-opt and product/control qualification before timing or promotion. Direct Java and CPython ports are separately labelled language controls, never inferred Rexx aggregate members. |
| Generic final/concrete scalar access | transferred PERF3-04 | Start with a hand-equivalent ceiling and a generic compiler proof across the expanded suite. Cover receiver identity and initialization, writable ownership, signals, debug/source identity, opt/no-opt and both concrete VMs. Missing proof retains the ordinary call for that site; do not add JSON-, vector- or numeric-width-specific opcodes. |
| RXAS control-flow and bounded compiler optimization | PERF3-11/12 performance programme | The reusable immutable CFG, signal policy, sparse component SSA/use index and transactional proof service are implemented. Current consumers include branch threading, conversion/copy placement, capability-lazy loop-scoped joined-key reuse and the completed exit-owned PARSE lowering. Continue only with evidence-selected bounded hoisting, register finalisation and late-inlining consumers, one production candidate and first Release verdict at a time; keep exact local normalizations in the cheap peephole and do not recreate dense whole-procedure scans. |
| String performance follow-up | #470 | Performance results are useful, but regressions or semantic fallout should be tracked through concrete bugs such as #583. |

The post-PERF3 performance order is therefore benchmark foundations, generic
scalar-access proof, then broader bounded optimizer consumers. VM handler
layout is frozen until release-candidate finalisation, when the current
portfolio and platform evidence must be regenerated before any low-level shape
decision.

## Concurrency Roadmap

This is the sole project roadmap entry that orders concurrency work. The
[`concurrency/WORKLIST.md`](../concurrency/WORKLIST.md) and
[`concurrency/QA-CLOSEOUT.md`](../concurrency/QA-CLOSEOUT.md) files are detailed
status and evidence ledgers; they do not create a separate product roadmap.

The initial concurrency implementation is feature-complete on `develop` for
its frozen Release 1 surface: local and isolated-process structured tasks,
bounded channels and endpoints, child-process redirection, the Level B control
classes, Level G task/parallel syntax, and the concurrent HTTP client/server
and LLM transports over one private protocol backend.

Release 1 concurrency work is now a bounded QA and publication programme. The
target-host commands and evidence rules are frozen in
[`concurrency/qa/`](../concurrency/qa/), and the independent continuation is
defined in
[`INDEPENDENT-REVIEW-PROMPT.md`](../concurrency/qa/INDEPENDENT-REVIEW-PROMPT.md).

| Gate | Status | Work | Exit condition |
| --- | --- | --- | --- |
| QA-A: test readiness | complete | Independently review every solution point, close direct test gaps, maintain the labelled matrix in [`concurrency/TEST-MANIFEST.md`](../concurrency/TEST-MANIFEST.md), and prepare exact platform commands. | The frozen candidate is test-ready on Mac and every solution point has a source/test/risk disposition. |
| QA-B: Mac closeout | active | Run focused Debug/Release, both applicable VM modes, optimized/unoptimized toolchain paths, sanitizer, stress, broad regression and the governed performance comparison. | Correctness, sanitizer, stress, install and package proof pass. The retained performance run is diagnostic because the host was on battery; replaying its exact manifests on quiet AC power is the remaining item. |
| QA-C: Linux qualification | ready, not run | Build and run the frozen matrix plus install/package smoke tests on the supported Linux host. | Linux results are complete; defects have been repaired on Mac and replayed, not developed interactively on the slow host. |
| QA-D: Windows qualification | ready, not run | Run the same frozen matrix and package checks on the supported Windows toolchain and TLS/process backend. | Windows results are complete under the same defect-return discipline. |
| QA-E: publication decision | complete: published as initial | Reconcile packages, release notes, compatibility boundary and residual risks. | Adrian selected publication as **initial** on 2026-08-16. The corrected publication commit `53b3de77a` passed Build CREXX on Windows x64, Linux x64, macOS arm64 and macOS x86_64, published the development snapshot, and passed CodeQL. Native Linux and Windows qualification follows and may still identify blocking defects. |

Feature development remains frozen during this programme. Concrete services
and `.taskscope.ask()`, provider type `3`, a public provider-plugin ABI, pool
telemetry, server TLS/readiness/background lifecycle, HTTP/2 and WebSockets are
post-Release-1 candidates requiring separate design approval. Shared writable
VM state, detached ordinary tasks and public worker identities remain outside
the model. These candidates are recorded here so they are not lost; they are
not an approved automatic sequence after QA-E.

The publication choice does not waive QA-B through QA-D and does not declare
the surface stable. CI failures receive bounded tactical repairs and replay;
native Linux and Windows failures return to Mac under the same defect-return
policy.

## Library, Plugin, And Host Integration Roadmap

| Theme | Source discussions | Direction |
|-------|--------------------|-----------|
| Math library expansion | #384 | Existing `rxmath` is the natural home. Larger ARB / standards-inspired math work should be treated as library expansion after decimal and numeric policy settle. |
| Regex support | #399 and closed issue #414 | RxLite now provides a pure-Rexx regex surface in `rxfnsb`. External/native regex dependencies remain a future packaging decision, not an open Release 1 blocker. |
| System plugin portability | #398 | Keep platform coverage under normal plugin test hardening. Open a fresh issue only for a failing platform-specific test. |
| REXX/SAA compatibility | #424 | Continue through the `crexxsaa` and RXPA host-integration path. Variable-pool emulation needs explicit design before new commitments. |
| IO endpoints, process pipes, and native handles | [`ai-context/CREXX_IO_PIPE_WORKING.md`](ai-context/CREXX_IO_PIPE_WORKING.md), #491 | Bounded provider type `4` endpoints and type `5` structured child processes now underpin ADDRESS redirection and concurrent HTTP streaming. Broader `rxio.*` stream classes, reusable pipeline helpers and any public native-handle surface remain future work. |
| Mixed Rexx/native libraries | #432 | Combining Rexx scripts and native plugin functions into one library remains an architecture direction for plugin packaging. |
| Threads and subtasks | #491, [`concurrency/WORKLIST.md`](../concurrency/WORKLIST.md) | Local-thread and isolated-process structured tasks form the published initial receiver-owned transfer surface with no shared writable VM state. GitHub Actions qualification is green; native-host qualification remains open. Durable single-owner services and open-host/provider extension are separately approved later work. |

## Closed As Already Handled Or Stale

The following discussion topics were not converted into issues because the
current tree already handles them, they are covered by existing closed issues,
or the information was too stale to carry forward as a commitment:

* #316: namespace `hello` repro no longer fails in the current compiler.
* #342: `HASH` naming concern is stale; current hash helpers exist under
  `fnv`/`rxmath` and stem internals.
* #467: duplicate of closed issue #441.
* #399: current regex support exists in `rxfnsb`; Windows native dependency
  question was handled in closed issue #414.
* #288: `SAYX REG` was handled in closed issue #409; broader instruction
  cleanup stays roadmap-only.
