# Initial cREXX document generation

Status: **complete for the approved macOS/Ubuntu initial-font scope, beta 3
bundle and Linux CI publication. Independent QA and downloaded asset readback
verify the beta 3 bundle and matching development snapshot.** Adrian has
authorised committing and publishing the work to `develop`, with the separate
release-documentation changes in a second commit.
This is the authoritative plan and acceptance record for document generation.
Issue: [#712](https://github.com/adesutherland/CREXX/issues/712).

## Vision and intended outcomes

Produce all four CREXX book PDFs through a reproducible, repository-owned
cREXX generation route that René can run and maintain. Preserve his authored
chapters, examples, templates, publication notices and original wrappers.
Keep the existing source-tree layout and recover superseded history from Git.
The result is initial cREXX document generation, not a proof-of-concept label
or an assertion that the current conversion-only checkpoint typesets correctly.

Finish the actual typesetting route, repair existing filename/path wiring,
and add a small lexical cREXX highlighter for book fragments. Printed fragments
must not need to compile or run. Preserve exact source text and use the existing
language-specific listing handlers for other languages. Only explicit authored
splices execute examples; declare and validate their dependencies separately.
The generation scripts must not require ooRexx or an external TextTools checkout.
An explicitly authored foreign-language demonstration can retain its own runtime
dependency; that is distinct from the generator's implementation language.

Use free fonts for initial generation: TeX Gyre Pagella for body/title serif
text, Heros for headings, and JuliaMono for code/terminal text. Keep the original
font settings selectable and document every substitution and its restoration
for René. The initial PDFs demonstrate functioning generation and usable layout;
matching the original typography is a later handover task.

Approved initial qualification target: macOS end-to-end, subsequently extended
to hosted Ubuntu Linux below. Preserve the existing native POSIX/Windows
file/process route. Windows PDF generation remains explicitly unqualified until
executed there; the macOS evidence alone does not qualify another platform.
The subsequent authorised `develop`
publication initially did not upload the books or cut a release. The Linux CI
follow-up below now covers book assets for matching snapshots and future tags.
No new branch/worktree,
language change or compiler/runtime redesign is in this plan.

## Checkpoint and retained evidence

- `ecbc2b4040aeefbda2e9e5554da41f80ffca73bb`: local cleanup outside documentation.
- `85060de54bf2199ce391282a2544d289fefb0733`: documentation cleanup and
  initial cREXX generation checkpoint.
- The maintained port covers all 17 TextTools helpers. Focused checks build the
  required product tools, compile/assemble/link/run the port, exercise
  failed/missing tools and outputs, and use real Pandoc 3.11 for 244 Markdown
  inputs and 1,422 listings. Four real PDFs and their independent reviews now
  qualify the initial profile; all listing snapshots match authored source.
- [BUILDING-DOCS.md](../BUILDING-DOCS.md) and
  [the port guide](../texttools/README.md) record the current route and limits.
  Detailed receipts are outside the source tree under
  `/Users/adrian/.codex/cleanups/CREXX-20260930-108257c3d/documentation-review-20261001/`.
- `binary_memory_instructions.md` refers to `bcopy.rxas`, `fixedwidth.rxas`,
  `textfields.rxas`, `move.rxas` and `compare.rxas`. Corresponding sources exist
  as `examples/binary_bcopy.rxas`, `binary_fixed_width.rxas`,
  `binary_text_fields.rxas`, `binary_move.rxas` and `binary_compare.rxas`.
  These are manual reference repairs, not lost example source. This chapter is
  not currently included by the VM structure; do not silently add it or invent
  new examples while repairing names. Audit the actual included chapter route.
- Adrian has authorised the `README.md`/`INSTALL-RUN.md` release-package edits
  as a separate commit. Preserve the unrelated `scripts/__pycache__/`.

## Numbered checkable acceptance criteria

1. **DG-AC-01 — passed: source preservation and repair inventory.** Record the
   actual include, listing, figure and explicit splice routes for each book.
   Repair stale names/relative paths using retained source or its verified
   generator. Pre/post hashes and a bounded intentional-change list establish
   that chapters, listings, attribution and templates were not accidentally
   removed, overwritten or silently omitted. Every unresolved input has a
   named disposition; active missing inputs prevent completion.
2. **DG-AC-02 — passed: real four-book generation.** A documented cREXX invocation
   produces fresh, readable PDFs for all four books in a detached output tree
   using real Pandoc, XeLaTeX, index/bibliography tools and required helpers.
   Retain command, tool versions, revision/source hashes, logs, PDF hashes and
   page counts outside the source tree. No mock typesetter qualifies this
   criterion; source inputs remain unchanged by generation.
3. **DG-AC-03 — passed within approved scope: initial and original font profiles.**
   An explicit font selection covers all active font requests, including title pages, headings,
   code, terminal output, generated instruction material and tables. The free
   profile works without proprietary fonts. Original font defaults remain
   recoverable/selectable. A complete substitution table explains what René
   needs to restore or change later. Verify profile selection and rendered
   samples; do not claim original-layout equivalence for substituted fonts.
4. **DG-AC-04 — passed: fragment-safe lexical highlighting.** A Level B cREXX
   lexer/formatter colours Rexx/cREXX listing tokens without invoking the
   compiler, resolving imports or executing snippets. Validate its rules against
   relevant compiler scanners and project syntax documentation. Fixtures cover
   comments including nesting, quote escapes, numeric/literal forms, operators,
   keyword-like words, Unicode, tabs, multiline and truncated fragments, empty
   input, TeX-special characters and unknown text. Tokens plus preserved gaps
   reconstruct the exact source; unterminated fragments remain printable.
   Lexical styling must not pretend to identify semantically resolved symbols.
   Other fence languages retain their existing handlers.
5. **DG-AC-05 — passed: execution and failure contracts.** Only explicit splices
   execute. Validate the included commands, dependencies and working directories
   with real tools; surface failed commands and missing inputs. Focused tests
   retain actionable failures for missing tools/fonts/assets, child nonzero exit,
   stale/missing PDF, malformed fence and unsafe output paths. No silent
   placeholder result, skipped required chapter or upload/view action is part
   of generation. Keep fake-tool orchestration evidence clearly separate.
6. **DG-AC-06 — passed: independent PDF and fidelity review.** Independent QA
   checks every extracted listing's source fidelity and scans the full real
   typesetting logs for missing files/glyphs, broken references and errors.
   Visually inspect representative pages from each book: title, contents,
   headings, long/fragment/Unicode listings, figures, API/instruction tables,
   citations, bibliography and index where present. Retain the review and
   resolve materially unreadable, clipped or omitted content before completion.
7. **DG-AC-07 — passed: usable handover and qualification limits.** Provide
   reproducible dependency/build commands, source-versus-generated guidance,
   manual repair notes, font restoration instructions and exact tested platform
   limits in the maintained guides. Record a small timing observation for the
   lexical pass on the actual listing set, with no parser/process launch per
   listing and no unmeasured speed claim. René can repeat the initial build
   from the documented inputs; no extra historical output copies enter HEAD.

## Numbered implementation steps and decision gates

1. **DG-STEP-01 — complete (AC-01/02/05/07).** Reconcile the checkpoint,
   read the repository/Level B/ADDRESS/syntax references and record actual active
   include/splice dependencies. Obtain the missing TeX Live/Pandoc/helper tools
   and record versions and installation steps. Distinguish tool installation
   from generation success. No installation or implementation begins before
   this plan is approved.
2. **DG-STEP-02 — complete; depends on STEP-01 (AC-01/03/05).** Repair bounded
   filename/path/case inconsistencies and expose an explicit initial/original
   font selection in the detached build route. Preserve authored font defaults
   and original notices; inspect generated inputs before running TeX. Adding a
   previously excluded chapter or materially changing prose/layout requires a
   separate scope decision, not an automatic reference repair.
3. **DG-STEP-03 — complete; may proceed alongside STEP-02 after STEP-01
   (AC-04/07).** Specify and implement the lexical span-to-TeX contract, using
   Peter Jacob's `lib/classlib/Scanlex.crexx` as the starting point. Preserve its
   author/doc tags and unrelated JSON scanner/API behavior; adapt shared code
   only where justified. Use maintained examples and focused regressions for
   scanner accuracy, source reconstruction and fragment rendering. Integrate
   through the generation agent after the formatter contract is reviewed.
4. **DG-STEP-04 — complete; depends on STEP-02/03 (AC-02–06).** Run the real
   four-book route, fix reproduced generator/typesetting defects within scope,
   exercise focused error cases and confirm source fidelity. Build/test the
   Level B modules through `rxc`, `rxas`, `rxlink` and `rxvm`. Reuse unchanged
   valid checks; do not launch broad release or overnight product matrices.
   Any required compiler/runtime/language change returns for a separately
   approved plan; first-party sanitizer findings follow the repository worklist.
5. **DG-STEP-05 — complete; depends on STEP-04 (AC-01–07).** Independent QA
   reviews retained evidence and rendered samples against every criterion.
   The coordinator checks the actual diffs, reproducing the smallest disputed
   or missing check rather than repeating all valid tests. Complete the
   maintained handover and present remaining font/platform follow-up clearly.
   Claim initial generation complete only when all criteria pass; keep any
   unresolved criterion visibly open.

## Approved agents and ownership

Agent dispatch follows plan approval. Use at most two implementation agents
plus one independent QA agent, with the coordinator in the existing develop
checkout. No agent creates another branch or worktree, commits, pushes or
contacts René. Assign separate files and serialize shared build/output access.

| Role | Proposed model / effort | Owned work and boundary |
| --- | --- | --- |
| Generator implementation | GPT-6 Sol / High | Dependency route, bounded manual path repairs, font profile, driver/TextTools integration and guide updates. Owns shared generator files; integrates the highlighter after its contract is stable. |
| Lexical highlighter implementation | GPT-6 Sol / Extra High | Compiler-grounded token rules, fragment tolerance, source spans, TeX escaping/formatter and focused lexer tests. Works in isolated source files; coordinates any Scanlex change and does not edit shared generator files concurrently. |
| Independent QA and preservation review | GPT-6 Astra / Extra High | Independently inspect author/input preservation, listing fidelity, failure behavior, real PDF evidence and rendered samples. Does not certify its own implementation. Reports failures against DG-AC IDs. |
| Coordinator | Current chat | Reconcile approved scope, resolve file ownership, maintain criterion status, inspect agent diffs and verify claimed evidence before handover. Escalate actual design changes rather than quietly expanding scope. |

The higher effort for lexical work and independent QA is for exact source
preservation, fragment/Unicode corner cases and assessment of real PDF output.
Manual path/font repairs do not require a new language or runtime architecture.

## Approval and current disposition

Adrian requested the two local checkpoints before reviewing and approving the
implementation plan. Those commits are complete. The approved plan defines the
initial macOS qualification boundary, explicit font profiles, lexical renderer
and agent assignments. Adrian approved this plan on 1 October 2026. He
subsequently approved JuliaMono
for code/terminal text (Pagella/Heros retained, explicit CJK/emoji fallbacks), and
a staged-only RXPP hierarchy repair: a dedicated RXPP chapter with its local
contents list as a subordinate section. Authored Markdown headings and web
anchors remain unchanged. These are the approved font/layout refinements;
original font selection remains available. Independent QA and coordinator
review pass all seven acceptance criteria for the approved initial macOS scope.
Adrian subsequently authorised two separate commits (document generation,
then release documentation), QA and publication to the existing `develop`
branch. Use the retained unchanged qualification plus focused checks for the
affected behavior, and verify the normal automatic publication workflows on
the pushed revision. That earlier publication authorisation did not cover a new release or book
upload; the Linux/beta-3 follow-ups below explicitly extend asset publication.
Restoration of René's preferred typography remains later work.

Evidence root:
`/Users/adrian/.codex/cleanups/CREXX-20260930-108257c3d/documentation-review-20261001/`.
The pre-edit book/wrapper inventory records 1,281 authored inputs and symlink
targets. Work used the existing develop checkout reconciled with origin/develop.
The coordinator owns this plan
and dependency installation; agents own the separate implementation/review files
listed above. Final evidence is reconciled below.

### Current implementation evidence

Detailed run history, intermediate outputs and qualification receipts stay
outside the repository. Paths below are relative to the documentation-review
root named above. All seven criteria pass; no active input or material PDF
finding remains unresolved.

- User-local TeX Live 2026, Pandoc 3.11, Inkscape 1.4.4 and Ghostscript 10.08.0
  are installed. The focused product build provides the actual embedded
  `rxvme` plus `crexx/rxc/rxas/rxlink/rxdas/rxdb/rxcpack` dependencies. Tool
  files and committed component trees are fingerprinted in
  `initial-generation/tools/product-tool-fingerprints.json`; cached printed Git
  suffixes do not identify the current source tree.
- The final lexical renderer SHA-256 is
  `485f5d23fdc66612b31c0cb9c7a3c49db964a14946207bb0b3699cf14127d1a2`.
  Focused four-stage execution and real XeLaTeX fixtures pass. Independent QA
  reconstructs all 19 adversarial fixtures and all 1,422 current authored fences
  exactly: 196,037 UTF-8 bytes, zero failures. The one-VM batch took 7.98 seconds,
  including extra reconstruction and evidence writes. This is a bounded timing
  observation, not isolated lexer speed or a promised PDF-build speedup.
  Proof is retained in `qa/corpus-independent-joiner/results.json` and
  `qa/corpus-evidence-current.json`.
- All 244 Markdown inputs convert through real Pandoc. Of 1,422 total fences,
  1,308 are in active book routes. Rexx/cREXX uses the static lexical renderer;
  other languages retain their supported handlers, with literal `text` blocks
  printed through Verbatim. Only authored splices execute. The active route
  inventory is `qa/active-route-summary.json` and `qa/active-input-current.json`.
- The qualified four-book generation records 1,270 unchanged baseline inputs,
  eleven bounded reviewed repairs and zero missing inputs. Symlink targets and
  original notices/templates/font requests remain intact. The intentional
  ledger is `qa/intentional-source-changes.json`. The new maintained RXPP book
  label map is included in the 1,282-entry final manifest. Pre/post generation
  comparison records zero mutations in `qa/authored-post-final.json` and
  `qa/pre-final-to-post-final-approved-deltas.json`. Retained optional VM guards
  and the excluded manual chapter have explicit dispositions; no required
  active input was silently skipped.
- Guarded child commands retain successful stderr and fail with actionable
  diagnostics for nonzero exits, missing commands and failed pipelines. Real
  negative fixtures cover missing fonts/assets and missing glyphs. A 220-line
  terminal fixture prints all lines once, in order, across five pages without
  clipping. makeindex rejected entries and unconverged TeX references now fail
  the driver. All included real splices complete. Real font/asset/command
  failures are distinguished from fake-tool orchestration/freshness/path tests
  in `qa/final-independent-review.json` and its referenced focused receipts.
- The real four-book route completed successfully. Book-specific reproduced
  layout defects were repaired and the affected books rebuilt. Final PDFs are
  Language Reference 430 pages, Programming Guide 260, VM Specification 361,
  and Library Reference 546. Final current prepared inputs for language, VM,
  library and shared boilerplate are byte-identical to their qualified inputs;
  the programming guide uses the latest linked driver. Independent equality
  proof in `qa/final-prepared-equivalence.json` supports reuse without
  repeating unchanged TeX work. Exact PDF/source/driver hashes, tool versions
  and output paths are in
  `initial-generation/generator/final-book-manifest.json`; actual argv, CWD,
  PATH, exit codes and logs are retained in `final-build-invocations.json`
  beside it. The latest focused four-stage/Pandoc check passes in
  `initial-generation/generator/final-ctest-fixture/results.json`.
- Independent QA reviewed all 1,597 page boundaries, all 1,422 listing
  snapshots, full final logs and representative title/contents/Unicode/figure/
  API/instruction/citation/bibliography/index pages. There are no material
  clipping, API collisions, missing glyphs/required assets, broken referenced
  targets or convergence failures. Nonblocking template warnings and
  unreferenced duplicate labels have explicit dispositions. All seven
  criteria and the exact qualified hashes are recorded in
  `qa/final-independent-review.json` and `qa/qualified-code-hashes.json`.
  Coordinator samples are recorded in
  `initial-generation/coordinator-visual-review.json`. The same-byte reviewed
  PDFs are collected in `initial-generation/publications/`, with their hashes
  and source locations in its compact manifest.
- The approved initial profile now uses JuliaMono 0.63.2 for code/terminal text,
  TeX Gyre Pagella/Heros for body/headings, and explicit GNU Unifont 18.0.01
  BMP/Upper fallbacks. Four JuliaMono faces and their OFL licence are installed
  in user-local TEXMFLOCAL. Candidate coverage, real PDF embedding and inspected
  raster evidence are in `initial-generation/tools/font-research/`. Full-book
  JuliaMono layout and embedding pass independent review. Contextual
  alternates are disabled to keep operators literal. Emoji remain monochrome
  component glyphs; joiners and presentation controls are invisible.
  Original typography and Windows PDF execution remain unqualified. The later
  Ubuntu qualification below independently verifies the initial free profile.
- The approved RXPP chapter/contents hierarchy repair is applied only in staged
  book output; the authored Markdown headings and GitHub links stay unchanged.
  The maintained guides document this conversion, bounded staged layout
  repairs, the current font profile, dependency installation, source/generated
  boundaries and exact platform limits. Final reread and guide hashes are in
  `qa/guide-review.json`. The legacy NetRexx/SQLite instruction/diagram refresh
  route is preserved and remains unqualified; the printed checked-in VM
  instruction chapter is explicitly a partial historical opcode view.
- The shared RXAS listing style now leaves comment delimiters visible to
  `listings`, removing an asterisk substitution that coloured `load` inside
  comments as an instruction. Its ordinary block-comment rule also recognises
  nesting. This is one additional intentional template repair; font requests
  and source listings remain unchanged. Real XeLaTeX and independent visual/
  colour checks pass for inline, comment-only and nested multiline comments,
  quoted markers and instructions before/after comments. Exact source hash and
  proof are in `initial-generation/rxas-comments/coordinator-review.json` and
  `initial-generation/rxas-comments/fixed/rxas-comment-review.json`. The four
  PDF hashes above still identify the original reviewed generation snapshots;
  the later beta 3 and Ubuntu editions below include this correction.
- Publication follows Adrian's authorisation above. The two cleanup checkpoints
  remain in the ancestry; detailed commit/QA/publication receipts stay outside
  the source tree. The subsequent Linux/beta 3 asset request is tracked below;
  no new release tag or branch/worktree is needed.

## Linux CI and versioned publication follow-up — 1 October 2026

Adrian requested a Linux runner to generate the books on each commit and keep
PDFs as release assets, with the version string on every cover. He chose to
publish binaries even when document generation fails, then specified that
**Deep Build QA must fail on a documentation failure**. This extends the initial
macOS scope; it does not cut a new product release or restore original fonts.
The two authorised commits are now on develop at
`b4d089ce9391ee7a7d0ea32dac0ea8c7e72a3da5`; their normal publication checks
remain separate from the first Linux book qualification.

### Vision and intended outcomes

Use one maintained Linux book job from ordinary Build CREXX and Deep Build QA.
Ordinary pushes/PRs generate all four books from the exact selected source SHA;
PRs retain CI artifacts, develop snapshots and future versioned tags attach the
four successful PDFs through the existing asset publisher. Each cover and its
publication data identify the same explicit workflow version as the binaries,
including the short source SHA for development builds. Authored title templates,
notices, chapters and examples remain preserved; stamps apply to staged output.

The ordinary book job reports failure prominently and retains logs but does not
prevent binary publication. A failed/incomplete generation uploads no book asset
set; a refreshed snapshot must not keep PDFs from an older commit. Deep Build QA
uses the same route with failures fatal and records no successful assurance
marker unless the books pass. The first actual Linux build qualifies Linux;
Windows PDF execution and original typography stay unqualified. Reuse valid
macOS and product evidence instead of dispatching broad overnight suites.

### Additional numbered acceptance criteria

8. **DG-AC-08 — passed: explicit version on covers.** All four staged cover pages
   and publication-data pages contain the supplied workflow version. Verify a
   tagged version and a long development version in real rendered pages; CLI
   regression checks cover version validation/TeX escaping and unchanged legacy
   invocation. Compare authored-input hashes before/after generation.
9. **DG-AC-09 — passed: real Linux four-book generation.** A hosted Ubuntu Linux
   job builds the required actual cREXX tools and fresh PDFs through the current
   generator, free font profile and real Pandoc/XeLaTeX/index/Biber tools. Retain
   dependency versions, exact source/PDF hashes, logs, page counts and literal
   listing fidelity. Independently inspect covers and representative Linux
   pages; no mock build qualifies Linux execution.
10. **DG-AC-10 — passed: publication policy and asset freshness.** Inspect/test
   successful, failed and partial document outcomes. A complete successful set
   contributes four clearly named PDFs and provenance to matching snapshot/tag
   assets. Failed generation permits ordinary binaries, explicitly reports no
   current PDFs and removes obsolete snapshot PDFs. Existing binary asset checks
   and latest-develop guard remain intact. Verify the first published PDF names,
   version/source provenance and hashes against its hosted build.
11. **DG-AC-11 — passed: Deep failure and maintained handover.** The scheduled or
   explicitly selected Deep job invokes the identical Linux route with failure
   required, and the assurance marker depends on success. Focused workflow/policy
   checks establish failure propagation without dispatching the full overnight
   matrix. The guides document CI triggers, dependencies, stamps, artifacts,
   failure policy and tested platform boundaries.

### Additional numbered implementation steps and ownership

6. **DG-STEP-06 — complete (AC-08/09).** The existing generation agent adds an
   optional explicit version to the driver and staged metadata, focused tests
   and CLI guide. Preserve existing invocations and authored templates. Root
   owns this plan and all CI files; serialize shared file/build access.
7. **DG-STEP-07 — complete (AC-09/11).** Root supplies a reusable Linux CI route,
   dependency/bootstrap instructions and automated PDF/provenance checks. Run
   focused local checks; publish only this bounded follow-up once reviewable.
   Actual Linux execution requires a successful independently reviewed hosted
   build. Any compiler/runtime defect returns to a separate approved plan.
8. **DG-STEP-08 — complete; depends on STEP-07 (AC-10/11).** Root integrates the
   reusable job into Build CREXX and Deep Build QA, with the approved differing
   failure policies and existing publishers. Verify optional/complete asset-set
   handling, stale-PDF removal, checksum/provenance and unchanged binary gates.
9. **DG-STEP-09 — complete; depends on STEP-06–08 (AC-08–11).** The existing
   independent QA agent reviews the diff, failure policy, source preservation
   and real hosted Linux outputs. Root verifies the exact matching publication
   and updates this plan's statuses with retained evidence. No broad overnight
   dispatch is needed to prove the new workflow's failure wiring.

### Follow-up review and deployment disposition

- AC-08 local preparation and real tagged/90-character development cover and
  publication-page rendering pass independent QA. Six authored title/shared
  templates remain byte-identical. The focused driver harness passes legacy,
  initial/original and rejected unsafe version cases. Receipts:
  `initial-generation/linux-ci/version-stamps/version-stamp-results.json` and
  `linux-ci/qa/version-stamp-independent-review.json`.
- Ordinary publication also treats a failed document artifact download or a
  partial/hash-mismatched received set as unavailable documents: it removes
  that set, reports the effective status and continues the existing binary
  gates. The approved policy is not limited to a typesetter failure.
- GitHub's live default branch is `master`. Scheduled Deep workflow files and
  relative reusable calls come from that branch. Adrian explicitly authorised
  applying the reviewed reusable workflow and required Deep gate there now.
  Remote commit `7f33356264bdfe1adef11baef1f190a60bef24ca` is independently
  verified; only the two workflow files changed. Scheduled activation is
  deployed. This establishes failure wiring and deployment, not a completed
  overnight Deep matrix. The source selected by scheduled assurance remains
  the actual resolved develop commit.

- Root's ten focused document-publication checks and the fourteen existing
  provider/publication guards pass. These execute actual collector/failure/
  stale-asset loops against fixtures; successful, failed, partial and corrupted
  document sets preserve binary gating and the approved best-effort policy.
  Workflow syntax and shell checks pass. Exact stamp-field comparison rejects
  shorter version prefixes; source snapshots include symlink targets as well
  as referent hashes. This is local integration proof, not Linux PDF proof.

- Independent local review passes all fourteen tagged/snapshot collector
  cases, exact real stamp-field validation, symlink-retarget detection and a
  case-sensitive audit of 566 active references (including eleven tracked
  documentation symlinks). No local functional blocker remains to the first
  hosted Linux qualification. Rechecks are retained in
  `linux-ci/qa/independent-repair-recheck.json` and
  `linux-ci/qa/path-case-audit.json`. Full actionlint is now clean, including its
  ShellCheck integration; standalone shell syntax/ShellCheck/diff checks pass.
- The original two-commit publication is complete at
  `b4d089ce9391ee7a7d0ea32dac0ea8c7e72a3da5`: normal Build CREXX and CodeQL
  succeeded, and the publication readback identified that exact SHA. The
  Linux CI follow-up is separately reviewed and qualified below.
- The actual hosted failure policy passes at `edf91e5c1827`: normal Build
  `36931215235` completed successfully and published matching binaries despite
  the CMake listing failure. The publication readback identified that exact
  commit, its body reported unavailable PDFs and no document assets remained.
  Receipt: `linux-ci/hosted/36931215235/failure-policy-verification.json`.
  The required Deep failure wiring is unchanged. Current successful Linux
  qualification and matching snapshot publication are recorded below.

## Beta 3 PDF bundle and scheduled gate activation — approved follow-up

Adrian explicitly authorised applying the two reviewed workflow files to master
now, overriding the ordinary release-line restriction for this bounded workflow
change. Commit `7f33356264bdfe1adef11baef1f190a60bef24ca` is pushed to master;
only `build-docs.yml` and `deep-build.yml` changed. Develop contains a tree-identical
history merge of that change. The default-branch scheduled required-documents
route is now deployed; no release tag was moved or newly cut.

Adrian additionally requested a PDF bundle in the existing beta 3 release assets.
Produce a release-version edition of all four books, with
`crexx-1.0.0-beta.3` on their covers/publication pages, through the same maintained
route. The qualified macOS tool route may generate this platform-independent
bundle while the first Linux run continues. Keep documentation source identity,
platform/tools/font profile and PDF hashes explicit. Verify production-source
identity against the beta 3 tag; do not rename development-version PDFs and
present them as a release edition. Preserve all authorship/notices and the known
initial-font/historical-instruction-content handover limitations.

12. **DG-AC-12 — passed: beta 3 book bundle publication.** Four fresh, reviewed PDFs
    stamped with the plain beta 3 version plus provenance are packaged as
    `CREXX-v1.0.0-beta.3-docs.zip`. Independent QA checks the actual covers,
    full typesetting logs and listing fidelity, reusing unchanged valid body/
    generator evidence where appropriate. Upload to the existing public beta 3
    release only after checks pass; reread the asset listing and verify the
    uploaded digest/contents. No new tag, release or original typography claim.
10. **DG-STEP-10 — complete (AC-12).** Root runs the release-version generation
    with the maintained qualified native tools/initial profile, records exact
    invocation and source-versus-beta3 identity and builds the bundle. Existing
    independent QA reviews its actual output and package before root publishes
    the explicitly authorised asset. Detailed receipts stay outside HEAD.

11. **DG-STEP-11 — complete (AC-06/09/12): footnote navigation.** Staged
    preamble loading places `setspace` and the existing `longtable` before
    `hyperref`, preserving the authored template and package count. The compiled
    port and permanent ordinary/table-footnote fixture pass; actual macOS beta 3
    and Ubuntu books resolve all 1,716 internal links, including all 39 footnotes
    on their correct pages. Printed notes remain readable. Receipts:
    `beta3-release/qa-v3/beta3-v3-independent-review.json` and the current Linux
    review below. No language/architecture change or broad overnight rerun.

12. **DG-STEP-12 — complete (AC-09/10/11): Linux dependency bootstrap.**
    Explicitly install `fonts-texgyre` and `texlive-science`, refresh the TeX
    filename cache, and preflight all eight Pagella/Heros OpenType faces plus
    `siunitx.sty`. The latter is used through PSTricks calculation packages.
    The direct and actual four-book dependency inventory maps files to official
    Ubuntu Noble package owners, with explicit equivalents for macOS-only
    filenames. On GitHub runners only, replace the observed slow Azure HTTP
    archive mirror with Canonical HTTPS; bounded inactivity/retries preserve
    package signature checks and local developer mirrors. The actual repaired
    setup fetched 543 MB in 16 seconds and later builds complete all four books.
    Shell/workflow and independent inventory checks pass. Receipts:
    `linux-ci/qa/bootstrap-repair-review.json` and
    `linux-ci/qa/tex-package-inventory.json`. Reuse unchanged macOS/product
    proof; no full TeX installation or manually dispatched Deep matrix.

13. **DG-STEP-13 — complete (AC-04/06/09/10): Ubuntu CMake listings.**
    Ubuntu `listings` 1.9 lacks the native CMake catalogue entry. Detached
    boilerplate first asks the installed package for its own handler, loading
    the bundled full upstream 1.11b definition only if absent. Preserve upstream
    author/LPPL notices and adapt only its public registration command; authored
    listings and native/registered handlers remain intact. Real official Noble
    and current-package fixtures verify literal comments/quotes, handler
    retention and identical text/rasters. All eleven active language routes
    pass; unrelated unknown languages still fail. The compiled port passes
    `rxc/rxas/rxlink/rxvm` preparation, and actual Ubuntu four-book generation
    passes including all CMake fences and 1,422 exact listing snapshots.
    Receipts: `linux-ci/listings-compat/step13-results.json` and
    `linux-ci/qa/listings-linux-compatibility-review.json`.
    Root owns CI/publication, the generator agent owns the helper/integration/
    fixtures, and independent QA reviews preservation and actual output.

14. **DG-STEP-14 — complete (AC-06/08/09/10):
    Linux physical clipping.** Repair detached output only: the existing
    `rxcpack -h` splice uses literal wrapped terminal output, and the VM
    architecture path permits zero-ink breaks after its slash and hyphens.
    Preserve authored chapters, command, punctuation and path characters;
    introduce no example execution or semantic content. The permanent real
    XeLaTeX fixture uses the Noble listings catalogue, captures actual help and
    uses the VM driver's persistent
    `\large` text size and original list width, reproduces the 104.18355pt
    baseline overflow/five clipped characters, and verifies the replacement
    with zero physical outliers, including a narrower stress case. Malformed
    raw-splice context fails actionably. Compiled-port/all-four-book preparation
    preserves all 1,422 snapshots/196,037 bytes and source/symlink records.
    Actual Ubuntu run `36941821058` independently resolves both
    `DG-LINUX-CLIP-01` (help page 165) and `DG-LINUX-CLIP-02` (path page 324).
    All 1,601 page bounds pass; the remaining 11.96378pt content-box warning
    enters blank margin with no clipping or overlap. Root independently
    inspects both actual rasters and exact source/PDF hashes. Earlier
    unqualified release artifacts were withheld; downloaded PDFs and reviews
    remain outside HEAD. Focused receipts:
    `linux-ci/layout-repair/step14-results.json` and
    `linux-ci/layout-repair/qa-v6/independent-review.json`.
    Root retains publication ownership; matching asset readback passes using
    unchanged help/product/body evidence without a broad rerun.

### Current Linux qualification and publication evidence

- Qualified source: `b59410ce77ab1c8097460e8370ebea2cab05e964`, hosted Ubuntu
  Build `36941821058`, document job `110635383167`. The actual product and
  cREXX port build successfully; real Pandoc 3.11, XeTeX/TeX Live 2023, Biber
  2.19 and the installed index/helpers produce fresh four-book output.
- The explicit version is `crexx-1.0.0-beta.3+dev-snapshot.gb59410ce77ab`
  on all four covers and publication pages. PDFs are 432/262/361/546 pages.
  The provenance manifest records exact PDF/product/tool hashes; its SHA-256
  is `c03b6b16d20b604ea22596e3d293c511476e3c4f6a08ee8c625603ea98a55915`.
- Independent review checks all 1,337 source records/11 symlinks, all 1,422
  listings/196,037 UTF-8 bytes, 997 lexical reversals, all 1,601 physical page
  bounds, 1,716 internal links/39 correct-page footnotes and 67 scoped references.
  Font embedding, final logs, indexes and API tables pass; no material finding
  remains. Current rendered covers/publication/help/path pages are inspected.
  Unchanged body/figure evidence is reused through 2,692 identical staged files
  and exact all-page text comparison except the intended path repair and
  explicit version/clock fields.
- Exact source-manifest digest:
  `061df4cb2b113fca5c02b1ebc6ca4fba35cbc4de1a597b7eaded1e4f42fbb892`.
  Successful collection compares fresh post-generation inputs with the retained
  pre-generation snapshot; independent QA also reconciles that snapshot with
  exact Git objects. The workflow retains the pre-generation snapshot.
- Receipts: `linux-ci/hosted/36941821058/qa/linux-independent-review.json`
  and `linux-ci/hosted/36941821058/coordinator-intake.json`. Actual release
  artifact `11200419683` and evidence artifact `11201515506` are retained;
  all five qualified asset files were published by the matching normal workflow.
  Build `36941821058` and publisher job `110643634218` complete successfully.
  The [development snapshot](https://github.com/adesutherland/CREXX/releases/tag/dev-snapshot)
  tag and body identify the exact source/version above. All four PDFs and
  `CREXX-dev-snapshot-docs.json` match GitHub's digests and fresh downloaded bytes.
  Readback receipt: `linux-ci/hosted/36941821058/publication-verification.json`.
  Normal product/optimizer/provider checks pass. Earlier CodeQL at `df9bf84d`
  passed on unchanged C inputs; the automatic b594 analysis is still running
  when this handover is written. This status-only closeout changes guides and
  this plan, preserving qualified generation/book/test/CI inputs. Subsequent
  automatic checks remain separate from the retained qualification above.
  Windows generation, original typography and the legacy NetRexx refresh route
  remain unqualified; this does not assert a completed overnight Deep matrix.

### Current beta 3 publication evidence

- Final document source: `54eb6aa8bec320a8538377dbd2686c53cc0e7282`.
  The two bounded footnote commits preserve the authored shared preamble and
  its package count. The compiled-port ordinary/table regression and independent
  actual four-book checks pass. All 1,716 internal PDF links resolve, including
  all 39 footnotes on their correct source pages; printed notes remain readable.
- Fresh plain-version books are 430/260/361/546 pages. Full logs, glyph/font/
  reference/index checks, all 1,597 page bounds, and representative visual/body
  comparisons pass. All 1,422 listing snapshots (196,037 bytes) and the 1,336
  authored-input/symlink records are preserved. Exact qualified PDF/tool hashes
  and independent receipts are in `beta3-release/build-v3/release-assets/` and
  `beta3-release/qa-v3/beta3-v3-independent-review.json`.
- Existing public beta 3 release asset `604148580` is
  [`CREXX-v1.0.0-beta.3-docs.zip`](https://github.com/adesutherland/CREXX/releases/download/v1.0.0-beta.3/CREXX-v1.0.0-beta.3-docs.zip):
  four PDFs, provenance JSON and initial-font/source-identity README, 3,524,392
  bytes, SHA-256 `5c17d6d6b726fb5b9d48022aec53dd59fe4e5c99abefdb50a844c93fb5002a88`.
  GitHub's digest and downloaded six-member CRC/byte/hash readback match.
  Receipt: `beta3-release/publication-verified.json`. The beta 3 tag/product
  commit remains `ae1607b8e145174422cee7f3e73fbcc37a65226c`; its compiler/runtime/
  library implementations match the release-channel tools used by the books.
  Differences are documentation links and removed disabled experimental
  benchmark options. No release body or tag changed.

## Narrow document-gate repair — 3 October 2026

### Vision and intended outcomes

Restore the scheduled Deep Build QA book job after René's `hidelinks` preamble
edit. Preserve that edit and his adjacent style changes. Keep the current
four-book generator and the distinct Deep-required/ordinary-optional failure
policies. The active document CI route uses no Python; its PDF acceptance check
is only that all four expected output files exist and are non-empty. This
maintainer-approved simplification supersedes the earlier ongoing CI checks
for PDF text, pages, hashes, listings and manifest identity in DG-AC-09–11;
their historical qualification receipts remain historical evidence. Product
QA and unrelated Python-based package tooling are outside this change.

### Acceptance criteria

13. **DG-AC-13 — passed: René's preamble builds.** Both cREXX staged-preamble
    checks accept and retain `\usepackage[hidelinks]{hyperref}` while preserving
    the existing package-order adjustment. Inspect the staged preamble and a
    focused generator run; the authored preamble and style file do not change.
14. **DG-AC-14 — passed: simple document gate.** The active document workflow,
    wrapper and document-specific publication path invoke no Python. A successful
    generation yields exactly the four named, non-empty PDFs. Missing or empty
    output prevents document publication. No PDF content, header, page, hash or
    manifest check is required. Inspect the workflow and exercise the four-file
    check with empty and non-empty fixtures.
15. **DG-AC-15 — open: existing failure policy survives.** Deep Build QA fails
    when its required document job fails; ordinary binary publication can proceed
    without document assets, and stale snapshot PDFs are removed. Inspect the
    workflow conditions and use normal hosted publication evidence. The scheduled
    workflow on `master` must use the same narrow document-job change.

### Implementation steps

10. **DG-STEP-10 — complete (AC-13).** Fast-forward the existing `develop` checkout,
    retain unrelated untracked files, and adapt only the generator's staged
    `hyperref` recognition/reordering to preserve René's option.
11. **DG-STEP-11 — complete; depends on STEP-10 (AC-14).** Remove document-path
    Python setup, helpers and detailed PDF validation. Collect four expected
    PDFs with a shell non-empty-file check; simplify optional publication to
    that same check and remove claims about a new provenance manifest.
12. **DG-STEP-12 — implementation complete; scheduled Deep outcome open;
    depends on STEP-11 (AC-14/15).** Run the smallest focused generation and
    four-file checks, publish the bounded `develop`
    change, then apply the matching reusable workflow change to `master` under
    Adrian's explicit approval for this narrow scheduled-gate exception. Check
    automatic workflow outcomes; do not dispatch the overnight matrix solely
    for this repair.

### Focused local evidence

- The `develop` checkout fast-forwarded to René's merge
  `2a6a209713997b1c1a8c1cd199619e3b2a80a920`. Its authored
  `preamble.tex` and `crexxalmanac.sty` are unchanged by this repair.
- The changed cREXX generator compiled, assembled and linked with the existing
  Release product tools. A one-book `prepare` run passed and its staged
  preamble retained `\usepackage[hidelinks]{hyperref}` after package reordering.
- `bash scripts/check-doc-pdfs.sh` passed with four non-empty fixture files and
  rejected an empty one. `bash -n`, `actionlint` for both edited workflows,
  and `git diff --check` passed. Scheduled Deep QA remains open.
- Published `develop` repair `d1c0258899f2711a32459d83588cebb091ebd3c7`
  passed the actual Ubuntu document job
  [`111159294580`](https://github.com/adesutherland/CREXX/actions/runs/37107663861/job/111159294580).
  Its downloaded `documentation-release-asset` contains exactly the four
  expected non-empty PDF files. The full
  [Build CREXX run](https://github.com/adesutherland/CREXX/actions/runs/37107663861)
  passed and published the matching development snapshot. That release has
  exactly four non-empty PDFs, its body names the repaired commit, and the old
  `CREXX-dev-snapshot-docs.json` asset is absent.
- The only scheduled-workflow edit on `master` is
  `77ba820c35e40450311823a3e7f171e37a7982f8`, which removes the
  documentation job's Python setup/install and obsolete snapshot artifact
  reference. The next scheduled Deep run remains open.
- The previous document failure proved the Deep-required path fails as intended,
  while the ordinary build at René's merge completed despite its optional
  document failure. The current repaired Build confirms the optional publication
  path. Deep's next successful scheduled outcome has not yet run.
