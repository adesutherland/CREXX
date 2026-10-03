# CREXX Release 1 Plan

Status: live Release 1 schedule and scope plan, rebaselined 2026-09-04;
core/functional/performance sequencing revised by Adrian on 2026-09-18;
beta 3 publication documentation finalized 2026-09-30.
Beta 3 target: 2026-09-30.
Release 1 cut target: 2027-05-01, for the planned May 2027 London Rexx
Symposium.

This plan describes the intended path from the tagged `v1.0.0-beta.2` release
baseline through core-complete beta 4, functionally complete beta 5,
the beta 6 Performance Beta and Release 1. This is a planning target,
not a release contract. The current product documentation covers
`1.0.0-beta.3` in its final publication form. Versioned tags and their
assets identify completed releases; documentation preparation does not
itself complete the release gates.

Beta 3 moved from the original July foundation target to 2026-09-30 because the
performance programme took longer than planned and produced substantial
product work requiring a trustworthy publication boundary. Release 1 is now
targeted for 2027-05-01 for the planned May 2027 London Rexx
Symposium. The exact 2027 symposium dates are not yet public on the
[`RexxLA symposium listing`](https://rexx.oorexx.org/events/symposium.rsp) and
must be updated when announced.

The beta 3 issue dispositions and working team guidance remain in
[`planning/beta-3/issue-candidates.md`](planning/beta-3/issue-candidates.md).
Many candidates have since become GitHub issues; product ordering and deferral
decisions belong in `docs/ROADMAP.md`, while this file owns the release cadence
and gates.

The 2027-05-01 cut target is fixed for planning and scope is managed by tiering.

## Rebaselined Release Train

Adrian's 18 September direction brings functional completion forward to
31 January and reserves February/March for performance and stabilization.
Each beta must publish a usable product and its honest limitations. The
existing RC1 and Release 1 dates remain unchanged.

| Milestone | Target | Gate |
| --- | --- | --- |
| Beta 3 | 2026-09-30 | Accumulated foundation/performance work published; current release defects and package/demo/policy status reconciled; named hosted gates green on the exact candidate. KeyAccess verdicts accepted, implementations retained and review closed on 2026-09-18. |
| Beta 4 — core complete | 2026-11-30 | Complete the Release 1 core, explicitly including compiled Level C and polymorphism, with documented conformance boundaries and toolchain/runtime tests. Level C means complete compatibility coverage except individually approved "won't implement" exceptions, not a selected first subset. Retain the existing RexxScript stabilization outcome. |
| Beta 5 — platform functionally complete | 2027-01-31 | Complete the Release 1 language/runtime/baseline-library functionality, including baseline Levels G and L, documentation, integration and representative examples. Incorporate core beta feedback. Any selected synchronous Pipes library/executor contract must meet this gate. Platform features and contracts freeze; applications continue separately. |
| Beta 6 — Performance Beta | 2027-03-31 | Measure and improve the functionally complete product through selected, correctness-preserving performance work; retain performance verdicts, regression dispositions and stabilization evidence. Optimization implementation freezes for RC1. Platform feature delivery remains closed; applications continue in parallel. |
| Release 1 RC1 | 2027-04-15 | Exact candidate passes focused and broad correctness, maintained sanitizers, performance qualification, package/install proof, examples, documentation, and known-limit review. Selected parallel applications/tools/showcases reach their release asset cut. |
| Release 1 | 2027-05-01 | Tag, checksums, signed/notarized assets where configured, release notes, and hosted evidence published for the planned May London symposium. |

Before Beta 5, platform feature scope may move between feature-bearing betas through an
explicit gate disposition. A feature missing 31 January must be fixed within
the agreed freeze policy or receive Adrian's explicit scope/date exception;
it does not automatically become Beta 6 work. Moving a feature out of Release 1
or moving the 2027-05-01 cut date requires an explicit maintainer decision.

## Expanded Release 1 Product Target

The longer runway expands Release 1 beyond the old beta 3 foundation boundary:

- a coherent, stable Level B toolchain and library contract;
- completed Release 1 core by Beta 4, including Level C and polymorphism.
  Level C's conformance matrix accounts for the complete compatibility contract,
  allowing only individually approved "won't implement" exceptions;
- baseline Level G and Level L product surfaces by Beta 5, with approved
  capability lists, supported APIs, tests, documentation, examples and packaging;
- a supported small RexxScript standalone/embedded product with an explicit
  sandbox, diagnostics, and examples;
- the synchronous cREXX Pipes reference executor as a clearly labelled
  contribution surface if selected and its contract and tests pass by beta 5;
- a dedicated Beta 6 performance phase, including evaluation of adaptive
  procedure-result memoization and the other recorded candidates, selected by
  current evidence rather than a requirement that every idea ship; and
- release-quality packages, tutorials, examples, known limits, and performance
  evidence for the exact candidate.

Applications, practical tools, LLM consumers and showcases form a parallel
asset track from now through RC1. They consume the platform and demonstrate its
value; they are not all prerequisites for January platform completion.

This is a product target, not approval of undecided language syntax,
ownership, ABI, ISA, or architecture. Those decisions retain their normal
design gates. Unresolved scope is visibly open; it is not permission to replace
the new completion targets with the previous demo/subset targets.

## Completion Contracts And Acceptance

**Vision:** deliver the expanded Release 1 product in dependency order: a
complete agreed core, usable G/L baselines, then performance on that complete
product, followed by qualification. Performance decisions must preserve the
published language/tool boundaries and the functionality completed in January.

The milestone ordering and dates are selected. Detailed capability inventories
remain to be agreed, especially the requested polymorphism surface and minimum
usable G/L products. For Level C, Adrian clarified that completion means the
complete compatibility contract with the right to designate individual features
"won't implement". Use the existing
[Classic compliance reference](../compiler/docs/levelc_compliance_reference.md)
and [BIF reference](../compiler/docs/levelc_classic_bifs.md) to construct the
coverage inventory. Each exception needs Adrian's explicit disposition, its
reason, user-visible behavior/diagnostic and documentation; distinguish it from
"not implemented yet", which remains an open completion blocker. Do not obtain
completion by excluding unfinished features without that decision.
The [Level C compatibility worklist](planning/release-1/levelc-compatibility-worklist.md)
tracks the component inventory, incremental delivery criteria and evidence under
this plan's `R1-AC-01/02` completion contract.

Existing interface dispatch is implemented; interface inheritance and overloads are not part of
the current documented Level B surface. Listing polymorphism here does not
silently select either extension. Similarly, TinyExpr is currently a
generated-output proof, not a delivered lexer/parser generator or a sufficient
baseline merely by being renamed. These are scope-definition tasks, not reasons
to mark the milestones complete or reduce them without Adrian's decision.

- [ ] **R1-AC-01 — scope contracts:** approve a capability/conformance matrix
  for core/Level C/polymorphism and baseline Levels G/L. Level C accounts for
  the complete compatibility surface as implemented, open, or explicitly
  approved "won't implement"; no unspecified remainder is silently deferred.
  Each row identifies its milestone, observable behavior, exclusions, owner and
  qualification evidence.
  Reconcile old should-ship and experimental descriptions against that matrix.
- [ ] **R1-AC-02 — November core:** by 2026-11-30, every required core row is
  implemented and documented and every exclusion has its explicit approved
  disposition. Retain applicable reference-equivalence, optimized/
  no-opt, toolchain and supported-platform evidence. Parser-only or a first
  narrow lowering example does not satisfy Level C completion.
- [ ] **R1-AC-03 — January platform functionality:** by 2027-01-31, all Release 1
  language/runtime/baseline-library feature rows, including baseline Levels G
  and L and any selected Pipes library/executor contract,
  pass their functional/integration gates and have usable documented examples
  and package/install coverage. Platform user-visible contracts freeze at this
  gate; independent applications may continue adding features against them.
- [ ] **R1-AC-04 — March performance:** by 2027-03-31, retain a current
  functionally complete baseline, explicit accept/reject/defer decisions for
  selected performance candidates, and correctness-gated throughput, lifecycle,
  memory and artifact evidence with material regressions resolved or explicitly
  dispositioned. Memoization/quickening success is not assumed or mandatory.
- [ ] **R1-AC-05 — release qualification:** qualify the exact 2027-04-15 RC1
  and 2027-05-01 release candidate through the existing correctness, sanitizer,
  performance, package/install, documentation and release gates. Earlier beta
  performance evidence alone does not qualify a changed release candidate.
- [ ] **R1-AC-06 — parallel application assets:** by RC1, each selected shipped
  application/tool/showcase has an owner, supported/optional/example status,
  dependency and setup documentation, a reproducible useful scenario with
  expected results, and applicable package/install/platform evidence. External
  assets are distinguished from bundled release assets. Platform changes they
  require obey the platform's scope and freeze gates.

1. **R1-STEP-01 — open** (AC-01): define and approve the capability matrices and
   update the component worklists; identify dependencies and owners before
   estimating November/January delivery. Scheduling is not detailed design approval.
2. **R1-STEP-02 — planned** (AC-02): complete and qualify the core for Beta 4.
   G/L design and independent library work can proceed concurrently with this
   phase; they must not wait until December to begin. Applications can likewise
   exercise emerging capabilities and expose gaps early.
3. **R1-STEP-03 — planned** (AC-03): complete G/L and remaining product
   integration, examples and contracts for Beta 5. Define any public annotation,
   effect assertion or metadata contract needed by a selected performance idea
   before this freeze; otherwise use existing contracts or seek an explicit exception.
4. **R1-STEP-04 — planned** (AC-04): use the Beta 5 product and representative
   C/G/L/application workloads to rank the Beta 6 candidates in
   [the performance roadmap](../performance/ROADMAP.md). Implement only selected
   mechanisms, preserving the mandatory first ordinary Release verdict and
   explicit architecture gates. Stop weak candidates early and protect RC1.
5. **R1-STEP-05 — planned** (AC-05): freeze optimization implementation at
   Beta 6, resolve release defects, and qualify RC1/release without introducing
   an unplanned new feature or optimization programme in April.
6. **R1-STEP-06 — parallel, planned** (AC-06): maintain an application/tool/LLM
   showcase asset list, demonstrate useful workflows at successive betas, and
   qualify the selected release assets by RC1. This runs alongside STEP-02–05,
   rather than waiting for their completion. Missing platform capabilities feed
   the relevant core/G/L worklist before January; later additions require an
   explicit platform-scope exception, not an application-track workaround.

## Outstanding preparation criteria

Release qualification is owned by the
[formal beta 3 candidate plan](planning/beta-3/formal-candidate-2026-09-30.md).
The completed September defect-batch execution is available in Git history.
The following still-open preparation criteria retain their stable IDs; removing
an expired weekly schedule does not satisfy them.

- [ ] **WEEK-AC-02 — Beta 3 readiness:** each of the seven milestone issues
  has a concrete acceptance/evidence disposition, and the package matrix names
  the intended assets, signatures, install/smoke paths and known limits.
  CI-F21 and historical Linux QA-C have explicit next actions/owners; any
  departure from their existing criteria is a maintainer decision, not closure
  by wording. Beta 3 notes describe the actual candidate and its supported scope.
- [ ] **WEEK-AC-03 — Beta 4 core map:** construct the full Level C statement,
  expression, BIF, numeric-context, variable-pool/stem, procedure/call, PARSE,
  condition/signal, source/TRACE and host-interface coverage matrix from the
  existing compliance references. Link each implemented row to evidence;
  record missing rows and proposed exceptions separately. Define the requested
  polymorphism delta beyond the existing interface dispatch before designing
  inheritance, overloads or generics. This serves R1-AC-01/02.
- [ ] **WEEK-AC-04 — first Beta 4 increment:** select and, after its normal
  implementation-plan gate, complete one bounded Level C control-flow slice.
  Bounded IF/THEN/ELSE is implemented; simple DO remains the next form.
  The AST remap framework has relevant builders. Retain Classic
  reference equivalence, nesting/negative controls, optimized/no-opt and
  toolchain execution evidence. Do not claim this slice completes Level C.
- [ ] **WEEK-AC-05 — useful parallel asset:** select one existing-capability
  LLM/tool workflow and retain a reproducible setup/demo with expected results
  and optional-provider requirements. Reuse the current common LLM API where
  applicable; do not make a new application or model download a Beta 3 release
  blocker. This is an early R1-AC-06 contribution, not its whole completion.

1. **WEEK-STEP-01 — ongoing where unmet** (AC-02): reconcile beta readiness,
   packaging/signing and exact-candidate evidence in the formal candidate plan.
2. **WEEK-STEP-02 — open where unmet** (AC-02/03): finish required product
   definitions and the complete Level C gap/exception and polymorphism matrices.
3. **WEEK-STEP-03 — open** (AC-03/04): select the bounded control-flow increment
   after the normal implementation-plan gate; use develop after the beta 3 cut.
4. **WEEK-STEP-04 — open** (AC-05): select and qualify the useful existing-capability
   application workflow, feeding concrete defects back into the appropriate track.
5. **WEEK-STEP-05 — ongoing where unmet** (AC-02): reconcile the named candidate,
   remaining blockers and final qualification actions with the formal plan.

## Release Principle

Release 1 should ship the agreed complete core, baseline Levels G and L, and
the supported small RexxScript product. A baseline is a usable documented
contract with tested limits; it does not claim every future capability of a
level is stable. The exact baseline must be selected, not inferred from the
current implementation or reduced to a demo to meet the date.

The Release 1 final two-week sprint from RC1 is reserved for QA,
documentation, usability, examples, packaging, and performance validation.
Platform user-facing feature work closes at beta 5 on 2027-01-31. Applications,
tools and showcases continue on their parallel track through RC1. Beta 6 closes the
performance implementation phase on 2027-03-31; April is qualification and
release-defect repair for the platform, with final application integration and
asset qualification through RC1, not another platform optimization/feature phase.

## Must Ship

Must-ship items are part of the Release 1 contract or release process.

1. Beta 3 branch baseline

   Keep `v1.0.0-beta.2` release notes as the historical beta 2 baseline, point
   current product documentation at `v1.0.0-beta.3`, and keep the beta 3
   planning note aligned with this timetable. README, install/security guidance
   and release notes are final publication copy; release execution status belongs
   in the candidate handoff until the tag and assets exist.

2. Release 1 governance

   Open the GitHub discussion from the approved version of this plan. Create
   issues for must-ship and should-ship items only after the discussion is
   accepted. Use labels for `must`, `should`, `experimental`, and `post-r1`.

3. Level B lockdown

   Reconcile Level B syntax and stable library surface against the original
   2026-07-03 design-lock goal, recording completed work and explicit exceptions
   for the current beta. The final user-facing feature freeze is 2027-01-31
   under the rebaselined train. Complete
   documentation and tests for UTF/binary, references, arrays, collections,
   interfaces/classes, ADDRESS, TRACE, imports, linking, and packaging.

4. Level B/G split

   Publish a short design note explaining what Level B owns and what Level G
   owns. The Release 1 wording should say: Level B is stable; Level G has an
   implemented initial task/parallel surface and library overlay, not a
   full stable language contract.

5. Core UTF contract

   Keep `.string` as valid UTF-8 and `.binary` as arbitrary bytes. Complete the
   Release 1 docs and tests around boundaries. Deprecate compiler/plugin-owned
   Unicode semantics in favor of VM codepoint validity plus Level G Unicode
   libraries.

6. Plugin policy

   Classify plugin directories and update CMake/package defaults to make the
   release surface obvious. Core algorithms should move to Rexx libraries where
   practical; tight runtime integration should move to VM/RXAS instructions;
   plugins should mainly represent external integration, OS/application
   boundaries, or experimental/edge capabilities. Include native-backed Rexx
   adapter modules and tests in this triage, not just C plugin directories.
   For beta 3, `Id`, `KeyDB`, and `Os` are restored as plugin-backed classlib
   adapters in the separate `classlib_native.rxbin` image with focused tests
   and explicit plugin dependencies. They are not part of core
   `classlib.rxbin`, so RexxScript and other pure-classlib consumers do not
   inherit unrelated native plugin requirements. Any later change to make them
   core, optional, deprecated, experimental, or removed should be explicit
   rather than caused by refactoring fallout.

7. Runtime lookup and late loading

   The beta 3 baseline has explicit-file late-load/relink coverage and
   sorted interface method/factory registries. Exact method dispatch uses a
   binary search; factory selection binary-searches the interface/member bucket
   and scans only matching providers. Runtime loading rebuilds both indexes.
   Cross-platform validation remains part of Release 1 QA.

8. Large constant foundation

   Immutable binary constants, typed binary-memory access, and packed jump
   tables provide the Release 1 minimum needed by current demos and compiler
   lowering. Dedicated immutable integer/string arrays, typed records, and
   broader source sugar are explicitly deferred; they can be represented in
   binary memory until a later design justifies separate forms.

   The scalar `.int` contract is signed 64-bit on every supported Release 1
   desktop build. The shared ABI typedef, parser boundaries, checked arithmetic,
   compiler folding, RXAS literal handling, and VM limits are covered together;
   legacy 32-bit-host validation remains separate platform work.

9. Performance baseline and targeted improvements

   Retain the completed governed baseline and close the exact beta 3 candidate
   gates. Ship only already accepted or defect-required changes in beta 3; no
   new broad optimization programme belongs in that cut. Beta 6 is the selected
   performance phase: establish the complete Beta 5 baseline, select bounded
   candidates from the performance roadmap, and retain the first Release verdict
   for each production edit. Refresh the final scorecard against RC1 after the
   2027-03-31 optimization freeze. Public feature/contracts remain frozen from January.

10. Demos, tutorials, and docs

    Ship curated Level B, Level G, Level L, and Level C demos with expected
    commands and outputs. Complete final docs in the last sprint.

11. Packaging status discipline

    For beta 3, package only formats whose build, signing, upload, and smoke
    checks are reliable by the 2026-09-30 beta 3 cut. Keep portable ZIPs as
    the fallback for every platform and document any installer gaps clearly.

12. Complete Level C and polymorphism by Beta 4

    The November core milestone requires complete Level C compatibility
    coverage, except individually approved "won't implement" features, and the
    agreed polymorphism contract. Existing lowering slices are the starting
    point. The conformance matrix and R1-AC-01/02 own completion; unfinished work
    remains open until implemented or explicitly dispositioned by Adrian.

13. Deliver baseline Levels G and L by Beta 5

    The January platform milestone requires usable supported G/L baselines,
    with approved capability lists, APIs, lifecycle/ownership contracts,
    conformance, integration, documentation, examples and packaging. Existing
    provider/concurrency/HTTP/LLM/UI/library work and TinyExpr inform those
    baselines but do not automatically define or complete them. R1-AC-01/03
    supersede the original should-ship/demonstration-only classifications.

14. Qualify the parallel application asset track by RC1

    Applications, LLM consumers, working tools and showcases may evolve through
    RC1 against the frozen platform contract. Keep selected bundled assets and
    independently distributed applications distinct, with useful scenarios and
    reproducible setup/package evidence under R1-AC-06.

## Should Ship

Should-ship items are important but have explicit fallback paths.

1. Perfect-hash `select`

   The RXAS packed-table surface, linear/open-hash/ACPH algorithms, measured
   `auto` policy, VM execution, disassembly round trip, corruption handling,
   and conservative `rxc` integer/string/binary lowering are implemented on
   the beta 3 baseline. Remaining Release 1 work is cross-platform QA
   and documentation/release review; arbitrary RXAS branch-ladder recognition
   is explicitly post-Release 1 CFG/dataflow work.

2. Level L lexer/parser demo

   Use large constants and optimized lookup if available. The first
   `rxfnsl.tinyexpr` slice is a generated-output proof: it is hand-written in
   the shape a future lexer/parser generator might emit, using packed binary
   tables, exposed token/layout constants, and token records. The real
   generator is not yet delivered; this slice should guide whether to port re2c
   or change a generator backend to emit cRexx/RXAS directly. It informs the
   January Level L baseline definition but does not itself discharge that
   product commitment. Select reusable runtime/generator capabilities explicitly;
   neither a real generator nor a demo-only baseline is silently assumed.

3. Plugin/demo cleanup

   Clean enough that users can tell core from optional. Fallback: docs and
   CMake options clarify status even if all source directories are not moved.

4. Windows installer user experience for beta 3

   Add a signed NSIS `setup.exe` from the signed Windows payload if the local
   signing flow can build, sign, upload, and verify it reliably before the beta
   3 tag. Fallback: keep the signed Windows ZIP as the supported Windows asset
   and document manual PATH setup.

5. Linux package hardening

   Keep the `.deb` path, but add install/uninstall smoke testing and dependency
   metadata review before treating it as more than a prototype. Fallback: ZIP
   remains the portable Linux asset and `.deb` remains dev-snapshot-only.

## Initial Or Experimental Only

Capabilities outside the approved required core/G/L matrices need not block
Release 1. The initial/experimental labels below record current status; they
cannot waive a capability subsequently selected for the January baseline.
Initial identifies the first bounded concurrency surface without claiming full
future compatibility; experimental describes unrelated research/prototypes.

- GPU VM plugin proof of concept.
- Initial structured concurrency until portable conformance, package proof
  and publication approval are complete.
- Level G rich Unicode beyond the approved first slice.
- Broad Level L syntax sugar.

Level C is not generally deferred as experimental. Any feature exclusion must
be an individually approved "won't implement" entry under R1-AC-01/02;
otherwise missing compatibility remains a November completion blocker.

## Team Plan

The owner below is accountable for driving the work and keeping the issue
honest. Ownership does not mean that person must implement every line.

Adrian:

- approve Level B/G/C/L language decisions;
- own VM/compiler architecture for runtime lookup, large constants, select
  optimization, and Level C lowering shape;
- review plugin policy where it affects language/runtime boundaries;
- use AI heavily for implementation slices, regression scaffolding, and docs
  drafts, while keeping final language decisions manual.

Peter:

- own PARSE-related compatibility and examples;
- own RexxScript demos and integration where relevant, while keeping
  RexxScript distinct from compiled Level C;
- help inventory plugins and classify plugin/demos;
- implement or update plugin demos and non-core plugin docs;
- contribute Level C and Level G demos where domain knowledge matters.

Rene:

- own performance baseline, benchmark runs, and optimizer measurement;
- own documentation completeness and release-note quality;
- own manual QA checklist, example validation, and usability feedback;
- help stabilize library APIs and examples;
- drive final sprint release-readiness reporting.

## Dependency Map

Decisions needed before implementation:

- Level B/G boundary and what counts as Level B stable.
- Plugin category policy and default build policy.
- UTF ownership: VM codepoint baseline, Level G rich Unicode, and retirement of
  compiler-owned Unicode plugin semantics.
- The complete Release 1 core Level C compatibility and polymorphism matrix;
  execution is required, and parser-only/first-proof fallback is superseded.
- Baseline Level G and Level L capabilities needed for January functional
  completion, including supported libraries/tooling, APIs and qualification.
- RexxScript beta 3 scope as an interpreted strings-only modern Rexx surface,
  separate from compiled Level C.
- Shared BIF strategy for RexxScript string-first use and Level C Classic
  value/pool use.
- VM multithreading and GPU scope: stable, experimental, or design-only.
- Large constant data representation and any source/RXAS syntax.
- `select` perfect-hash semantics, fallback behaviour, and supported types.

Technical dependencies:

- Large constant structures depend on RXBIN/RXAS representation, loader support,
  rxdas round-trip, compiler emission, and tests.
- Perfect-hash `select` depends on static-case detection, constant table
  emission, VM/RXAS lookup support or branch-sequence generation, and fallback.
- Level L demos depend on large constant structures, packed binary lookup
  ergonomics, readable generated binary literal conventions, and a proved
  generated-output shape before real lexer/parser generator APIs are designed.
  Source-module-local exposed constants are Release 1; cross-module constants,
  wildcard expose forms such as `TINY_TOK_*`, and binary memory structs are
  Release 2 candidates.
- Class and interface constants are a Release 2 design item. The starting point
  should be private constants owned by the class/interface body, matching the
  current member privacy standard; a controlled public constant view should be
  investigated as part of that work.
- Level C execution depends on canonical AST lowering, variable-pool model,
  PARSE helpers, command/ADDRESS lowering, source provenance, and runtime tests.
- RexxScript integration depends on its interpreter/evaluator boundary, shared
  string-first BIF entry points, source provenance, and status/error reporting.
- Shared Rexx BIF work depends on choosing the first BIF slice and separating
  string-first RexxScript behavior from Classic value/pool behavior.
- Level G Unicode depends on vendoring/build/licensing decision for `utf8proc`
  or a Rexx-first alternative.
- Plugin split depends on inventory, CMake defaults, packaging impact, and docs.
- Performance work depends on baseline benchmarks and linked/non-linked test
  coverage.

Documentation dependencies:

- Beta 2 release notes carry the historical beta 1 to beta 2 delta; keep them
  aligned with the actual beta 2 tag assets.
- Beta 3 release notes carry the final beta's scope and limitations; keep README,
  `docs/releases`, security policy, examples, install docs, and language
  reference aligned on `1.0.0-beta.3`. Retain prior beta notes as history and
  keep pending release operations in the candidate handoff.
- `docs/ai-context/CREXX_LIBS.md` should describe `rxfnsc` as the Level
  C/RexxScript runtime foundation now that the library directory exists.
