# CREXX Roadmap

Status: consolidated project direction; core/functional/performance milestones
and the parallel application track revised by Adrian on 2026-09-18; beta 3
documentation baseline finalized 2026-09-30; compiler-profile and fast-path
study recorded 2026-10-07. This is not a release contract.

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

- The current product and documentation baseline is `1.0.0-beta.3`; the
  [beta 3 notes](releases/v1.0.0-beta.3.md) describe its scope and limitations.
  Release publication is identified by versioned tags and their assets. The
  release train has been rebaselined after the extended performance
  programme: beta 3 targets 2026-09-30 and Release 1 targets 2027-05-01, ready
  for the planned May 2027 London Rexx Symposium. The symposium's exact public
  dates remain TBC.
- Level B is the principal implemented language surface. The initial Level G
  concurrency and provider layers are implemented development content, not a
  claim that the full Level G language contract is stable.
- Level C has closed whole-instruction reviews `LC-I-01–24`, including ARG,
  PARSE, CALL, SIGNAL and TRACE on their agreed contracts. INTERPRET is parked
  as not implemented, without a Release 1 "won't implement" decision. BIF,
  host, source, expression, shared-AST and full qualification gaps remain. The
  [Level C worklist](planning/release-1/levelc-compatibility-worklist.md)
  owns the current ten-point gap register, status and evidence. Unsupported
  shapes still reject rather than silently changing semantics.
- RexxScript is already a distinct standalone and embedded interpreted product.
  It is sandboxed and string-first, shares Classic BIF foundations where
  appropriate, and remains separate from the compiled Level C path.
- PERF3 and `POSTPERF-01` through `POSTPERF-05` are complete. The retained
  scorecard is strong overall. Beta 6 is now explicitly selected as a new
  performance phase against the functionally complete platform; historical
  programme closure does not automatically select its individual mechanisms.
- The [18 September optimization-boundary audit](planning/release-1/optimization-boundary-audit-2026-09-18.md)
  found **OPT-BOUNDARY-01**, an RXAS incoming-argument alias defect, now repaired
  with permanent regressions and 102 passing affected tests. Repair `f7a8b08c1`
  is published to develop; normal combined-head hosted gates are tracked in
  [the defect-batch execution record](qa/beta3-defect-batch-2026-09-18/README.md).
  Adrian accepted the RXC-owned imported-inline and public
  RXAS status-bit boundaries. The follow-up maintainability/fusion-ownership
  mechanisms remain unselected individually; the Beta 6 phase below records
  the later scheduling decision without approving a particular architecture.
