# Initial cREXX document generation

Status: **initial macOS generation and beta 3 PDF bundle complete; independently
reviewed and asset publication verified on 1 October 2026. First hosted Linux
qualification and snapshot PDF publication remain open below.** Adrian has
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

Approved initial qualification target: macOS end-to-end. Preserve the existing
native POSIX/Windows file/process route. Linux/Windows PDF generation remains
explicitly unqualified until executed there; do not describe the initial macOS
result as cross-platform qualification. The subsequent authorised `develop`
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
  Original typography and Linux/
  Windows PDF execution remain unqualified.
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
  PDF hashes above still identify the reviewed generation snapshots; the
  correction will be included when those books are next generated.
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

8. **DG-AC-08 — passed locally; Linux cover review remains AC-09: explicit version on covers.** All four staged cover pages
   and publication-data pages contain the supplied workflow version. Verify a
   tagged version and a long development version in real rendered pages; CLI
   regression checks cover version validation/TeX escaping and unchanged legacy
   invocation. Compare authored-input hashes before/after generation.
9. **DG-AC-09 — open; first hosted bootstrap failed, repair/retry required: real Linux four-book generation.** A hosted Ubuntu Linux
   job builds the required actual cREXX tools and fresh PDFs through the current
   generator, free font profile and real Pandoc/XeLaTeX/index/Biber tools. Retain
   dependency versions, exact source/PDF hashes, logs, page counts and literal
   listing fidelity. Independently inspect covers and representative Linux
   pages; no mock build qualifies Linux execution.
10. **DG-AC-10 — local policy passed; hosted publication open: publication policy and asset freshness.** Inspect/test
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

6. **DG-STEP-06 — complete locally; hosted covers pending (AC-08/09).** The existing generation agent adds an
   optional explicit version to the driver and staged metadata, focused tests
   and CLI guide. Preserve existing invocations and authored templates. Root
   owns this plan and all CI files; serialize shared file/build access.
7. **DG-STEP-07 — implemented locally; hosted proof open (AC-09/11).** Root supplies a reusable Linux CI route,
   dependency/bootstrap instructions and automated PDF/provenance checks. Run
   focused local checks; publish only this bounded follow-up once reviewable.
   Actual Linux execution is an open qualification gate until the hosted build
   succeeds. Any compiler/runtime defect returns to a separate approved plan.
8. **DG-STEP-08 — implemented locally; publication proof open; depends on STEP-07 (AC-10/11).** Root integrates the
   reusable job into Build CREXX and Deep Build QA, with the approved differing
   failure policies and existing publishers. Verify optional/complete asset-set
   handling, stale-PDF removal, checksum/provenance and unchanged binary gates.
9. **DG-STEP-09 — local review passed; hosted review open; depends on STEP-06–08 (AC-08–11).** The existing
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
  succeeded, and the live dev-snapshot tag/body identify that exact SHA. The
  Linux CI follow-up is a separate reviewable commit and its new hosted output
  remains required before Linux generation/publication is claimed complete.

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

11. **DG-STEP-11 — complete for macOS; hosted Linux verification remains AC-09 (AC-06/09/12).** The new actual
    PDF-destination audit found 39 missing `Hfootnote` destinations in both the
    beta 3 edition and previously reviewed PDFs. Footnote text is present and
    readable; this is a package-order navigation defect, not source loss.
    Independent real fixtures prove that loading `setspace` after `hyperref`
    overwrites the latter's footnote anchor wrapper. The v2 rebuild repaired
    35 ordinary targets; four language-reference table notes also require
    `longtable` to be loaded before `hyperref` so its supported delayed-footnote
    wrapper is installed. The generation agent stages `setspace` and `longtable`
    before `hyperref`, preserving the authored template. Verify the compiled
    port and real ordinary/table-footnote regression fixture,
    regenerate the four beta 3 PDFs and independently inspect actual destinations
    and unchanged text/layout before publication. Root includes this bounded
    repair in the next develop publication; no language/architecture decision
    or broad overnight matrix is needed. Receipts: `beta3-release/qa/`.

12. **DG-STEP-12 — in progress; Linux bootstrap repair (AC-09/10/11).** Hosted
    run `36918100438` failed before typesetting because the selected Ubuntu
    packages omit the Pagella/Heros OpenType files. Explicitly install
    `fonts-texgyre`, refresh the TeX filename cache and check all eight required
    serif/sans faces. The retained apt log also records 535 MB taking 1 hour
    24 minutes from the runner's Azure HTTP mirror. On GitHub runners only,
    replace that specific mirror with Canonical's HTTPS archive and use bounded
    inactivity/retry settings; do not rewrite local developer mirror settings
    or weaken package signature checks. Verify shell/workflow syntax and the
    official Ubuntu package inventory, then publish this bounded installer fix
    and inspect the new actual Linux outputs. Cancel the superseded run with
    the known missing dependency so it cannot repeat the same long failed setup.
    Reuse unchanged macOS book/product qualification; no manual Deep matrix.
    The repaired run `36929391128` fetched 543 MB in 16 seconds, found all eight
    fonts and built the actual product/port, then stopped in XeLaTeX because
    `siunitx.sty` is absent. Audit the packages loaded by the qualified four-book
    route against Ubuntu's installed package closure, add the missing bundle
    and check it before generation; retain the inventory and independent review
    before another hosted retry. Receipts: `linux-ci/hosted/36918100438/` and
    `linux-ci/hosted/36929391128/linux-documentation-evidence/`.
    The independent inventory maps the direct and actual four-book runtime
    dependencies to official Ubuntu Noble file inventories; the only omitted
    package owner is `texlive-science`, which supplies `siunitx.sty` for
    `pstricks-add` → `pst-math` → `pst-calculate`. Install that bundle and check
    `siunitx.sty` before generation. MacOS 2026-only filenames have explicit
    Ubuntu 2023 equivalent dispositions; no full TeX installation is needed.
    Inventory receipt: `linux-ci/qa/tex-package-inventory.json`.

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
