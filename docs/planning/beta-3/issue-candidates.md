# CREXX Beta 3 Issue Candidates

Status: working planning scratchpad for beta 3. This file captures candidate
GitHub issues before they are created. It is not a release contract.

Target milestone: `1.0.0-beta.3`.
Current baseline: `develop` is beta 3 WIP after the tagged `v1.0.0-beta.2`
release.

## How To Use This File

- Keep GitHub issues as the action tracker.
- Keep durable design/progress notes in the repository when a topic has
  architecture, examples, source-mapping decisions, test evidence, or release
  guidance that should survive beyond issue comments.
- Suggested note path for issue-specific notes:
  `docs/planning/beta-3/notes/<issue-number-or-slug>.md`.
- Do not create all subissues blindly. Create parent issues first, then add
  subissues only where the work has independent ownership or acceptance
  criteria.

## Issue Creation Waves

Create these immediately with the beta 3 milestone:

- Parent trackers: B3-DL-00, B3-FT-00, B3-PKG-00, B3-DOC-00.
- All design-lock issues: B3-DL-01 through B3-DL-12.
- Starter foundation/inventory issues that can proceed before design lock:
  B3-F-06, B3-F-13, B3-F-14, and B3-F-15.
- High-risk early prototype issue: B3-F-16. The Level C canonical-AST lowering
  proof should start early as investigation/prototype work, even if the exact
  supported source slice is finalized by B3-DL-11.

Create the remaining foundation implementation issues after their design-lock
decisions are approved or narrowed.

## Created First-Wave Issues

