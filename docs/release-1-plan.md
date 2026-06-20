# CREXX Release 1 Plan

Status: draft plan for maintainer review and GitHub discussion, updated after
the beta 2 tag and beta 3 branch baseline.
Date: 2026-06-20.
Target release: end of August 2026.

This plan describes the intended path from the tagged `v1.0.0-beta.2` release
baseline through the beta 3 foundation milestone to Release 1. It is not itself
a release contract. After the GitHub discussion is approved, create the issue
candidates below as GitHub issues with owners, labels, acceptance criteria, and
fallback decisions.

Beta 3 issue candidates and working team guidance are captured in
[`planning/beta-3/issue-candidates.md`](planning/beta-3/issue-candidates.md)
before they are turned into GitHub issues.

The release date is fixed. Scope is managed by tiering.

## Release Principle

Release 1 should ship a coherent, stable Level B toolchain with a credible
first Level G layer, a visible Level C compatibility milestone, and a clear path
for language-engineering work. It should not try to make every future level
stable.

The final two-week sprint is for QA, documentation, usability, examples,
packaging, and performance validation. User-facing feature work should be
complete by 2026-08-14 unless it is fixing a must-ship release defect.

## Gates

| Date | Gate | Exit condition |
| --- | --- | --- |
| 2026-06-17 | Beta 3 opens | Beta 2 has a tag, beta 1 to beta 2 delta is documented, `develop` is labelled beta 3 WIP, and the beta 3 planning note exists. |
| 2026-07-03 | Design lock | Level B/G split, plugin policy, UTF ownership, Level C MVP, GPU/threading scope, and issue owners/labels are approved. |
| 2026-07-31 | Beta 3 foundation target | High-risk VM/compiler foundations either landed with tests or moved out; large constants, perfect-hash select, Level C canonical-AST lowering proof, and beta 3 package shape have explicit go/no-go decisions. |
| 2026-08-14 | Feature complete | User-facing surface is frozen; demos and tutorials are ready for manual testing; known limitations are drafted. |
| 2026-08-31 | Release 1 | Release 1 is shipped, or a release candidate is ready with explicit residual risks. |

## Must Ship

Must-ship items are part of the Release 1 contract or release process.

1. Beta 3 branch baseline

   Keep `v1.0.0-beta.2` release notes as the historical beta 2 baseline, point
   current `develop` documentation at `v1.0.0-beta.3` WIP, and keep the beta 3
   planning note aligned with this timetable. Do not call beta 3 released until
   the `v1.0.0-beta.3` tag and assets exist.

2. Release 1 governance

   Open the GitHub discussion from the approved version of this plan. Create
   issues for must-ship and should-ship items only after the discussion is
   accepted. Use labels for `must`, `should`, `experimental`, and `post-r1`.

3. Level B lockdown

   Freeze Level B syntax and stable library surface by 2026-07-03. Complete
   documentation and tests for UTF/binary, references, arrays, collections,
   interfaces/classes, ADDRESS, TRACE, imports, linking, and packaging.

4. Level B/G split

   Publish a short design note explaining what Level B owns and what Level G
   owns. The Release 1 wording should say: Level B is stable; Level G has an
   initial library overlay and selected demos, not a full separate language
   contract.

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
   adapter modules and tests in this triage, not just C plugin directories:
   for example the current `Id`, `KeyDB`, and `Os` wrappers that are no longer
   part of `classlib.rxbin` still need a Release 1 decision as core,
   integration, optional, deprecated, experimental, or remove.

7. Runtime lookup and late loading

   Harden late load/relink tests and replace interface method/factory hot-path
   linear scans with indexed lookup. Treat this as the enabling VM work for
   classes, large tables, and later Level G/L performance.

8. Large constant foundation

   Implement or explicitly document the Release 1 minimum for immutable constant
   bytes, integer arrays, string arrays, and table records. If full source sugar
   is not ready by 2026-07-31, expose only the RXAS/VM and compiler-internal
   pieces needed for demos and keep broader syntax post-release.

9. Performance baseline and targeted improvements

   Establish performance baselines before changes. Ship only measured or
   strongly justified RXAS/rxc optimizer improvements. Use the existing inlining
   gate inventory to choose small, valuable optimizer slices.

