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

## Bounded repair: captured RXVML signal diagnostics (RXS-DIAG)

Expose the directly captured signal code/name and external procedure name in
`rxvml_last_error()` when an embedding call fails with an unhandled VM signal.
This diagnostic increment does not identify or repair the intermittent ARM/Intel
ADDRESS-registration failure. The original hosted `FUNCTION_NOT_FOUND` summary
identifies `_enable_native_address_environment`; the controlled loader failure
below reproduces that path through a missing `rxfnsb.strip`. Extend the captured summary with a bounded signal message and
exact module/canonical instruction location copied before cleanup. It changes no
signal routing, cleanup, public API or ABI. A bounded context-owned summary retains
copied message bytes, but no VM payload object, frame, procedure or source pointer;
distinct context owners remain
independent. Existing literal errors and successful-call behavior are preserved.

A controlled concurrent-load interleaving now proves loss of legacy reader-list
state after the first module of a linked image and the same registration failure.
The VM must use one existing
caller-owned reader per complete file/buffer load and thread-local RXBIN error
storage. This repairs a reproduced loader mechanism; attribution of the hosted
ADDRESS incidents and affected platform qualification remain separate open gates.
Preserve partial-load/status behavior, concatenated containers and transferred
pool/graph ownership; do not change other legacy-wrapper consumers.