- Level B Unicode issue
  [#583](https://github.com/adesutherland/CREXX/issues/583) is closed. Keep the
  resulting code, tests, and documentation as release evidence rather than an
  active roadmap item.

## Release Train To Release 1

The longer runway supports core completion in November, platform functional
completion in January, and a dedicated Performance Beta in March. Applications,
working tools and showcases proceed in parallel through RC1. Dates are planning
targets; a missed gate needs an explicit disposition, not automatic movement
of feature work into the performance phase.

| Milestone | Target | Intended product outcome |
| --- | --- | --- |
| Beta 3 | 2026-09-30 | Publish the accumulated foundation and performance work; close release defects, package/demo/policy reconciliation, and the named hosted gates. KeyAccess verdicts were accepted and closed on 2026-09-18. Do not add a new broad architecture programme to this cut. |
| Beta 4 — core complete | 2026-11-30 | Complete the core, including Level C and polymorphism. Level C means complete compatibility coverage except individually approved "won't implement" exceptions. Preserve RexxScript stabilization, documentation and conformance evidence. |
| Beta 5 — platform functionally complete | 2027-01-31 | Deliver baseline Levels G and L and complete Release 1 platform functionality, integration and representative examples. Freeze language/runtime/baseline-library features and contracts. |
| Beta 6 — Performance Beta | 2027-03-31 | Improve and qualify the functionally complete product through selected performance candidates, with current baselines and explicit verdicts. Freeze optimization implementation for RC1. |
| Release 1 RC1 | 2027-04-15 | Exact-candidate correctness, sanitizer, performance, package, install and documentation qualification. Parallel application/tool/showcase assets reach their release cut; platform feature work remains closed. |
| Release 1 | 2027-05-01 | Tag and publish the stable Release 1 assets for launch and presentation at the planned May London symposium. Update the event reference when RexxLA publishes the exact 2027 dates. |

Release 1 therefore includes completed core Level C/polymorphism, baseline
Levels G/L, the supported small RexxScript product and a parallel collection
of useful applications and demonstrations. The selected Pipes contribution
remains a candidate within those tracks, not permission for new platform
features after January. The precise G/L baselines and polymorphism completion
matrix remain open design work. Level C exceptions must be deliberate,
individually recorded decisions; "unfinished" is not "won't implement".
The [release plan](release-1-plan.md#completion-contracts-and-acceptance) owns
the numbered acceptance criteria and delivery steps.

Current release qualification is tracked in the
[formal beta 3 candidate handoff](planning/beta-3/formal-candidate-2026-09-30.md).
Outstanding preparation criteria remain in the consolidated release plan.

## Delivery Priorities And Parallel Tracks

The milestone sequence is selected by Adrian. Detailed language, ownership,
ABI, ISA and architecture decisions still require explicit design selection.
The tracks below have dependencies, but independent library and application
work should proceed now rather than waiting for the preceding beta to ship.

### 1. Cut beta 3 and maintain the Release 1 cadence

**Scope:** publish the accumulated foundation and accepted performance work by
2026-09-30; reconcile release defects, packages, demos, policy and named hosted
gates. Keep beta 3 bounded. The later train is core completion, platform
functional completion, performance, then RC1/release qualification.

At the 18 September checkpoint, the Beta 3 milestone's seven open issues were
preprocessor definition [#610](https://github.com/adesutherland/CREXX/issues/610),
RexxScript [#612](https://github.com/adesutherland/CREXX/issues/612), class-library
principles [#616](https://github.com/adesutherland/CREXX/issues/616), plugin policy
[#617](https://github.com/adesutherland/CREXX/issues/617), plugin inventory
[#622](https://github.com/adesutherland/CREXX/issues/622), package validation
[#624](https://github.com/adesutherland/CREXX/issues/624), and demos/tutorials
[#625](https://github.com/adesutherland/CREXX/issues/625). Shared BIF issue
[#615](https://github.com/adesutherland/CREXX/issues/615) is already closed and
is not another release-policy task. The formerly active
[#680](https://github.com/adesutherland/CREXX/issues/680) closed on 7 September
and remains completed evidence.

**Exit:** each beta has matching tags/assets, documented scope/limitations and
qualified evidence. RC1 remains 2027-04-15 and Release 1 remains 2027-05-01
unless Adrian explicitly changes the plan.

### 2. Complete the core, including Level C and polymorphism, by Beta 4

**Scope:** complete Level C against the Classic compatibility and BIF
references, with individually approved "won't implement" exceptions. Map every
feature to implemented evidence, open work or an explicit exception with its
reason and user-visible behavior. Preserve source provenance, diagnostics,
shared Classic value/variable-pool/BIF foundations, structured AST rewriting and
fail-closed behavior for excluded forms. The existing six execution slices are
a starting point, not the completion definition.

Make polymorphism an explicit core workstream: inventory the existing
interface/class dispatch, factory and type-checking surface, select the
remaining required behavior and qualify it across compiler, assembly, linking
and runtime boundaries. Do not silently equate this target with either the
current implementation or every possible inheritance/overload extension.

Retain the separate RexxScript product outcome: stable standalone/embedded
contract, sandbox, status/error categories, source diagnostics and examples.
It shares Classic foundations where appropriate but is not the compiled Level
C path. Broader RexxScript CALL/stem/ADDRESS/INTERPRET/object features remain
separate product decisions.

**Exit:** the complete core matrix passes its reference, positive/negative,
optimized/no-opt, toolchain and platform gates by 2026-11-30. Incomplete Level C
features cannot be relabelled exceptions without Adrian's decision.

### 3. Deliver baseline Levels G and L by Beta 5

**Scope:** select usable, supported baseline capability lists early, then
complete their APIs, ownership/lifecycle behavior, integration, documentation,
examples and packaging by 2027-01-31. G/L design and independent implementation
can run alongside November's core work.

For Level G, reconcile the existing provider, concurrency, HTTP, UI, LLM and
Unicode/library surfaces into an explicit baseline. The ownership and nested
container question remains substantive: compare explicit owner objects, owned
heterogeneous/nested containers and generic-like directions; preserve weak
reference lifetime rules and typed-array fast paths; define construction,
transfer, mutation, iteration and destruction before syntax. Use Storage, List
and a graph workload as equivalence/performance controls. Selection of a
baseline does not automatically select every extension.

For Level L, define the minimum useful language-engineering library/tooling
product. The current TinyExpr generated-output proof informs its design; it is
not itself a generator or automatically the complete baseline. Decide the
reusable lexer/parser/generator capabilities explicitly and preserve the
compiler's coherent AST-rewrite support.

Retain cREXX Pipes as a contribution candidate: Phase 1 synchronous stage
interfaces/reference executor, immutable PipePlan and named ports, deterministic
observable behavior and independently usable extension points. Any selected
library/executor contract belongs before January's platform freeze; applications
and demonstrations using it can continue afterwards. Asynchronous execution and
broader stream APIs remain separately selected phases.

**Exit:** the agreed platform baseline is functionally complete, documented and
usable on 2027-01-31. No placeholder demo or implicit reduction substitutes for
an agreed capability. Freeze platform features/contracts at this milestone.

### 4. Deliver applications, working tools and showcases in parallel through RC1

**Scope:** maintain a separate chain of useful assets: LLM consumers, practical
working tools, end-to-end examples and demonstrations. They exercise emerging
platform capabilities now and can add application functionality through
2027-04-15 using stable core/G/L contracts. They need not wait for platform
completion and need not all finish at January's platform freeze.

Each selected asset should state its owner, intended users, supported/optional/
showcase status, dependencies, setup, reproducible useful scenario and applicable
package/platform evidence. Distinguish bundled release assets from independent
applications; a new application is not automatically a new core dependency.

Feed missing capabilities back into the core/G/L plans early. An application
requiring a new public platform contract after January needs an explicit scope
exception; this parallel track does not bypass the platform freeze.

**Exit:** selected Release 1 assets are useful, reproducible and qualified by
RC1. Demonstrations at successive betas make progress visible. Application
integration and platform performance inform one another without becoming one
serial delivery queue.

### 5. Make Beta 6 the Performance Beta

**Scope:** use February/March to measure and improve the functionally complete
Beta 5 product, including representative Level C/G/L and application workloads.
Capture ideas now in the [performance roadmap](../performance/ROADMAP.md),
prepare proof/framework prerequisites during normal core work, and rank actual
implementation candidates using current residual cost and semantic evidence.

Recorded candidates include adaptive procedure-result memoization, the R1/R2
fusion ownership/benefit decision, other runtime specialization only where a
measured client exists, compiler/inlining and RXAS improvements, and runtime
value/copy/conversion costs. Consolidating the shared RXAS proof/application
interfaces supports this work; it is not a reason to postpone known correctness
repairs or duplicate proof machinery for a new optimization.

Every production performance change retains its first ordinary Release verdict.
Public annotations/effect assertions or metadata contracts needed by a proposal
must be designed before January's platform freeze or explicitly excepted; the
name "optimization" does not exempt a new language or handoff contract.
Negative/inconclusive experiments may be rejected without jeopardizing the
milestone. No particular quickener, cache or ISA change is selected merely by
assigning a performance phase.

**Exit:** Beta 6 on 2027-03-31 has measured accepted improvements, explicit
regression dispositions and retained rejected/deferred ideas. Optimization
implementation freezes for RC1; April qualifies and repairs the candidate.

## Compiler profiles and low-latency compilation — COMP-PIPE-01

**Status:** Adrian requested this connected roadmap study on 2026-10-07.
The B/G language boundary, separate executables, fast assembler build, direct
compiler-to-assembler handoff and compiled execution mode are proposals, not
approved product architecture or a change to Release 1 dates. The parked
Level C INTERPRET instruction remains unimplemented under the
[Level C worklist](planning/release-1/levelc-compatibility-worklist.md#remaining-gap-decision-register-2026-10-07).

**Vision and intended outcome:** give Level B, Classic Level C and Level G
clear source-language contracts and potentially separate compiler executables,
while keeping one validated canonical AST/symbol/flow interface to the shared
optimizer and emitter, one RXAS/RXBIN contract, and compatible build tools.
Freeze or explicitly delimit Level B before treating Level G as its extension.
Provide a low-latency route that can compile and execute source without
optimizing, and examine whether it can later support a fully correct compiled
INTERPRET. Preserve the ordinary optimized ahead-of-time product and a
human-readable `.rxas` path for inspection and diagnostics.

**Current basis and dependencies.** One `rxc` selects source levels and emits
RXAS through a `FILE *`; `rxclib` already links `rxaslib`. Both `rxc -n` and
`rxas -n` exist. `rxaslib` can initialize its parser from a bounded in-memory
text buffer, so removing the intermediate `.rxas` *file* can be tried before
inventing a typed assembler API. Removing RXAS text generation and parsing is
a further interface change. Level G already gates task/parallel syntax, while
authored `ASSEMBLER` is Level B only. Certified compiler exits currently lower
some built-in B/G statements, so deciding that user-defined compiler exits
belong only to Level G must separately preserve those compiler-owned forms.
The Level C parser/lowerer already targets the canonical compiler tree.

**Acceptance criteria — all open:**

1. **CP-AC-01 — B/G contract:** record an approved Level B freeze or precise
   supported baseline and a positive/negative B/G capability matrix, including
   syntax, libraries, diagnostics, source defaults, imports, authored
   `ASSEMBLER`, and compiler exits. Distinguish user-defined exits from
   certified compiler-owned lowering. Prove that an allowed G-only feature is
   rejected in B, and inventory existing B users before any approved break or
   migration; preserve Level C behavior.
2. **CP-AC-02 — shared compiler boundary:** if separate B/C/G executables are
   selected, they feed one documented canonical AST, symbol, source-anchor and
   flow contract into shared optimization and emission. Preserve existing `rxc`
   and `crexx` invocations or define an explicit migration, installed tools,
   DSLSH/parser mode, imports and supported-platform builds. Verify semantic,
   optimized/no-opt, diagnostic and artifact parity against the current tool.
3. **CP-AC-03 — fast assembler:** specify the mandatory parsing, validation,
   symbol resolution, backpatching, metadata and RXBIN writing steps that a
   no-optimizer profile must retain. Compare existing `rxas -n` with a lean
   build or executable before selecting one. The fast path produces valid
   linked and executable RXBIN with correct errors, source/TRACE information,
   signals and procedure/label binding.
4. **CP-AC-04 — direct handoff:** first evaluate compiler output into the
   existing in-memory RXAS scanner, removing the intermediate file while
   retaining its text grammar. Select a typed compiler-to-assembler interface
   only if measured parser/formatting cost warrants its extra contract. Either
   path must match the file-based RXAS/RXBIN, import, diagnostic, metadata,
   linking and no-opt behavior, with a retained text-output mode.
5. **CP-AC-05 — compiled execution mode:** define an explicit compile-and-run
   entry with optimization disabled in both stages, clear source/argument,
   output, status, cleanup and installed-product behavior. Compare cold and
   repeated compile/load/first-result latency, peak memory, artifact size and
   subsequent execution cost against today's `crexx` and optimized pipeline.
   This mode does not itself satisfy Classic INTERPRET's current-frame control,
   local calls, conditions or bounded generated-module lifetime.
6. **CP-AC-06 — selection and qualification:** retain same-input correctness
   and exact-output comparisons across B/C/G, both VM engines where relevant,
   debug/source metadata and supported platforms. Record measured latency,
   memory, package-size and maintenance tradeoffs before selecting executable
   splitting, a lean assembler, typed handoff or a public execution mode.
   Production performance edits follow the first ordinary Release verdict in
   `performance/AGENTS.md`; unselected alternatives remain proposals.

**Numbered steps and decision gates:**

1. **CP-STEP-01 (CP-AC-01; before the January platform contract freeze):**
   inventory current B/G differences and certified/custom exits; propose the
   Level B freeze and G-only features for Adrian's language-design approval.
2. **CP-STEP-02 (CP-AC-02; depends on 01):** map today's B/G and C parser,
   validation, symbol and lowering outputs to the shared AST/flow/emitter
   contract. Prototype executable wrappers or lean front ends only after an
   approved architecture and prove preservation of existing CLI consumers.
3. **CP-STEP-03 (CP-AC-03/06; independent of 02):** establish the current
   `rxc -n` plus `rxas -n` correctness and compile-time baseline, then measure
   a bounded no-optimizer assembler build against it. Select a separate binary
   only if it materially improves the chosen latency/size/lifecycle target.
4. **CP-STEP-04 (CP-AC-04/06; depends on 03):** try the in-memory RXAS text
   handoff; compare its gain and parity with the file path. Design a typed
   direct handoff only after that result and a separately approved interface
   contract. Do not duplicate assembler validation or change RXBIN implicitly.
5. **CP-STEP-05 (CP-AC-05/06; depends on 03/04):** qualify a compile-and-run
   mode with exact error, cleanup and repeated-use behavior; choose its public
   name and artifact/cache policy before exposing it. Keep full INTERPRET
   parked until Adrian explicitly reopens its semantic architecture decision.

**Concerns to resolve.** Separate binaries may duplicate linked code and
installed assets without reducing startup or compilation time. A B freeze that
simply disables all compiler exits would break built-in certified lowering.
The shared AST/symbol boundary must carry each language's validated meaning;
it cannot erase Classic pool/condition semantics or silently admit G syntax
to B. Existing B code using custom exits needs an explicit compatibility
decision if those exits become G-only.
The in-memory text handoff saves file I/O but still formats and parses RXAS;
a typed handoff is more intrusive and must keep assembler validation and
versioned metadata authoritative. No-opt compilation still needs semantic
lowering, type/flow checks, label resolution and safe module loading. Faster
compilation makes a compiled INTERPRET route more plausible, but cannot solve
its caller-frame transfers, resumable handlers or generated-module reclamation.
Treat those as separate Level C acceptance and approval gates.

## Constrained C89 And CMS Portability — PORT-C89

**Status:** roadmap work selected by Adrian on 2026-09-18; implementation and
qualification remain open. Start alongside the existing delivery tracks, with
release placement still to be agreed. Any selected Release 1 language/runtime
contracts must meet the 2027-01-31 platform freeze. This is the scope and
acceptance record for the work until a linked implementation worklist is needed.

**Vision and intended outcomes:** maintain one upstream source tree with a
strict C89, limited-capability configuration that ordinary development can test,
and use it as the shared foundation for Ross's native VM/370 CMS port. Bring his
portability changes into the common code and maintained platform adaptations.
Ross can continue native work immediately; converge library/executable target by
target against an agreed current development revision. His reported beta 1 base
needs an early review against the current beta 3 development code, not a large
migration deferred until the whole port is finished. The outcome includes a
native CMS build and useful execution tests, not only a host-side syntax check.

The initial profile uses switch dispatch, byte-character strings with explicit
ASCII/EBCDIC handling, single-threaded execution and static library/plugin
linkage where supported. Inventory GCCLIB, headers, compiler builtins, filenames,
external symbols, generated-function size and memory/addressability limits;
strict C89 flags alone do not enforce these constraints. In particular,
`NTHREADED` selects switch dispatch; it does not remove OS-thread dependencies.
The byte-string profile needs an explicit compatibility boundary with the normal
UTF-8/codepoint string contract. Reuse of portable host interfaces for
WebAssembly is a follow-on opportunity, with its own capability checks and no
assumption that its limits match CMS.

Two design questions remain open:

- **Integer width:** compare a software 64-bit implementation with an explicitly
  identified 32-bit `.int` profile. Adrian is open to 32-bit integers if needed;
  this records permission to evaluate that alternative, not a selected language
  change. S/360 and S/370 have 32-bit general registers but support selected
  doubleword operations through register pairs; missing compiler support is
  distinct from mathematical impossibility. Assess arithmetic cost and storage
  alongside compiler constant folding, overflow/conversions, RXAS/RXLINK/RXBIN,
  native interfaces and library/capability representations. A narrower language
  integer does not automatically eliminate fixed-width serialization or internal
  64-bit needs. Identify incompatible artifacts and reject unsupported values
  explicitly; do not silently truncate or change the desktop `.int` contract.
- **Green threads:** evaluate cooperative VM tasks after the single-threaded
  baseline. A scheduler could run bounded bytecode slices without OS threads,
  but suspension/resumption, nested native callbacks, execution-local ownership,
  cancellation, deadlines and blocking CMS I/O need proof. Reuse the existing
  task/channel abstraction if its contract can be met; expose unsupported
  capabilities honestly. No parallel CPU execution or interruptible native I/O
  is implied. A green-thread backend is an experiment, not a prerequisite for
  the first usable CMS targets or an already approved architecture.

**Companion candidate — GCC370-01, GCC/370 Toolchain:** Adrian proposed a
separate compiler effort on 2026-09-18. Start with a reproducible native/cross
GCCMVS baseline and retained failing C programs, then repair demonstrated backend
defects in the existing compiler lineage. Mike reports strong evidence of a
literal-pool addressability defect (`IFO209`) in GCCCMS 3.2.3; computed-goto
failures need their own exact reproducer and generated-assembly check. Distinguish
those from the assembler's ESD-entry capacity (`IFO264`), listing-generation
failures and runtime/startup setup: Mike's tests currently use PDPCLIB because
his GCCLIB load fails, whereas Ross reports a working GCCLIB environment. A
repair verdict must include assembly, linking and execution on CMS, followed by
the relevant real cREXX target. His grouped-handler tests are useful fallback
evidence, including structural proxies, not qualification of the full runtime.
Repository/source-lineage selection and implementation remain open. Restoring a
working legacy compiler is the bounded starting point; a port to modern GCC is
a separate scope decision. This work does not require Ross to pause or select
the February/March interpreter-loop performance experiments.

**Checkable acceptance criteria — all open:**

- [ ] **C89-AC-01 — constrained profile:** document and enforce the C89 language,
  library, encoding, resource and optional-capability matrix. A host CI build and
  positive/negative tests detect accidental newer-C, builtin or host-API use;
  record any explicitly permitted implementation extensions.
- [ ] **C89-AC-02 — upstream convergence:** record the shared revision and each
  target's status for `rxc`, `rxas`, `rxlink`, `rxvm` and the selected baseline
  libraries. Portability fixes are maintained upstream, with target-specific
  regressions and no untracked downstream source transformations. Partial target
  completion is not whole-toolchain completion.
- [ ] **C89-AC-03 — native CMS evidence:** integrate Ross's existing automation;
  retain compiler/runtime/assembler identities, build logs and semantic smoke
  results on CMS, including EBCDIC and static linkage. The reported baseline is
  GCCMVS for CMS 3.2.3 MVS V8.5, GCCLIB and IFOX00; verify it when qualifying.
- [ ] **C89-AC-04 — cross-compiler experiment:** retain a reproducible host-side
  GCCMVS build or a concrete feasibility blocker, compare it with the native
  compiler, and assemble/link/run representative output on CMS. Pin the target
  backend, options, headers, runtime, encoding and assembler requirements.
  Prefer the same compiler base/patch set; a different i370 GCC version is
  additional coverage, not proof of compatibility with Ross's toolchain. Record
  compile-time capacity limits that only native CMS builds can expose.
- [ ] **C89-AC-05 — integer decision:** retain bounded 64-bit/32-bit feasibility
  and cost evidence, then obtain Adrian's selection of the profile contract.
  Qualify its limits, arithmetic, conversions, artifact identification and
  interoperability/rejection rules across the affected tools and libraries.
- [ ] **C89-AC-06 — execution capability:** prove the baseline builds and runs
  without OS threads or C11 atomics and rejects unavailable task capabilities
  clearly. Record a separate accept/reject/defer verdict for green threads;
  acceptance requires fairness, blocking-call, lifecycle and ownership evidence
  against the existing concurrency contract.

**Implementation steps:**

1. **C89-STEP-01 — open** (AC-01/02/03): review Ross's branch and issue list,
   agree the current upstream revision and first target, and record the profile
   matrix and division of work. Native porting proceeds in parallel.
2. **C89-STEP-02 — planned** (AC-01/02/03): establish the strict host build and
   bring forward small target-specific fixes with native CMS regression checks;
   repeat in dependency order through the agreed baseline toolchain/libraries.
3. **C89-STEP-03 — planned, parallel** (AC-04): run the bounded GCCMVS
   cross-compiler compatibility experiment for upstream developers, Ross and
   Mike. It complements native CMS development and does not gate his progress.
4. **C89-STEP-04 — planned, early** (AC-05): prototype and compare the integer
   alternatives before dependent runtime work; implement a changed contract
   only after explicit selection. Other targets may progress meanwhile.
5. **C89-STEP-05 — planned** (AC-06): qualify single-threaded operation, then
   assess the optional cooperative-task backend. Document remaining capability
   limits and integrate the maintained host/CMS checks before claiming support.

Planning references: Ross's 18 September reply and
[`f_buildcms`](https://github.com/RossPatterson/CREXX/tree/f_buildcms);
[current integer contract](books/crexx_language_reference/data_types.md);
[concurrency ownership/provider contract](ai-context/CREXX_CONCURRENCY.md);
[IBM S/370 Principles of Operation, chapter 7](https://www.bitsavers.org/pdf/ibm/370/princOps/GA22-7000-6_IBM_System_370_Principles_of_Operation_7th_ed_198003.pdf);
[GNU Pth's cooperative scheduling and blocking-I/O discussion](https://www.gnu.org/software/pth/pth-manual.html).
These sources inform feasibility; no CMS compiler, integer or scheduler
prototype has been qualified by this roadmap update.

## Important Work Below The Cut

### Required closeout that does not consume a strategic slot

- Resolve the formal Linux performance QA-C disposition and run the named
  final-candidate hosted gates. Preserve the distinction between the frozen
  Apple scorecard and a newer release candidate.
- Close concrete release defects and reconcile issue status, package shape,
  examples, plugin classification, release notes, and documentation. Closed
  work should be consolidated into durable decisions; recover its detailed
  investigation and run records from Git history.

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
- Class/interface constants remain a post-Release-1 design candidate; Release 1
  constants remain procedure-scoped.
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
  common drivers implemented on `develop`; not assigned to a release): applications
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
  on 17 September. The implementation and documentation are integrated through
  `e457f5ec38806907ac94303dead240b92eac3381`; its normal
  [Build](https://github.com/adesutherland/CREXX/actions/runs/35280981918) and
  [CodeQL](https://github.com/adesutherland/CREXX/actions/runs/35280981774) checks
  are terminal success. The separate 18 September path-only model-setup
  follow-on remains local work in progress, with its own acceptance record in
  the linked plan. These publication checks do not close the parent native
  plan's trained-model/device or provenance criteria. This item
  does not expand or block the current native-inference release qualification.
  The [17 September approved implementation plan](planning/llm-provider-interface.md)
  records the former two-artifact native restriction and approved D-01–04,
  LLM-AC-01–10 and LLM-STEP-01–05 for common drivers and broader model compatibility.
  Selecting an unavailable optional llama plugin must raise an application-catchable
  exception; common clients must compile/start without that plugin and must not
  silently fall back to a different driver.

### Evidence-gated performance follow-ons

- CD, DeltaBlue, Towers and Havlak receive bounded mechanism reviews. NBody
  and Permute remain evidence questions in the Beta 6 candidate queue, not
  assumed optimizer defects or mandatory changes.
- Later Level L inline slices, register finalisation, value caching,
  string-copy fast paths, signal specialization, VM/link hygiene, and RXAS
  instruction-family studies require fresh attribution and separate selection.
  The Beta 6 queue owns candidate selection; broad host-opcode/ISA migration
  retains its existing post-Release-1 disposition unless explicitly changed.
- JIT/MIR/LLVM-style backend work remains research. It does not displace the
  interpreter/bytecode product sequence.

### Later platform and service directions

- **19 September, published to develop:** the native provider's
  [per-layer GGUF repair](planning/native-inference-layer-geometry-20260919.md)
  accepts validated scalar/array KV-head and feed-forward geometry. Scalar
  reservation and resource bounds remain unchanged. Local 12B execution works
  at 512 context tokens; 4096 remains over the configured budget. Qualification
  and publication state are recorded in that delivery record.

- Local native inference is governed by the approved
  [parent plan](planning/native-inference-backlog.md), including CREXX-NI-01–07
  and AC-01–14. CPU/Metal embedding and generation, persistent preparation,
  shared-worker ownership, typed C factories, installation guides and examples
  have implemented acceptance evidence. The
  [pipeline plan](planning/native-inference-ci.md) owns binary delivery and
  signing. Remaining trained-model/device, resource-failure, sharing/drain,
  provenance and supported-platform criteria stay open where the parent plan
  records them. SAN-009 is closed in the
  [sanitizer register](SANITIZER-WORKLIST.md#san-009--cpu-backend-probe-unloadreload-re-registers-apple-asan-globals);
  its closure does not close those separate product criteria. The llama provider
  remains optional. Product retrieval and graph policy belong to crexx-rag.
- Public provider-plugin ABI, durable services, pool telemetry, server
  lifecycle, HTTP/2, WebSockets, and GPU work beyond the native-inference
  plan remain post-Release-1 design candidates.
- `.rpm`, MSI/WiX, `winget`, broader legacy 32-bit validation, MVS/370 and a
  full z/VM CMS port require dedicated platform ownership and evidence.
  VM/370 CMS now has the selected
  [PORT-C89 portability workstream](#constrained-c89-and-cms-portability--port-c89)
  above; its target-by-target qualification determines the supported scope.

## Completed Or No Longer Active

- KEYACCESS-01 and KEYACCESS-02: Adrian accepted the
  Windows Release verdicts, retained both implementations and closed the work
  on 2026-09-18. [The decision](../performance/DECISIONS.md#keyaccess-decision-2026-09-18--accept-retain-and-close)
  closes PERF-CLOSEOUT-02; normal exact-candidate release qualification remains
  separate.
- Compiler variadic-argument crash
  [#680](https://github.com/adesutherland/CREXX/issues/680) closed on
  2026-09-07; it is no longer an active beta 3 defect.
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
  RXC-PROJECT-01
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
  and RXC-PROJECT-01
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
- Durable performance decisions:
  [`../performance/DECISIONS.md`](../performance/DECISIONS.md)
- Level C implementation status:
  [`planning/release-1/levelc-compatibility-worklist.md`](planning/release-1/levelc-compatibility-worklist.md)
  (the remapping target is a historical early design note).
- RexxScript product documentation:
  [`../rexxscript/doc/user-guide.md`](../rexxscript/doc/user-guide.md) and
  [`../rexxscript/doc/developer-guide.md`](../rexxscript/doc/developer-guide.md)
- Pipes contribution proposal:
  [`../contrib/crexx-pipes/ARCHITECTURE_PROPOSAL.md`](../contrib/crexx-pipes/ARCHITECTURE_PROPOSAL.md)