10. Demos, tutorials, and docs

    Ship curated Level B, Level G, Level L, and Level C demos with expected
    commands and outputs. Complete final docs in the last sprint.

11. Packaging status discipline

    For beta 3, package only formats whose build, signing, upload, and smoke
    checks are reliable by the beta 3 foundation target. Keep portable ZIPs as
    the fallback for every platform and document any installer gaps clearly.

## Should Ship

Should-ship items are important but have explicit fallback paths.

1. Perfect-hash `select`

   Target static string/int `select x when ...` cases. Fallback: current nested
   `IF` lowering remains the Release 1 behaviour, with the optimization moved
   post-release.

2. First Level G library version

   Make `rxfnsg` coherent around the existing LLM work and possibly first
   Unicode helpers. Fallback: ship LLM plus docs/tutorial and mark Unicode as
   planned.

3. Initial Level C canonical-AST lowering proof

   Try for a small compiled Classic Rexx lowering slice by transforming the
   Level C parse/AST shape into the canonical compiler AST shape. Candidate
   source forms include variables, `SAY`, `IF`, simple `DO`, `ARG`, and
   string-literal `ADDRESS`, but the approved beta 3 slice should be smaller if
   needed. Fallback: ship DSLSH/highlighter as the Release 1 Level C milestone
   and publish the lowering plan as the next phase.

4. Level L lexer/parser demo

   Use large constants and optimized lookup if available. Fallback: a smaller
   demo using current arrays/tables, with performance work documented.

5. Plugin/demo cleanup

   Clean enough that users can tell core from optional. Fallback: docs and
   CMake options clarify status even if all source directories are not moved.

6. Windows installer user experience for beta 3

   Add a signed NSIS `setup.exe` from the signed Windows payload if the local
   signing flow can build, sign, upload, and verify it reliably before the beta
   3 tag. Fallback: keep the signed Windows ZIP as the supported Windows asset
   and document manual PATH setup.

7. Linux package hardening

   Keep the `.deb` path, but add install/uninstall smoke testing and dependency
   metadata review before treating it as more than a prototype. Fallback: ZIP
   remains the portable Linux asset and `.deb` remains dev-snapshot-only.

## Experimental Only

These should not block Release 1.

- GPU VM plugin proof of concept.
- VM multithreading/subtask prototype.
- Level G rich Unicode beyond the approved first slice.
- Broad Level L syntax sugar.
- Full Level C runtime compatibility.

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

## Issue Candidates

Create these after the GitHub discussion is approved. Suggested labels assume a
common `rel1` label plus the tier and area labels shown here.

### Must-Ship Candidates