Created on 2026-06-17 under GitHub milestone
[`1.0.0-beta.3`](https://github.com/adesutherland/CREXX/milestone/1).

| Candidate id | GitHub issue | Owner | Role |
| --- | --- | --- | --- |
| B3-DL-00 | [#606](https://github.com/adesutherland/CREXX/issues/606) | Adrian | Parent tracker |
| B3-FT-00 | [#607](https://github.com/adesutherland/CREXX/issues/607) | Adrian | Parent tracker |
| B3-PKG-00 | [#608](https://github.com/adesutherland/CREXX/issues/608) | Rene | Parent tracker |
| B3-DOC-00 | [#609](https://github.com/adesutherland/CREXX/issues/609) | Rene | Parent tracker |
| B3-DL-01 | [#610](https://github.com/adesutherland/CREXX/issues/610) | Adrian | Design lock |
| B3-DL-02 | [#611](https://github.com/adesutherland/CREXX/issues/611) | Adrian | Design lock |
| B3-DL-03 | [#612](https://github.com/adesutherland/CREXX/issues/612) | Adrian | Design lock |
| B3-DL-04 | [#613](https://github.com/adesutherland/CREXX/issues/613) | Adrian | Design lock |
| B3-DL-05 | [#614](https://github.com/adesutherland/CREXX/issues/614) | Adrian | Design lock |
| B3-DL-06 | [#615](https://github.com/adesutherland/CREXX/issues/615) | Rene | Design lock |
| B3-DL-07 | [#616](https://github.com/adesutherland/CREXX/issues/616) | Adrian | Design lock |
| B3-DL-08 | [#617](https://github.com/adesutherland/CREXX/issues/617) | Peter | Design lock |
| B3-DL-09 | [#618](https://github.com/adesutherland/CREXX/issues/618) | Adrian | Design lock |
| B3-DL-10 | [#619](https://github.com/adesutherland/CREXX/issues/619) | Adrian | Design lock |
| B3-DL-11 | [#620](https://github.com/adesutherland/CREXX/issues/620) | Adrian | Design lock, high risk |
| B3-DL-12 | [#621](https://github.com/adesutherland/CREXX/issues/621) | Adrian | Design lock |
| B3-F-06 | [#622](https://github.com/adesutherland/CREXX/issues/622) | Peter | Foundation starter |
| B3-F-13 | [#623](https://github.com/adesutherland/CREXX/issues/623) | Rene | Foundation starter |
| B3-F-14 | [#624](https://github.com/adesutherland/CREXX/issues/624) | Rene | Foundation starter |
| B3-F-15 | [#625](https://github.com/adesutherland/CREXX/issues/625) | Rene | Foundation starter |
| B3-F-16 | [#626](https://github.com/adesutherland/CREXX/issues/626) | Adrian | Foundation starter, high risk |

## Team Guidance To Capture

These principles should be turned into issue text, acceptance criteria, or
design notes as the beta 3 issues are created.

1. The preprocessor is a first-class part of the toolchain.

   Preprocessing should not be treated as text glue hidden before compilation.
   It needs explicit ownership, tests, diagnostics, source provenance, and a
   documented relationship to source files, generated source, and downstream
   compiler stages.

2. Source provenance is a release concern.

   Errors, warnings, traces, generated code, preprocessed code, and RexxScript
   integration should preserve enough mapping information to report the right
   user source line. If exact mapping is not yet possible everywhere, beta 3
   should document the known gaps and choose the next foundation slice.

3. RexxScript integration is a distinct development line, not Level C.

   RexxScript should be treated as a modern, interpreted-only Rexx-family
   surface that primarily uses strings. It may later migrate toward a light
   Classic Rexx subset, and it should share BIF implementations with Classic
   Rexx / Level C where that is clean, but it is not the Level C compiler path.

4. Level C is compiled Classic Rexx through canonical AST lowering.

   Level C should compile Classic Rexx by transforming the Level C parse/AST
   shape into the canonical compiler AST shape used by the normal validation,
   optimization, and emission pipeline. This is separate from RexxScript.

5. Classic Rexx support needs a deliberate value model.

   Classic Rexx variables should be represented by a dedicated value class that
   owns the canonical string value and can cache derived forms such as integer,
   decimal, boolean, or binary where appropriate. The cache rules must preserve
   Classic Rexx semantics rather than leaking Level B type assumptions.

6. Classic Rexx variable pools are a first-class runtime abstraction.

   A variable-pool class should model Classic Rexx variable lookup, assignment,
   stems, host-visible variable access, and any SAA/ADDRESS integration needs.
   BIFs and Level C work should use this abstraction instead of ad hoc maps.

7. Rexx BIFs should be curated for shared use where possible.

   The beta 3 plan should identify a clean set of BIFs that can serve
   RexxScript and Classic Rexx / Level C where possible. The implementation
   should avoid bespoke conversions per BIF: RexxScript may call string-first
   BIF entry points, while Level C uses the Classic Rexx value class and
   variable pool where Classic semantics require them.

8. Level B classes should feel designed for Level B.

   The Level B class library should be reviewed for Classic Rexx oddities,
   historical compatibility leftovers, confusing names, or APIs that are only
   there because of older implementation paths. Keep what supports Level B
   clearly; move Classic-oriented behavior toward the Level C/value-pool layer.

9. Plugins and exits need curation, not just cleanup.

   Classify plugins, compiler exits, host exits, demos, and legacy integrations
   as core, integration, optional, deprecated, experimental, or remove. The
   default build and package contents should make that classification visible.

## Candidate Parent Issues

| Candidate id | Gate | Title | Owner | Type | Acceptance signal | Fallback |
| --- | --- | --- | --- | --- | --- | --- |
| B3-DL-00 | Design lock | Beta 3 design lock tracker | Adrian | Parent | All design-lock decisions below are linked, assigned, and have explicit approve/defer outcomes by 2026-07-03. | Keep unresolved items out of beta 3 scope and record them as post-beta-3. |
| B3-FT-00 | Foundation | Beta 3 foundation tracker | Adrian | Parent | Foundation issues below have land/defer decisions, tests or design notes, and package/release-note status by 2026-07-31. | Defer incomplete foundation work with explicit known limitations. |
| B3-PKG-00 | Foundation | Beta 3 package shape tracker | Rene | Parent | ZIP fallback, macOS pkg, Linux deb hardening, and Windows NSIS status are known before beta 3 publication. | Ship portable ZIPs plus only proven installers. |
| B3-DOC-00 | Foundation | Beta 3 docs, demos, and release-note tracker | Rene | Parent | Demos, tutorials, known limitations, and beta 3 release notes match the shipped tag. | Ship fewer curated demos with clear limitations. |

## Candidate Design-Lock Issues

| Candidate id | Title | Owner | Labels | Acceptance signal | Notes |
| --- | --- | --- | --- | --- | --- |
| B3-DL-01 | Define the preprocessor as a first-class toolchain concern | Adrian | `beta3`, `design-lock`, `compiler`, `preprocessor` | Design note states ownership, supported inputs/outputs, source-provenance expectations, test strategy, and relationship to compiler stages. | Should include whether preprocessing is a separate stage, an rxc mode, or both. |
| B3-DL-02 | Define source mapping and diagnostics provenance contract | Adrian | `beta3`, `design-lock`, `compiler`, `diagnostics` | Errors and warnings have a documented expectation for original source line mapping through preprocessing and generated code. | Cross-cuts preprocessor, RexxScript, parser mode, TRACE, and debug metadata. |
| B3-DL-03 | Define RexxScript integration strategy for beta 3 | Adrian | `beta3`, `design-lock`, `rexxscript` | Design note identifies beta 3 RexxScript goal, integration points, out-of-scope items, and tests/demos. | RexxScript is not Level C; keep it separate from compiled Classic Rexx. |
| B3-DL-04 | Define Classic Rexx value class semantics | Adrian | `beta3`, `design-lock`, `level-c`, `runtime` | Design note specifies canonical string ownership, cached numeric/binary forms, invalidation rules, comparison/conversion semantics, and memory ownership. | Foundation for Classic BIFs and variable pool. |
| B3-DL-05 | Define Classic Rexx variable pool model | Adrian | `beta3`, `design-lock`, `level-c`, `runtime`, `saa` | Design note specifies lookup, assignment, stems, exposure, host-variable access, and SAA/ADDRESS interaction. | Should avoid coupling Level B class APIs to Classic variable behavior. |
| B3-DL-06 | Define shared Rexx BIF surface for RexxScript and Level C | Rene | `beta3`, `design-lock`, `rexxscript`, `level-c`, `bifs` | Candidate BIF list is grouped by must/should/defer and identifies whether each BIF can share string-first and Classic value/pool entry points. | First slice should be useful for RexxScript demos and later Level C lowering. |
| B3-DL-07 | Define Level B class library design principles | Adrian | `beta3`, `design-lock`, `level-b`, `library` | Principles identify what belongs in Level B classes, what should move to Classic/Level C support, and what compatibility oddities should be removed or deprecated. | Turns "clean class library" into reviewable guidance. |
| B3-DL-08 | Define plugin, exit, and integration classification policy | Peter | `beta3`, `design-lock`, `plugins`, `exits` | Classification categories are approved and mapped to default build/package policy. | Covers plugins, compiler exits, host exits, demos, and legacy integrations. |
| B3-DL-09 | Define Level B versus Level G boundary | Adrian | `beta3`, `design-lock`, `level-b`, `level-g` | Short design note states stable Level B contract and first Level G overlay scope. | Already in Release 1 plan; keep linked here. |
| B3-DL-10 | Define UTF/text ownership | Adrian | `beta3`, `design-lock`, `unicode`, `level-b`, `level-g` | `.string`, `.binary`, VM codepoint baseline, and Level G Unicode ownership are agreed. | Keep Classic Rexx value semantics separate from Level B `.string`. |
| B3-DL-11 | Define Level C canonical AST lowering milestone | Adrian | `beta3`, `design-lock`, `level-c`, `compiler`, `high-risk` | Decide whether beta 3 includes a small compiled Classic Rexx proof by transforming Level C AST into the canonical compiler AST shape. | RexxScript is separate and should not be used as the Level C lowering proof. |
| B3-DL-12 | Decide GPU/threading Release 1 status | Adrian | `beta3`, `design-lock`, `vm`, `experimental` | Stable, experimental, design-only, or post-R1 status is explicit. | Prevents accidental release commitment. |

## Candidate Foundation Issues

| Candidate id | Title | Owner | Labels | Acceptance signal | Fallback |
| --- | --- | --- | --- | --- | --- |
| B3-F-01 | Add source-provenance foundation for preprocessor output | Adrian | `beta3`, `foundation`, `compiler`, `preprocessor`, `diagnostics` | Minimal source map or provenance metadata exists with focused tests for mapped diagnostics. | Document unmapped diagnostics and keep preprocessing opt-in where needed. |
| B3-F-02 | Build RexxScript integration beta 3 slice | Adrian | `beta3`, `foundation`, `rexxscript` | Agreed demo/test shows the selected interpreted RexxScript path and reports useful status/errors. | Ship design/demo only and move deeper integration post-beta-3. |
| B3-F-03 | Implement Classic Rexx value class prototype | Adrian | `beta3`, `foundation`, `level-c`, `runtime` | Value class handles string canonical form plus selected cached conversions with tests. | Keep design note and defer BIF migration. |
| B3-F-04 | Implement Classic Rexx variable pool prototype | Adrian | `beta3`, `foundation`, `level-c`, `runtime`, `saa` | Variable pool supports the agreed first lookup/assignment/stem behavior with tests. | Keep current ad hoc paths and mark Level C integration blocked. |
| B3-F-05 | Implement first shared Rexx BIF slice | Rene | `beta3`, `foundation`, `rexxscript`, `level-c`, `bifs` | First curated BIF set has tests for RexxScript string-first use and a clear path to Classic value/pool use. | Publish BIF list and defer implementation. |
| B3-F-06 | Inventory and classify plugins and exits | Peter | `beta3`, `foundation`, `plugins`, `exits` | Table lists each plugin/exit/demo as core, integration, optional, deprecated, experimental, or remove. | Docs-only classification if CMake/package changes are not ready. |
| B3-F-07 | Align default build/package set with plugin classification | Peter | `beta3`, `foundation`, `plugins`, `packaging` | CMake/package defaults match approved classification and release docs. | Keep defaults but document optional/deprecated status clearly. |
| B3-F-08 | Review and curate Level B class library APIs | Adrian | `beta3`, `foundation`, `level-b`, `library` | Review identifies keep/deprecate/remove/move decisions for Level B classes and tests. | Defer breaking changes until after Release 1 and document oddities. |
| B3-F-09 | Harden late load, relink, and class/interface rebinding | Adrian | `beta3`, `foundation`, `vm`, `classes` | Tests cover current expected behavior for late load and rebinding. | Document unsupported cases and keep hot-path changes gated. |
| B3-F-10 | Replace hot-path linear lookup with indexed lookup | Adrian | `beta3`, `foundation`, `vm`, `performance` | Method/factory lookup uses indexed search and preserves late-load correctness. | Keep current lookup and record performance risk. |
| B3-F-11 | Add large immutable constant foundation | Adrian | `beta3`, `foundation`, `rxas`, `rxbin`, `vm` | RXAS/RXBIN/VM have tested minimum representation for approved constant structures. | Defer source sugar; keep only documented internal support. |
| B3-F-12 | Decide perfect-hash `select` implementation path | Adrian | `beta3`, `foundation`, `compiler`, `performance` | Static select optimization is either implemented with tests or explicitly deferred. | Keep current nested IF lowering. |
| B3-F-13 | Establish beta 3 performance baseline | Rene | `beta3`, `foundation`, `performance`, `tests` | Baseline command set and recorded results exist before optimizer claims. | Ship without optimizer claims. |
| B3-F-14 | Validate beta 3 package shape | Rene | `beta3`, `foundation`, `packaging`, `qa` | Asset list, signing status, install smoke tests, and known limitations are verified against tag assets. | Publish only proven package formats. |
| B3-F-15 | Curate beta 3 demos and tutorials | Rene | `beta3`, `foundation`, `docs`, `demos` | Examples and tutorials match beta 3 capabilities and expected output. | Ship fewer demos with clear boundaries. |
| B3-F-16 | Prototype Level C canonical AST lowering proof | Adrian | `beta3`, `foundation`, `level-c`, `compiler`, `high-risk` | A small Classic Rexx source slice transforms from Level C AST into canonical compiler AST and reaches the agreed downstream phase. | Keep Level C as parser/highlighter plus published lowering plan. |

## Suggested GitHub Issue Body Shape

Use this shape when converting candidates into GitHub issues:

```markdown
## Purpose

Why this issue exists and what Release 1 or beta 3 risk it reduces.

## Guidance

How to approach the work, including boundaries and things not to do.

## Acceptance

- Concrete observable result.
- Tests, docs, or note updates expected.
- Fallback/defer decision if the work does not land.

## Notes

Link to the durable repo note, related issues, and important source paths.
```

## Parent Issue Guidance

### B3-DL-00: Beta 3 Design Lock Tracker

Purpose:
- Coordinate the decisions due by 2026-07-03. This issue should not become an
  implementation bucket. Its job is to make sure every design-lock question has
  an explicit owner, decision, and follow-on issue or deferral.

Guidance:
- Use a task list of linked design-lock issues. Each linked issue should end in
  one of three states: approved for beta 3, approved but deferred, or rejected.
  When a decision changes scope, update `docs/releases/v1.0.0-beta.3.md` and
  the relevant planning note.

Acceptance:
- All design-lock child issues have a recorded outcome, the beta 3 release note
  matches those outcomes, and no unresolved language or architecture decision is
  silently left in the beta 3 path.

### B3-FT-00: Beta 3 Foundation Tracker

Purpose:
- Coordinate implementation and risk-retirement work due by 2026-07-31. This
  tracker exists so foundation work can be consciously landed, narrowed, or
  deferred before feature-complete pressure starts.

Guidance:
- Track only work that enables Release 1 quality or reduces beta 3 risk. Avoid
  mixing exploratory future features into this tracker. Every child issue should
  include a test, a design/progress note, or a release-note limitation.

Acceptance:
- Foundation child issues have land/defer decisions, and the remaining beta 3
  known limitations are clear enough for release notes.

### B3-PKG-00: Beta 3 Package Shape Tracker

Purpose:
- Give Rene one place to coordinate the package matrix, signing status, smoke
  tests, and fallback decisions before beta 3 publication.

Guidance:
- Treat portable ZIPs as the cross-platform fallback. Promote installer formats
  only when build, signing, upload, and basic install/remove validation are
  repeatable. Record exact asset names and platform caveats. Do not describe an
  installer as supported because it builds once locally.

Acceptance:
- The issue has a checked package matrix for Linux, Windows, macOS arm64, and
  macOS x86_64; each package type has a status of shipped, prototype,
  deferred, or not applicable; release notes and install docs match that
  status.

### B3-DOC-00: Beta 3 Docs, Demos, And Release-Note Tracker

Purpose:
- Give Rene one place to keep the public story coherent: demos, tutorials,
  release notes, known limitations, and examples should all describe the same
  beta 3 product.

Guidance:
- Prefer fewer demos that run reliably over broad demo coverage. Every curated
  demo should have a command, expected output, and a note about which language
  level or feature it is demonstrating. Remove or clearly mark examples that
  depend on deferred features.

Acceptance:
- The beta 3 release note, README, docs index, tutorials, and examples agree on
  what is supported, experimental, deferred, or merely planned.

## Design-Lock Issue Guidance

### B3-DL-01: Define The Preprocessor As A First-Class Toolchain Concern

Purpose:
- Decide what the preprocessor is in the CREXX architecture instead of treating
  it as invisible text rewriting. This matters because diagnostics, source
  provenance, RexxScript integration, and generated code all depend on it.

Guidance:
- Identify the current preprocessor entry points and outputs. Decide whether
  beta 3 treats preprocessing as a separate tool, an `rxc` mode, an internal
  compiler phase, or some combination. Describe how preprocessed output should
  be inspected by users and tests. Do not design new macro syntax unless it is
  needed to settle architecture.

Acceptance:
- A design note states the supported preprocessor role for beta 3, how it is
  invoked or observed, which source paths it owns, and what tests are expected.

### B3-DL-02: Define Source Mapping And Diagnostics Provenance Contract

Purpose:
- Make wrong-line errors a first-class release risk. Users should be able to
  trust diagnostics even when code passes through preprocessing, generated
  source, RexxScript integration, parser mode, TRACE, or debug metadata.

Guidance:
- Start from the current compiler diagnostic paths and identify where original
  source locations are lost. Define the minimum beta 3 contract first, then
  list known gaps. Keep the contract practical: exact source mapping for every
  generated construct may be a later goal, but misleading diagnostics should be
  named and tested.

Acceptance:
- A note documents the provenance model, the metadata shape or source-map
  approach to use, and at least one focused test scenario that should pass in
  beta 3 foundation work.

### B3-DL-03: Define RexxScript Integration Strategy For Beta 3

Purpose:
- Make RexxScript a planned integration path rather than an incidental set of
  demos. RexxScript is not Level C. For beta 3, treat it as a modern,
  interpreted-only Rexx-family surface that primarily uses strings, with a
  possible future migration toward a light Classic Rexx subset.

Guidance:
- Inventory current RexxScript evaluator and demo status, then choose the next
  beta 3 slice. Keep the first slice small enough to test and document. Link it
  to source provenance and the shared BIF strategy where useful, but do not
  route RexxScript through the compiled Level C plan.

Acceptance:
- A design note names the beta 3 RexxScript goal, explicit non-goals, required
  source mapping behavior, shared-BIF expectations, and one candidate demo or
  regression test.

### B3-DL-04: Define Classic Rexx Value Class Semantics

Purpose:
- Establish the value object that Classic Rexx and Level C work can build on:
  canonical string value plus cached derived forms where that improves
  correctness or performance.

Guidance:
- Preserve Classic Rexx semantics first. Decide ownership, mutability,
  invalidation, numeric cache precision, failed conversion behavior, binary
  handling, comparison hooks, and memory lifetime. Keep this separate from
  Level B `.string` and `.binary` APIs unless there is a deliberate bridge.

Acceptance:
- A design note is clear enough for implementation of a prototype and for Rene
  to use when curating Classic C BIF behavior.

### B3-DL-05: Define Classic Rexx Variable Pool Model

Purpose:
- Define the runtime abstraction for Classic Rexx variables, stems, exposure,
  host-variable access, and SAA/ADDRESS interactions.

Guidance:
- Start with lookup and assignment semantics, then add stems and host-visible
  access. Note where existing CREXXSAA or ADDRESS mechanisms already solve part
  of the problem. Avoid letting Level B class design inherit Classic variable
  pool oddities.

Acceptance:
- A design note defines the pool API responsibilities, what is in the beta 3
  prototype, and what remains post-beta-3.

### B3-DL-06: Define Shared Rexx BIF Surface For RexxScript And Level C

Purpose:
- Give Rene a clear target for BIF work that can serve RexxScript and compiled
  Classic Rexx / Level C without scattering one-off conversions and partially
  compatible helpers.

Guidance:
- Build an inventory from existing docs, tests, C helpers, and user-visible
  needs. Group BIFs as must, should, defer, or reject for beta 3. For each
  must/should BIF, note whether it can be string-first for RexxScript, whether
  it needs the Classic value class, whether it needs the variable pool, and
  whether it has host/parser/language dependencies. Do not start by
  implementing every familiar Rexx BIF; choose the first slice that supports
  RexxScript now and Level C lowering later.

Acceptance:
- The issue produces a curated BIF table with rationale, dependencies, and
  expected tests. The table is linked from any implementation issues.

### B3-DL-07: Define Level B Class Library Design Principles

Purpose:
- Make the Level B class library feel intentional rather than a blend of typed
  Level B APIs and older Classic compatibility experiments.

Guidance:
- Define what belongs in Level B: typed APIs, explicit interfaces, predictable
  ownership, and names that teach the Level B model. Identify APIs that are
  Classic-oriented, compatibility oddities, or implementation leaks. Avoid
  breaking changes before Release 1 unless the benefit is clear and tests/docs
  can move with the change.

Acceptance:
- A design note gives review principles and a first pass list of keep,
  deprecate, remove, rename, or move-to-Level-C candidates.

### B3-DL-08: Define Plugin, Exit, And Integration Classification Policy

Purpose:
- Give Peter clear guidance for cleanup: the goal is not to delete code
  randomly, but to make the release surface visible and intentional.

Guidance:
- Classify each plugin, compiler exit, host exit, demo, and legacy integration
  as core, integration, optional, deprecated, experimental, or remove. For each
  category, describe expected build default, package inclusion, docs status,
  and test expectation. Capture why each item exists before proposing removal.
- Treat native-backed Rexx adapter modules as part of this policy too. The
  `Id`, `KeyDB`, and `Os` wrappers and their tests currently live under
  `lib/classlib` but are not linked into `classlib.rxbin`; their final home and
  Release 1 status should be decided through this triage, not as an incidental
  source cleanup.

Acceptance:
- An approved policy and classification categories exist, and the foundation
  inventory issue can apply them without re-litigating definitions.

### B3-DL-09: Define Level B Versus Level G Boundary

Purpose:
- Prevent Level G ideas from destabilizing the Level B Release 1 contract while
  still making the first Level G overlay visible.

Guidance:
- State what Level B owns for Release 1 and what Level G may add as libraries
  or demos. Keep Level G broad ambitions out of the stable Level B contract.
  Tie Unicode and LLM-related work to this boundary where relevant.

Acceptance:
- A short design note is reflected in release notes, language docs, and library
  docs.

### B3-DL-10: Define UTF/Text Ownership

Purpose:
- Keep text semantics coherent across Level B, Level G, Classic value handling,
  and VM/runtime behavior.

Guidance:
- Treat `.string` as valid UTF-8 and `.binary` as arbitrary bytes unless a
  design decision explicitly changes that. Decide which layer owns rich Unicode
  behavior. Keep Classic Rexx value-string semantics separate from Level B
  typed string guarantees.

Acceptance:
- The decision names the owner of UTF validation, byte/codepoint boundaries,
  richer Unicode helpers, and Classic string behavior.

### B3-DL-11: Define Level C Canonical AST Lowering Milestone

Purpose:
- Decide what beta 3 should honestly claim for compiled Classic Rexx. The
  intended Level C architecture is to transform the Level C parse/AST shape
  into the canonical compiler AST shape, then use the normal validation,
  optimization, and emission pipeline where possible.

Guidance:
- Connect this issue to source provenance, Classic values, variable pool,
  shared BIF curation, parser-mode validation, and the existing Level C working
  architecture. Keep the milestone small and demonstrable. Avoid wording that
  implies broad Classic Rexx compatibility, and do not use RexxScript as the
  Level C proof.

Acceptance:
- The beta 3 Level C milestone has a one-paragraph public description, an
  explicit canonical-AST lowering approach, a test or demo target if applicable,
  and explicit non-goals.

### B3-DL-12: Decide GPU/Threading Release 1 Status

Purpose:
- Avoid accidental release commitments for experimental VM directions.

Guidance:
- Decide whether GPU integration and VM threading/subtasks are stable,
  experimental, design-only, or post-Release-1. If experimental, define what
  users may rely on and what they may not. Keep this independent from the
  stable Level B contract.

Acceptance:
- The release note and roadmap agree on status, and no package/doc path
  presents these as stable beta 3 features unless explicitly approved.

## Foundation Issue Guidance

### B3-F-01: Add Source-Provenance Foundation For Preprocessor Output

Purpose:
- Implement the minimum mechanism needed to map diagnostics from preprocessed
  or generated text back to the user source.

Guidance:
- Start with a narrow test: source input, generated/preprocessed form, and an
  intentional diagnostic that should name the original line. Prefer a simple
  provenance structure over broad infrastructure if that gets beta 3 over the
  main risk. Document gaps.

Acceptance:
- At least one focused diagnostic mapping test passes, and the behavior is
  documented in the source-provenance note.

### B3-F-02: Build RexxScript Integration Beta 3 Slice

Purpose:
- Turn the RexxScript decision into one visible, testable interpreted
  strings-only integration slice.

Guidance:
- Use the smallest demo/test that proves the chosen integration path. Include
  useful status and error reporting. Reuse shared BIF entry points where that
  is clean. If source mapping is not complete, make the limitation visible
  rather than hiding it.

Acceptance:
- A demo or regression test shows the selected RexxScript path, and release
  notes describe the status accurately.

### B3-F-03: Implement Classic Rexx Value Class Prototype

Purpose:
- Provide a reusable value object for Classic Rexx semantics instead of
  scattering string/numeric conversion rules through BIFs and Level C work.

Guidance:
- Implement the smallest useful prototype: canonical string storage, selected
  numeric cache, cache invalidation, failed conversion behavior, and tests.
  Keep public API names conservative until the design note is accepted.

Acceptance:
- Unit or focused functional tests cover canonical string behavior, cached
  conversion, invalidation, and at least one failure case.

### B3-F-04: Implement Classic Rexx Variable Pool Prototype

Purpose:
- Provide a shared home for Classic Rexx variable lookup, assignment, stems,
  and host-visible variable access.

Guidance:
- Start with a minimal pool that can support the first BIF/RexxScript/Level C
  slice. Avoid prematurely modelling every Classic edge case. Record known
  unsupported behavior in the design note.

Acceptance:
- Tests cover basic variable lookup/assignment and the first agreed stem or
  host-access behavior.

### B3-F-05: Implement First Shared Rexx BIF Slice

Purpose:
- Let Rene prove that a small BIF surface can serve RexxScript and later
  compiled Classic Rexx without duplicating conversion behavior in every BIF.

Guidance:
- Choose a small BIF set from B3-DL-06. Prefer string-first entry points for
  RexxScript and Classic value/pool entry points where Classic semantics need
  them. Do not create per-BIF conversion rules that bypass the shared model.
  Each migrated BIF should have regression tests for normal values and at least
  one conversion edge.

Acceptance:
- The selected BIFs use the shared value/pool model, tests pass, and deferred
  BIFs remain listed with rationale.

### B3-F-06: Inventory And Classify Plugins And Exits

Purpose:
- Give Peter a concrete inventory of what exists before build/package defaults
  are changed.

Guidance:
- Walk the source tree and list plugins, compiler exits, host exits, demos,
  legacy integrations, and related docs/tests. Classify each item using the
  approved categories. Capture dependencies, build option, package status, test
  status, and "why keep this?" in the table.
- Include Rexx wrapper modules and tests that expose native plugin surfaces,
  even when they are not currently in a release library image. In particular,
  record the `Id`, `KeyDB`, and `Os` classlib-adjacent wrappers as package/core
  triage entries with their native dependencies and proposed destination.

Acceptance:
- A repo note or table lists each item and its proposed category. Items marked
  deprecated or remove have a reason and migration/replacement note where
  needed.

### B3-F-07: Align Default Build/Package Set With Plugin Classification

Purpose:
- Make the actual build and release package match the classification policy.

Guidance:
- Change defaults only after the inventory is reviewed. Keep source available
  behind explicit options when removal is risky. Update package docs and tests
  with every default change. Peter should call out anything that needs Adrian's
  language/runtime decision before changing CMake.

Acceptance:
- CMake defaults, package contents, docs, and tests agree on what is core,
  optional, deprecated, or experimental.

### B3-F-08: Review And Curate Level B Class Library APIs

Purpose:
- Apply the Level B class principles to the real class library and decide what
  needs cleanup before Release 1.

Guidance:
- Inventory public classes/interfaces and mark each as keep, document,
  deprecate, rename, remove, or move toward Classic/Level C support. Prefer
  documenting and isolating oddities over risky churn late in Release 1 unless
  the API is actively misleading.

Acceptance:
- Review notes identify decisions and follow-up implementation/doc issues.

### B3-F-09: Harden Late Load, Relink, And Class/Interface Rebinding

Purpose:
- Reduce VM/class correctness risk before lookup optimizations or larger class
  demos depend on this path.

Guidance:
- Build minimal repros for expected late-load and rebinding behavior before
  changing VM code. Keep tests close to the behavior being stabilized. If a
  current behavior is ambiguous, feed that back into design notes before
  hardening it.

Acceptance:
- Focused tests cover the current expected late-load/rebinding behavior, and
  known unsupported cases are documented.

### B3-F-10: Replace Hot-Path Linear Lookup With Indexed Lookup

Purpose:
- Improve method/factory lookup performance without breaking late-load
  correctness.

Guidance:
- Start after B3-F-09 gives confidence in semantics. Measure or reason about
  the current hot path, implement indexed lookup conservatively, and keep a
  fallback path or assertions where late-load mutation can invalidate indexes.

Acceptance:
- Tests for lookup and late-load behavior pass, and any performance claim is
  backed by the beta 3 baseline commands.

### B3-F-11: Add Large Immutable Constant Foundation

Purpose:
- Provide VM/RXAS/RXBIN support for large constant structures needed by parser,
  lexer, table, or demo work.

Guidance:
- Start at the representation and loader level. Decide the minimum structures
  needed for beta 3: bytes, integer arrays, string arrays, or table records.
  Keep source syntax sugar separate unless it is essential for an approved
  demo.

Acceptance:
- The minimum representation round-trips through RXAS/RXBIN/VM with tests, or
  the feature is explicitly deferred with design notes.

### B3-F-12: Decide Perfect-Hash `select` Implementation Path

Purpose:
- Either land a useful static `select` optimization or stop carrying it as an
  implied beta 3 promise.

Guidance:
- Identify the supported static cases and dependencies on large constants or
  lookup primitives. Implement only if the slice is small, testable, and
  measured. Otherwise document that nested `IF` lowering remains the beta 3
  behavior.

Acceptance:
- Either tests demonstrate the optimization or the release plan says it is
  deferred.

### B3-F-13: Establish Beta 3 Performance Baseline

Purpose:
- Give Rene a stable benchmark story so optimizer and VM improvements are not
  judged by anecdotes.

Guidance:
- Record commands, build type, platform, CPU, commit, warm-up behavior, and
  result format. Include linked/non-linked cases where relevant. Keep the
  baseline small enough to run repeatedly. Do not claim optimizer wins unless
  the baseline can reproduce them.

Acceptance:
- A baseline note or committed data file records the command set and initial
  results, and later performance issues reference it.

### B3-F-14: Validate Beta 3 Package Shape

Purpose:
- Give Rene a release-readiness checklist for the exact package assets that
  beta 3 will publish.

Guidance:
- Check the tag assets, not assumptions from `develop`. For each package,
  record asset name, signing status, install command, smoke command, uninstall
  or cleanup path, and known caveats. Use portable ZIPs as fallback if an
  installer is unreliable.

Acceptance:
- Release notes and install docs match the verified asset matrix before beta 3
  is announced.

### B3-F-15: Curate Beta 3 Demos And Tutorials

Purpose:
- Keep the user-facing examples coherent with the beta 3 contract.

Guidance:
- Choose demos that tell the beta 3 story: stable Level B, selected Level G,
  a possible RexxScript interpreted demo, a possible Level C canonical-lowering
  proof, packaging/install smoke, and known boundaries. Remove, mark, or defer
  demos that depend on unstable or experimental behavior. Every curated demo
  should have expected output.

Acceptance:
- The curated examples run against the beta 3 build, tutorials match their
  behavior, and known limitations are visible.

### B3-F-16: Prototype Level C Canonical AST Lowering Proof

Purpose:
- Prove the compiled Classic Rexx architecture with a deliberately small source
  slice: parse as Level C, transform to the canonical compiler AST shape, then
  reach the agreed downstream phase.

Guidance:
- Start after the design-lock milestone defines the exact slice. Favor one or
  two boring Classic forms over a flashy demo: for example simple assignment,
  `SAY`, or a tiny `IF` if that is the agreed path. Keep source mapping visible
  through the transformation. Do not depend on RexxScript for this proof.

Acceptance:
- A focused test or demo exercises the Level C AST-to-canonical-AST
  transformation, documents what downstream phase it reaches, and records what
  remains unsupported.

## Open Questions

- Should preprocessor source maps be a shared data structure used by TRACE,
  debug metadata, and parser-mode diagnostics, or should beta 3 start with a
  narrower compiler-diagnostic-only slice?
- Which RexxScript string-first BIFs can share implementation with later
  Classic Rexx / Level C value-pool BIFs?
- What is the smallest useful Level C canonical-AST lowering proof?
- Which Level B class APIs are compatibility leftovers versus genuinely useful
  typed Level B surfaces?
- Should plugin/exits cleanup remove source immediately, disable from defaults,
  or preserve source but move it behind explicit experimental options?
