# CREXX Roadmap

Status: consolidated project direction, refreshed 2026-09-04; library discovery
and dependency status reconciled 2026-09-13. This document is not a release
contract.

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

- RXVECTOR-02 is published through hotfix/develop as `5949ef27efd8` and installed
  in `~/.local`: generic C float32 binary owner and exact search in `rxvector`,
  preserving the packed-double API and RXVIDX/1 format. The normal local gate
  passes 210/210; automatic publication CI closure is recorded with the evidence.
  The downstream comparison retains all twenty RAG passage orders at the same
  0.965 s median as USearch, allowing that dependency to be removed. Rebuilt RAG
  `7bf0c6b` is published to main and installed, with 130/130 required local passes,
  installed CLI/MCP acceptance and unchanged reference-query passages/scores.
  RAG platform/endurance and Linux leak-specific assurance remain separate.
  [Bounded acceptance and evidence](planning/rxvector-binary-owner-20260919.md).

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
- Incremental build-and-run as the ordinary `crexx` default is a possible
  future usability improvement. It would need to preserve immediate execution,
  program arguments and clear output/rebuild controls. Automatically adding
  imported sources to a build and improving missing-runtime-input diagnostics
  are separate possible changes; none is selected here. Existing import
  discovery, packaged-library autoload and incremental project builds are
  completed capabilities, recorded below.
- **LLM-API-01 — Common LLM provider interface** (requested 2026-09-16;
  approved implementation in progress; not assigned to a release): applications
  select hosted HTTP, local-server or in-process `llama.rexx` inference during
  setup and reuse the same processing surface for supported capabilities.
  `.llm.open(config)` supplies common HTTP/native drivers while retaining
  the legacy Ollama factory and direct typed native session/request API. The current `.openai` client targets the hosted
  Responses endpoint, not an arbitrary OpenAI-compatible local server.
  The implementation supplies a common generation contract and separate embedding
  capability, with consistent results, errors, limits, finish reasons and
  capability discovery. Preserve explicit preparation, repeated/batch requests,
  incremental processing/cancellation and shared native weights. Keep HTTP/JSON
  diagnostics on provider-specific surfaces and preserve existing callers.
  Reuse the native C factories/owners; HTTP-only applications must not acquire
  a mandatory llama dependency. Interchangeability covers supported operations,
  not identical model output or unsupported feature emulation.
  **Exit evidence:** one common consumer/conformance suite switches providers;
  existing HTTP clients remain compatible; native requests retain one prepared
  model across repeated/batch work; embedding results preserve packed values
  and model/preprocessing identity so incompatible indexes are not silently
  reused; optional install and installed/native consumers pass their checks.
  API/architecture selection and the numbered implementation plan were approved
  on 17 September. This item
  does not expand or block the current native-inference release qualification.
  The [17 September approved implementation plan](planning/llm-provider-interface.md)
  records the former two-artifact native restriction and approved D-01–04,
  LLM-AC-01–10 and LLM-STEP-01–05 for common drivers and broader model compatibility.
  Selecting an unavailable optional llama plugin must raise an application-catchable
  exception; common clients must compile/start without that plugin and must not
  silently fall back to a different driver.

### Evidence-gated performance follow-ons

- CD, DeltaBlue, Towers, and Havlak receive bounded mechanism reviews only.
  NBody and Permute remain next-release product evidence.
- Later Level L inline slices, register finalisation, value caching,
  string-copy fast paths, signal specialization, VM/link hygiene, and RXAS
  instruction-family studies require fresh attribution and separate selection.
- JIT/MIR/LLVM-style backend work remains research. It does not displace the
  interpreter/bytecode product sequence.

### Later platform and service directions

- **19 September, local implementation:** the native provider's
  [per-layer GGUF repair](planning/native-inference-layer-geometry-20260919.md)
  accepts validated scalar/array KV-head and feed-forward geometry. Scalar
  reservation and resource bounds remain unchanged. Local 12B execution works
  at 512 context tokens; 4096 remains over the configured budget. Qualification
  and publication state are recorded in that delivery record.