| # | Candidate issue | Owner | Labels | Acceptance signal |
| --- | --- | --- | --- | --- |
| 1 | Open beta 3 branch baseline after beta 2 tag | Rene | `rel1`, `must`, `docs`, `release` | VERSION, README, release index, install docs, examples, security policy, and beta 3 release note identify `develop` as beta 3 WIP while preserving beta 2 as the latest completed tag. |
| 2 | Keep beta 3 release note aligned with Release 1 gates | Rene | `rel1`, `must`, `docs` | Beta 3 note carries high-level scope, timetable, package expectations, known limitations, and explicit WIP status until the tag exists. |
| 3 | Define Release 1 scope tiers and final feature-freeze date | Adrian | `rel1`, `must`, `planning` | GitHub discussion records tiers, dates, and fallback policy. |
| 4 | Lock Level B Release 1 language surface | Adrian | `rel1`, `must`, `level-b`, `language` | Syntax and stable library surface are frozen or explicitly listed as exceptions by 2026-07-03. |
| 5 | Define Level B versus Level G language and library boundary | Adrian | `rel1`, `must`, `level-b`, `level-g` | Short design note states what each level owns for Release 1. |
| 6 | Stabilize Level B core library API and iterator/reference contracts | Rene | `rel1`, `must`, `library`, `tests` | Public API names, examples, and focused tests agree. |
| 7 | Complete Unicode/text semantics issue #583 for Level B | Adrian | `rel1`, `must`, `unicode`, `level-b` | `.string`, `.binary`, conversion, comparison, and BIF behaviour are documented and tested. |
| 8 | Normalize tool output path behaviour issue #584 | Adrian | `rel1`, `must`, `toolchain` | `rxc`, `rxas`, and driver workflows have consistent `-o` behaviour and tests. |
| 9 | Complete RXAS float precision coverage issue #585 | Rene | `rel1`, `must`, `rxas`, `tests` | Regression coverage distinguishes stored binary64 precision from display formatting. |
| 10 | Complete RXAS instruction coverage issue #586 | Rene | `rel1`, `must`, `rxas`, `tests` | Instruction inventory and regression coverage are updated. |
| 11 | Retire/deprecate compiler-owned Unicode plugin path | Adrian | `rel1`, `must`, `unicode`, `plugins` | Obsolete path is removed, disabled, or documented as deprecated with replacement guidance. |
| 12 | Inventory and classify all plugins and native-backed adapters as core, integration, optional, deprecated, or experimental | Peter | `rel1`, `must`, `plugins` | Classification table exists and matches build/package defaults, including Rexx wrapper modules/tests such as `Id`, `KeyDB`, and `Os` that depend on native plugins. |
| 13 | Change default plugin build/package set to match Release 1 policy | Peter | `rel1`, `must`, `plugins`, `packaging` | Default build makes the release surface clear; optional legacy paths are opt-in. |
| 14 | Harden `METALOADMODULE` late load and class/interface rebinding | Adrian | `rel1`, `must`, `vm`, `classes` | Late-load and rebinding tests cover current expected behaviour. |
| 15 | Replace interface method/factory linear scans with indexed lookup | Adrian | `rel1`, `must`, `vm`, `performance` | Hot lookup paths use indexed search and retain late-load correctness. |
| 16 | Design fast register/class attribute metadata lookup | Adrian | `rel1`, `must`, `vm`, `compiler` | Metadata representation and lookup policy are documented and tested where implemented. |
| 17 | Add large immutable constant structures to RXAS/RXBIN/VM | Adrian | `rel1`, `must`, `vm`, `rxas` | Bytes, integer arrays, string arrays, and table records have a tested minimum representation or explicit deferral. |
| 18 | Expose large constant structures through rxc for lexer/parser use | Adrian | `rel1`, `must`, `compiler`, `level-l` | Compiler can emit the minimum constant tables needed by approved demos, or surface syntax is deferred with VM/RXAS support documented. |
| 19 | Add performance benchmark baseline for Release 1 | Rene | `rel1`, `must`, `performance`, `tests` | Baseline command set and recorded results exist before optimizer changes are claimed. |
| 20 | Run final demo/tutorial usability pass | Rene | `rel1`, `must`, `docs`, `qa` | Curated examples have commands, expected output, and manual pass/fail notes. |
| 21 | Run final packaging/signing/notarization validation | Rene | `rel1`, `must`, `packaging`, `qa` | Release assets, signing status, and platform package notes are verified before publishing. |
| 22 | Publish Release 1 known limitations | Rene | `rel1`, `must`, `docs`, `release` | Known limitations are in release notes and match the shipped feature set. |

### Should-Ship Candidates

