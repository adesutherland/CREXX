# CREXX Roadmap

Status: consolidated project direction, refreshed 2026-09-04. This document is
not a release contract.

This is the single portfolio-ordering view for CREXX. It ranks product outcomes
rather than every issue, experiment, or completed programme stage. Detailed
worklists retain evidence and implementation state; they do not create competing
roadmaps.

Use this authority split:

- code and tests describe what is implemented;
- release notes and tags describe what has been released;
- GitHub issues track concrete accepted work;
- this roadmap orders proposed outcomes and future decisions;
- [`../performance/ROADMAP.md`](../performance/ROADMAP.md) tracks only live
  performance closeout and evidence-gated performance candidates.

## Current Baseline

- `v1.0.0-beta.2` remains the latest versioned beta tag. Beta 3 material on
  `develop` is work in progress until a `v1.0.0-beta.3` tag and release assets
  exist. The release train has been rebaselined after the extended performance
  programme: beta 3 targets 2026-09-30 and Release 1 targets 2027-05-01, ready
  for the planned May 2027 London Rexx Symposium. The symposium's exact public
  dates remain TBC.
- Level B is the principal implemented language surface. The initial Level G
  concurrency and provider layers are implemented development content, not a
  claim that the full Level G language contract is stable.
- Level C has progressed beyond a parser-only proof. Six fail-closed execution
  lowering slices now cover scalar values, expressions, selected BIFs, internal
  procedures/arguments/calls, and stems through the Classic value and variable
  pool foundation. Unsupported shapes still reject rather than silently changing
  semantics.
- RexxScript is already a distinct standalone and embedded interpreted product.
  It is sandboxed and string-first, shares Classic BIF foundations where
  appropriate, and remains separate from the compiled Level C path.
- PERF3 and `POSTPERF-01` through `POSTPERF-05` are complete. The retained
  scorecard is strong overall; remaining performance work is release closeout or
  separately selected product evidence, not an automatically continuing
  optimization programme.