- Local native inference has a requirements backlog:
  [`CREXX-NI-01` through `CREXX-NI-07`](planning/native-inference-backlog.md),
  captured 2026-09-11 and approved as a plan on 2026-09-14. Adrian's
  revised direction includes GPU support from the first delivery, runtime
  hardware selection, persistent preparation and batch processing, embeddings
  and lightweight local generation. The plan selects `llama.rexx` / `rxllama`,
  CPU/Metal/CUDA/Vulkan packages, shared-model ownership evaluation, and numbered
  outcomes, acceptance criteria and implementation steps. The provider remains
  optional to install. STEP-01 output is approved and STEP-02 controls are
  complete; [the STEP-02 report](planning/native-inference-step-02.md) retains
  CPU/Metal, persistence/sharing, cancellation, scratch, normal/ASan and Release
  measurement evidence, including failed historical controls and timing
  uncertainty. [STEP-03](planning/native-inference-step-03.md) is implemented
  through local CPU/Metal lifecycle, shared ownership and packaging controls;
  the native-worker transition repair and its measured legacy-call cost are
  approved, native four-worker CPU/Metal controls pass, and all 2,314
  non-measurement Debug CTests pass. Adrian approved STEP-03 closure with the
  remaining SAN-009/S3-D01 sanitizer proof assigned to STEP-06 native-inference
  release QA, owned by Codex under his direction. STEP-04 native embedding
  requests and S4-D01 complete-text handling pass normal CPU/Metal controls,
  including real workers, both VM/optimization modes and installed/native
  consumers. Adrian accepted the indicative Release overhead and unresolved
  Metal variation. The [typed C llama API and installed persistent/shared-worker
  examples](planning/native-inference-typed-interface-proposal.md) now pass their
  normal local acceptance and delivery checks: 28 Debug and 42 installed/relocated
  native Release runs across CPU/Metal and optimization/VM modes. The generic
  [C RXPA object surface](planning/rxpa-native-objects.md) and its interface-only
  provider/typed callback-return repairs also pass focused and installed SDK checks.
  [S4-05 and S4-AC-01–06](planning/native-inference-step-04.md) are ticked complete
  locally with retained evidence. The 15 September [QA01/QA02 repair and C
  factory cleanup](qa/native-inference-qa01/README.md) fixes imported task
  lowering and static archive relinking, removes the obsolete `linearfit` Rexx
  construction shim, and updates human/agent guidance. All 2,347 ordinary Debug
  tests now have passing evidence from the broad run plus affected rechecks;
  the original failed baseline is retained historically. STEP-04 is closed
  following acceptance of this report. [STEP-05 generation](planning/native-inference-step-05.md)
  is now locally complete: persistent/batched CPU/Metal generation, owned UTF-8
  chunks, shared workers and installed/native examples pass. Adrian accepted its
  first Release comparison; no material positive Metal slowdown recurred. The
  [closeout evidence](qa/native-inference-step05/README.md) retains 24 additional
  Debug consumers, 32 installed VM and 16 relocated native runs, plus all 2,347
  ordinary Debug CTests passing in a fresh broad run. Sanitizer execution and
  Windows/Linux/CUDA/Vulkan qualification remain STEP-06; SAN-009 is still open.
  The [dependency pins and contract](planning/native-inference-step-01.md)
  and live parent plan retain the full scope and remaining qualification boundary.
  Adrian's 15 September sequencing direction puts **STEP-07 documentation,
  model-download/installation guides, runnable examples and review before
  STEP-06 full QA and acceptance**. Stable step IDs are retained. Focused recipe
  checks establish documentation readiness; an AC-01–14 coverage map guides
  subsequent full qualification and final evidence reconciliation. The
  [installed guide set](../lib/plugins/llama/README.md) and
  [STEP-07 coverage/review ledger](qa/native-inference-step07/README.md) now hold
  the completed documentation work. Adrian approved S7-AC-06 on 15 September;
  STEP-07 is closed and [STEP-06 qualification](qa/native-inference-step06/README.md)
  is in progress. Local full build/preparation and 2,349/2,349 Apple-ASan tests
  now pass, alongside focused CPU/Metal and installed/relocated inference checks.
  Linux/Windows, real CUDA/Vulkan and supported Linux leak qualification remain
  open. Unchanged
  valid test evidence is reused; missing platform evidence and SAN-009 remain
  open. The detailed
  API and package proposal is approved for the planned implementation. This plan does not change the five
  priorities or assign Release 1 scope. Product
  retrieval and graph policy remain in the companion crexx-rag backlog.