| # | Candidate issue | Owner | Labels | Fallback |
| --- | --- | --- | --- | --- |
| 23 | Decide and implement Level G Unicode baseline | Adrian | `rel1`, `should`, `level-g`, `unicode` | Ship LLM-focused Level G and document Unicode as planned if `utf8proc` or API design is not settled. |
| 24 | Add build-time perfect hash optimization for static `select` | Adrian | `rel1`, `should`, `compiler`, `performance` | Keep current nested `IF` lowering. |
| 25 | Add RXAS/VM lookup primitives needed by perfect-hash select | Adrian | `rel1`, `should`, `rxas`, `vm` | Generate optimized branch sequence or move primitive post-release. |
| 26 | Add Level L lexer/parser library demo | Peter | `rel1`, `should`, `level-l`, `demos` | Ship a smaller demo using current arrays/tables. |
| 27 | Define Level G first library baseline | Rene | `rel1`, `should`, `level-g`, `library` | Document the existing LLM surface as the first baseline and move extra APIs post-release. |
| 28 | Add Level G tutorial and demos | Rene | `rel1`, `should`, `level-g`, `docs` | Ship one tutorial plus known limitations if the broader demo set is not ready. |
| 29 | Define initial Level C Release 1 milestone | Adrian | `rel1`, `should`, `level-c`, `planning` | Ship parser/highlighter milestone plus canonical-AST lowering plan. |
| 30 | Implement first Level C canonical-AST lowering/execution proof if approved | Adrian | `rel1`, `should`, `level-c`, `compiler` | Keep normal Level C compilation unsupported and document the next phase. |
| 31 | Establish `lib/rxfnsc` as the initial shared Level C/RexxScript runtime foundation | Adrian | `rel1`, `should`, `level-c`, `library` | Keep the current scalar/stem/pool runtime surface small and document later BIF/lowering work. |
| 32 | Add Level C demo and known-limits documentation | Peter | `rel1`, `should`, `level-c`, `docs` | Ship DSLSH/highlighter demo with explicit no-compile limitation. |
| 33 | Define RexxScript beta 3 integration slice | Adrian | `rel1`, `should`, `rexxscript`, `planning` | RexxScript is documented as an interpreted strings-only modern Rexx surface, not the Level C compiler path. |
| 34 | Curate shared Rexx BIF surface for RexxScript and Level C | Rene | `rel1`, `should`, `bifs`, `level-c`, `rexxscript` | First BIF list separates string-first RexxScript use from Classic value/pool needs. |
| 35 | Add RXAS peephole optimizer improvements from measured cases | Rene | `rel1`, `should`, `rxas`, `performance` | Keep baseline optimizer and publish benchmark results. |
| 36 | Add rxc optimizer/inlining improvements from current fail-closed gates | Rene | `rel1`, `should`, `compiler`, `performance` | Keep gates fail-closed and document deferred cases. |
| 37 | Clean up plugin demos and separate core from non-core examples | Peter | `rel1`, `should`, `plugins`, `demos` | Clarify status in docs/CMake even if directories are not moved. |

### Experimental Or Post-Release Candidates

| # | Candidate issue | Owner | Labels | Fallback |
| --- | --- | --- | --- | --- |
| 38 | Add Level L syntax-sugar demo if syntax is approved | Adrian | `rel1`, `experimental`, `level-l` | Keep Level L demo library-only for Release 1. |
| 39 | Add GPU VM plugin proof of concept behind experimental status | Adrian | `rel1`, `experimental`, `vm`, `plugins` | Publish design notes or keep the work out of the release branch. |
| 40 | Define VM multithreading/subtask design and Release 1 scope | Adrian | `rel1`, `experimental`, `vm` | Ship design-only; keep shared-memory subtasks post-Release 1. |

## Dependency Map

Decisions needed before implementation:

- Level B/G boundary and what counts as Level B stable.
- Plugin category policy and default build policy.
- UTF ownership: VM codepoint baseline, Level G rich Unicode, and retirement of
  compiler-owned Unicode plugin semantics.
- Level C MVP and whether it includes execution or only DSLSH plus a
  canonical-AST lowering proof.
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
- Level L demos depend on large constant structures and lexer/parser library
  APIs.
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
- Beta 3 release notes carry the current WIP scope and timetable; keep README,
  `docs/releases`, security policy, examples, install docs, and language
  reference aligned on beta 3 WIP versus completed beta 2 status.
- `docs/ai-context/CREXX_LIBS.md` should describe `rxfnsc` as the Level
  C/RexxScript runtime foundation now that the library directory exists.

## Final-Sprint Focus

The final sprint is reserved for:

- full automated test pass and CI triage;
- manual testing of release packages and all curated examples;
- documentation, tutorials, known limitations, and usability cleanup;
- performance measurement and safe optimizer/RXAS improvements only;
- package/signing/notarization checks;
- release notes and GitHub release materials.

Feature work that misses the 2026-08-14 gate should move to post-Release 1
unless it fixes a must-ship defect.