- Level B Unicode issue
  [#583](https://github.com/adesutherland/CREXX/issues/583) is closed. Keep the
  resulting code, tests, and documentation as release evidence rather than an
  active roadmap item.

## Release Train To Release 1

The longer runway deliberately allows feature-bearing betas rather than
treating beta 3 as the last place new Release 1 capability can land. Dates are
fixed planning targets; scope moves between betas when a vertical slice misses
its quality gate.

| Milestone | Target | Intended product outcome |
| --- | --- | --- |
| Beta 3 | 2026-09-30 | Publish the accumulated foundation and performance work; close release defects, package/demo/policy reconciliation, KeyAccess decisions, and the named hosted gates. Do not add a new broad architecture programme to this cut. |
| Beta 4 | 2026-11-30 | First user-meaningful compiled Level C vertical slice; stabilized RexxScript product contract, diagnostics, and standalone/embedded examples. |
| Beta 5 | 2027-01-31 | Extend the selected Level C/RexxScript surfaces from beta feedback; decide the Level G ownership/nested-container contract and, if approved, land its smallest useful implementation slice. |
| Beta 6 | 2027-03-31 | Complete the selected Release 1 feature set, including the synchronous cREXX Pipes reference executor if its contract is ready; publish migration/compatibility boundaries and freeze user-facing features. |
| Release 1 RC1 | 2027-04-15 | Exact-candidate correctness, sanitizer, performance, package, install, documentation, and example qualification only. Feature work is closed. |
| Release 1 | 2027-05-01 | Tag and publish the stable Release 1 assets for launch and presentation at the planned May London symposium. Update the event reference when RexxLA publishes the exact 2027 dates. |

Release 1 therefore includes more than the old beta 3 foundation plan: a
useful Level C subset, a supported small RexxScript product, the selected Level
G ownership/container increment if its design gate passes, and a demonstrable
Pipes contribution surface. A feature that misses its beta gate does not move
the Release 1 date automatically; it needs an explicit scope decision.

## Proposed Priority Order

This is the working top five for maintainer review. A language syntax,
ownership, ABI, ISA, or architecture decision still requires Adrian's explicit
selection before implementation.

### 1. Cut beta 3 and operate the feature-bearing Release 1 train

**Why now:** beta 3 slipped to the end of September because the performance
programme ran longer than the original calendar. The extra work should now be
published behind a trustworthy boundary, while the longer Release 1 horizon is
used deliberately for richer betas rather than an indefinitely moving
development snapshot.

**Scope:** freeze the beta 3 candidate for 2026-09-30; close or explicitly defer
the remaining package, demo, policy, and release-defect work; reconcile release
notes and known limits; and run the named hosted build, sanitizer, package,
concurrency, and performance gates on the appropriate exact SHA. Then maintain
the beta 4, beta 5, beta 6, RC1, and Release 1 cadence above with explicit
feature and fallback decisions. Current beta 3 inputs include plugin policy/inventory
[#617](https://github.com/adesutherland/CREXX/issues/617) and
[#622](https://github.com/adesutherland/CREXX/issues/622), package validation
[#624](https://github.com/adesutherland/CREXX/issues/624), demos/tutorials
[#625](https://github.com/adesutherland/CREXX/issues/625), and release defects
such as [#680](https://github.com/adesutherland/CREXX/issues/680).

**Exit:** beta 3 has a tag and matching assets, documentation, known
limitations, package evidence, and exact-SHA hosted results by 2026-09-30. Each
following beta publishes at least one user-visible vertical increment, and
Release 1 is cut on 2027-05-01 unless Adrian explicitly changes the date.

### 2. Deliver a useful compiled Level C vertical slice

**Why now:** compiled Classic REXX is a central CREXX differentiator, and the
risk has fallen materially because the canonical-AST lowerer, `RexxValue`,
`RexxVariablePool`, stems, procedures, and a broad shared BIF foundation now
exist.

**Scope:** turn the existing six execution slices into one user-meaningful
Classic program contract. Select the next bounded control-flow slice, with
`IF` and a simple `DO` form as candidates; add source provenance and diagnostics
that point to the user's Classic source; verify observable behavior against a
named reference interpreter; and keep every unsupported shape fail-closed.
Source provenance and shared BIF work are prerequisites inside this workstream,
not competing roadmap items.

**Exit:** a documented, tested Classic program compiles through `rxc`, `rxas`,
`rxlink`, and both applicable VMs in optimized and no-opt modes, with reference
equivalence and explicit unsupported boundaries.

### 3. Stabilize RexxScript as a small product

**Why now:** RexxScript already has a real standalone/embedded runtime and a
documented sandbox, but its integration strategy issue
[#612](https://github.com/adesutherland/CREXX/issues/612) remains open and the
implemented surface has outrun the old beta-planning description.

**Scope:** reconcile and close the positioning decision; freeze the supported
subset and sandbox boundary; provide stable status/error categories and
source-mapped diagnostics; curate one standalone and one embedded example; and
make host integration explicit through the existing facade. General `CALL`,
stems, `ADDRESS`, `INTERPRET`, and object-model breadth remain later decisions
unless required by the selected product contract.

**Exit:** the user and developer guides, tests, examples, issue state, and
runtime behavior describe the same small product. It remains explicitly
separate from Level C while sharing `rxfnsc` behavior where clean.

### 4. Decide the Level G ownership and nested-container model

**Why now:** Storage, List, and graph-workload evidence repeatedly points to a
product capability mismatch rather than a local VM micro-optimization. Current
Level B forms require extra owner/wrapper objects because nested reference
containers are not ordinary owned object values.

**Scope:** compare explicit owner objects, owned heterogeneous/nested
containers, and generic-like Level G collection directions; preserve weak
reference lifetime rules and typed-array fast paths; specify construction,
transfer, mutation, iteration, and destruction before syntax; and use Storage,
List, plus one complex graph workload as equivalence and performance controls.

**Exit:** Adrian selects a documented ownership/lifetime contract and a minimal
vertical implementation gate. No Level B syntax or benchmark-specific shortcut
is implied by placing the decision here.

### 5. Establish cREXX Pipes through a synchronous reference executor

**Why now:** this is a high-value Rexx ecosystem contribution that can reuse
the implemented endpoint/process/concurrency substrate while remaining a
separately maintainable project. It also offers a practical compatibility path
for Rexx/CMS and CognitivePipelines-style use without making background-service
claims prematurely.

**Scope:** begin with the contribution proposal's Phase 1 synchronous stage
interfaces and reference executor, then freeze an immutable `PipePlan` and named
ports. Preserve deterministic command and observable-result behavior across the
reference executor before selecting asynchronous task/channel execution.

**Exit:** the Phase 1 contract suite, examples, ownership boundaries, and
maintainer-facing extension points are clear enough for independent
contribution. Broader `rxio.*` streams and concurrent execution remain later
phases, not part of the first claim.

## How The Five Relate

Priority 1 supplies the cadence and trustworthy product boundaries. Priorities
2 and 3 share the Classic value, variable-pool, BIF, and provenance foundations
but remain different products. Priority 4 converts repeated performance
evidence into an explicit product-design decision. Priority 5 can progress at
the contract and reference-executor level without reopening the concurrency
model, and enters Release 1 only through a beta quality gate.

## Important Work Below The Cut

### Required closeout that does not consume a strategic slot

- Review and accept, revise, or reject the completed
  [`KEYACCESS-01`](../performance/KEYACCESS-01-WORKLIST.md) and
  [`KEYACCESS-02`](../performance/KEYACCESS-02-WORKLIST.md) first Release
  verdicts. This is an immediate product decision, not a reason to start
  another performance programme.
- Resolve the formal Linux performance QA-C disposition and run the named
  final-candidate hosted gates. Preserve the distinction between the frozen
  Apple scorecard and a newer release candidate.
- Close concrete release defects and reconcile issue status, package shape,
  examples, plugin classification, release notes, and documentation. Closed
  work should move to evidence/history rather than remain in the active list.

### Next-wave language, library, and integration work

- First-class source provenance, shared Classic BIF behavior, and variable-pool
  integration are carried by Level C and RexxScript before becoming independent
  programmes.
- Preprocessor product definition and macros
  ([#610](https://github.com/adesutherland/CREXX/issues/610),
  [#663](https://github.com/adesutherland/CREXX/issues/663)); SAA/data queues
  (historical discussion #424 and
  [#665](https://github.com/adesutherland/CREXX/issues/665)); class-library
  principles
  [#616](https://github.com/adesutherland/CREXX/issues/616) and future
  iteration ergonomics; broader
  Level G Unicode/collation; math-family expansion; and mixed Rexx/native
  libraries remain valuable, but are less decisive than the five outcomes
  above.
- Compile-time build metadata, loose comparison changes, optional-argument
  redesign, broader native regex packaging, and richer callback/generic
  collection syntax wait for a concrete user contract.

### Evidence-gated performance follow-ons

- CD, DeltaBlue, Towers, and Havlak receive bounded mechanism reviews only.
  NBody and Permute remain next-release product evidence.
- Later Level L inline slices, register finalisation, value caching,
  string-copy fast paths, signal specialization, VM/link hygiene, and RXAS
  instruction-family studies require fresh attribution and separate selection.
- JIT/MIR/LLVM-style backend work remains research. It does not displace the
  interpreter/bytecode product sequence.

### Later platform and service directions

- Public provider-plugin ABI, durable services, pool telemetry, server
  lifecycle, HTTP/2, WebSockets, and GPU work remain post-Release-1 design
  candidates.
- `.rpm`, MSI/WiX, `winget`, legacy 32-bit validation, VM/370, MVS/370, and a
  full z/VM CMS port require dedicated platform ownership and evidence. The
  current CMS direction is deterministic demos and compatible host/environment
  contracts rather than a full platform promise.

## Completed Or No Longer Active

- Level B Unicode [#583](https://github.com/adesutherland/CREXX/issues/583),
  tool output paths #584, RXAS float precision #585, and RXAS instruction
  coverage #586 are closed quality evidence.
- Runtime capability composition, packed numeric owners, the exact CPU
  `rxvector` provider, generic scalar access, the reusable RXAS proof service,
  and the five-stage POSTPERF sequence have completed governed verdicts.
- The initial concurrency surface is implemented and published as initial
  development content. Its detailed portability and evidence history remains
  in [`../concurrency/WORKLIST.md`](../concurrency/WORKLIST.md); future services
  do not follow automatically from that history.
- Regex functionality exists through RxLite in `rxfnsb`; a native dependency is
  a future packaging choice, not an open Release 1 blocker.
- Stale or duplicate discussions such as #316, #342, and #467 should not be
  revived without a current reproducer or product need.

## Detailed Authorities

- Release scope, cadence, and dependencies:
  [`release-1-plan.md`](release-1-plan.md)
- Beta 3 draft release note:
  [`releases/v1.0.0-beta.3.md`](releases/v1.0.0-beta.3.md)
- Current performance order and results:
  [`../performance/ROADMAP.md`](../performance/ROADMAP.md) and
  [`../performance/RESULTS.md`](../performance/RESULTS.md)
- Completed PERF3 history:
  [`../performance/PERF3-PROGRAMME-LEDGER-2026-08-17.md`](../performance/PERF3-PROGRAMME-LEDGER-2026-08-17.md)
- Level C implementation status:
  [`../compiler/docs/levelc_remapping_target.md`](../compiler/docs/levelc_remapping_target.md)
- RexxScript product documentation:
  [`../rexxscript/doc/user-guide.md`](../rexxscript/doc/user-guide.md) and
  [`../rexxscript/doc/developer-guide.md`](../rexxscript/doc/developer-guide.md)
- Pipes contribution proposal:
  [`../contrib/crexx-pipes/ARCHITECTURE_PROPOSAL.md`](../contrib/crexx-pipes/ARCHITECTURE_PROPOSAL.md)