- Public provider-plugin ABI, durable services, pool telemetry, server
  lifecycle, HTTP/2, WebSockets, and GPU work beyond the native-inference
  plan remain post-Release-1 design candidates.
- `.rpm`, MSI/WiX, `winget`, legacy 32-bit validation, VM/370, MVS/370, and a
  full z/VM CMS port require dedicated platform ownership and evidence. The
  current CMS direction is deterministic demos and compatible host/environment
  contracts rather than a full platform promise.

## Completed Or No Longer Active

- Level B Unicode [#583](https://github.com/adesutherland/CREXX/issues/583),
  tool output paths #584, RXAS float precision #585, and RXAS instruction
  coverage #586 are closed quality evidence.
- Source-input report [#685](https://github.com/adesutherland/CREXX/issues/685)
  is resolved as working as designed, with clearer user documentation.
  `crexx a b` compiles the explicitly listed sources and runs them together;
  imports do not add source members. Separately built binary libraries use
  the existing discovery/loading mechanisms. `--program` and `--library`
  remain optional incremental workflows for larger or repeatedly built projects.
- New-build Phase 2/P2B completed deterministic compiler import discovery:
  separate source and binary roots, documented precedence, curated production
  inputs, executable-root isolation and an opt-in resolver report. Phase 3
  completed packaged RXBIN autoload: retained binary imports identify the exact
  package for the VM to find through runtime roots. Source and RXAS imports
  deliberately emit no package hint. The
  [new-build programme](planning/release-1/new-build-system-vision-and-migration-plan-2026-08-27.md)
  is completed implementation history.
- New-build Phase 4 implemented installed `crexx --program` and `--library`
  workflows with explicit source membership, incremental builds and atomic
  publication. The later
  [`RXC-PROJECT-01`](../performance/RXC-PROJECT-SCALING-WORKLIST.md)
  repair implemented compiler-owned dependency snapshots and selective member
  rebuilding; both bounded Release performance verdicts are accepted.
  Dependencies used to decide rebuilds are discovered automatically; the
  sources included in the linked product remain explicit. These capabilities
  are implemented on `develop`; dated qualification notes in the detailed
  records do not make them unimplemented or authorize a new discovery project.
- Runtime capability composition, packed numeric owners, the exact CPU
  `rxvector` provider, generic scalar access, the reusable RXAS proof service,
  and the five-stage POSTPERF sequence have completed governed verdicts.
  In particular, RCC-1/RCC-2 implemented declarative native-provider identity,
  static-first trusted lookup and automatic native-package selection. This
  native-plugin resolution is distinct from bytecode-package autoload.
- The initial concurrency surface is implemented and published as initial
  development content. Its detailed portability and evidence history remains
  in [`../concurrency/WORKLIST.md`](../concurrency/WORKLIST.md); future services
  do not follow automatically from that history.
- Regex functionality exists through RxLite in `rxfnsb`; a native dependency is
  a future packaging choice, not an open Release 1 blocker.
- Stale or duplicate discussions such as #316, #342, and #467 should not be
  revived without a current reproducer or product need.

## Detailed Authorities

- Current compiler import discovery, program/library builds and runtime lookup:
  [`rxc`](books/crexx_programming_guide/rxc.md),
  [`crexx`](books/crexx_programming_guide/crexx.md), and
  [`RXVM_INTERPRETER.md`](ai-context/RXVM_INTERPRETER.md)
- Completed build migration and dependency-selection evidence:
  [new-build programme](planning/release-1/new-build-system-vision-and-migration-plan-2026-08-27.md)
  and [`RXC-PROJECT-01`](../performance/RXC-PROJECT-SCALING-WORKLIST.md)
- Native-provider discovery and composition:
  [runtime capability composition](planning/release-1/runtime-capability-composition-roadmap.md)
- Native inference requirements and the crexx-rag dependency:
  [`planning/native-inference-backlog.md`](planning/native-inference-backlog.md)
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