| ID | Observable contract or step | Status |
| --- | --- | --- |
| **RXS-DIAG-AC01** | Captured failures report the exact numeric code, existing canonical signal name and a bounded external-procedure name; formatting uses private per-context storage without a new allocation/failure path. | done |
| **RXS-DIAG-AC02** | One permanent forced-signal regression verifies failure/NULL response, two-context diagnostic isolation and subsequent successful entry/cleanup. | done |
| **RXS-DIAG-AC03** | Focused normal build/checks and the owning error-lifetime guide qualify the diagnostic increment; automatic publication CI remains distinct from local proof and the original intermittent failure remains undiagnosed. | done; local diagnostic profile |
| **RXS-DIAG-AC04** | The coordinator owns ARM/Intel replay of concurrent ADDRESS registration with the captured signal/phase, diagnosis and any material repair, followed by the affected hosted checks. Local success does not close this platform investigation; retain concurrency and the existing oracle. | loader race reproduced; hosted attribution/qualification open |
| **RXS-DIAG-AC05** | At the existing callback-capture boundary, copy a bounded signal-message prefix and exact module/canonical PC into a zero-initialized caller-owned private record before cleanup. Scope its optional sink with nested save/restore; initialize the active pointer to NULL, retain no VM pointers and add no error-path allocation. Preserve outer code/name/procedure and bounded unknown/truncated fallbacks. | done; focused local profile |
| **RXS-DIAG-AC06** | Extend the existing forced-signal fixture with constant `FUNCTION_NOT_FOUND` and unique text; verify copied message/location, two-context isolation, successful reentry and poisoned-storage initialization. Preserve existing execution/cleanup assertions. | done; focused local profile |
| **RXS-DIAG-AC07** | Both VM load paths own one stack reader for the complete operation, drain all modules/containers and close on every EOF/error/empty exit before closing a file or exiting. Delivered modules retain ownership; unread image state is released. Preserve return and partial-load behavior. | done; focused local profile |
| **RXS-DIAG-AC08** | RXBIN errors use calling-thread storage with the existing C99/MSVC/single-thread portability pattern and borrowed lifetime. Legacy convenience readers remain serial-only; no new public API, ABI or other-consumer migration is selected. | done; focused local profile |
| **RXS-DIAG-AC09** | Extend the existing span harness with exact owner-local serial module counts, fresh-context file/buffer load totals and late callback execution, retaining intentional two-context overlap in separate FILE and MEM pairs. A deterministic two-thread control verifies borrowed error isolation across another thread's reader operation. Fatal peer startup failures stop barrier waiting without weakening failed assertions. Controls prove their mechanisms, not automatic hosted-incident closure. | done; focused local profile |
| **RXS-DIAG-STEP01** | Review the existing external-entry and diagnostic ownership contracts before implementation; AC01–03. | done |
| **RXS-DIAG-STEP02** | Add the bounded summary and existing-fixture forced-signal controls without changing the capture/cleanup algorithm; AC01–02. | done |
| **RXS-DIAG-STEP03** | Qualify selected normal checks and the guide; isolated publication uses normal automatic CI under the existing authority; AC03. | done; local diagnostic profile |
| **RXS-DIAG-STEP04** | Read the affected ARM/Intel results and diagnose/repair any reproduced concurrent registration failure; AC04. | open |
| **RXS-DIAG-STEP05** | Implement only bounded detail transport and its existing-fixture controls under the reviewed private capture contract; AC05–06. Change no nested factory-match policy. | done |
| **RXS-DIAG-STEP06** | Freeze for source/memory review, then centrally qualify the affected build and fixture before publication; AC05–06. Hosted investigation remains AC04, not diagnostic-transport closure. | done; focused local profile |
| **RXS-DIAG-STEP07** | Replace only the two VM convenience-reader uses with local explicit readers, make error storage thread-local and extend the maintained harness/docs; AC07–09. | done |
| **RXS-DIAG-STEP08** | Freeze for cleanup/ownership review, then centrally qualify the affected normal and maintained memory checks and affected hosted gates. Keep original concurrency and semantic assertions; AC04/07–09. | source/memory review and focused normal done; maintained memory and hosted gates open |

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
  [historical defect-batch execution record](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/beta3-defect-batch-2026-09-18/README.md).
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
[#624](https://github.com/adesutherland/CREXX/issues/624), and examples/tutorials
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

**Explicit ownership candidate — deferred; implementation unselected.** A
fresh, fully typed value could live in stable heap storage behind a small
copyable owning handle: copying retains the owner, and the last strong release
destroys its value. Keep self/back-links weak, resource shutdown separate from
storage lifetime, and the initial owner context-affine. Preserve ordinary value
copies and existing weak-reference lifetimes. This candidate excludes arbitrary
local promotion, cycle collection and a general VM/native ownership service.
Full typed construction and failed-construction cleanup, safe scoped access
that pins the owner through normal and signal exits, and mutation/rebinding
and identity rules remain open design requirements.

HTTP controller lifetime and List's separate node arena motivate this question;
the narrow stable-controller/small-proxy HTTP repair is independent. A future
List diagnostic must use the generic owning capability and preserve its Rexx
algorithm. Storage's wrapper/nested-array limitation remains a separate gap.
Weak-reference lookup has not been established as the cause of slow benchmarks.
Before implementation selection, define bounded allocation, handle-copy and
last-release costs without bookkeeping on unrelated VM paths. The first
ordinary profiling-off Release verdict must compare an equivalent consumer and
control, guard unchanged arithmetic/control execution, and account separately
for lifecycle and peak memory before Adrian decides on broader closeout.

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

**Status:** Adrian requested this connected roadmap study on 2026-10-07 and
expanded the architecture review on 2026-10-08. The detailed
[compiler split proposal](../compiler/docs/compiler_frontend_split_proposal.md)
records the source audit, language principles/capability matrix, physical
module boundaries and proof obligations. This section remains the authoritative
implementation plan. The current task is review and proposal, not implementation
or architectural acceptance. Adrian's requested direction is a compiler per
B/C/G level with a common backend; B serves bootstrap/infrastructure integration,
G general-purpose evolution, and C Classic compatibility. B requires explicit
mapping for all stored class attributes; Adrian's later 2026-10-08 clarification
supersedes the earlier infrastructure-only rule. G permits only automatic
source-authored layout.
B owns authored assembly and G language extensions. INTERPRET starts with C
through a reusable service. The 24-bit target runs the standalone compiler,
assembler and VM separately, not together in memory. The proposal distinguishes
these requirements from the remaining decisions. G compiler exits may be
implemented in B, but all returned replacement/helper source must be G; this
separates implementation language from generated-language authority.
Separate B/C/G compilers are the requested direction. Their detailed physical
decomposition, remaining B/G capability choices, fast assembler build, direct
compiler-to-assembler handoff and compiled execution mode remain proposals,
not approved implementation work or a change to Release 1 dates. The parked
Level C INTERPRET instruction remains unimplemented under the
[Level C worklist](planning/release-1/levelc-compatibility-worklist.md#remaining-gap-decision-register-2026-10-07).

**Vision and intended outcome:** give Level B, Classic Level C and Level G
clear source-language contracts and separate compiler executables,
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
2. **CP-AC-02 — shared compiler boundary:** separate B/C/G executables
   feed one documented canonical AST, symbol, source-anchor and
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
   memory, package-size and maintenance tradeoffs when qualifying the physical
   split and selecting packaging, a lean assembler, typed handoff or a public
   execution mode.
   Production performance edits follow the first ordinary Release verdict in
   `performance/AGENTS.md`; unselected alternatives remain proposals.
7. **CP-AC-07 — compiled INTERPRET:** specify the reusable, exact-length runtime
   compilation service and its Classic activation/control/lifetime bridge.
   Demonstrate nested INTERPRET, pool replacement, arguments, local calls,
   RETURN/EXIT/SIGNAL and resumable CALL ON, bounded generated-code residency,
   failures and installed operation. Reuse `LC-87-01–05` as the semantic
   acceptance authority; review authorization does not reopen implementation
   of the parked instruction or approve a VM lifecycle change.
8. **CP-AC-08 — physically lite profiles:** define independent language,
   optimization, tool-service and host-capability choices. Prove by link maps
   that a lite compiler omits optional optimization code while preserving
   mandatory constant evaluation, validation, lowering, metadata and emission.
   Measure full resident and peak working memory for each standalone stage on
   the selected 24-bit target, including that stage's runtime dependencies and
   working buffers; compiler, assembler and VM run separately. Concurrent
   compiler/assembler/VM residency is outside this target by Adrian's explicit
   2026-10-08 clarification. Do not substitute host executable size for this
   proof. Preserve language semantics within the declared host capabilities and
   document unsupported services. Embedded runtime compilation has a separate
   budget under CP-AC-07 and is not implied for the 24-bit profile.

**Numbered steps and decision gates:**

1. **CP-STEP-01 (CP-AC-01; before the January platform contract freeze):**
   inventory current B/G differences and certified/custom exits; propose the
   Level B freeze and G-only features for Adrian's language-design approval.
2. **CP-STEP-02 (CP-AC-02; full B/G migration depends on 01):** map today's B/G and C parser,
   validation, symbol and lowering outputs to the shared AST/flow/emitter
   contract. Prototype executable wrappers or lean front ends only after an
   approved architecture and prove preservation of existing CLI consumers.
   The proposed C-only first phase below has its own approval gate and preserves
   current B/G behavior; it does not complete the full migration or AC-01/02.
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
6. **CP-STEP-06 (CP-AC-01–08; architecture review, 2026-10-08):** audit the
   current source and retained Level C evidence; document B/C/G principles and
   the capability matrix, canonical AST invariants, current-to-target module
   ownership, bootstrap/import boundaries, INTERPRET service and lite profiles.
   Check the proposal against actual callers and failure/lifecycle paths;
   distinguish source-supported findings from future experimental proof.
   Present the concrete proposal and outstanding decisions to Adrian before
   product implementation. This review may proceed before CP-STEP-01's final
   language approval; no proposed language restriction takes effect here.
7. **CP-STEP-07 (CP-AC-07; after architecture approval and CP-STEP-02/04):**
   prove the embedded compilation API and Classic activation bridge in bounded
   experiments under the existing LC-87 plan. Obtain a specific decision on
   any necessary VM module-lifecycle interface before changing it.
8. **CP-STEP-08 (CP-AC-08/06; after architecture approval):** separate mandatory
   semantic services from optional optimizers, prove language/profile parity,
   and measure actual target residency against agreed budgets. Keep a failed
   memory or lifecycle criterion open; do not silently remove language features
   to fit. Production performance changes retain the Release verdict gate.

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

### Proposed C-first extraction phase — 2026-10-10

**Status:** approved bounded implementation and local qualification complete on
`develop`, based on synchronized `91313bcc5a447e9403f95b1f0bfc4dd9503d0618`.
Adrian approved implementation on 2026-10-10: "work in develop and split level c
out as agreed". The standalone tool is `rxc-c`; existing `rxc`/`crexx` retain
compatibility. Adrian authorized development publication on 2026-10-10; the
qualified split and follow-up repairs are published together to `origin/develop`.
This is development integration; no release is claimed.
This phase serves CP-STEP-02/06 and CP-AC-02/06. The complete CP-AC-01–08
outcome and wider Level C compatibility remain open.

Adrian explicitly assigned **SAN-011 and SAN-012 closure to Codex at the next
Release 1 release-QA gate** on 2026-10-10. Both have permanent focused repairs
and normal/maintained-sanitizer proof but remain open and release-blocking.
This permits bounded local phase completion, not sanitizer-clean or release-ready
claims. SAN-QA-018 retains the same previously assigned owner/gate. The
[SAN register](SANITIZER-WORKLIST.md) owns all three closure obligations.

**Vision and intended outcomes:** deliver a separate Level C frontend and
standalone `rxc-c` compiler over the existing common semantic/optimizer/emitter
backend. Preserve current Classic behavior, optimized/no-opt compilation,
typed-library imports, source diagnostics, installed `rxc`/`crexx` use and editor
services. Give C an exact-length, request-owned frontend entry suitable for reuse
by the later runtime compiler. Keep current B/G source contracts while their
full split is reviewed. INTERPRET, fast/lite profiles, direct RXBIN handoff and
VM code reclamation retain their existing criteria and decision gates; this
phase supplies a prerequisite.

**Numbered phase acceptance criteria:** evidence is retained in the
[historical local qualification receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/classic-frontend-split-2026-10-10/README.md).

1. **CP-C-AC-01 — physical boundary (verified):** C scanner/glue/grammar/
   diagnostics/validation/lowering form one frontend feeding shared backend
   services. The standalone C product does not generate or link the B/G parser,
   G task mapper or typed source-exit host. Clean parser-mode-off, testing-off
   `rxc-c` builds generate no B header/scanner. Final Release symbol evidence
   excludes the typed parser/task/exit symbols. The backend is compiled once
   per configuration into shared object components.
2. **CP-C-AC-02 — imports and bootstrap (verified):** common declaration builders
   consume existing callable/class/constant/layout metadata without reparsing
   synthetic B/G source. Signed external CALL, source-provider discovery and
   runtime-library bootstrap retain their contracts. Typed source dependencies
   use their owning `rxc` outside the C process, preserving caller import order,
   RXAS opt-in and disabled-exit policy. Existing enum/payload identities remain
   unchanged. Source/binary/inline/provider regressions and the 109-command
   direct/linked, optimized/no-opt installed parity workload verify the route,
   including B and G providers and a source-only RXAS dependency.
3. **CP-C-AC-03 — user-visible parity (verified):** `rxc`/`crexx` retain existing
   routing while B/G/L behavior stays compatible. Preserve explicit/default
   level selection, routine mode, parser/DSLSH tooling, errors, source/TRACE
   identity and essential metadata. The 1,048-test relevant correctness sweep
   plus 84 final focused controls cover positive/negative, import/inline/editor,
   Classic expression/instruction and Unicode/stream behavior. Provenance and
   parser-debug golden changes are explained in the receipt; executable
   instruction equivalence is retained for changed optimized class fixtures.
4. **CP-C-AC-04 — reusable frontend ownership (verified):** `rxcc_parse` accepts
   source bytes plus exact length and explicit options/import context, returns
   owned status/diagnostics and releases success, malformed and backend
   import-failure state. Tests cover embedded NUL, caller-buffer mutation,
   concurrently live units and repeated failure/success requests. This is a
   parsed/mapped frontend API whose success precedes backend validation/emission;
   it does not claim runtime source-to-RXBIN, concurrent embedding or INTERPRET.
5. **CP-C-AC-05 — product qualification and delivery (verified for the approved
   bounded local phase):** Debug core product/preparation, relevant normal
   correctness, final focused regressions, Release stage/install and installed
   parity pass on recorded code/test/build inputs. The maintained single-threaded
   compiler passes six assembly/bytecode/execution comparisons with exits
   enabled. Focused Apple ASan and supported Linux ASan/LSan ownership checks
   pass. Human/agent guides and receipts are included in the substantive local
   change. Windows, Intel, native CMS/TSO and 24-bit residency are unrun here;
   full sanitizer closure remains explicitly assigned to the later gate above.
   Reuse unchanged evidence; normal automatic workflows accompany a future
   authorized publication. No hosted pass or release qualification is claimed.
6. **CP-C-AC-02a — independent metadata reader (verified):** standalone binutils
   `rxcontract` parses owned records without compiler Context/AST or B/G
   source-parser dependencies. `metadata_contract_conformance` links only
   binutils; source/link inspection proves that closure.
7. **CP-C-AC-02b — language-neutral contracts (verified):** B/G providers and
   serialized fields preserve callable/result/reference/optional/vararg/task,
   class/interface and layout identity through the reader and separate AST
   adapter. Reader/builder unit tests, source/binary/inline regressions and
   linked B/G provider parity supply evidence. Future Classic-to-G conversions
   and task-call semantics require their own language decision.
8. **CP-C-AC-02c — metadata conformance boundary (verified):** the
   [current-format conformance document](../compiler/docs/metadata_conformance.md)
   specifies admitted grammar, malformed fields, ownership, versions and
   separately validated optional inline payloads. Permanent positive/negative
   reader and builder tests verify the boundary. RXBIN 007 and callable/inline
   wire formats are unchanged.

**Numbered implementation steps and decision gates:**

1. **CP-C-STEP-01 (AC-01–05; complete):** pin source/baseline receipts and
   inventory C/common token, import, generated-helper, validation and exit-bridge
   dependencies. The [split proposal](../compiler/docs/compiler_frontend_split_proposal.md#23-level-c-progress-and-remaining-scope)
   retains the audit. Adrian approved the boundary before product edits.
2. **CP-C-STEP-02 (AC-01/02/04; complete; depends on 01):** introduce neutral
   token and declaration services, preserve binary/source contracts and isolate
   CLI from request-owned C parsing/lowering. Focused class/constant/signed-call,
   malformed-input and ownership checks pass. No source/helper contract was
   silently disabled.
3. **CP-C-STEP-03 (AC-01–04; complete; depends on 02):** wire the Classic
   component and common backend into `rxcc_lib`/`rxc-c`, retaining full
   `rxclib`/driver/editor compatibility and B/G bootstrap. Clean build and final
   linked closure establish physical extraction. Stage/install include `rxc-c`.
4. **CP-C-STEP-04 (AC-01–05; complete for the assigned local phase; depends on
   03/02a):** freeze final inputs, qualify normal/package/portable behavior,
   reconcile docs and retain receipts. The 6,976-file final code/test/build
   manifest matches the Linux focused inputs; final import/ownership changes
   have the recorded focused replay. Publish only if authorized. SAN-011/012
   broad proof was explicitly assigned later, not counted as passed. Report
   this phase separately from CP/Level C completion; LC-87 / CP-AC-07 still
   require their activation/control/lifetime architecture decision.
5. **CP-C-STEP-02a (AC-02a–c; complete; approved refinement of 02):** pin
   existing spellings, extract the independent reader, wire a separate AST
   adapter, preserve forward-declaration/inline attachment order and document
   the conformance boundary. The original direct-to-AST field parser was
   replaced. Adrian approved the reusable reader with "Yes", then confirmed
   "Keep the agreed metadata reader". No new source syntax, conversion rules or
   serialized format was approved or introduced.

**Final implementation checkpoint, 2026-10-10.** `rxc_backend_objects`,
`rxc_classic_objects` and `rxc_typed_objects` establish the physical boundary.
Common/C code uses checked `rxcp_tokens.h`. `binutils/rxcontract` reads owned
language-neutral records; `rxcp_contract` constructs common declarations.
C-only typed source imports and editor requests use the installed owning `rxc`
through explicit argv and private temporary files. Supported imported inlining
is preserved, including factory body/call-site result distinctions. Source
contracts retain actual provider identity; tokenless metadata has unknown
coordinates rather than manufactured source positions.

Qualification repaired borrowed declaration names, historical POS/LASTPOS
literal defaults, whitespace-only zero-argument signatures, factory-inline
identity, syntax diagnostics before lowering, and caller-policy forwarding.
SAN-011 repairs cross-context declaration/generated source-anchor ownership;
SAN-012 frees semantic sidecars on all Context-owned source anchors, including
those detached from the visible tree. Both have permanent regressions and
focused Debug, Apple ASan and Linux ASan/LSan passes. Apple leaks-off records
its unsupported LSan capability; Linux focused proof keeps leaks enabled.
Full Apple `20261010-171343-full` reached CTest and full Linux
`20261010-161345-full` stopped during preparation after Adrian's later-gate
assignment. Neither is a completed full-gate pass. The SAN register and receipt
retain the owner, next action and evidence. Broader CP/Level C acceptance remains
open; delivery is local, with no push or release.

**Clean CLion rebuild follow-up — CP-C-FIND-04 (2026-10-10, closed locally).**
A clean Debug `all` build at 32 jobs failed four RexxCPS fixture compilations
at `TRACE VALUE`, reporting `CLASS_NOT_FOUND`. A minimal Level B reproducer
has the same failure while `library`, `classlib` and `rxcexits` exist but
`bin/rxfnsc.rxbin` is absent. TRACE's generated helper imports RexxValue from
that consolidated runtime; these consumers lack the producer dependency.
This exposed a gap in CP-C-AC-05's clean-build qualification. The repair and
passing Debug/Release product builds below restore that proof; passed functional
checks and the agreed metadata format/semantic boundary are retained.

**Vision and intended outcomes:** make a clean CLion rebuild reliably produce
TRACE-dependent fixtures after their owning runtime, and restore both existing
Debug and Release build profiles. Preserve benchmark sources, generated runtime
semantics and metadata. Keep the repair in this C-first issue; no compiler or
production performance change is planned.

1. **CP-C-F04-AC-01 (verified):** reproduce the failure with minimal authored
   `TRACE VALUE`, then prove the same unchanged source compiles once `rxfnsc`
   is available. Retain before/after logs.
2. **CP-C-F04-AC-02 (verified):** both RexxCPS fixture families depend on the TRACE
   runtime producer and output; verify the generated graph orders producer
   publication before all four optimized/no-opt fixture commands even when
   that published artifact is absent.
3. **CP-C-F04-AC-03 (verified):** finish the actual CLion Debug/Release `all`
   builds and pass focused RexxCPS runtime/register controls. Retain commands,
   inputs and logs; reuse unchanged compiler/sanitizer evidence.

1. **CP-C-F04-STEP-01 (AC-01; complete):** reproduce and inspect TRACE's declared
   imports and the missing producer/output; build only the identified producer
   and replay the unchanged minimal source.
2. **CP-C-F04-STEP-02 (AC-02; complete; depends on 01):** add the existing TRACE runtime
   dependency to the two owning benchmark registrations. Reconfigure and test
   absent-output scheduling, without broadening the compiler-exit bootstrap
   bundle dependency (which would create a runtime/bootstrap cycle).
3. **CP-C-F04-STEP-03 (AC-03; complete; depends on 02):** complete both user build
   profiles, run focused controls, update the receipt/plan and commit the local
   repair. SAN-011/012/SAN-QA-018 keep their assigned next Release 1 QA closure.

Final follow-up evidence: the unchanged minimal source succeeds after runtime
publication; absent-output scheduling at 32 jobs publishes `rxfnsc` before all
four fixtures. Full existing Debug and Release `all` builds pass, and five
RexxCPS runtime/register controls pass in 14.41 seconds. Only the two consumer
build prerequisites changed. The [historical follow-up receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/classic-frontend-split-2026-10-10/clion-clean-rebuild.md)
retains final inputs/log hashes; source/metadata semantics and the assigned SAN
closure gates are unchanged. Delivery remains local, unpublished.

**CLion stdin and quiet-build follow-up — CP-C-FIND-05/06 (2026-10-10, closed locally).**
Adrian requested repair of the two stdin-blocked PULL tests and removal of
rebuild warnings/routine chatter. Starting local develop is `c2cc9cb59`;
`origin/develop` remains contained and unrelated active-test artifacts are kept.
The blocked VMs have been sampled in `FREADLINE`/`read` with an inherited open
CLion stdin pipe. Only those two confirmed blocked children were terminated;
the remainder of the user's CTest run continues. Its separate missing-fixture
failures are retained, not treated as passes or hidden by this repair.

**Vision and intended outcomes:** negative runtime fixtures have deterministic
stdin under the IDE, terminals and CI, while preserving their expected Classic
error/source/output contracts. Normal Debug/Release builds produce no avoidable
compiler warnings or custom successful builder chatter. Keep useful failure
output and saved builder logs, preserve existing C++17/C++20 UTF-8 path handling,
local Rexx scopes, documented APIs and the verbose direct Level G builder.
Compiler/metadata formats and language semantics are outside this repair.

1. **CP-C-F05-AC-01 (verified):** both PULL variants finish and retain 26.4,
   authored source location and exact `before` output when parent stdin stays
   open. Verify in normal Debug and Release without another full test run.
2. **CP-C-F05-AC-02 (verified):** the negative-runtime helper declares deterministic
   EOF and a generous whole-test hang backstop; neighboring negative cases
   retain their existing assertions. Existing scheduling/workload is unchanged.
3. **CP-C-F06-AC-01 (verified):** eliminate observed C++20 filesystem deprecations
   and Rexx scope/import/shadowing warnings at their sources, with no blanket
   diagnostic suppression. Verify both standard path tests and affected
   preprocessor, RexxScript, activation-argument and vector controls.
4. **CP-C-F06-AC-02 (verified):** normal CMake Level G success emits no custom
   WAVE/START/BUILT/PUBLISHED output, while preserving logs and visible failure
   diagnostics/nonzero status. Explicit verbose output and direct builder
   diagnostics remain available. Verify real success and controlled failure.
5. **CP-C-F06-AC-03 (verified):** relevant Debug/Release rebuild logs are warning-
   free and focused correctness passes on retained final inputs. Include docs
   and receipts in local reviewable commits; publication remains unauthorized.
   SAN-011/012/SAN-QA-018 retain Codex / next Release 1 release-QA closure.

1. **CP-C-F05-STEP-01 (AC-01/02; complete):** preserve the blocked-read diagnostic,
   supply an empty input file in the generated negative-runtime helper, add a
   600-second hang backstop, and replay the two cases with an open parent pipe.
2. **CP-C-F06-STEP-01 (AC-01; complete):** use the C++20 UTF-8
   path constructor with the existing C++17 route, disambiguate intentional
   local bindings and remove the unused test import. Preserve source doc tags.
3. **CP-C-F06-STEP-02 (AC-02; complete):** capture routine Level G
   builder output through a CMake launcher, retain full stdout/stderr logs,
   preserve stderr/failure reporting, and provide explicit verbose replay.
4. **CP-C-F06-STEP-03 (AC-01–03/F05-AC-01/02; complete):** rebuild affected
   products in the existing profiles after active testing permits safe reuse,
   run the smallest decisive controls, retain input/log hashes, update this
   checkpoint and commit locally. Reuse unchanged broader evidence.

Final follow-up proof: both negative PULL cases pass with a deliberately open
parent stdin pipe in Debug and Release (2/2 each), retaining their complete
error/source/output assertions. Debug and Release `all` rebuilds pass with zero
warnings or custom successful builder chatter; 37 affected controls pass in
each profile. Real quiet success, explicit verbose replay and controlled
failure diagnostic/status preservation pass. The
[historical stdin receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/quiet-build-stdin-2026-10-10/stdin-repair.md) and
[historical quiet-build receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/quiet-build-stdin-2026-10-10/quiet-build.md) retain final
input/log hashes. Ordinary build-tool progress remains; all RexxDoc blocks,
metadata formats and call semantics are preserved. The earlier full CLion run
also lacked optional QA fixtures after its product-only rebuild, so it is not
counted as a full pass. Its prerequisite issue remains separate from this
requested bounded repair. Delivery is local/unpublished; assigned SAN closure
is unchanged.

**Complete CLion CTest and visible diagnostics — CP-C-FIND-07 (2026-10-10, closed locally).**
Adrian explicitly reopened full CTest repair and rejected output capture as a
quiet-build solution: remove unnecessary notices at their source and do not
hide log output. The previous F06 launcher approach is superseded; its focused
warning repairs and F05 deterministic-stdin proof remain valid.

**Vision and intended outcomes:** preserve the agreed product/QA build tiers,
correct CLion's stale `all` prerequisite for its "All CTest" configuration,
prepare every registered test's required artifacts explicitly, and the complete
Release CTest suite must pass. Diagnose every observed failure rather than treating
missing artifacts as an acceptable closeout. Eliminate unnecessary normal
parser/builder/package work and notices at their sources, with actual command
stdout/stderr, warnings and failures visible. Preserve metadata formats,
call semantics, test assertions and accepted language boundaries. Keep the
existing user's Release test run intact while inspecting it; build elsewhere
until it ends. Delivery remains local unless publication is explicitly requested.

1. **CP-C-F07-AC-01 (verified):** retain the live/full failure inventory and classify
   missing producers, test-harness errors and any product regressions using raw
   logs and minimal reproducers. Give each distinct repair reviewable evidence.
2. **CP-C-F07-AC-02 (verified):** CLion "All CTest" builds the explicit `qa-prep-all`
   correctness/qualification/stress/measurement preparation closure before CTest.
   Ordinary `all` retains the agreed product-only preparation cost and dedicated
   `rxc-c` closure retains its behavior. Verify IDE settings, generated
   dependencies and failing families after the appropriate QA preparation.
3. **CP-C-F07-AC-03 (verified):** remove the output-hiding launcher, delete routine
   builder notices at their emitting source, retain all command diagnostics,
   and make unchanged package generation a genuine no-op. Parser generation
   emits no unnecessary reports/notices. Verify successful/no-op builds and
   a controlled failure without output suppression.
4. **CP-C-F07-AC-04 (verified):** the full existing Release CTest suite passes with
   no exclusions, weakened assertions or added warning suppression. Validate
   actual repairs with focused Debug checks, then reuse unchanged passing
   evidence. Keep all outstanding criteria open until decisive proof passes.
5. **CP-C-F07-AC-05 (verified):** current guides/receipts reflect visible diagnostics
   and full normal-test closure accurately. Supersede the rejected capture
   policy, retain original failures and final code/test/build hashes, commit
   locally, and preserve SAN-011/012/SAN-QA-018's assigned Release 1 QA closure.

1. **CP-C-F07-STEP-01 (AC-01; complete):** inspect the current CLion run and
   producer graph without stopping or overlapping its Release build tree.
2. **CP-C-F07-STEP-02 (AC-02/03; complete):** repair CLion's QA
   prerequisite configuration, source-level notices and incremental package producers;
   remove the rejected capture wrapper. Keep source/runtime diagnostics visible.
3. **CP-C-F07-STEP-03 (AC-01/04; complete):** prepare/replay failed families in
   Debug, then in Release once the user's run ends. Repair actual remaining
   causes, without promoting artifact/setup failures to unsupported product
   defects or dropping failed tests.
4. **CP-C-F07-STEP-04 (AC-04/05; complete):** freeze final inputs,
   complete the whole Release CTest gate, retain evidence and update status.
   No sanitizer-clean or release-ready claim is authorized by normal CTest.

Checkpoint: the retained CLion run had 152 missing-artifact failures: 136
missing source/bytecode inputs and 16 missing executables (nine were never
launched). Explicit `qa-prep-all` preparation restores every failed test;
Release replay passes 152/152 in 44.74 seconds. Debug affected controls pass
83/83 in 138.01 seconds; the final strengthened RXPP diagnostic case passes
separately. Both final profile builds have no warnings. Unchanged product/QA
builds report no work; provider manifest hashes and unchanged bytes/mtime are
verified. Successful and failing child-command probes preserve stdout/stderr.
Private SHA256/stats/vector fixture import roots exclude unrelated old linked
images without changing import formats or compiler semantics. The ignored local
CLion `All CTest` configuration now names `qa-prep-all`, retaining its Build
before-launch task, test selection and visible-output arguments. The full
3,326-test Release replay passes with zero failures in 917.77 seconds;
AC-04/05 are verified by that proof, the final preparation controls and receipt. The [historical current receipt](https://github.com/adesutherland/CREXX/blob/70e8c300f94c2c8e9698d5789491a8e8f80f3746/docs/qa/clion-qa-preparation-2026-10-10/README.md)
owns final evidence. No hosted or new broad sanitizer run is claimed.

The final preparation audit found two C-first unit executables outside their
directory's QA finalization. Their declared `CREXX_PREP_TARGETS` were correct,
but the compiler directory had not scheduled its own finalizer. Scheduling it
connects both unit targets and the owned parity test to their existing tiers.
Debug `qa-prep-all` restores both absent unit executables and both tests pass;
the generated QA graph then covers every direct first-party executable.
Final Release QA-only preparation and both focused controls pass. All 3,326
runtime commands/properties remain identical after final preparation wiring,
so the full passed runtime evidence remains valid. The generated graphs cover
every direct first-party executable in both profiles. Adrian authorized the
combined development publication and a clean local `develop` checkout on
2026-10-10. Delivery is published to `origin/develop`; earlier local/unpublished
checkpoints above describe their original qualification state. Normal automatic
product/smoke, optimizer-parity and CodeQL checks accompany this publication;
their successful completion is expected, not claimed as observed. No manual
overnight assurance dispatch or redundant local full run is required. Assigned
Release 1 SAN closure is unchanged.

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
  [`../research/pipes/ARCHITECTURE_PROPOSAL.md`](../research/pipes/ARCHITECTURE_PROPOSAL.md)
