# Level L language tooling research — 10 October 2026

### Authoring and build refinements — approved 13 October 2026

**Vision:** make a Level L language module a normal part of a cREXX project.
Programmers can use `.crexx` with `options levell`, or a short dedicated suffix,
and let `crexx` generate, assemble and link the module. Teach ordinary lists
using the existing repetition notation and useful named fields. Preserve the
language accepted by each example, original source diagnostics, the Level B
bootstrap and existing consumers. This increment supplements LANG-AC02/04/11–13;
the wider research and language criteria remain unchanged.

**Status:** Adrian approved the source forms, wrapper integration, list cleanup,
regression tests and guide update. A combined program build includes its grammar
modules directly; creating a separate library is optional. **UX-AC01–05 are
locally verified** by [the qualification receipt](evidence/authoring-20261013/README.md).
The existing `develop` checkout was fast-forwarded to `54b610b95` before editing;
incoming Fortran/Forth work was preserved. Before publication, develop packaging
fixes through `1031ef530` were integrated and their combined installed workflow
was checked. Adrian authorized commit/push to `develop` and light monitoring
on 13 October. The enclosing commit is the publication payload; normal automatic
CI is expected to pass, with its actual results reported in the conversation.
No new release tag or overall programme completion is claimed.

**Approved decisions:**

- Use `.crexx` with `options levell` as the ordinary source form. Offer `.rxl`
  as the short Level L suffix; retain `.levell` for existing sources. For
  `.rxl`, Level L is the default when OPTIONS is absent. An explicit different
  level is an error. Namespace declarations remain part of the source.
- Route through the installed `crexx` driver and existing `--library` and
  `--program` commands. Generate packed RXAS, assemble with `rxas`, and link
  generated modules and their Level L runtime dependencies. A standalone
  grammar builds a reusable module; an application supplies execution.
- The compiler source-import scan identifies Level L headers and leaves their
  namespaces to generated modules; it does not feed grammar declarations into
  the typed source parser. Its dependency snapshot still tracks changes in
  source kind. This is source selection, not a parser/scanner replacement.
- Build generated grammar providers before compiling their consumers. Extend
  existing caching, dependency and atomic-publication rules; generator/runtime
  changes invalidate affected products. Required bootstrap construction still
  works without Level G or the high-level project driver.
- Reuse existing `*`, `+`, `?`, groups and `as` captures. Convert ordinary
  hand-authored list/tail scaffolding where it simplifies the source. Preserve
  deliberate recursive backend demonstrations. Keep Pratt operator declarations
  as the existing concise expression form.

**List-construction finding:** the first simplified RexxScript run exposed
quadratic intermediate frontier copies in the named-tree view: the existing
3,000-statement regression exhausted `tree.entries`. The view now counts
transparent frontiers and materializes visible links for named parents only.
UX-AC04 includes a direct 3,000-item test at an exact 6,001-entry budget, its
one-entry-too-small refusal, order/span checks and transparent-root selection.
This repairs the observed regression without increasing user budgets.

**Acceptance criteria — locally verified on macOS ARM64 Debug:**

1. **UX-AC01 — Source selection.** `.crexx` with `options levell`, `.rxl` with
   its agreed default, and existing `.levell` sources generate equivalent
   modules. Tests cover leading comments, case, conflicting headers, paths
   containing spaces and unchanged B/G/C selection. Locations refer to original
   source even when the level is supplied by a suffix.
2. **UX-AC02 — Integrated builds.** Installed `crexx` builds a standalone
   grammar library and a mixed grammar/application project, links the required
   runtime, and runs the example through bytecode and native packaging. Verify
   dependency ordering rather than relying on input-file order or old outputs.
3. **UX-AC03 — Build lifecycle.** Focused tests prove clean/no-op builds,
   grammar/tool/runtime invalidation, output collision checks and preservation
   of the last good artifact on errors. Grammar failures stop dependent work
   and retain the authored filename, rule and location.
4. **UX-AC04 — List clarity and equivalence.** Simplified settings and
   RexxScript lists preserve acceptance, item order, captures and source spans,
   with readers adapted to the intended flatter named tree. Test zero/one/many
   items, blank lines, repeated separators, trailing separators, missing
   separators, malformed items, diagnostics and recovery. Exercise LL/LR/PEG/
   Earley where the selected grammar is admitted; preserve their distinct
   semantics and report capability failures against authored rules.
5. **UX-AC05 — Bootstrap and teaching continuity.** L0/L1/L2 regeneration still
   agrees with no Level G bootstrap dependency. Update the guide, diagrams and
   progressive settings/calculator instructions to use tested wrapper commands
   and repetition-first teaching. Existing recursive backend lessons remain
   clearly identified. Retain focused toolchain/both-VM regression evidence.

**Implementation steps — approved:**

1. **UX-STEP01 (AC01–05):** agree suffix/header behavior and build/list scope;
   reconcile the current guide and consumers against those decisions. **Agreed.**
2. **UX-STEP02 (AC01–03/05):** implement source selection and driver generation,
   dependency ordering, linking and lifecycle handling. Depends on STEP01;
   retain the independent Level B bootstrap path. **Verified.**
3. **UX-STEP03 (AC04/05):** simplify ordinary lists and adapt tree consumers;
   compare with retained original grammars and inspect diagnostics/recovery.
   Depends on STEP01; new semantics require a separate decision. **Verified:**
   384 old/new observations plus capture, recovery and interpreter regressions.
4. **UX-STEP04 (AC01–05):** run focused installed/build/backend regressions,
   update the user guide and record evidence and remaining criteria. Depends
   on STEP02/03; follow the repository's ordinary publication policy. **Verified:**
   30 existing CTests, 56 installed authoring commands, 22 installed package
   commands, 13 packaging unit tests and 80 valid guide links. Wider research
   and host-platform qualification criteria below remain unchanged.

### AST infrastructure — implementation approved 13 October 2026

#### AST-STEP04 first increment — locally qualified

**Vision:** exercise the common AST in a real script engine, remove positional
tree plumbing from expressions, assignment, SAY and IF, and make reusable
analysis and checked constant folding available without adding grammar syntax.
The enclosing sequence/block/loop wiring uses named fields where required
to carry this slice; loop algorithms, language rules, BIF effects and Level G
numeric behavior remain application policy. This is the approved acyclic
increment of STEP04, not closure of the later cyclic-analysis or recovered-origin
work. Adrian authorized publication of this increment on 14 October 2026.

**Acceptance criteria for this increment:**

1. **AST4-AC01 — Migration.** RexxScript executes the named AST for expressions,
   assignment, SAY and IF. Existing language acceptance, values, branch and
   short-circuit behavior, BIF order/effects and source diagnostics agree with
   the retained pre-migration implementation. Existing loops, return and
   control validation continue to work around the migrated slice.
2. **AST4-AC02 — Analysis.** A Level B children-first service records one
   application-defined result per node, reads only completed dependencies and
   publishes only a complete result. Wrong-tree, expired-owner, missing/future
   dependency, incomplete pass and resource failures have explicit tests.
   Rewritten/copied trees cannot consume another tree's facts accidentally.
3. **AST4-AC03 — Meaning-preserving fold.** RexxScript analysis reports known
   constants and conservative variable reads/effects. Folding calls the same
   scalar operations as execution, preserves types and exact representable
   values, never invokes BIFs, and leaves failing/unsupported computations for
   execution. Folded and ordinary execution agree on results, output, state,
   effects and reachable-error locations; input ASTs remain usable.
4. **AST4-AC04 — Structure and limits.** Checked one-pass reconstruction is
   finite, source ranges survive replacements, and failed analysis/rewrite
   publishes no partial result. Cover deep/wide/empty trees, fresh runs,
   cancellation and bounded interpreter work; record the structural proof and
   separate it from application-equivalence evidence.
5. **AST4-AC05 — Integration and teaching.** Level B bootstrap, normal product,
   both VMs, opt/noopt focused tests and installed RexxScript/Level L examples
   pass. The guide explains the analysis-to-rewrite pipeline and a complete
   worked example. Preserve wider AST-AC01–06 obligations above/below.

**Implementation steps:**

1. **AST4-STEP01 (AC01/03):** retain the current interpreter as a differential
   control; map existing grammar rules with captures and `then tree`, preserving
   recognition and keeping the ordinary source surface unchanged. **Complete.**
2. **AST4-STEP02 (AC02/04):** implement tree-owned result tables and checked
   children-first analysis, with lifecycle, dependency and budget regressions.
   **Complete.**
3. **AST4-STEP03 (AC01/03/04):** migrate named-field execution and add RexxScript
   facts and checked folding, sharing numeric/literal operations. Depends on
   STEP01/02; unsupported or non-round-trippable constants remain unchanged.
   **Complete.**
4. **AST4-STEP04 (AC01–05):** compare against retained controls and unfolded
   execution, qualify affected builds/tests and installed use, and update the
   guide, contract and evidence. New tests should be prebuilt direct VM tests,
   not nested aggregate builds. Keep subsequent analysis profiles visibly open.
   **Complete.**

**Increment status:** AST4-AC01–05 and AST4-STEP01–04 are verified locally on
macOS ARM64 Debug, based on develop `50926edb8`. Adrian authorized commit/push
to `develop` and light CI monitoring on 14 October. Before publication the
checkout was synchronized through `60f58c0a9`; all 20 recorded inputs still
matched their qualification hashes. Incoming changes concern separate test
fixtures, their evidence and Apple packaging retries. The enclosing commit is
the publication payload; only publication documentation changed after those
checks. [Qualification and exact inputs](evidence/ast-step04-20261013/README.md)
retain the results and publication reconciliation. Normal automatic CI is
expected to pass; actual results are reported in the conversation, without a
status-only commit. The unchanged Level B bootstrap contract is preserved.

| Criterion | Verification |
|---|---|
| AST4-AC01 | Existing 60-case RexxScript suite on both VMs; 28 exact command-line comparisons with the retained pre-migration interpreter; named-field execution review. |
| AST4-AC02 | Dedicated analysis tests cover dependencies, exact/exhausted budgets, early/double finish, missing/duplicate facts, expired/copied/rewritten/reassigned owners and current-cursor access. |
| AST4-AC03 | 40 new cases compare folded and ordinary output, state, scalar types/values, float bits, short-circuit effects and exact error strings; exact-float and integer replacement shapes checked. |
| AST4-AC04 | Existing empty, 1,000-item and 2,000-wrapper AST controls; atomic rewrite/analysis failures; source anchors; fresh executions, cancellation and fuel controls; updated structural contract. |
| AST4-AC05 | Warning-free product; four equal opt/noopt L1/L2 self-descriptions; both VMs and compiler modes; 267 AST surface observations plus 51 unoptimised capture/diagnostic observations; eight installed commands; updated guide and example. |

**Previous delivery:** the reviewed first AST surface is implemented and locally qualified
on macOS ARM64, based on synchronized develop `ec481f8e6`. It includes attached
mappings, inferred field contracts, owned trees, reference handles, iterative
walks, checked one-pass reconstruction, calculator/folding and settings examples.
[Qualification and scope](evidence/ast-20261013/README.md) retain the results and
identities. Adrian authorized commit/push to `develop` and light CI monitoring
on 13 October. The enclosing commit is the publication payload; its qualified
code and test inputs are unchanged. Normal automatic CI is expected to pass,
with actual results reported in the conversation rather than a status-only
commit. This advances LANG-AC04/09/10/12/13 and research AC-26 without closing
their wider obligations.

**Cumulative criterion status:**

| Criterion | Verified here | Still open in the wider criterion |
|---|---|---|
| AST-AC01 | Simple attached mappings; named text/child fields; required, optional and list counts; located refusals; settings/calculator | None for the reviewed inferred-schema surface. |
| AST-AC02 | Independent owned results; borrowed handles; request release; exact/composite/derived source ranges; Unicode; stale/foreign-handle rejection; counted budgets | Synthetic and recovered origin infrastructure; broader migration measurements. |
| AST-AC03 | Iterative walks; field/list access; application folds; checked reconstruction; empty/deep/wide/malformed/shared inputs; atomic failure; opt/noopt and both VMs; explicit traversal bounds | General search/fold convenience services and the wider builder/grouping profiles. |
| AST-AC04 | Schema-checked one-pass rewrites; acyclic application facts; owner/snapshot invalidation; checked RexxScript constant folding with exact numeric round trips and diagnostic anchors | Restricted monotone cyclic analysis, grouping/lowering and their proofs. |
| AST-AC05 | Equivalent admitted AST fixtures across general backends; Pratt/LR/Earley expressions; composite settings; explicit ambiguous/failed refusal; named RexxScript slice, effects/control/error comparisons and retained pre-migration control | Recovered-origin integration. |
| AST-AC06 | Level B bootstrap in both optimization modes; identical L1/L2 output; installed combined/library builds; progressive manual | Compiler-shaped builder/validator example and production migration work. |

AST-STEP02/03 and the first-delivery portion of AST-STEP05 are complete within
the scope above. The acyclic RexxScript increment of AST-STEP04 is implemented;
its local integration gates are verified above. Cyclic analysis, grouping/lowering
and the remaining portions of AST-STEP05 stay open.
The complete AST infrastructure programme is not being marked complete.

**Approved first surface:**

```rexx
program: rule = (statement as item)*
  then tree (items = item*)
sum: infix arithmetic plus precedence 10 left
  then tree Add(left = left, right = right)
```

- The enclosing label supplies the default AST kind; an explicit kind renames
  it. Indentation is formatting, not scope. Named captures supply ordinary
  fields; operator declarations supply left/right/inner operand roles.
- Text fields retain spelling; application code chooses numeric conversions.
  `*`, `+` and `?` describe ordered lists and optional fields. Construction is
  separate from recognition and occurs only for a successful selected parse.
- Walks offer parents-first and children-first order. A children-first rewrite
  exposes rewritten children to their parent, visits each original node once,
  and does not recursively revisit replacement nodes. It preserves its input
  and publishes only a checked complete result, with origins retained.
- The worked calculator demonstrates this surface, constant folding in ordinary
  Level B, and original-source diagnostics. Basic grammars need no AST schema.
  The first schema is inferred from mappings; richer explicit schema and
  multiple-projection syntax remain design extensions to review, not silently
  accepted additions to the source language.
- Regional component wrapper mappings are rejected with `LL_AST_REGION`, because
  that wrapper is absent from the grafted result. Token/infix/prefix and enclosing
  parser-rule mappings remain available; the checker never silently drops a
  requested wrapper mapping. This admission boundary is documented and tested.

**Implementation sequence for this delivery:**

1. AST-STEP02: mapping grammar/lowering, inferred field contracts and generated
   definitions; retain L0/L1/L2 bootstrap parity (AST-AC01/02/06).
2. AST-STEP03: owned result storage, small reference handles, iterative walks,
   checked builders and one-pass reconstruction (AST-AC02/03/04).
3. AST-STEP05: calculator and settings examples, positive/negative backend,
   ownership, origin and rewrite regressions, installed build and guide evidence
   (AST-AC01–06). **Verified for the first-delivery scope above.** The wider analysis/grouping/RexxScript criteria below stay
   individually visible; completing this delivery is not their automatic proof.

**Vision and intended outcome:** let a language author turn a successful parse
into useful application nodes with named fields and lists, then inspect, check
and transform them through a common Level B library. The same application AST
should work with every admitted parser backend. Source ownership and diagnostic
origins survive each stage. Settings, calculators, script engines and later
compiler frontends use the same services; cREXX-specific symbols, types, control
flow and runtime behavior remain consumer policies.

**Why this next:** the product already exposes DFA, LL(1), canonical LR(1),
packrat PEG, Pratt and Earley. Its named view supports arbitrary child counts
and captures, but application-tree projection still has a binary 0/1/2-child
schema. Builders, grouping and restricted attribute scheduling have research
controls, not one settled product API. A stronger AST layer therefore improves
all current algorithms and tests the intended compiler-migration direction.
A new parser should follow a concrete grammar, diagnosis or resource need that
the current portfolio cannot meet. Pika and Squirrel remain research work;
there is no proposed change to their support status here.

**Infrastructure contracts:**

- Keep the ordinary named tree and captures useful without a schema. An optional
  AST declaration defines named node kinds, required/optional fields and ordered
  lists. Variable child counts do not require hand-written binary list nodes.
  Review concrete settings/calculator before-and-after surface examples first.
- Keep original source, parser derivations/forests and application ASTs distinct.
  An ambiguous forest cannot become an AST by silently choosing its first path.
  Existing explicit result/status contracts remain visible.
- Prefer owned node storage and tree-scoped node IDs, with snapshots frozen after
  construction. A checked builder or rewrite creates a new result; source maps
  relate new nodes to old nodes and original text. Derived analysis records
  identify their owning tree/revision. Exact, composite, synthetic and recovered
  origins remain distinguishable. Measure copying and lifecycle behavior before
  promising a particular physical representation or zero-copy API.
- Supply iterative walks in source order and child-before-parent order, named
  field/list access, searches and folds (combine child results into a parent
  result). Add pure checked rewrites for wrapper removal, list construction and
  one declared grouping/lowering example. Failed passes publish no partial AST.
- Start analysis scheduling with acyclic dependencies; then promote the existing
  restricted monotone finite-domain worklist control. Such analyses only add
  facts until stable. Every stronger operation needs its own admitted contract.
  A work budget bounds execution; it does not prove equivalence or convergence
  of arbitrary rewrite callbacks.
- Keep syntax mechanics separate from execution: branch choice, short-circuiting,
  BIF effects and language-specific arithmetic stay in the application evaluator.
  Preserve the Level B bootstrap; production rxc/rxas frontend migration remains
  a separately approved programme.

The implemented count analysis, traversal bounds, ownership rules and rewrite
transaction are described in [AST-CONTRACT.md](AST-CONTRACT.md). In particular,
finite structural reconstruction does not prove arbitrary application semantics.

**Numbered acceptance criteria — open until verified:**

1. **AST-AC01 — Shape and simple authoring.** A basic grammar still works without
   AST declarations. Optional schemas express named required/optional fields and
   zero/one/many lists. Invalid field names, kinds and cardinalities produce
   located grammar/schema diagnostics. Verify runnable settings/calculator
   examples and negative build fixtures against the reviewed surface.
2. **AST-AC02 — Ownership and origins.** Construct and retain two independent
   trees, interleave traversals, release their requests and preserve valid owned
   results. Check original bytes, Unicode spans, trivia/source access, synthetic
   and recovered origins, and rejection of foreign or stale node handles.
   Retain measured storage/copy behavior and named resource-limit failures.
3. **AST-AC03 — Common algorithms.** Iterative walks, field/list lookup, search,
   folds and checked reconstruction preserve order and origins on empty, deep,
   wide and malformed trees. A failed pass leaves its input usable and publishes
   no partial result. Test exact budgets and equivalent opt/noopt execution on
   both VMs; derive bounds for the admitted traversal profiles.
4. **AST-AC04 — Analysis and rewrite contracts.** Validate before/after schemas
   and invalidate derived facts after a relevant change. Execute a structural
   rewrite with stated pre/postconditions, an acyclic attribute calculation and
   a restricted monotone cyclic analysis against independent controls. Cycles,
   unsupported operations and resource exhaustion have distinct diagnostics;
   only proved profiles carry termination/equivalence claims.
5. **AST-AC05 — Backend and application independence.** Equivalent admitted
   unambiguous fixtures produce the same application AST across the current
   applicable backends. Forests need explicit resolution; recovered input
   remains marked and cannot execute as a clean RexxScript program. Migrate
   settings, calculator and one bounded RexxScript slice, preserving outputs,
   errors and control-flow behavior while removing positional tree plumbing.
6. **AST-AC06 — Product continuity and migration evidence.** The supported AST
   library builds through Level B, L0/L1/L2 still agree, and combined/installed
   program builds pass. Update the progressive manual and one compiler-shaped
   builder/validator example. Record the remaining cREXX AST/proof migration
   work without claiming a production compiler migration.

**Numbered implementation steps:**

1. **AST-STEP01 (AC01–06):** inventory the product and research tree contracts;
   present small before/after surface examples, ownership rules and diagnostics.
   **Verified:** Adrian approved the calculator walkthrough and attached
   `then tree` form before requesting implementation. See decisions above.
2. **AST-STEP02 (AC01–03/06):** implement the reviewed schema, owned storage,
   builders, fields/lists and provenance, retaining existing parse views.
   Depends on STEP01; qualify settings and calculator first.
3. **AST-STEP03 (AC02–04):** add iterative traversal/folds, validation and pure
   reconstruction; retain independent controls and explicit bounds. Depends on
   STEP02; no unrestricted rewrite engine is implied.
4. **AST-STEP04 (AC04/05):** promote the admitted analysis schedules and one
   grouping/lowering pass; adapt a bounded RexxScript consumer. Depends on
   STEP03; compare with the existing consumer before removing duplicate code.
5. **AST-STEP05 (AC01–06):** qualify backend equivalence, failure/lifecycle,
   bootstrap and installed examples; update the manual and migration map.
   Depends on STEP02–04; report criteria separately from wider research closure.

**Design references:** the [LLVM AST tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl02.html)
illustrates an application-oriented syntax tree. [MLIR's rewrite contracts](https://mlir.llvm.org/docs/PatternRewriter/)
separate pattern matching, mutation and traversal/scheduling, and distinguish
bounded recursion from general repeated rewriting. These are reference designs,
not dependencies or proofs for Level L. Reuse and requalify the local `llgroup`
and `llschedule` research controls where their restricted contracts fit.

### Develop CI repair — approved 12 October 2026

**Vision:** make the published Level L checkpoint build and check out on all
maintained platforms, then verify the normal `develop` workflows are green.
This repairs the approved implementation without changing the language surface.

**Acceptance criteria:**

1. **CI-AC01 — Portable checkout.** Remove Windows device names from tracked
   evidence and generated comment fixtures, preserving their bytes. A permanent
   path guard runs in the existing publication safeguards; Windows checkout
   succeeds in the resulting hosted run.
2. **CI-AC02 — Native linkage.** The modern RexxScript runner links its static
   providers before their RXPA implementation, as the existing `crexx` runner
   does. Local product and runner checks pass; Linux Release and CodeQL builds
   confirm GNU linker closure.
3. **CI-AC03 — Green publication.** Commit the repair and evidence together,
   push to `develop`, and inspect the resulting Build CREXX and CodeQL runs.
   Diagnose remaining failures; retry service failures when the logs establish
   that cause. Report actual green results in the conversation, with no
   status-only follow-up commit.

**Steps:**

1. **CI-STEP01 (CI-AC01/02):** retain failed-job evidence and identify causes.
   **Verified:** `bb973735f` Build run `38298866712` fails Windows checkout on
   `nul.txt` and Linux linkage on RXPA symbols. CodeQL `38298866314` hits the
   same linkage failure. macOS ARM product/QA/package checks pass; Apple accepts
   the installer but ticket retrieval loses its network connection (Error 68).
2. **CI-STEP02 (CI-AC01/02):** repair fixture paths and runner link order, add
   the path regression, and run focused local checks. **Locally verified:**
   product build, four CTests, native runner checks, 586 comment-panel
   observations, Windows/path and package guards pass. All renamed input bytes
   and 3,169 retained panel hashes match. See [the repair receipt](evidence/develop-ci-20261012/README.md).
3. **CI-STEP03 (CI-AC03):** publish the qualified repair and follow normal
   hosted CI to completion. **The enclosing commit is the approved repair
   payload.** Hosted Windows/GNU link and final green-workflow evidence follows
   publication and is reported in the conversation. Depends on STEP02.

### Develop publication — approved 12 October 2026

**Vision:** publish the implemented Level L toolkit, modern RexxScript consumer
and reviewed manual on `develop`. A clean product build must deliver both new
commands, normal QA must prepare their generated inputs, and an installed
consumer must generate and execute a parser without using build-tree tools.
This integrates the supported checkpoint; the wider LANG/research obligations
below remain open. Adrian explicitly authorizes integration and publication
after the following gates pass, preserving unrelated checkouts and work.

**Acceptance criteria:**

1. **PUB-AC01 — Product closure.** `stage-product` builds `levell`,
   `rexxscript1` and their libraries from a fresh build directory. Inspect the
   generated dependency graph and retain the successful product build log.
2. **PUB-AC02 — QA closure.** Standard QA preparation builds the generated
   Level L and modern RexxScript test images and concrete VM prerequisites.
   Existing tests execute in the smoke/correctness tiers on the combined tree.
3. **PUB-AC03 — Integration correctness.** Commit the reviewed manual/examples,
   integrate current `develop`, and pass product smoke plus the affected Level L,
   legacy/modern RexxScript, compiler-exit, RXPP and Unicode correctness tests.
   Reuse unchanged deeper backend evidence with explicit input boundaries.
4. **PUB-AC04 — Installed tools.** In an isolated install prefix, generate a
   grammar, compile/assemble/link its consumer and run it on available concrete
   VMs. Run modern RexxScript with Unicode, expressions and BIFs; check syntax
   failure prevents execution. Retain permanent package smoke coverage.
5. **PUB-AC05 — Reviewable publication.** Include receipts and current plan in
   the substantive publication, push the qualified descendant to `develop`
   without force, and verify the remote commit. Normal automatic publication
   CI follows; no new release tag or full-programme completion is claimed.

**Steps:**

1. **PUB-STEP01 (PUB-AC01/02/04):** repair stage and QA dependencies; add
   installed-consumer checks. **Verified.** No language-design change.
2. **PUB-STEP02 (PUB-AC03):** commit the manual and merge current `develop`
   into the authorized `levell` checkout. **Verified:** manual `fb4972050`, clean
   merge `2121969ce` includes `a415090dc`. Depends on STEP01.
3. **PUB-STEP03 (PUB-AC01–04):** fresh product/QA build, relevant correctness
   runs and installed checks; fix reproduced integration defects. **Verified**
   by [the integration receipt](evidence/develop-integration-20261012/README.md).
4. **PUB-STEP04 (PUB-AC05):** record exact inputs/results, commit and publish
   only after STEP03 passes; verify remote state. **Local publication payload
   complete; approved delivery is the fast-forward push of the enclosing commit
   to `develop`.** Remote identity and actual CI state are reported in the
   conversation, without a status-only follow-up commit.

**PUB-AC01–04 verified:** fresh product and QA dependency closures, byte-identical
L1/L2 generated RXAS, 249/249 normal correctness tests, six backend/diagnostic/
recovery/comment/Unicode panels and 24 installed-product commands pass. The
initial install attempt needed the optional example artifacts; preparing the
same targets as the publication workflow resolved it without a product edit.
The reviewed guide has 80 valid links. Adrian additionally requested a PDF while
QA ran; the 46-page local artifact was rendered and checked, with embedded
Unicode fonts. The receipt records exact inputs, outputs and qualification
boundaries. Wider research and full LANG criteria remain open.

Readiness inspection found missing product dependencies for both new commands
and missing QA preparation dependencies for their test images. The committed
tips merge cleanly; current `develop` is `a415090dc379c79ed62ef31298d183d93be7cc91`.
The latest registered sanitizer findings are closed. No new sanitizer finding
has been observed in this work; ordinary publication/overnight policy applies.

### User-manual editorial review — 12 October 2026

**Vision:** the Level L user guide is the enduring first reference for a
programmer learning language tools. It teaches the source-to-result process,
provides runnable examples, and states the implemented syntax and API in clear
English. History, qualification receipts, decisions and future work belong in
the existing planning and implementation documents. This is a documentation
review of the approved language, not a change to its semantics or wider scope.

**Acceptance criteria:**

1. **DOC-AC01 — Learning sequence.** Explain lexer, token, grammar, parser and
   tree before relying on those terms; put the first runnable example before
   advanced backend/composition material. Verify by reading the whole guide.
2. **DOC-AC02 — Accurate reference.** Check declarations, defaults, tree/API
   contracts, all request outcomes and resource settings against owning source.
   Retain practical restrictions with the relevant operation, and explain what
   the reader should do. Check links and execute changed tutorial commands.
3. **DOC-AC03 — Enduring prose.** Remove dated delivery/QA/roadmap statements,
   repetitive qualification provisos and unexplained specialist vocabulary.
   Preserve needed usage constraints without making claims beyond the code.
4. **DOC-AC04 — Reviewable closeout.** Record corrected documentation errors,
   validation and remaining product scope here. Production code and retained
   implementation qualification remain unchanged unless a separately justified
   repair becomes necessary.

**Steps:**

1. **DOC-STEP01 (DOC-AC01–03):** audit the current guide and owning code, then
   arrange introduction, tutorial, progressive examples and reference. **Verified.**
2. **DOC-STEP02 (DOC-AC01–03):** rewrite the manual and fill reference gaps. **Verified.**
   Adrian additionally asks for practical first-use cases for each backend.
   Include a runnable PEG configuration-value example, using existing syntax,
   with priority, reversed-priority and CFG controls in the focused teaching
   panel. This changes examples/tests, not the Level L language or runtime.
3. **DOC-STEP03 (DOC-AC02–04):** check internal links, current source contracts
   and executable instructions; record the results and present the revised guide.
   **Verified** by [the manual-review receipt](evidence/manual-review-20261012/README.md).

**DOC-AC01–04 verified for this editorial increment.** The guide now leads from
tokens/grammar/trees to a complete scalar-UTF-8 settings tutorial, then grows
that example and supplies a separate declaration/API reference and glossary.
Practical first projects explain DFA, LL(1), LR(1), PEG, Pratt and Earley. The
new PEG configuration-value lesson demonstrates deliberate priority, its
reversal, and the corresponding CFG ambiguity. The manual contains no dated
qualification diary, acceptance ledger, roadmap or repeated proof caveats.
Needed restrictions remain beside the affected operation.

Correctness review added the missing strict-request status 5, the separate
recovery statuses/API, token-boundary forest coordinates, runtime name-lookup
case behavior, exact charset subtraction precedence, and the two-token meaning
of `unless next`. It documents the basic example's actual multiline strings,
Pratt message restrictions, projection child counts and both scanning uses of
`scan.work`. Adjacent tool notes now refer to the complete current resource
reference instead of stale NFA/DFA defaults and a six-setting policy.

Validation: 80 guide links resolve; the complete printed specification matches
the checked-in basic example; all 18 printed defaults match `levell --limits`.
Three copied tutorial command sequences pass, with matching output on both VMs.
The projection example passes both VMs. The new focused `--part priorities`
panel passes 72 runtime observations and two expected LL/LR build refusals
through source/RXAS and optimized/unoptimized generated consumers, on both VMs.
The product `levell_cli` build passes. Production generator/runtime code is
unchanged; earlier implementation evidence remains applicable. The documentation/example update has been reviewed and approved for the
`develop` integration recorded above.

The complete LANG and research acceptance criteria below remain unchanged.

**Overall status: expanded programme approved by Adrian on 10 October 2026;
implementation in progress.** The
initial pilot below is complete. The new strategy, mathematical work and
implementation programme in the second part of this document are now authorized
and are not covered by that completion claim. This file remains the single
authoritative scope and acceptance record.

### Remote safety checkpoint — 11 October 2026

Adrian has requested a local commit and remote backup on the existing `levell`
branch. This checkpoint includes the implementation, worked examples, guides,
research sources, mathematical notes and retained qualification evidence, plus
the corresponding Level L entry in `performance/ROADMAP.md`. The intended
destination is `origin/levell` in `adesutherland/CREXX`; merge to `develop` and
release remain subject to the previously agreed end-of-work review.

Before committing, all 80 source hashes and 29 artifact hashes in
`evidence/common-surface/host-budget-inputs.json` were checked against the
checkout and matched. The retained functional results below therefore remain
applicable; this safety checkpoint does not rerun unchanged qualification or
claim hosted, cross-platform or sanitizer results. Build directories, downloaded
reference repositories, bytecode and Python caches remain outside the commit.
Recorded textual generated outputs, binary test inputs, raw logs and historical
failed attempts are deliberately retained with their receipts and dispositions.
Git text conversion is disabled for retained evidence so CR/CRLF inputs and raw
observations keep their original bytes. The staged whitespace check identifies
raw-log/fixture whitespace and six pre-existing source/test whitespace findings;
these are preserved with the qualified input snapshot, not runtime failures.
The private inbox URL has been removed from the source audit; pinned public
repository references retain the research provenance.

Expected delivery is one source-and-evidence commit pushed to `origin/levell`.
The conversation will report the actual commit and remote verification; no
status-only follow-up commit is needed. No acceptance criterion is closed by
this backup. Earlier dated statements that work was unpublished describe their
historical checkpoint, before this explicit remote-backup authorization.

### Continuation authorized — 12 October 2026

Adrian directs continuation on `levell`: finish clean user-facing resource
limits, then Unicode, diagnostic reporting and resynchronization, with additional
regressions across backends and synchronized guide/plan updates. The complete
vision and LANG-AC01–14 below remain unchanged. This increment serves
LANG-STEP02/04/05/07/08 and does not close the broader programme by itself.

1. **CONT-AC01 (LANG-AC05/10/11/13):** every remaining construction or recovery
   safeguard is a connected optional host budget or a named, checked format/
   stack ceiling. Tiny budgets produce resource outcomes, raised successful
   budgets preserve artifacts, loaders accept valid artifacts independently of
   build defaults, and overflow fails before index/allocation arithmetic.
2. **CONT-AC02 (LANG-AC03/11/12):** compile pinned Unicode 17 property sets using
   Level B inputs without Level G; exercise identifiers and scalar boundaries
   against independently read raw data, retaining exact source-byte spans.
   Comparison-key and display policies retain their full LANG-AC03 obligations.
3. **CONT-AC03 (LANG-AC06/07/08/14):** regress diagnostic category, expected
   tokens, authored context, byte/line coordinates and later valid structure
   across admitted backends; keep resource failure distinct from syntax,
   recovery and ambiguity, and preserve no-execution/last-good-output contracts.
4. **CONT-AC04 (LANG-AC12/13):** retain exact test inputs/tool identities and
   update runnable examples, guide and this plan with measured scope and gaps.

1. **CONT-STEP01 (CONT-AC01/04):** inventory fixed limits and connect remaining
   construction/recovery settings; preserve safe stack/format ceilings, add
   loader/overflow/budget regressions and qualify the Level B bootstrap.
2. **CONT-STEP02 (CONT-AC02/04):** after STEP01, generate pinned property ranges,
   add optional charset property references, extend the same settings example
   and qualify scalar/data/build provenance without a Level G cycle.
3. **CONT-STEP03 (CONT-AC03/04):** after the relevant STEP01/02 services, improve
   accurate contextual/Unicode diagnostics and grammar/target resync; add
   backend-specific regressions and retain failures honestly.
4. **CONT-STEP04 (CONT-AC01–04):** reconcile acceptance evidence and update the
   guide and status. New language semantics beyond approved LANG-D03–10 still
   require maintainer review; routine lowering and test details are authorized.

All CONT criteria start open. Existing evidence is reused only for unchanged
inputs. No new aggregate CTest is registered by this increment; focused scripts
provide the additional qualification without expanding broad-suite scheduling.

**12 October checkpoint — verified native increment:** CONT-STEP01–04 and
CONT-AC01–04 are verified for their stated bounded continuation scope. The
complete LANG-AC01–14 and original seven research criteria remain open.

| Continuation criterion | Current evidence and boundary |
|---|---|
| CONT-AC01 | Eighteen connected host settings, discoverable defaults/ceilings, checked dynamic LR item membership, fixed stack/format messages and independent loaders. `resource-cleanup/limits-fixed` and `limits-noopt`: 35 commands each plus 24 boundary/loader checks each; 114 request cases + seven build controls in each complete library mode. Defaults remain engineering safeguards; global memory/time and workload calibration are not established. |
| CONT-AC02 | Level B compiles 45 pinned Unicode 17 sets from three checksummed raw files. Independent raw-data comparison passes four native cells per library mode. XID lexer: 12,523 inputs in eight cells, including every interval edge/neighbor, random scalars, NUL and byte spans; wrong-profile/unknown-property refusals. The seventh settings lesson executes source/RXAS on both VMs. No normalization/folding key or terminal-cell service is claimed. |
| CONT-AC03 | 78 located/message/speculation cases per complete library mode across all six parser routes. 160 record-recovery cases per mode, including malformed UTF-8 and source ordering. Eight origin checks retain original bytes and associate sorted diagnostics with error nodes. L1/L2 declaration candidates: 24 cases + twelve last-good-output checks per library mode across quote/comment/group damage. Strict parsing and execution remain strict; nested/token-stage recovery and unclosed target comments retain their stated limits. |
| CONT-AC04 | User guide, diagnostic assessment, mathematics and progressive example updated. `evidence/continuation-20261012/` records inputs and logs; current Level B-only cold/no-op property bootstrap runs three cases with zero re2c calls and no Level G source tree. Exact L1/L2 and opt/no-opt self-description agree. Current consumer checks, six native CLI controls and unoptimized 60-case interpreter cells pass. |

The constructor now uses binary membership masks, queue closure and sorted
sparse state keys. The full worked XID language has 2,993 DFA states; the 4,096
DFA default admits it without extra settings. The independent XID probe has
16,127 NFA states and 2,954 DFA states. The 65,536 NFA/lexical-clause defaults
accommodate these larger lexical graphs separately from dense parser analysis.
MATH.md records the preservation argument and remaining construction costs;
this is not a formal speed verdict.

**CONT-F01, repaired:** the regular builder overwrote NFA exhaustion with
"no lexical rules". Named-budget refusal regression is permanent.
**CONT-F02, repaired:** stale linked RexxScript drivers/tests supplied an obsolete
four-argument diagnostic prototype during native packing. Module and driver
imports now stage exact current dependencies. The permanent import-isolation
probe leaves an obsolete export beside outputs, forces consumer compilation,
and passes native packing plus both interpreter consumers. Generated Level L
consumer tests use the same exact-import discipline.

Earlier compiler warnings and a scalar-in-pattern test-fixture error are retained
as intermediate failures, not passes. Final optimized/unoptimized builds are
warning-free. After replacing a shadowed seed-array declaration with an explicit
empty-value reset, all 32 retained coordinate/recovery/property artifacts
regenerate byte-identically. Runtime sources did not change, so their retained
execution evidence remains valid for those artifacts. No source/test input is
silently normalized merely to qualify it.

**Delivery checkpoint:** source, guides and qualification receipts are prepared
for a further safety commit on `origin/levell`, within Adrian's authorized
branch-review scope. Actual local/remote commit verification is reported in the
conversation. Integration and release remain separate; no acceptance criterion
is closed merely by pushing this checkpoint.

### Teaching examples and comment/migration review — 12 October 2026

Adrian requests a commit and clearer backend selection, success/error-path
explanations, an end-to-end calculator, generalized comments, and a review of
handwritten RexxScript code. The preceding continuation is already committed
and remotely backed up as `13041b644`. This extension preserves the complete
vision and all open original/LANG obligations; examples do not close them.

**Vision:** a reader can choose a backend by intended language semantics,
understand a selected tree versus a forest, and build a complete calculator
whose visitor supplies meaning. RexxScript is explained as the larger example
of that same pattern. Comment handling should eventually be a shared,
source-preserving facility for configurable line, nonnested block and nested
block forms, with explicit quoting, delimiter and newline policies. General
stages should replace consumer syntax mechanics when their contracts are
implemented and equivalent; application semantics and required legacy paths
must retain their owners.

1. **EX-AC01 (LANG-AC02/03/09/13):** inventory implemented comment behavior and
   record a concrete generalized-comment proposal with boundaries, admission,
   diagnostics and tests. New spellings/semantics require Adrian's decision;
   proposal preparation is not implementation or acceptance of that feature.
2. **EX-AC02 (LANG-AC01/05/07/12/13/14):** runnable small grammars demonstrate
   predictive admission, left recursion, common-prefix/ordered choice and
   genuinely ambiguous arithmetic. Verify admitted/refused engines, complete
   input, forest multiplicity and honest error messages on both VMs and
   optimized/unoptimized consumer builds. Explain that a failed parse path
   is not itself a user error, and a conflict is not proof of ambiguity.
3. **EX-AC03 (LANG-AC04/07/12/13):** a complete calculator includes grammar,
   generated module, Level B tree evaluator, input, build instructions and
   expected results. Test precedence, associativity, grouping, decimals,
   alternative comment delimiters, malformed input and located division by
   zero. Parsing must succeed before evaluation; invalid input emits no result.
4. **EX-AC04 (LANG-AC09/12/13):** reconcile modern and legacy RexxScript guides
   against their sources; inventory syntax mechanics that can migrate and
   semantics that remain handwritten. No legacy removal or forced Pratt
   conversion without consumer/semantic proof and maintainer direction.

1. **EX-STEP01 (EX-AC01/04):** inspect source and prepare the comment/migration
   review. Record remaining decisions before changing language architecture.
2. **EX-STEP02 (EX-AC02):** add backend selection examples/probe and guide
   walkthrough using existing syntax. No automatic fallback or new resolver.
3. **EX-STEP03 (EX-AC03):** implement the calculator using existing operator,
   lexer/comment and source-tree APIs; qualify its complete toolchain.
4. **EX-STEP04 (EX-AC02–04):** qualify focused teaching regressions, retain
   exact inputs, correct guide drift and commit the concrete increment.
5. **EX-STEP05 (EX-AC01):** after approval of the concrete generalized-comment
   contract, implement it in the common model/runtime/emitters/bootstrap and
   qualify quoting, prefixes, nesting, newline and recovery behavior across
   admitted backends. Pending design approval; no implication that teaching
   example completion satisfies this step.

**Current status:** EX-STEP01–04 delivered for review; EX-AC02–04 verified in
the bounded native increment. EX-AC01's inventory/concrete proposal is ready,
The generalized-comment design was subsequently approved and EX-STEP05
is tracked to closure by COM-AC01–05 below.
SURFACE.md records the proposed `line`, `block`, `nested`, optional shielding
pattern and newline contracts. At that earlier checkpoint, the one-pair frontend incompletely rejected
duplicate declarations. The generalized-comment increment below repairs this
comment-specific gap; the wider LANG-AC02/05 obligations remain open.

`evidence/teaching-20261012/` retains 808 runtime checks and 16 expected build
refusals across complete opt/no-opt Level L builds and both VMs, with source/RXAS
calculator emission and both consumer compiler modes. A separate seven-command
guide replay prints exactly `RESULT|11` on both VMs. All checks are warning-free;
local links/fences pass, and all 64 checked prior core-source/artifact identities
match. No aggregate CTest is added/expanded, product core is unchanged, and its
qualified evidence is reused. This increment adds six backend grammars, an
end-to-end calculator and modern/legacy/migration guidance; it does not close
full LANG or original research obligations. Source/examples/evidence are
prepared for the requested commit and existing authorized remote safety branch.

### Generalized comments implementation — approved 12 October 2026

Adrian approves the complete generalized-comment contract in SURFACE.md and
instructs implementation, simple defaults, backend regressions and updated
plan/guide. This opens EX-STEP05; no further approval of the same contract is
required. Preserve the programme vision and all wider open criteria above.

**Vision:** one Level B source coordinator implements named line, block and
nested forms for every generated backend. Several forms compose coherently,
quote shielding is explicit when advanced patterns are needed, original bytes
and newline policy survive, and unsupported/damaged inputs produce accurate
build/runtime/recovery outcomes. Source and RXAS carry identical checked
metadata. Existing nested declarations and the acyclic L0/L1/L2 bootstrap keep
working. No compiler/assembler re2c migration or RexxScript semantic change.

1. **COM-AC01 (EX-AC01, LANG-AC02/05/13):** the three forms, distinct custom
   delimiters, optional `outside PATTERN` and `newlines keep` execute. Multiple
   forms choose longest opener and reject duplicate openers with related source
   locations. Unknown/empty/invalid/recursive/nullable shielding and unsupported
   qualifiers fail admission; no ignored declaration.
2. **COM-AC02 (LANG-AC03/04/07/10):** a shared coordinator keeps exact original
   bytes/spans, preserves line-comment CR/LF and declared block newlines, honors
   quote/regular-pattern opacity and same-form nesting/close priority. Named
   unterminated diagnostics point to the original opener. UTF-8, source/work
   limits and contract errors remain distinct; scanning progresses or exhausts.
3. **COM-AC03 (LANG-AC08/09/14):** strict/recovery routes use the same comment
   policy; delimiters/separators inside opaque regions cannot trigger spurious
   comments or record recovery. Unclosed blocks never become clean/executable.
   Recovery and later valid structure remain checked within admitted profiles.
4. **COM-AC04 (LANG-AC11/12):** source/RXAS, both VMs and complete opt/no-opt
   Level L builds pass focused independent comment panels. L0/L1/L2 agree,
   current settings/calculator/RexxScript consumers pass, and the required
   Level B-only bootstrap remains independent of Level G/re2c. Reuse unchanged
   broader proof; no new nested aggregate CTest.
5. **COM-AC05 (LANG-AC12/13):** grow the existing worked example, document simple
   and advanced contracts/limits, retain exact test/tool identities and reconcile
   COM/EX progress without closing unrelated LANG/research obligations.

1. **COM-STEP01 (AC01/02):** implement checked common descriptors/coordinator and
   preserve the legacy prepared/definition constructors. Add focused native
   preparation tests before broad integration.
2. **COM-STEP02 (AC01/04):** extend shared lowering/self-grammar, compile shielding
   patterns through the existing regular builder, emit descriptors/packed images
   identically in source/RXAS, and rebuild/verify all bootstrap stages.
3. **COM-STEP03 (AC02/03):** connect strict/recovery/source-author routes and test
   original spans, errors, newline boundaries, speculative quote patterns and
   lowered budgets. Depends on STEP01/02.
4. **COM-STEP04 (AC01–05):** run backend/consumer/optimization panels and focused
   build lifecycle checks, update guide/plan/receipts and preserve remaining
   programme scope. Depends on the relevant preceding implementation steps.

**Current native checkpoint:** COM-STEP01–04, COM-AC01–05 and EX-STEP05 are
verified for the approved comment profile. EX-AC01's proposal/admission scope is verified. The full LANG and original
research programme remain open. `evidence/comments-20261012/` owns the receipts.

- Common descriptors and one coordinator execute all three forms, custom
  delimiters, form-specific regular opacity and optional kept newlines. Duplicate
  openers, empty/equal delimiters, invalid qualifiers and nullable/recursive/
  overlapping outside patterns are refused; no comment declaration is ignored.
- Independent original-byte/mask oracle and generated LL/LR/PEG/Earley, Pratt,
  region and DFA routes pass: 2,300 comment runtime cases, 28 expected build
  refusals, 96 standalone lexer cases and 19 native contract assertions in
  eight optimizer/VM cells. Source/RXAS preserves original token/owned-tree
  data, including Unicode, NUL/control markers, numeric spelling distinction,
  nesting/close priority, quote shielding, recovery containment and exact first
  exhaustion reporting. Target unclosed blocks remain unrecoverable.
- Six L0/L1/L2 × VM cells produce identical new comment artifacts and a failed
  duplicate declaration preserves the prior output. Complete opt/no-opt
  self-description artifacts agree; the earlier frontend's 17 controls × three
  stages pass. Current three-case cold/no-op Level B-only bootstrap has no G
  source tree or re2c invocation. No new aggregate CTest was registered.
- The eighth Unicode/field/region settings lesson passes in eight cells. Current
  generated-language/RexxScript CTests pass on both VMs, the 404-case teaching
  runtime panel and eight refusals pass, and retained unchanged-path Unicode/
  unoptimized interpreter controls are recorded with their exact boundaries.
  Final product/standalone builds are warning-free; guide/source links pass.

**COM-F01, repaired:** RXAS emission reread the frontend's earlier descriptor
value after custom shielding was added to a local value. Pass the completed
checked policy to both emitters; metadata and escaped-quote regressions remain.
**COM-F02, repaired:** optional integer RXAS arguments lacked `REGTP_VAL`, so
`newlines keep` could become its default on some backend routes. Every occupied
literal argument now has explicit call-ABI presence flags. The minimal fix and
cross-backend metadata/consumer regressions are retained.
**COM-F03, repaired:** dispatch continued after shielding work exhaustion and
replaced the first count/location. Stop and retain the first resource outcome;
the unproductive-prefix regression requires budget 100 / lower bound 101.
**COM-F04, repaired:** loose string equality treated `01` and `1` as equal
comment delimiters. Literal identity now uses strict comparison; numeric
spelling controls execute in both emission formats.
**COM-F05, repaired:** unescaped NUL/control delimiters produced invalid RXAS.
The Level L emitter uses supported C-style hexadecimal escapes; source/RXAS
NUL/control/Unicode-marker lexer controls execute. No rxc/rxas C scanner edit.

The failed reproductions and fixture corrections remain dispositioned in the
receipt. All remaining Unicode comparison/display, general nested/token-stage
recovery, broader pipeline/tree contracts, cost/default calibration and original
research/support-reconciliation obligations stay open. Native evidence does not
claim hosted, install/package, Windows/Linux or sanitizer qualification.
Surface implementation restrictions remain explicit admission errors.

### Current step status — latest review

| Original step | Current state |
|---|---|
| STEP-01–07 | Initial pilot complete; AC-01–11 verified. |
| STEP-08 | Approved strategy, semantic/source inventory and corpus established. |
| STEP-09 | Shared analysis, capability contracts and independent controls implemented/tested; final certificate audit open. |
| STEP-10 | Required traditional parser/lexer implementations execute; final portfolio/evidence reconciliation open. |
| STEP-11 | Faithful native Pika/Squirrel/c14 ports and distinct refinements execute; final bounds/cost audit open. |
| STEP-12 | Representative stages, tree migration, useful-language and composite consumers execute; applicability matrix drafted. |
| STEP-13 | Grammar-author and recovery representative gates verified, including held-out lexical damage. |
| STEP-14 | Lifecycle correctness passes in four cells. First timing block is inconclusive; corrected release boundary passes, uncontended serial measurements remain. COST.md and tranches17/18. |
| STEP-15 | REPORT.md/AUDIT.md drafted; final acceptance reconciliation and cost interpretation open. |
| RS-STEP01–02 | Product tool, packed RXAS generation, UTF-8 source services and actual RexxScript grammar implemented and locally verified. |
| RS-STEP03 | Interpreter implemented; historical four-cell evidence retained, and the 60-case panel passes both VMs after develop integration. RS-F01 is closed and RS-AC04 verified for the declared scalar profile. |
| RS-STEP04 | CMake/library/native runner, dual-format artifacts, second language, negative exits and local toolchain checks verified within the documented scope. |
| RS-STEP05 | Local implementation and review guides delivered; qualification limitation and original research obligations remain explicit. |
| GUIDE-STEP01–03 | Comprehensive Level L guide, runnable settings language and diagnostic audit delivered; GUIDE-AC01–03 verified locally. This row records the earlier guide checkpoint; the current recovery increment is below. LL-D01–06 remain open for their full obligations. |
| BOOT-STEP01–05 | Checkpoint 1 implemented and verified locally: Level B foundation, L0/L1/L2 bootstrap, native Unicode lexing and retained consumer/lifecycle evidence. BOOT-AC01–08 verified within the stated native-host scope; unmerged for review. |
| LANG-STEP01 | Portfolio and proposed common-surface direction approved; one settings language is the progressive worked example. |
| LANG-STEP02–08 | In progress: all six components have authored product routes. Shared messages, declaration resync, flat-record recovery, Pratt/LR regions and tree projection have executable increments and bounded evidence below. Captures, eighteen host budgets, pinned property classes, escaped coordinates and lexical/UTF8 recovery have bounded functional evidence under CONT-STEP01–04. Full Unicode comparison/display policies, wider composition/recovery contracts, default calibration and final qualification remain open. |

The original research acceptance audit has 19 verified criteria and seven still open.
Implemented/tested steps do not automatically close every cross-cutting
criterion they serve. Remote branch backup is now authorized as recorded above;
the implementation remains unmerged and the full programme remains open.

### Approved initial backend set and common-surface checkpoint — 11 October

Adrian approves the established-algorithm portfolio and explicitly requires
**LL(1) and Earley initially**, alongside DFA, canonical LR(1), ordinary packrat
PEG and Pratt. This supersedes suggestions to keep LL(1) internal or deliver
Earley only in a later phase. It expands the initial general-toolkit scope;
the existing RexxScript consumer and bootstrap may continue using DFA/LR.
An implementation sequence is not permission to drop any of these six from
the initial delivery. Research controls do not by themselves satisfy product
support. Pika/Squirrel/c14 remain research options; ALL(*) is reserved, and
IELR is possible later LR table compaction rather than an initial requirement.

Adrian authorizes proceeding with the common surface, error reporting and
resynchronization, and requests discussion of the surface, Unicode complexities
and declarative composite approaches. The backend decision is settled. The
concrete new spellings and Unicode/composition policies in
[SURFACE.md](SURFACE.md#common-surface-unicode-and-composition-review--11-october)
are a review proposal, not an assertion that those features already execute.
Implement agreed contracts without re-requesting portfolio approval; obtain the
requested language-design decision before adopting new syntax or semantics.

**Implementation approval, 11 October:** Adrian accepts the proposed easy
`diagnostic rule ... = '...'` form and instructs implementation, keeping the
basic surface simple and growing one worked example through custom messages,
Unicode, recovery and composite parsing. LANG-D03–10 now guide the authorized
implementation; routine spelling/lowering details can be resolved consistently
with that proposal. No further approval of the same contracts is required.
Document actual delivered syntax and keep substantive departures reviewable.
The worked example will remain the settings language, with numbered runnable
increments rather than unrelated demonstrations. LANG-STEP02 is in progress;
all acceptance criteria remain open until their recorded evidence is complete.

**Vision and intended outcome:** a Level B-based language-engineering tool
whose Rexx-like declarations describe source policies, lexical rules, parser
rules, operators, tree construction and checked stages. Each of the six initial
components consumes the appropriate common model and explains its admission
limits. A grammar author can understand why a specification cannot be built;
a target-language user receives located, contextual diagnostics and can inspect
later valid structure after recoverable damage. Recovery preserves original
source, reports damage explicitly and never permits RexxScript execution.
The same infrastructure serves configuration, binary/text formats, embedded
languages and future compiler frontends without depending on their ASTs.
Unicode and composition must preserve the completed acyclic bootstrap and
source-ownership contracts. Packed tables remain shared by reference.

**Adrian's subsequent surface requirement:** keep the ordinary language simple,
with more complicated capabilities available as options and a gentle learning
curve. This is a product requirement, not optional guide polish. A small grammar
must not require the author to understand backend theory, pipeline contracts,
Unicode normalization or a separate tree schema. Advanced capability must be
added to the same grammar in place. The candidate short forms and defaults in
SURFACE.md require deliberate review; simplicity cannot hide semantic changes.
Adrian further requires bespoke, context-aware error messages to be easy in
that beginner surface. A named rule must be able to supply useful wording
without first introducing a custom recovery algorithm, tree schema or callback.
Source location and relevant parsing context are supplied by Level L.

| Component | Initial obligation |
|---|---|
| DFA | Regular byte/Unicode-scalar lexing with declared longest-match/priority, progress and source-span policy. |
| LL(1) | Explicit selectable predictive CFG backend; build-time FIRST/FOLLOW admission and named conflicts. |
| Canonical LR(1) | Deterministic CFG backend with named conflicts, verified expectations and recovery integration. |
| Ordinary packrat PEG | Ordered-choice backend with a declared pure, progressing profile; reject unsupported left recursion and unsupported predicates/actions explicitly. |
| Pratt | Operator-expression specialization, shared operator/AST contracts and checked embedding boundaries. |
| Earley | General CFG recognition and shared forests, explicit ambiguity handling and bounded resource/recovery behavior. Ambiguity is not silently resolved by production order. |

**Decision ledger:**

| ID | Status | Decision / proposal |
|---|---|---|
| LANG-D01 | Approved | All six components above are initial supported-delivery requirements. No Pika/Squirrel dependency and no ALL(*) implementation in this checkpoint. |
| LANG-D02 | Existing contract retained | Rexx-like named declarations; grammar meaning separate from backend choice; no silent CFG/PEG conversion. New common spellings below remain reviewable. |
| LANG-D03 | Approved direction; implementation in progress | Preserve `utf8` scalar, `bytes` and explicit `utf8bytes` compatibility profiles; version property data; separate identifier recognition from normalization/case comparison. No implicit source rewrite or Level G bootstrap dependency. |
| LANG-D04 | Approved direction; implementation in progress | Named captures/fields and list-aware tree construction; explicit forest versus selected-tree results, error nodes and original/composite/synthetic origins. Retain existing views during migration. |
| LANG-D05 | Approved direction; implementation in progress | Named stage pipelines, finite token passes, declared nested regions/parser boundaries and pure tree passes. No inference of arbitrary handwritten fixups or unrestricted stage feedback. |
| LANG-D06 | Approved direction; implementation in progress | One diagnostic/result contract, grammar-owned expectation and synchronization declarations; backend-owned recovery mechanisms. Strict parsing remains available; collection/recovery is explicit, and any error prevents build publication or script execution. |
| LANG-D07 | Approved direction; implementation in progress | Retain `semantics contextfree` / `semantics ordered` with context-dependent `|`; lexer choice retains longest match then priority. Reject wrong-profile features rather than silently reinterpret them. |
| LANG-D08 | Approved direction; implementation in progress | Token grammars use named tokens; any quoted token spelling requires an explicit unique token binding. Never lower such spelling to its numeric byte value. |
| LANG-D09 | Approved; defaults implemented | Simple core with optional advanced declarations and a gentle learning curve. Useful default trees and diagnostics; no mandatory backend, pipeline, schema or Unicode-policy boilerplate for a small grammar. Version and explain defaults; advanced features extend the same file without changing unrelated rules. |
| LANG-D10 | Approved; core spelling implemented | Easy bespoke messages attached to rules or expectation sites, with built-in context placeholders and automatic locations. Richer capture/opener context is optional. Message customization must not alter recognition or require recovery configuration. |

**Numbered acceptance criteria — all open until implementation evidence is retained:**

1. **LANG-AC01 — Six integrated components.** Generate and execute useful
   specifications through DFA, LL(1), LR(1), packrat PEG, Pratt and Earley using
   the product tool/library, not research-only entry points. Record admitted
   profiles, expected refusals, explicit selection and artifact identities.
2. **LANG-AC02 — Consistent common surface.** Implement the reviewed rule,
   operator, token, tree and diagnostic declarations with named source ranges.
   Every accepted declaration affects the artifact or checked metadata; no
   ignored annotation or literal/token-ID reinterpretation. Exercise shared CFG
   examples across LL/LR/Earley and operator examples across applicable engines;
   preserve ordered semantics separately. Covers remaining LL-D01/LL-D03 gaps.
3. **LANG-AC03 — Unicode/source policy.** Preserve scalar and byte contracts,
   malformed-input locations, NUL-as-data and original spans. Implement the
   reviewed property/identifier/comparison/display profile with pinned data
   and no dependency on generated Level G services. Independently test scalar
   boundaries, combining sequences, supplementary characters, case expansions,
   normalization differences, tabs and line endings under each admitted policy.
   Unsupported policies reject explicitly; no silent fallback to ASCII.
4. **LANG-AC04 — Trees, forests and ownership.** Preserve named captures,
   list structure, trivia/source references and agreed application-tree shape
   through lowering and backend changes. Earley exposes ambiguity/forest
   results until an explicit selection is made. Recovered nodes and edits
   retain origins. Verify independent/interleaved requests, release behavior
   and shared packed tables without per-request table copies.
5. **LANG-AC05 — Grammar-author explanation.** Report undefined/duplicate
   declarations, empty tokens, recursion/progress failures, LL and LR conflicts,
   unsupported profile features and construction exhaustion at original rules.
   Include related ranges, contextual wording and bounded witnesses where
   available; an exhausted witness search does not prove unambiguity.
6. **LANG-AC06 — Specification resynchronization.** The proper generated
   frontend collects two independent errors and retains a valid later
   declaration across the agreed quote/comment/group recovery boundaries.
   Suppress consequences of already-invalid declarations. Any error returns
   failure and preserves the last good generated artifact. L0 remains minimal.
7. **LANG-AC07 — Target diagnostics.** Present unexpected input, meaningful
   expectations, construct context and related openers consistently across
   backends. Verify LR expectations against the stack and distinguish exact
   facts from bounded hypotheses. Unify filenames, byte ranges and agreed
   display coordinates. Covers LL-D02, LL-D04 and LL-D06.
8. **LANG-AC08 — Target resynchronization.** All initial components integrate
   the common recovery contract within their admitted profiles, including
   lexical damage before parsing, nested boundaries, EOF and repeated errors.
   Test clean-input invariance, two independent errors plus later structure,
   inserted/skipped origins, bounded progress, exhaustion and no execution of
   recovered RexxScript. Do not require identical repairs across algorithms or
   claim global optimality. Covers LL-D05 together with LANG-AC06.
9. **LANG-AC09 — Declarative composition.** Implement the reviewed surface for
   token-stage pipelines, parser regions and tree passes. Exercise a layout or
   contextual-word stage, Pratt inside an outer parser, an embedded-language
   boundary and a structural tree pass. Verify source/AST projection, delimiter
   ownership, recovery containment and rejected unsafe composition. Forests
   cannot flow into a single-tree consumer without explicit resolution.
10. **LANG-AC10 — Admission and bound honesty.** Shared analysis supplies facts;
    providers own algorithm checks and implementation-specific cost claims.
    Composition additionally checks input units, contexts, progress, result
    multiplicity and boundaries. Retain negative certificate/proof cases and
    explicit unknown/resource outcomes. Keep host resource budgets outside
    grammar meaning, reconcile constructors and loaders with representation
    checks, and retain overflow-safe preflight and measured default rationale.
    Finite fuel is a work limit, not proof
    of correct recovery, successful parsing or a grammar-specific speed bound.
11. **LANG-AC11 — Bootstrap and build continuity.** Retain Level B-only required
    closure, minimal L0, L1/L2 deterministic regeneration, packed RXAS emission,
    clean/no-op/incremental/failure build behavior and Level L's re2c-free
    boundary. New property tables must be generated from pinned raw data using
    the bootstrap foundation. Existing rxc/rxas and their re2c uses remain out
    of scope. Cross-host/target limits remain explicit rather than implied.
12. **LANG-AC12 — Consumer and guide qualification.** Extend the settings,
    RexxScript and Unicode consumers plus independent backend fixtures. Run
    affected rxc/rxas/rxlink and both-VM checks, with opt/no-opt coverage of new
    semantics. Retain exact inputs and limits, reuse unchanged evidence and
    update runnable guides/capability tables. Report this checkpoint separately
    from open research criteria, full frontend migration and publication.
13. **LANG-AC13 — Simple surface and progressive learning.** Deliver one small
    complete grammar with only token/lexer/parser concepts, useful default
    tree output and located errors. Grow that same example through captures,
    expressions, recovery, Unicode policies and composition without a rewrite
    into a different notation. Exact defaults are documented and versioned;
    omitted options never trigger an undocumented semantic change. Review the
    introductory sequence with Adrian before calling the surface settled.
    Runnable before/after examples and targeted grammar-writer feedback are
    evidence; an arbitrary line-count target is not a usability proof.
14. **LANG-AC14 — Easy contextual messages.** The beginner example adds a
    bespoke rule message with a single small declaration and no host callback,
    custom tree or recovery configuration. Retain rendered filename/location,
    unexpected lexeme/EOF and context examples, plus an optional captured-name
    or related-opener example. Reject unknown template fields at generation;
    never invent an unavailable or ambiguous context value. Exercise nested
    rules, PEG alternatives/predicates, Earley alternatives and recovery to
    show speculative failures do not produce misleading final messages.
    Enabling a message changes presentation, not clean acceptance or tree shape.

**Numbered implementation steps:**

1. **LANG-STEP01 (AC01–14):** record the approved portfolio, reconcile existing
   product/research contracts, and present concrete surface/Unicode/composition
   choices around a progressively extended beginner example. Portfolio recorded;
   proposal prepared; new design decisions pending
   the requested discussion. No existing acceptance criterion is closed here.
2. **LANG-STEP02 (AC02–05, AC10–11, AC13–14):** after surface review, establish the
   common versioned declaration model, source/result/diagnostic contracts and
   admission checks; repair ignored annotations and token-literal traps.
3. **LANG-STEP03 (AC01–04, AC10–11):** integrate all six components behind the
   common model, including product LL(1)/Earley and operator lowering. Qualify
   deterministic/shared CFG and ordered examples; expose forest multiplicity.
   Depends on STEP02. Delivery is not complete after any partial backend subset.
4. **LANG-STEP04 (AC03–05, AC07, AC11):** implement reviewed Unicode property,
   source-map and display behavior using Level B/bootstrap-safe inputs. Can
   proceed beside STEP03 after STEP02; no implicit normalization or G cycle.
5. **LANG-STEP05 (AC05–08, AC14):** connect easy contextual authored messages,
   structured explanations and implement
   specification recovery, then whole-pipeline target recovery for every
   admitted backend profile. Depends on STEP02 and the relevant STEP03/04
   services; preserve clean recognition and strict execution/build boundaries.
6. **LANG-STEP06 (AC02, AC04, AC08–10):** implement reviewed pipeline/region/tree
   declarations and backend-specific composition checks. Test equivalent
   unstaged cases and unsafe boundaries, including recovery ownership.
7. **LANG-STEP07 (AC01–14):** qualify backend/Unicode/recovery/composite cases,
   build lifecycle and affected consumers. Measure new aggregate scheduling
   before registering or expanding aggregate CTests, per AGENTS.md; do not add
   redundant broad or performance runs to a documentation/design checkpoint.
8. **LANG-STEP08 (AC12–14):** reconcile every criterion, update user/developer
   guides and retain review evidence. Keep incomplete criteria open and report
   exact native-host limits. Work remains on `levell`, with remote safety backup
   authorized above; integration and release follow the previously agreed
   end-of-work review boundary.

**Implementation in progress, 11 October:** short lexer/parser headers,
explicit backend selection, rule messages and token-literal rejection are in
the seed and proper readers. All six components now have product routes:
DFA, canonical LR(1), LL(1), ordinary PEG, Pratt and Earley. Both packed RXAS
and Level B source emission execute. Earley exposes ambiguous forests and
refuses arbitrary tree selection. A declared Pratt expression region embeds
inside LR with delimiter/virtual-token admission checks.

The settings language grows through six files in `lib/rxfnsl/examples/settings`:
basic grammar, authored messages, explicit Unicode ranges, record recovery,
embedded expressions, then named fields. Named captures also survive the Pratt/LR region in the final worked example. Explicit Unicode ranges are not full XID/property support.
Parser messages currently support `{found}`, `{expected}`, `{rule}` and escaped
braces. Actual LR stack trials determine expected next tokens. Ambiguous/shared
contexts retain generic wording; PEG negative predicates suppress child messages.
Full speculative-context qualification remains open.

Retained local evidence under `evidence/common-surface`:

- `lr-retained.json`: 128 LR observations from an interrupted early panel.
- `backend-panel/receipt.json`: 384 LL/PEG/Earley observations and 21 negative
  stage cases, both VMs and artifact routes, optimized/unoptimized generated
  modules and consumers. This predates subsequent runtime edits.
- `composite-panel/receipt.json`: 96 execution observations and six admission
  refusals: precedence/grouping, standalone Pratt, LR regions, Earley ambiguity,
  shared LR contexts and PEG predicate suppression. Optimized library, both
  VMs and artifact routes, optimized/unoptimized generated modules/consumers.
- `recovery-smoke/receipt.json`: 28 LR recovery observations and two proper
  reader multi-error cases. Broader panels exposed an Earley tree-span defect
  after skipping an EOF-damaged record. Parent spans now include empty child
  positions across original-source gaps; a relinked minimal reproducer retains
  the earlier valid assignment. The repaired optimized and unoptimized panels below pass.
- `tree-noopt/receipt.json`: 64 observations of emitted tree schemas, checked
  arity failures, rejected projection after parse failure, source/origin
  ownership, both artifact routes/VMs and all four general parser backends.
  The complete Level L library was built without optimization for this panel.

`tested-build-inputs.json` identifies the earlier baseline. The no-opt fixture
`build/surface-noopt-host/inputs.json` records its selected runtime and tool
images. The first no-opt recovery rerun used a stale consolidated library after
an intervening base-module rebuild; it is not evidence against the repair.
Rebuild and rerun the consolidated product before claiming the repaired gate.

Proper specification readers now collect syntax errors at lexically proved
declaration boundaries, retaining good later names without producing an
artifact. L0 remains minimal. Explicit `llrecover.request` reports/skips damaged
records and keeps later structure with nonzero status; malformed UTF-8/unclosed
comments and token-stage reset contracts remain limits. `tree` declarations
now configure an explicit source-preserving projection, instead of being ignored.

Named fields use `term as fieldname`, with repeated/optional occurrences and
lookup in the named view. The first self-grammar spelling allowed all keywords
as field names and exceeded the inherited 4096 canonical-state budget. A
temporary identifier-only restriction was removed following Adrian's direction
below. Keyword field names, including `as`, now have a passing 80-case panel
across all four general parser backends, both artifact routes and both VMs
(`keyword-fields/receipt.json`); three lexical-capture refusals also pass.

**Resource-policy direction accepted, 11 October:** Adrian wants a clean user
surface rather than restrictions introduced to fit arbitrary implementation
ceilings. Restore the normal field-name vocabulary; keep grammar meaning and
provider admission independent of host resource policy. Implement one optional
budget object for build/request APIs, with ordinary defaults and clear exhausted
resource/limit/usage reports. Basic grammars require no budget declarations.
Expose advanced build overrides through the tool, not as a dialect change.
Distinguish representable image/index dimensions from defaults: loaders must
accept valid generated images within checked representation limits, not silently
reapply a smaller generator default. Centralize and thread budgets incrementally,
retaining unconverted limits visibly until qualified. Raising a cap must not be
reported as a proof or a calibrated default. This serves LANG-STEP02/07 and
LANG-AC02/05/10/11/13; it adds no algorithm to the approved set.

**Limit audit prompted by Adrian, 11 October:** the 4,096 canonical-state
ceiling is an inherited resource safeguard, not a theoretical LR(1) boundary
or a measured default. The guide now distinguishes numerical resource policy,
representation/index constraints and semantic/provider restrictions. LANG-AC10
also requires reconciling centralized configurable budgets, loader agreement,
overflow-safe preflight estimates and measured/default rationale. Work fuel is
not a global resident-memory or elapsed-time guarantee. The temporary narrower
grammar built 3,732 canonical states, versus 3,707 before captures. That
restriction was withdrawn: the full field-name vocabulary builds 4,291 states
and passes execution tests. No LR-theory limitation justified narrowing it.

Resource-policy integration now restores keyword capture names (including
`as`), rather than imposing the provisional naming restriction. The initial
optional `lllimits.policy` connects `lr.states`, `source.bytes`, `scan.work`,
`parse.work`, `earley.cells` and `tree.entries`; `--limit name=count` is an
advanced host-tool option. Default LR construction allows 8,192 states; image
loading separately checks packed-key dimensions and the reserved accept action.
The restored self-grammar constructs 4,291 states. Optimized and unoptimized
L1/L2 builds agree byte-for-byte. `budget-final/receipt.json` and
`budget-noopt/receipt.json` each pass 114 runtime cases and seven tool controls,
covering all parser routes, recovery budget propagation, exact source-size
boundaries, invalid settings and explicit exhaustion. A larger successful budget
emits identical artifacts; a conflicting grammar still fails LR admission.
Connected exhaustion messages identify the setting, limit and an observed lower
bound where available. Fixed recursive-depth failures are reported separately.

Changing the frontend factory exposed stale metadata from an older linked
stage driver in its binary import directory. Import tracing identified the
old embedded factory despite selection of the new standalone frontend module.
Stage imports now use exact private syntax/driver roots, excluding linked
outputs. The common module metadata was correct; no compiler C edit is made.
`lifecycle-budgets/receipt.json` passes nine isolated build cases: configure,
cold build, no-op, an extended frontend factory signature while all older
linked drivers remain, its settled no-op, grammar edit, runtime edit, invalid
grammar with artifact preservation, and restoration. No re2c invocation or
Level G source tree was available to this build. Its copied fixture predates
the final resource-message and integer-division refinements; private import/build edges are
identical. The current optimized/unoptimized product builds and 17 independent
specification controls across all three stages also pass
(`frontend-budgets/receipt.json`). LANG-AC11 remains open for its wider new
property-data and platform obligations, not because this regression is pending.

Latest repaired runtime evidence: `recovery-fixed/receipt.json` passes all
112 recovery cases and both proper-reader syntax-resync cases. The EOF Earley
span repair is included. `capture-panel/receipt.json` passes 80 field/list/
optional/message cases plus three lexical-capture refusals; the first attempt
caught a test-probe scoping warning, corrected before retaining the pass.
Both panels use the main optimized runtime, both VMs and artifact routes.
They precede the resource-policy integration. Current unoptimized-library
receipts `capture-final-noopt/receipt.json` and
`recovery-final-noopt/receipt.json` pass 80 field cases, three lexical-capture
refusals, four executions of the actual composite/field example, 112 recovery
cases and two specification-resync cases.

The current Unicode adapter retains matching token/record/error observations
across 17 inputs and eight VM/consumer-optimization cells, apart from the already
documented NUL correction (`unicode-budgets/receipt.json`). Both Level L and
RexxScript consumer CTests pass on both VMs, and the rebuilt native interpreter
passes six CLI controls (`rexxscript-cli/receipt.json`). The RexxScript CMake generation path
now explicitly depends on the verified L2 bytecode; it no longer uses a stale
pre-bootstrap tool that happened to exist in an incremental build directory.

Pinned Unicode properties/comparison policies, richer application-node/list
shaping, general pipelines, complete nested/lexical recovery, wider contextual
message controls, wider runtime optimization parity and affected consumer/
lifecycle qualification remain required work for their wider obligations. No C compiler/assembler scanner
has been changed. All work remains local for review.

LANG-AC01–14 and LANG-STEP02–08 remain open for their complete obligations.
Original AC-14, AC-15, AC-18 and AC-22–25 remain open independently. No scope is
removed by delivering these increments. Existing bootstrap/develop evidence is
retained for its original inputs; the expanded checkpoint is local/unmerged.

### Develop integration and numeric parity — 11 October

Adrian requests merging current `develop` into the local `levell` checkout and
testing the decimal/float optimisation repair. The intended outcome is to
retain all local Level L/RexxScript work on the updated toolchain, establish
whether RS-F01 is repaired here, and keep the earlier evidence as historical
rather than silently relabelling it. This integrates upstream; it does not
publish the experimental Level L work or merge it back into `develop`.

1. **SYNC-AC01 — Preserve and integrate.** Record the exact upstream revision,
   preserve tracked and untracked local inputs, merge it into `levell`, and
   inspect any overlap. Verify the unrelated roadmap edit and local source
   inventory remain present. Evidence: merge log and before/after hashes.
2. **SYNC-AC02 — Numeric repair.** Rebuild the native tools from the integrated
   sources. Run the permanent numeric/float contract regressions, the original
   RS-F01 reproducer and the 28-case compiler/RexxScript scalar comparison with
   optimized/unoptimized compilation on both VMs. No remaining optimizer
   disagreement is admitted as a pass.
3. **SYNC-AC03 — Consumer continuity.** Rebuild the Level L three-stage tool,
   verify deterministic stage agreement, and run focused generated-language,
   RexxScript and Unicode normalization consumer checks. Preserve scope limits
   and record actual source/tool identities. Update RS-F01 status only from
   these observations; do not close unrelated research criteria.

1. **SYNC-STEP01 (AC01):** fetch/pin `origin/develop`, retain a local-work
   manifest and tracked patch, integrate upstream and restore/verify changes.
2. **SYNC-STEP02 (AC02):** rebuild tools and consumer prerequisites, then run
   the focused numeric controls and scalar overlap. Depends on STEP01.
3. **SYNC-STEP03 (AC03):** execute affected consumer/stage checks, reconcile
   evidence and update the guide/plan. Depends on STEP02; report unexpected
   regressions before expanding the scope.

**SYNC-STEP01–03 and SYNC-AC01–03 verified locally.** `levell` fast-forwarded
from `91313bcc5` to `54f736804`; tracked changes restored without conflicts and
979 preserved inputs were byte-identical immediately after integration. The
build driver retains both upstream and local changes. Subsequent local edits
are the strict scalar regression check, reproducer comment and status/docs.

| Criterion | Retained result |
|---|---|
| SYNC-AC01 | `evidence/develop-integration/merge.json`: pinned upstream, no conflicts, before/after preservation checks and tracked patch backup; no publication or merge back into develop. |
| SYNC-AC02 | Fresh native tools; both numeric contracts pass (632 tool commands, 264 VM executions). Original reproducer returns 0 in four cells. All 28 scalar cases match RexxScript and optimized/unoptimized Level G on both VMs, with zero accepted disagreements. RS-F01 closed; RS-AC04 verified for the declared profile. |
| SYNC-AC03 | Warning-free consumer build; rebuilt L1/L2 RXAS identical to each other and the prior artifact. Eight consumer CTests pass: generated language on both VMs, RexxScript's 60-case panel on both VMs, four normalization cells. Six native CLI checks pass. Logical generated normalization source unchanged. |

The [integration receipt](evidence/develop-integration/README.md) records exact
inputs, tools, commands and retained limits. The original broad research and
bootstrap receipts remain historical; this focused integration run does not
claim a new full no-opt Level L matrix, release, sanitizer or platform sweep.

### User guide and diagnostic audit — 11 October

Adrian requests a comprehensive, clear user guide and an assessment of error
reporting and resynchronization for both grammar authors and target-language
users. The intended outcome is a usable, runnable guide to the current local
product, with an evidence-backed account of what its diagnostics can and cannot
do. Research recovery engines must remain visibly distinct from the product.
This documentation/example increment does not authorize new recovery syntax,
change language semantics, or close the wider research criteria.

1. **GUIDE-AC01 — Usable guide.** Explain installation/build assumptions,
   complete language authoring, generation/linking, lexer-only and supplied-token
   use, trees/source ownership, finite token stages, limits and troubleshooting.
   Verify its worked example through rxc, rxas, rxlink and both VMs.
2. **GUIDE-AC02 — Diagnostic evidence.** Exercise malformed specifications and
   target inputs, including two independent errors and a valid later statement.
   Record actual messages, exit status and whether processing resumes; inspect
   claimed diagnostic customization against the generated artifact/runtime.
3. **GUIDE-AC03 — Honest recovery assessment.** Reconcile retained research
   measurements with their exact denominators, document current product gaps,
   and recommend a staged improvement path without implying it is implemented.

1. **GUIDE-STEP01 (AC01–02):** inspect the current frontend/runtime; add a
   small independent tutorial language and consumer, plus bounded diagnostic
   probes. Reuse valid earlier product qualification.
2. **GUIDE-STEP02 (AC01–03):** write the owning guide under `lib/rxfnsl/doc/`,
   link existing entry points, and retain a diagnostic/recovery assessment with
   exact evidence. Depends on STEP01; new language-design choices remain for
   review.
3. **GUIDE-STEP03 (AC01–03):** execute the tutorial and diagnostic probes,
   check documentation links/examples, and record criterion outcomes here.

**Current status: GUIDE-STEP01–03 and GUIDE-AC01–03 verified for this local
guide/audit increment.** The owning guide is `lib/rxfnsl/doc/user-guide.md`,
with the assessment and open LL-D01–06 findings in
`lib/rxfnsl/doc/diagnostics.md`. Evidence:

| Criterion | Observable result |
|---|---|
| GUIDE-AC01 | Complete settings specification/reader/input under `lib/rxfnsl/examples`; rxc/rxas/rxlink and both VM runs produce the documented output. Guide covers the authoring/API/ownership/build/limit topics. |
| GUIDE-AC02 | `evidence/user-guide/receipt.json`: 46 bounded command checks, including ten damaged settings inputs on both VMs, grammar failures, two independent errors, inactive custom diagnostic/tree declarations and the actual RexxScript no-execution boundary. No new recovery code was implemented. |
| GUIDE-AC03 | `cases/reconcile-guide-recovery.py` rereads retained results and verifies the lexical output hash against tranche16 provenance. `evidence/user-guide/recovery-counts.json` separates damaged-only retention from clean controls; RECOVERY.md now uses consistent damaged-only denominators. The diagnostic assessment recommends staged improvements with explicit design gates and closure evidence. |

At that earlier guide checkpoint, the tutorial/audit used the then-current
optimized local product tools; it did not claim a new optimizer/sanitizer/platform
matrix. At that point the existing product
code/build/test inputs remain unchanged: all 58 entries in the previous final
source manifest still match. The new guide, tutorial and audit files are
additional inputs outside that manifest. Reuse that earlier product
qualification. The broader research acceptance criteria, RS-F01/RS-AC04 and
new LL-D01–06 findings remain open. All work remains local and unpublished.

**Overview clarification requested by Adrian:** extend GUIDE-AC01 with a
conceptual introduction that explains generation versus use, the declaration
groups, optional token passes, raw/named/application trees, walking versus
transformation, and the exact RexxScript IF/composite pipeline. The intended
outcome is that a new reader can decide which component owns each language
problem before learning the API. Acceptance evidence: inspect real token/tree
output for the settings example and IF attachment, verify explanations against
the owning grammar/program/execution sources, and check guide links. This is
documentation of current behavior, not an architectural change.

1. **GUIDE-STEP04 (GUIDE-AC01):** trace representative source through tokens,
   named tree and application handling; retain bounded inspection output.
2. **GUIDE-STEP05 (GUIDE-AC01):** add an overview before the command tutorial,
   using one running example, pipeline diagrams, the IF derivation, and a
   component-selection checklist; verify it against STEP04.

**Overview extension complete: GUIDE-STEP04–05 verified locally.** The guide
now separates build/use, declaration groups, token passes, grammar categories,
raw/named/application trees and application walks. Its IF explanation uses the
actual closed/open/continued rules and interpreter child positions. The
bounded `cases/overview-inspect.crexx` probe builds without warnings and emits
identical settings token/tree and two nested-IF tree observations on both VMs.
`evidence/overview-guide/` retains the outputs and verification receipt.
Earlier guide/audit evidence remains valid; no recovery or broader programme
criterion is closed by this extension.

### Approved checkpoint: Level B bootstrap and re2c retirement

Adrian's 11 October direction is to establish the final build shape first:
Level L must be based on Level B, preferably without any required final Level G
phase because Level G may itself use Level L. Build a minimal seed frontend,
use it to generate the proper frontend, then regenerate with that frontend.
Existing Unicode workloads must work without re2c. Improved error reporting,
resynchronization and the supported backend portfolio follow this checkpoint.
Adrian subsequently clarified that re2c retirement in this checkpoint applies
only to Level L and its workloads. The existing rxc/rxas/etc. are unchanged
bootstrap prerequisites. Their later migration away from re2c requires a
separate bootstrap strategy that has not yet been developed. This entry records
that agreed boundary and implementation details. Adrian approved implementation
on `levell`, with merge deferred until the end. The implementation and current
verification status are recorded below; historical gaps describe the state
before this checkpoint.

**Vision and intended outcome:** an acyclic, reproducible Level L build from a
small Level B foundation, with its own lexer/parser described in Level L and
verified by regeneration. Level L supplies the language tools needed by later
Unicode/Level G builds; it never requires the completed Level G library to
build itself. Existing lexer contracts and Unicode consumers retain their
behavior while their re2c-dependent generation paths are replaced. The seed is
a deliberately small permanent bootstrap input, not an alternative evolving
product frontend. The fixed Level B machinery includes source/token/tree
services, grammar analysis, DFA/LR construction, emission and the runtime;
self-hosting first replaces the Level L specification frontend, rather than
implying that the Level B compiler or all generator algorithms are themselves
generated from grammar rules.

The three stages use one authoritative lexer/parser specification G:

1. Compile the minimal handwritten Level B frontend and shared Level B
   generator/runtime into **L0**. It must parse the subset actually used by G,
   lower it correctly and stop safely on errors; resynchronization is absent.
2. Run **L0(G)** to produce the generated frontend and assemble/link it with
   the fixed Level B machinery into **L1**. L1 is the first self-hosted tool
   with the complete selected language surface and basic diagnostics.
3. Run **L1(G)** to produce **L2** through the same build pipeline. Compare
   L1 and L2 generated images/metadata and behavior. Require exact
   identity for deterministic artifacts, explicitly accounting for any
   nonsemantic build metadata. L2 becomes the delivered tool for downstream
   builds. Agreement is a reproducibility check, not independent proof of
   correctness; separate fixtures and consumer controls remain necessary.

Use the already selected DFA + canonical LR machinery for this bootstrap.
Its selection is not a decision to ship all research backends. A later optional
Level G facade may depend on the finished Level L core; neither core generation
nor its required CLI may depend on that facade or any generated Level G service.
For cross builds, generation runs using host tools and the target artifacts
must not accidentally be invoked as host executables.

**Baseline gaps inspected before implementation (now addressed below):**

- `lib/rxfnsl/rexx/CMakeLists.txt` currently imports the base `library` and
  Level L modules, but many implementation files say `options levell`.
  Establish an explicit Level B implementation contract and trace transitive
  dependencies, including native packaging and compiler exits; import spelling
  alone is not a clean-build proof.
- `llfront` is still handwritten; there is no current L0/L1/L2 self-host chain.
- The historical authored `gennorm2.levell` at pinned revision
  `9a86fb37c0d0d7b9cd395d676e8a0a3ef45e39b8` uses automatic token IDs,
  `scalar except (...)`, codepoint atoms, EOF and named error actions. The
  current byte-only product frontend cannot serve as an equivalent replacement.
  `lib/rxfnsg/unicode/tools/level_l_gennorm2.crexx` is a retained generated
  scanner, not a newly reproducible native Level L generation path.
- Preserve the established Unicode-scalar meaning of the historical `utf8`
  profile, byte spans, longest match/priority and NUL-as-data boundary. The
  PoC's `utf8` byte-charset interpretation is a compatibility discrepancy to
  resolve explicitly, not silently reinterpret. Choose and document the
  lowering/compatibility plan before migrating existing product specifications.
  The implementation uses sorted scalar intervals and UTF-8 byte-DFA lowering,
  with the proof argument in `lib/rxfnsl/doc/bootstrap.md`. Historical `utf8`
  scalar meaning is preserved; existing PoC byte rules explicitly use
  `utf8bytes`. This needs no Level G Unicode tables or duplicate decoder.
- Keep normalization/casing/grapheme executors on the existing RXVM codepoint
  substrate. Lexer replacement must not import the Level G Unicode facade,
  duplicate the runtime decoder or silently alter Unicode versions/policies.
- Outside this checkpoint, `compiler/CMakeLists.txt` and
  `assembler/CMakeLists.txt` invoke re2c for four C scanners, and
  `cmake/CrexxClassicUnicode.cmake` reads categories from
  `re2c/include/unicode_categories.re`. These existing toolchain dependencies
  remain unchanged; do not claim a repository-wide re2c-free build.

**Confirmed checkpoint boundary:** build the existing C tools in their normal
way, then prove that Level L and the migrated Unicode generation workloads do
not invoke re2c or consume its templates/data as their generator implementation.
Keeping re2c in the repository for rxc/rxas is permitted and expected. Tests must
distinguish prerequisite tool construction from Level L generation itself.
No C-scanner output backend, rxc/rxas migration, C bootstrap seed or production
compiler frontend change is included here. Those require the later, separately
developed strategy identified by Adrian; they are not implicitly the immediate
next step after this checkpoint.

Checkpoint acceptance criteria (retain all outcomes; evidence ledger follows):

1. **BOOT-AC01 — Level B foundation.** Clean-build the required Level L core,
   seed, generator and CLI with no transitive dependency on `rxfnsg` or generated
   Level G services. Retain the actual dependency graph and build trace.
2. **BOOT-AC02 — Three-stage bootstrap.** Build L0, L1 and L2 from the
   authoritative specification, with stage-isolated outputs and checked
   dependency order. The seed has safe bounded failure and enough syntax for G.
3. **BOOT-AC03 — Regeneration agreement.** L1/L2 canonical generated
   artifacts agree, and both tools agree on a positive/negative grammar corpus,
   token/rule/tree contracts and downstream generated languages. State the
   independent correctness controls and their coverage separately.
4. **BOOT-AC04 — Unicode lexer compatibility.** Regenerate and execute the
   historical TinyExpr/gennorm2 workloads natively, preserving observable tokens,
   spans, EOF, basic error identities and semantic records. Test scalar/range
   boundaries, subtraction, multibyte characters, supplementary scalars,
   malformed UTF-8 and embedded NUL with the explicitly agreed source policy.
   Document rather than conceal any necessary compatibility adapter.
5. **BOOT-AC05 — Unicode consumer qualification.** Inventory current Unicode
   generation consumers and replace the affected retained scanner/generation
   paths. Preserve pinned input hashes, generated logical table contents and
   runtime contracts; run the owning conformance gates for changed data/tool
   inputs. Unchanged valid evidence may be reused. No new G-to-L cycle is added.
6. **BOOT-AC06 — re2c retirement boundary.** No re2c invocation, template,
   category-file or other hidden generator dependency remains in Level L or the
   migrated workload generation steps. After building the unchanged prerequisite
   C toolchain, exercise a fresh Level L generation/build with re2c unavailable
   to that phase and retain its command/input trace. Existing C-toolchain uses
   remain allowed and are explicitly outside this criterion.
7. **BOOT-AC07 — Build lifecycle and consumers.** Cold build, no-op rebuild,
   grammar edit and runtime edit produce the intended regeneration/relinking;
   failure preserves the last valid artifact and returns nonzero. Run the
   generated settings and RexxScript consumers on both VMs. Account for host
   generator versus target artifacts in supported build configurations.
8. **BOOT-AC08 — Reviewable delivery.** Update the guide/build documentation,
   record stage identities, Unicode/data provenance and retained checks, and
   report this checkpoint for review. Basic rejection and named lexical actions
   work now; rich explanation, repair and multi-error recovery remain the next
   checkpoint. Original research and any unresolved conformance obligations remain explicit. RS-F01 was subsequently closed by the develop integration recorded above.

Numbered implementation steps, **approved; STEP01–05 implemented and verified locally**:

1. **BOOT-STEP01 (AC01, AC04–06):** freeze the dependency/Unicode workload
   inventory under the confirmed retirement boundary; present the scalar/byte
   compatibility details and Level B seed contracts before implementation.
2. **BOOT-STEP02 (AC01–02):** factor the Level B-only foundation and minimal
   seed, with isolated import roots and stage outputs. Depends on STEP01.
3. **BOOT-STEP03 (AC02–04):** author Level L's lexer/parser specification and
   implement its tree-to-common-IR lowering; generate L1, then L2, and check
   agreement. Add the Unicode lexical capability required by the frozen
   workloads without Level G services. Depends on STEP02.
4. **BOOT-STEP04 (AC04–06):** migrate affected Unicode generation workloads,
   replacing Level L's re2c-generated artifacts and generator-data dependencies
   with the native tool. Preserve independent references and qualification;
   leave existing C scanners and their toolchain dependencies unchanged.
5. **BOOT-STEP05 (AC03, AC05–08):** verify the clean acyclic build, stage
   agreement, incremental/failure behavior and affected consumer conformance;
   update documentation and deliver checkpoint 1 for review.

### Bootstrap implementation and bounded qualification — 11 October

**BOOT-AC01–08 and BOOT-STEP01–05 are verified for this local checkpoint.**
[The checkpoint evidence index](evidence/bootstrap/README.md) records exact
inputs/results, reused evidence and scope limits. This closes the bootstrap
checkpoint, not the overall programme.

The current implementation has a Level B foundation and an isolated seed reader.
The authoritative `lib/rxfnsl/grammar/levell.levell` generates a 50-state DFA and
1,969-state canonical LR parser. A declared finite token stage recognizes
`name :` declaration starts before parsing; ordinary keyword stages preserve the
Rexx-like surface. The shipped library contains the generated reader, with no
seed fallback. A standalone CMake graph imports only existing C tools and the
Level B library, while the normal native CLI packages L2 without `rxvme`/Level G.

The Unicode workload inventory found one retained Level L scanner in the
normalization generator. It is now a small Level B ABI adapter over the native
gennorm2 grammar; the typed-record parser and all normalization/casing/grapheme
executors remain unchanged. Historical TinyExpr is an independent lexer-only
control. `rxc`, `rxas`, re2c and Classic Unicode build inputs are untouched.

| Criterion | Implementation / retained local evidence |
|---|---|
| BOOT-AC01 | Standalone cold build from a source snapshot containing neither Level G nor re2c; exact command trace and tool hashes in `evidence/bootstrap/lifecycle-final/receipt.json`. Native CLI dependency inspection and product build exclude Level G. |
| BOOT-AC02 | Stage-isolated L0/L1/L2 graph; seed expression/set nesting limits and generated frontend; `frontend/receipt.json` records 17 positive/negative cases through all three stages. |
| BOOT-AC03 | L1/L2 generated RXAS are byte-identical; optimized/fully unoptimized bootstrap agreement. Five independent lexical panels also agree across all three generators, with decoded-scalar/regex oracles and both VMs. |
| BOOT-AC04 | `controls/receipt.json` and `noopt-controls/receipt.json`: 2,700 cases per core configuration, four probe/VM cells; scalar boundaries/subtraction, bounded Unicode expressions, TinyExpr and gennorm2. `gennorm2-compat/receipt.json`: 17 owning token/typed-record/error cases in eight old/new × optimizer × VM cells. NUL correction below is explicit. |
| BOOT-AC05 | Level G normalization generation consumes the new image through the adapter; regeneration leaves the logical normalization source unchanged. Four owning Unicode-normalization conformance cells pass. Pinned Unicode 17.0.0 data/manifests and current source identities are retained with checkpoint evidence. |
| BOOT-AC06 | Seven isolated lifecycle phases with a failing re2c executable first in PATH; zero invocations, no Level G/re2c source present. Current C-toolchain dependencies are outside this proof. |
| BOOT-AC07 | Cold/no-op/grammar-edit/runtime-edit/failure/restoration phases pass; failure preserves prior generated artifacts and downstream library. Settings guide/diagnostic checks run both VMs; generated-language and 60-case RexxScript panels pass both VMs. Native host build qualified locally; cross builds reject explicitly and host/target packaging remains unqualified. |
| BOOT-AC08 | Bootstrap guide, authoring guide and diagnostic assessment updated. Current source/tool/stage/Unicode hashes and command receipts are in `evidence/bootstrap/receipt.json`; source/RXAS emission also passes 160 inputs in four cells. All work remains local and unmerged. Rich recovery/backend selection, RS-F01 and the original open research criteria remain separate. |

**Observable compatibility correction:** the legacy gennorm2 lexer accepts NUL
inside a comment as an EOF sentinel. The native scanner correctly treats it as
input and returns the existing invalid-character status at its original byte
position. All other observations in the owning comparison agree, including
accepted-prefix tokens on failure and EOF only on success. No failure is hidden
by removing the NUL case from the comparison.

This checkpoint is macOS ARM64 local functional qualification. It is not a
release, installation, sanitizer or cross-platform result. Seven original
research criteria (AC-14, AC-15, AC-18, AC-22–25) and the complete LL-D01–06
diagnostic work remain open. At this bootstrap checkpoint RS-F01/RS-AC04 were
also open; the later develop integration above closes that compiler blocker.
The local Level L implementation has not been published or merged into develop.

After checkpoint 1, design and implement the shared diagnostic/recovery
contracts for grammar authors and target-language users. The initial supported
portfolio is now decided in LANG-D01 above, including LL(1) and Earley; qualify
all six components against those contracts and retained evidence.
Allow error-node/provenance/result-status representation to be reserved now so
that later recovery does not require replacing the core interfaces, but do not
turn rich recovery into an unstated prerequisite for this first checkpoint.

## Vision and intended outcomes

### Overall programme: common rules, several implementations

Give a cREXX programmer one understandable Rexx-like Level L authoring surface
for lexers, parsers and declared structural transformations, with as much
sharing as their semantics permit. A rule's meaning must survive a change of
algorithm. Backends must explain unsupported constructs at generation time;
they must not silently select a different language, precedence or tree.

**Adrian's scope correction:** this is a general language-engineering toolkit.
The cREXX B/G/C frontends are one demanding example, not the product's purpose
or privileged architecture. Serve lexer-only tools, parsers consuming existing
tokens, compilers/interpreters/transpilers, DSLs, configuration/data formats,
embedded languages and source inspection/transformation. The reusable model
must not depend on cREXX compiler AST classes, globals or runtime semantics.
cREXX is the implementation and authoring environment; languages being
processed need not resemble Rexx.

**Adrian's long-term direction:** cREXX internals, including its AST and the
proofs/invariants around it, should eventually migrate onto the new Level L
infrastructure. Build general source, token/tree, visitor/transformation and
contract/certificate services that can support that migration. cREXX becomes
one consumer, with its dialect/type/runtime policies above the reusable layer.
This research must test migration feasibility and describe the path; the full
production migration is a subsequent programme, not silently included in or
claimed complete by parser PoCs.

Determine, through mathematics and executable cREXX implementations, which
algorithms deserve supported code, which need restricted grammar profiles, and
which should remain experiments. Investigate whether compiler-style staged
parsing and hand-written fixups can be generated or scheduled from declarative
rules, with justified termination, equivalence and cost claims. Include
grammar-author diagnostics as well as errors in the text being parsed.

Adrian clarified that the compiler question concerns **the B/G/C frontends
that create the initial AST**, not a general parsing API for Level C callers.
The end product of this research is an evidence-backed language-tooling
strategy, working comparative implementations and a concrete support recommendation.
The research must cover useful language examples, not just arithmetic speed
tests. Slower parsing may be a good result when it buys useful grammar support,
clearer rules or better recovery. Actual replacement of the production
frontends and final adoption of new language syntax require a later decision.

### Accepted first-product direction — RexxScript, 10 October

Adrian selects RexxScript as the first product consumer. It is an embedded
external-rules/configuration language serving Level G applications: a reduced,
modernized Level G surface, initially without script-defined classes. RexxValue
remains the value substrate. Included expressions should follow Level G rules;
the current RexxScript evaluator is historical evidence, not the new language
specification. Simple classes may be considered later. Level L remains general
language-engineering infrastructure, independent of this consumer.

Adrian subsequently accepts the recommended initial direction with these
explicit qualifications: declarations are unavailable; SIGNAL, GOTO and
INTERPRET are unavailable; host integration is to be confirmed later; use
the Level C BIFs through RexxScript's existing rxfnsc linkage. Adrian explicitly
corrected the earlier Level B reference to Level C and confirmed the existing
connection. This is reuse of the current shared BIF service, not selection or
implementation of a new function library. His item7 refers to
the user-facing host-integration decision, not RS-D07's internal numbering.
All other recommended starting points are accepted as the initial baseline.
No variable/type declaration syntax is introduced: assignment establishes a
script-local name, and reading an undefined name is an error. Initially there
are no script-defined classes or collection-language extensions.

This fixes the first-consumer direction and backend selection while leaving
the explicit host boundary and detailed conformance manifest to be completed.
It neither removes the approved research obligations nor retroactively changes
the frozen old-RexxScript oracle. AC-21's historical gate remains verified for
its original observations. A new product implementation/conformance plan must
follow selection of the decisions below. No production code changes in this
decision pass; STEP-14/15 and their open criteria remain open.

**Historical first-consumer minimum (unchanged for the existing RexxScript
consumer; superseded as the toolkit delivery scope by LANG-D01):** one DFA lexical backend and one
deterministic CFG backend, initially canonical LR(1). Expressions can remain
inside the same LR grammar. Pratt is an optional generated specialization,
admitted only with a proved boundary/tree mapping; it is not required to launch.
Finite-context token passes, a nested-comment counter where required, and pure
source-tree passes supply the composite infrastructure. They do not require a
second general parser or arbitrary fixed-point evaluation. Keep re2c/Lemon as
established comparison/transition tools; the native PoCs still need product
qualification. LALR compaction is optional and must reject merge-only conflicts.
No native IELR project is required for this first consumer.

Ordinary normalized non-left-recursive PEG is a credible later toolkit backend,
not equivalent to the experimental Pika/Squirrel recursive/recovery profiles.
It need not be shipped for this minimum RexxScript implementation. Pika,
Squirrel/c14 and generalized parsing remain research/reference options and are
not first-product dependencies. Do not promise LR admission of the full new
language until its actual grammar is constructed and checked.

### First-product decision ledger — accepted baseline and remaining detail

The baseline recommendations below are accepted, subject to the explicit
qualifications above. Exact grammar examples, BIF signatures/adapters and
product qualification are still implementation/design work; this acceptance
does not claim they already exist. The host registration, exposure, transport
and write-back contract remains expressly undecided. Exact new Level L
declaration spelling still needs the concrete example review described below.

| ID | Decision | Recommended starting point / evidence needed |
|---|---|---|
| RS-D01 | RexxScript consumer backend set | DFA + canonical LR; retain Pratt as optional for this consumer. Confirm grammar admission, named conflicts and source-tree agreement. The broader initial toolkit set is now LANG-D01, including PEG, LL(1), Pratt and Earley. |
| RS-D02 | Exact Level G expression profile | Pin one documented Level G profile, initially NUMERIC_COMMON, including precedence, associativity, comparisons, concatenation and evaluation order. Share a declarative operator contract and differential tests; do not copy current RexxScript's left-to-right evaluator. Test `1+2*3`, unary/power combinations, `/`, `%`, numeric strings and side-effecting operands. Other options require explicit capability selection. |
| RS-D03 | Values and variable rules | Accepted: RexxValue values throughout evaluation and a Level G-compatible operator adapter; fresh script-local state, implicit names established by assignment, undefined-name errors. Variable/type declarations are unavailable. Scalar-only initially; classes/collections are later work. Pin numeric precision, conversions and truth conditions against the selected Level G profile. Host bindings remain pending under RS-D06. |
| RS-D04 | Initial statement/function subset | Accepted baseline: assignments, grouped expressions, IF/THEN/ELSE, selected structured DO forms, LEAVE/ITERATE, calls and RETURN. SIGNAL, GOTO and INTERPRET are explicitly unavailable. Imports/exits and class declarations are outside the initial subset. Exact output/COLLECT behavior and any script-defined function syntax must be specified in the concrete grammar; no implicit addition of declarations or host effects. |
| RS-D05 | Lexical/context profile | Define source encoding, identifier/case policy, comment forms/nesting, line continuation, statement endings and whitespace/abuttal. Recommend the chosen Level G rules with owned UTF-8 source and byte-span diagnostics; native ASCII-only PoCs are insufficient. Context stages must be finite/declared and source-preserving. |
| RS-D06 | Host and library boundary | Host integration expressly deferred: registration, exposed inputs/outputs, retained facade, transport and write-back are not selected. BIF choice settled: Level C BIFs using the existing RexxScript/rxfnsc linkage, shared RexxValue call context, argument-presence flags and error contracts. Retain that connection and adapt new parsed expressions to it; do not introduce a Level B BIF adapter or a duplicate library. Level G expression semantics and Level C function contracts are distinct. Existing function dispatch/state requirements are integration details to verify, not a renewed library-selection decision. |
| RS-D07 | Effects and execution limits | Host-selected capabilities for existing file/time operations; no ambient shell/import dispatch. Fuel/loop/recursion/output/allocation limits and cancellation have distinct results. These are application policies above pure parsing, not evidence that arbitrary called host code is bounded. |
| RS-D08 | Errors and write-back | Accepted: located stable diagnostic codes, source/rule context and a useful expected set; diagnostic recovery never permits execution after syntax errors. Success-only exposed-output commit remains a recommendation within the deferred host contract, not an accepted host API. Function/I/O effects require their own policy. |
| RS-D09 | Execution and reuse | Parse/validate once into an immutable source-mapped tree or small checked instruction plan; execute with fresh per-run state. Choose between tree interpretation and small private instructions from measured lifecycle needs. Avoid making native-code compilation/JIT/full compiler semantics first-release requirements. Cache keys include language/profile, host signature and grammar/runtime versions. |
| RS-D10 | Compatibility and versioning | Modern syntax gets an explicit language/API version; never guess by trying multiple dialects. Preserve the old implementation as a separately selected legacy path only if actual consumers require it. Existing tests must be classified as retained behavior, intentional change, or unsupported feature. |
| RS-D11 | Level L authoring/diagnostic surface | Preserve the accepted Rexx-like declaration family; approve exact parser/pass/operator declaration spelling using a complete RexxScript example. Use explicit semantics and stage contracts; common spelling does not imply interchangeable CFG/PEG choice. Source diagnostics must hide raw Lemon/table internals by default. |
| RS-D12 | Product acceptance and packaging | Select supported hosts, standalone/embedded entry points and first useful scripts. Require rxc/rxas/rxlink/both-VM correctness, Level G overlap conformance, host/error/lifecycle tests, retained limits and corrected measurements. Resolve/reproduce frozen-tool findings as applicable to the selected product toolchain. Research ports alone are not production qualification. |

The accepted baseline is sufficient to develop the grammar, admission proof,
expression conformance fixtures and isolated parser/evaluator PoC without
waiting for host integration. Prepare the exact feature/semantic/BIF matrix and
concrete language examples next. Keep host-dependent work behind the deferred
contract and present any actual semantic gap rather than asking again about
accepted choices. A numbered product implementation/acceptance plan must be
recorded before production edits. This is a decision ledger
within the existing authority, not a competing implementation plan or a claim
that the new RexxScript product has been built.

### Authorized product implementation — Level L and RexxScript 1

Adrian's subsequent instruction to implement the tool, grammar and connected
interpreter authorizes this concrete implementation for review. The accepted
RS-D baseline above applies; no further approval is needed to implement it.
The exact surface is now delivered as executable examples for careful review,
not declared a released language. All work stays local on `levell`.

**Vision:** a reusable Level L command compiles one Rexx-style specification
into checked native DFA and canonical LR tables and source-owned trees. A
RexxScript specification is its first product consumer. A versioned modern
interpreter executes the selected tree with typed RexxValue values, Level G
NUMERIC_COMMON operators, structured control flow and the existing Level C
BIF services. Grammar errors fail the build; script errors are located and
never execute a repaired/damaged tree. Parsing and execution have separate
owned lifetimes. Existing legacy consumers remain explicitly selectable while
the modern implementation is reviewed. Host exposure/registration/write-back
remains the expressly deferred decision; it does not block a standalone runner
or an isolated reusable parsed-program/run API.

Numbered acceptance criteria (current evidence/status follows the steps):

1. **RS-AC01 — Tool and authored source.** A native cREXX Level L command
   accepts the established declaration family, admits regular lexical rules
   and context-free parser rules, and emits deterministic reusable artifacts.
   Test another small language as well as RexxScript; reject unsupported
   constructs, invalid specifications, conflicts and construction exhaustion.
2. **RS-AC02 — Lexer and source.** Native byte DFA handles valid UTF-8 source,
   byte spans, longest match and declared priority; malformed UTF-8 fails with
   an offset. Nested comments and contextual classification have explicit
   stages. Test non-ASCII strings, comments, separators, continuation,
   whitespace/abuttal and damaged lexemes; no ASCII-only success claim.
3. **RS-AC03 — Actual grammar.** The checked-in RexxScript grammar is accepted
   by canonical LR(1), drives runtime parsing and preserves named source tree
   nodes. Explicit modern version and feature manifest cover assignments,
   IF/THEN/ELSE, structured DO, LEAVE/ITERATE, calls, output and RETURN;
   declarations, classes, SIGNAL/GOTO/INTERPRET are rejected.
4. **RS-AC04 — Values and expressions.** Preserve typed RexxValue results,
   implement the documented scalar NUMERIC_COMMON precedence, associativity,
   conversions and left-to-right evaluation, and compare overlap cases with
   the existing Level G compiler. Undefined reads fail; assignment creates
   local names. Non-overlap/unsupported constructs are explicit.
5. **RS-AC05 — Shared BIF integration.** Use existing rxfnsc functions,
   RexxBifCallContext, argument-presence flags and script-local pool/config.
   Exercise nested calls, omitted arguments, typed results and BIF errors.
   Host-dependent services report capability errors until their contract is
   selected; do not silently create a new host API or duplicate the library.
6. **RS-AC06 — Execution and diagnostics.** Parse once, reuse an owned program,
   execute in fresh state; test two independent/interleaved runs, control flow,
   return, missing variables, syntax/BIF errors, fuel/depth/output limits and
   cancellation. Stable codes and source locations accompany failures.
7. **RS-AC07 — Build and review.** Integrate generation/library/runner into
   CMake, run focused rxc/rxas/rxlink checks in optimized/no-opt modes and both
   concrete VMs, retain deterministic generation and negative-exit evidence,
   and document executable examples, compatibility and remaining limitations.
   Existing applicable legacy tests remain valid. No publication is implied.

Implementation steps:

1. **RS-STEP01 (RS-AC01–02):** promote the shared library core to its owning
   rxfnsl directory, keep one maintained source, extend the frontend with the
   accepted lexer declaration family and checked DFA generation. Add reusable
   table serialization/loading and a native generator command.
2. **RS-STEP02 (RS-AC02–04):** author the complete first RexxScript grammar,
   specify lexical/context stages, construct canonical LR tables, resolve
   conflicts through explicit grammar structure and retain rejection examples.
   Depends on RS-STEP01; semantic changes outside RS-D return for review.
3. **RS-STEP03 (RS-AC04–06):** implement typed tree evaluation, structured
   execution, shared Level C BIF dispatch, source diagnostics and execution
   budgets. Keep the host decision isolated. Depends on RS-STEP02.
4. **RS-STEP04 (RS-AC01–07):** connect product generation, library and runner;
   execute conformance/lifecycle/failure fixtures and appropriate existing
   tests with pinned tool provenance. Retain actual failures and limitations.
5. **RS-STEP05 (RS-AC07):** update user/developer guidance, examples and the
   acceptance ledger, and deliver the local implementation for review.

These product criteria supplement AC-01–26; they do not close or remove the
seven open research criteria, including the corrected lifecycle measurements.

### Product acceptance audit — local implementation for review

The implementation is available for review on `levell`; **complete qualification
is not claimed**. Evidence is retained in `evidence/product-rxas/final/`, including
the precise source/tool/artifact hashes, commands and the known compiler finding.
Scope is macOS ARM64 Debug with the existing local toolchain, not a release,
installation, sanitizer or cross-platform qualification.

| Criterion | Current status and retained evidence |
|---|---|
| RS-AC01 | **Verified locally.** Native tool emits deterministic RXAS for RexxScript and a second language. Nine generator cases cover named admission failures, unsupported semantics, construction exhaustion, UTF-8/I/O and atomic preservation of an old output after failure. Packed source/RXAS images match byte-for-byte. |
| RS-AC02 | **Verified for the documented profile.** UTF-8 source/strings, nested comments, spans, layout, continuations and damage are exercised in the 60-case interpreter matrix and CLI negatives. ASCII identifier restrictions and unavailable wider Level G lexical forms remain explicit. |
| RS-AC03 | **Verified.** Actual grammar: 149 DFA states, 1,612 native canonical LR states; independent Bison admits the identical common graph without conflicts (1,613 states including its final EOF state). Structured statements and exclusions execute in the functional matrix. |
| RS-AC04 | **Verified locally for the declared scalar NUMERIC_COMMON profile.** After integration of develop `54f736804`, all 28 compiler/interpreter scalar comparisons agree in optimized/unoptimized modes on both VMs. The original reproducer returns 0 in all four cells; permanent upstream numeric contracts pass. `evidence/develop-integration/` closes RS-F01 without claiming complete Level G compatibility. Historical discrepancy outputs remain intact. |
| RS-AC05 | **Verified for the accepted initial boundary.** Shared rxfnsc bodies, call context, presence flags and fresh pool/config exercised by nested/omitted calls, typed values, VALUE effects, Unicode and BIF failures. Native Unicode/stream/provider dependencies linked. Host-dependent calls fail explicitly; the deferred host contract remains undecided. |
| RS-AC06 | **Verified for the documented synchronous API.** Sixty cases per complete opt/no-opt module build on each VM include owned program reuse/fresh runs, control and early return, parse-before-execute, failure codes/locations, source/depth/fuel/output/value limits and pre-run cancellation. No asynchronous cancellation or universal BIF allocation bound is asserted. |
| RS-AC07 | **Verified local review delivery; RS-F01 subsequently repaired and integrated.** CMake product targets and four current product CTests pass. Six standalone CLI cases pass. Second-language/image/ownership tests pass 4,725 assertions in each of four cells. Source-emitted grammar also runs all 60 interpreter cases on both VMs. Unchanged legacy correctness evidence is retained and reused; product docs/examples describe compatibility and limits. No publication. |

**RS-F01 — compiler scalar folding disagreement, closed after local integration.**
The pre-integration `rexxscript/tests_functional/float-comparison-reproducer.rexx`
printed `0` with `rxc -n` and `1` with optimized rxc, on both VMs. The broader scalar test first
exposed an interpreter adapter error: using Classic RexxValue comparisons for
typed Level G operands rounded a float through decimal text. The adapter is
corrected to typed operations and its regression passes. That original product increment did not change compiler/VM sources. The
subsequent upstream repair is now integrated at `54f736804`; fresh local tools
return `0` in all four reproducer cells, and both permanent upstream numeric
contracts pass. Historical disagreement outputs remain retained. The integrated 28-case scalar
panel and focused consumer checks also pass under SYNC-AC02–03, closing RS-AC04
for the declared profile. The scalar check now rejects every optimizer
disagreement, including recurrence of the original case.

**RS-F02 — native runner incremental dependency, repaired.**
The runner's original CMake dependency ordered library generation but did not
invalidate its embedded bytecode when the library file changed. The runner and
test link commands now depend on actual runtime library files; the rebuild log
and final native behavior retain the repair evidence. Initial missing Unicode,
stream and filesystem provider linkage is also repaired in the native runner.

**Ownership review:** loaded tables retain portable packed images. Private
lookup calls borrow storage through synchronous read-only EXPOSE arguments;
retained optimized assembly shows attribute links/swaps without binary copies
or per-lookup weak-reference construction. The final DFA image is35,012 bytes,
LR image294,972 bytes and LR directory25,792 bytes. Request/program ownership
still copies at public value boundaries, including language-to-request and
table-to-parser initialization. This is bounded per-request initialization
cost, not a claim that tables are shared without copies across all requests.
Further borrowing/shared-definition work needs a clear owner-lifetime contract;
the current API does not let a borrowed object escape its owner.

### Completed initial pilot

Give Adrian executable evidence for a useful Level L language-engineering
toolkit: native lexer and parser generators, an economical re2c integration,
and an honest assessment of Pika parsing, diagnostics, recovery and lex-first
optimization. Mike Beer's eight published language repositories, Prolog and
the current RexxScript runtime supply practical requirements. This is broad
research and runnable proofs, not adoption of a new cREXX grammar or a
production compiler architecture. Keep both positive and negative results.

Work in local branch `levell`, based on `91313bcc5a447e9403f95b1f0bfc4dd9503d0618`
(exact base also recorded in evidence).
Reuse `/Users/adrian/CLionProjects/CREXX-level-l-inline-storage`; preserve its
historical `temp/level-l-inline-storage` branch and the unrelated dirty develop
checkout. No publication is requested. Existing `options levell` and the
`rxfnsl` library are the implementation substrate; no new source language
syntax is implied by an experimental grammar specification.

**Accepted steering, 10 October:** Adrian specifies the Rexx-like surface of
the existing re2c wrapper as the intended Level L authoring style. Reuse the
documented `symbol: kind ...` declarations and named token/charset/pattern/rule
actions from the existing `.levell` bootstrap, retained at Unicode branch commit
`9a86fb37c0d0d7b9cd395d676e8a0a3ef45e39b8`. Numeric `.lex` and PEG files in this
research are internal experiment formats, not the selected authored surface.
Connect the existing frontend AST to a native backend without changing that
syntax. Parser-maker spelling remains a later language-design gate; accepting
the style does not approve arbitrary new declaration kinds or semantics.

## Initial-pilot acceptance criteria

1. **AC-01 — Practical source audit.** Inspect the eight repositories linked
   in Mike's 10 October email; distinguish author claims from observed code,
   record pinned revisions, lexer/parser approaches, diagnostics and reuse
   opportunities. Include Prolog if its source can be located and current
   RexxScript owning sources. Never imply these projects were qualified here.
2. **AC-02 — Native lexer generation.** A generator implemented in Level L
   compiles reusable lexical specifications into executable cREXX scanner
   data/code. Demonstrate longest match, rule priority, progress, spans,
   strings/comments and malformed input. State the supported regex subset.
3. **AC-03 — Native parser generation.** A generator implemented in Level L
   compiles grammar specifications into executable cREXX parser data/code.
   Demonstrate useful expression and statement/format grammars, direct and
   indirect recursion, precedence, full-input checking and rejection cases.
   Precisely bound the supported PEG subset and do not call a reduced engine
   a complete port of the Pika reference implementation.
4. **AC-04 — re2c comparison.** Execute a re2c-backed scanner with the same
   lexical contract; document Level L wrapping versus native generation and
   custom re2c syntax backends, dependencies and replacement limits.
5. **AC-05 — Actual Pika reference.** Pin, inspect, compile and execute the
   linked reference implementation; exercise left recursion, ordered choice,
   malformed input and recovery. If unavailable, retain the concrete failure
   and keep this criterion open.
6. **AC-06 — Useful diagnostics.** Native PoCs report source location,
   unexpected input, contextual expectation and at least one concrete repair
   suggestion; multiple-error fixtures retain valid subsequent statements.
   Differentiate coverage gaps, diagnostic explanation and edit repair.
7. **AC-07 — Mathematics and counterexamples.** Derive lex-first cost and
   memory bounds, break-even and token-boundary equivalence conditions.
   Execute maximal-munch/ordered-choice and contextual-token counterexamples.
   State complexity assumptions and adversarial limitations explicitly.
8. **AC-08 — Measured experiments.** Correctness-gated, reproducible serial
   character/token comparisons retain operation counts and raw timings,
   including lexing cost, short/long identifiers, strings/comments and
   malformed inputs. Label these single-workload research observations,
   not a representative product performance verdict.
9. **AC-09 — Toolchain and reproducibility.** Exercise rxc, rxas, rxlink and
   rxvm; compare optimized/no-opt and both concrete VMs on relevant native
   cases. Retain source/build provenance, commands, outputs and limitations.
10. **AC-10 — Reviewable delivery.** Keep a concise research report, runnable
    commands, acceptance status and a ranked next-experiment list in this
    checkout; show Adrian concrete results and decisions still needing his
    approval. All above criteria must be verified for overall completion.
11. **AC-11 — Existing Rexx-like surface.** Reuse the pinned existing bootstrap
    parser unchanged; execute an authored `.levell` fixture through that AST
    into the native DFA generator and compare token kinds/spans/EOF with the
    established TinyExpr control. Retain original syntax provenance and state
    backend limitations. Document the same surface family for future parser
    tooling without inventing an approved parser syntax.

## Initial-pilot implementation steps

1. **STEP-01 (AC-01, AC-09).** Synchronize selected checkout, record plan and
   source/build provenance; audit owning docs and public repositories.
2. **STEP-02 (AC-02, AC-04, AC-09).** Implement native regex/scanner generator
   and a pinned re2c control; verify common token/span contracts.
3. **STEP-03 (AC-03, AC-05, AC-09).** Implement native grammar generator and
   bounded reverse-DP parser; run the actual Java reference as an independent
   control. Depends on specification contracts from STEP-02.
4. **STEP-04 (AC-06, AC-07).** Add diagnostic/recovery fixtures and formal
   equivalence/cost analysis; test grammar and lexing counterexamples.
5. **STEP-05 (AC-08, AC-09).** Freeze passing experiment sources and run
   bounded serial measurements with raw evidence and exact inputs.
6. **STEP-06 (AC-10).** Reconcile every criterion against evidence, write
   findings and prioritized follow-ons. Production adoption, new cREXX syntax,
   ABI/compiler changes and publication are separate maintainer decisions.
7. **STEP-07 (AC-11, AC-02, AC-04, AC-09).** Add a backend-only adapter from
   the existing validated Level L AST; reproduce the authored TinyExpr proof
   through native DFA and retained direct re2c/cREXX emission. Depends on the
   established syntax and existing backend proofs; do not edit the Unicode
   checkout or reinterpret Unicode input contracts as ASCII contracts.

## Initial-pilot status and evidence

**Initial research delivery complete; STEP-01 through STEP-07 and AC-01 through
AC-11 verified.** This closes the initial research/PoC scope, not the overall Level L
product, a complete Pika port, a production replacement or release qualification.
No publication is selected. Adrian's Rexx-like surface direction is preserved.

| Criterion | Verification and retained evidence |
|---|---|
| AC-01 | SOURCE-AUDIT.md; evidence/mike-sources.json; all eight repository front ends plus owning local Prolog and RexxScript sources inspected. |
| AC-02 | tools/lexgen.crexx and src/lllex/lldfabuild/lldfa; four evidence/native-*-*.out cells cover longest match, priority, all 127 non-NUL ASCII byte probes, spans, trivia, strings and malformed input. |
| AC-03 | tools/parsegen.crexx; expression/statement/array/indirect/character grammars; precedence-tier memo queries, full-input rejection and the mixed-LR limitation/factored fixture in the same four native cells. Scope is the explicitly documented reduced recognizer. |
| AC-04 | evidence/re2c-*.out and surface-*-*.out; both external C transport and established direct cREXX re2c emission executed; README/SURFACE document custom syntax backends and limits. |
| AC-05 | evidence/pika-reference.out; unmodified pinned Java reference compiled and run, including recursion, ordered choice, recovery and source-acknowledged counterexamples. |
| AC-06 | Native output retains line/column, unexpected token, contextual expectations, hints, EOF and two valid later islands; actual Pika memo-table explanation points at both missing constructs. |
| AC-07 | MATH.md; live reference object counts, native rollback transition counts, PEG priority and hard/soft-keyword counterexamples. Recognition-equivalence assumptions and cost bounds are explicit. |
| AC-08 | evidence/measure-{rxtvm,rxbvm}.csv and summary-*.md; one warmup plus six balanced serial pairs on five equivalent fixtures, including full native lex cost. Not a representative/formal product verdict. |
| AC-09 | Native and authored-surface opt/no-opt tests on both VMs; all four product tools exercised. evidence/provenance.json and source-hashes.json identify actual inputs and local platform limits. |
| AC-10 | README.md, SOURCE-AUDIT.md, MATH.md, SURFACE.md and run-*.sh; ranked follow-ons and maintainer decision gates are reviewable. |
| AC-11 | Existing bootstrap and authored TinyExpr source recovered unchanged from pinned Git objects. Native DFA/direct re2c logical kind/span/EOF/diagnostic comparisons pass in four cells; Unicode backend limits reject explicitly. |

The timing image retained SHA-256
`943c2d7aa784ee7d7df8eb00aada1447941c1abd181ed1516fa6abccd3288ff9`
after later functional and surface additions. No timing rerun was needed.
Existing tool binaries carry older dirty build versions; their hashes pin the
actual experiment tools, and no exact-current-head product qualification is
claimed. The requested local branch remains local and preserves the historical
Level L branch and unrelated develop work.

## Expanded programme proposed for approval

### Direction and decision boundaries

1. Keep the established `symbol: kind ...` authoring style. Share names,
   terminals, sequences, repetition, source spans, trees and diagnostics.
   Preserve distinct meanings for regular-expression alternation, PEG ordered
   choice, CFG alternatives and longest lexical match. A common spelling must
   not conceal these distinctions. Propose explicit semantic profiles or
   choice declarations; exact new spelling remains provisional in research.
2. Separate **what the rules mean**, **how they are evaluated**, and **how
   damaged input is explained/recovered**. A grammar is not itself an algorithm.
   A PEG authoring profile does not require Pika as its implementation.
3. Treat algorithm selection as a checked lowering. A requested backend either
   implements the declared semantics or reports a located capability error.
   An optional automatic selection mode may choose among equivalent admitted
   implementations and must explain its choice; it cannot silently approximate.
4. Build an executable common IR and cREXX research engines, not merely wrappers
   around foreign parsers. Use pinned upstream implementations as independent
   controls, retaining licenses and attribution. Separate faithful ports from
   modified algorithms so a repair cannot erase an upstream counterexample.
5. Compare all B/G/C initial-AST strategies behind an algorithm-independent
   frontend contract. Include source-owned structure, trivia/abuttal, dialect,
   diagnostics, ownership and subsequent normalization/lowering obligations.
   Keep compilation/import/type-resolution services distinguishable from
   recognition. Do not infer those services from successful syntax recognition.
   Apply the same consumer contract to other languages. Grammar modules,
   embedding boundaries, token-only ingress, lexer-only egress, source
   preservation and reusable tree visitors/transforms are toolkit concerns;
   adapters to a compiler's particular AST are consumer concerns.
   The eventual direction is to replace those legacy adapters with cREXX use
   of the shared Level L infrastructure, including applicable AST invariants
   and proof obligations. Do not bake today's compiler layout into the shared
   model or leave migration feasibility as an untested aspiration.
6. Stay on local `levell` in the existing checkout. Approval of this programme
   authorizes research implementations, provisional surface examples and local
   comparisons; it does not adopt a production grammar, replace Lemon/re2c,
   change B/G/C syntax or publish the work. A negative result is useful evidence,
   but does not excuse an unimplemented required comparison.

### Source reconciliation performed for this proposal

- The existing `.levell` lexer contract remains pinned as recorded above. Its
  parser declaration examples are extension sketches, not a complete contract.
- `docs/books/crexx_vm_spec/Logical-Grammar-Specification.tex` is an earlier
  review-stage PEG proposal. It contains custom repetition, whitespace,
  abuttal, case and tree conventions. Inventory those requirements and resolve
  discrepancies with standard PEG semantics rather than treating that document
  as an already executable specification or replacing it without approval.
- Current `compiler/docs/parsing_pipeline_anatomy.md`, B/G glue
  `compiler/rxcpbpar.c`, Classic glue `compiler/rxcpcpar.c`, source-tree handling
  and validation orchestration confirm that the present frontends are staged.
  Token normalization, context-dependent token classification, structural tree
  grouping and symbol/type iteration have different responsibilities.
- The main `develop` checkout contains concurrent uncommitted frontend
  extraction work, including `compiler/rxcc.h`. It was inspected read-only.
  Its internal parsed/mapped-unit service is not a completed general compilation
  service. Pin the actual source/build inputs used for later comparisons and
  coordinate with the owning work before any production integration.
- Pika's pilot counterexamples remain retained. Squirrel is pinned for this
  proposal at `bb1ae84701060462636c07c814286ac2875e0568`. Its repository
  manuscript is newer than the arXiv HTML: it acknowledges an adversarial
  quadratic case and specifies greedy seed-growth semantics. The packaged
  `dart/lib/src/parser/parser.dart` performs recognition without active repair;
  `dart/experiments/recovery/c14.dart` is the separately documented standing
  recovery engine. Audit and run these separately. Upstream results are not
  local qualification.
- Fetching origin for this planning update left `origin/develop` and the
  research base at `91313bcc5a447e9403f95b1f0bfc4dd9503d0618`. The pilot's code,
  generated images and measured evidence are unchanged. Its source-hash
  snapshot includes the earlier PLAN/README text; this later planning edit
  does not refresh or misrepresent that historical snapshot.

### Hypotheses and decisive experiments

| ID | Hypothesis | Evidence that could support or refute it |
|---|---|---|
| H-01 | One authoring surface can serve several engines without confusing semantic differences. | The same admitted specification produces equivalent token/tree results on at least two engines; ordered/ unordered choice and unsupported features produce explicit counterexamples or build errors. |
| H-02 | PEG-style rules are easier for cREXX authors on many practical tasks. | Side-by-side Classic keyword, expression, nested structure and damaged-input tasks; compare ability to predict acceptance/tree and repair a rule. Ordered-choice traps and grammar-extension regressions count against PEG. Author preference is not a theorem. |
| H-03 | Much existing parser glue and structural fixup work can be generated. | Replace selected *research copies* of token normalization, contextual classification and tree grouping with declarative rules; compare tokens, source AST and diagnostics. Identify the remaining semantic knowledge explicitly. |
| H-04 | Lexing first can reduce chart/memo work while preserving language and source meaning. | Derive sufficient boundary conditions, check them where possible, then compare scannerless/tokenized recognition and ASTs; include counterexamples involving priorities, keywords, abuttal and predicates crossing boundaries. |
| H-05 | Pika can be useful despite its observed limits. | Faithful native port, independent reference conformance, tested semantic restrictions, sparse scheduling and shared tree experiments; measure useful recovery against complete end-to-end cost and adversarial growth. |
| H-06 | Squirrel is a viable PEG/LR engine with a tractable supported profile. | Run packaged core and c14 separately, port both, compare recursive semantics and tree/recovery results, and audit memo invalidation, expansion overlap and retained objects. A modern manuscript claim is still a claim to test. |
| H-07 | A generated composite can approach deterministic-parser speed without imposing its restrictions on the whole grammar. | Extract proven regular/predictive/operator regions; use an admitted general engine for the rest; prove interface conditions and compare against unspecialized evaluation, including failed optimization attempts. |
| H-08 | Good diagnostics are achievable across algorithms. | Compare located grammar witnesses and source repairs using the same mutation corpus, user-facing rubric and bounded repair oracle. Include LR recovery so Pika/PEG do not receive an artificial advantage. |
| H-09 | Useful automatic complexity certificates are feasible. | A checker derives bounds for declared fragments, rejects false certificates and reports unknown cases honestly; adversarial families stay within every issued bound. |
| H-10 | The shared model serves language engineering beyond compiler parsing. | The same library/IR supports standalone lexing, existing-token parsing, a configuration/data format, an embedded-language example and a tree-based source transformation without importing cREXX compiler internals. |
| H-11 | General Level L infrastructure can eventually own cREXX's AST and its contracts. | A representative builder, traversal and structural rewrite run on shared trees with explicit invariant checks and source provenance, compared through an adapter with the existing compiler representation; document remaining semantic/lifecycle gaps and a staged migration. |

### Proposed implementation portfolio

All required native entries below must execute through the cREXX toolchain.
Generated tables plus a cREXX runtime count as implementation; a foreign
process wrapper alone does not. Each has a documented semantic subset and
negative capability tests. Initial implementations favor clarity over tuning.

| ID | Required native experiment | Role and admission conditions |
|---|---|---|
| ALG-01 | Thompson NFA control, native DFA generator/runtime, and a memoized maximal-munch variant | Extend the existing lexical proof. Regular rules, rule priority, Unicode/binary profiles and captures have explicit contracts; nested comments need a declared stack/counter stage, not a false claim that a DFA handles them. Retain direct re2c emission as an independent backend/control. |
| ALG-02 | Predictive LL(1) plus a Pratt/precedence expression specialization | Small, explainable fast controls and candidate internal specializations. FIRST/FOLLOW conflicts reject LL(1); Pratt is admitted only for an explicit operator grammar, preserving precedence/associativity and tree mapping. |
| ALG-03 | Canonical LR(1) generator/runtime, with LALR state-merging comparison | Deterministic CFG baseline, including a grammar that is LR(1) but not LALR(1). Preserve Lemon as the current compiler control. Use Bison IELR as a reference if available; a native IELR implementation is not needed to test this decision. |
| ALG-04 | Ordinary packrat PEG interpreter/generator path | Semantic control for well-formed non-left-recursive PEG, including predicates and ordered choice. Normalize/memoize costly subexpressions so a rule-only cache does not earn an unjustified linear certificate. |
| ALG-05 | Faithful Pika port plus separately named optimized variants | Cover the reference clause kinds, recursion scheduling and tree selection; exercise documented reference limitations. Compare character, token and selective/sparse evaluation; reject unsupported semantics in the public experimental backend. The existing dense positive-subset PoC is only a control. |
| ALG-06 | Squirrel core port and separate c14 recovery port | Pin algorithm and result semantics, run the Dart references, compare to ordinary PEG on the shared domain, and preserve known differences in recursive cases. Do not port hundreds of archived recovery experiments or equate c14 with the packaged library. |
| ALG-07 | Earley with a shared packed forest; small weighted repair oracle | General CFG and ambiguity baseline with an established conservative bound. Preserve alternatives rather than enumerate exponentially many trees. Handle nullable cycles explicitly, including cyclic/infinite derivations. Small exact recovery is an oracle, not a promised fast production feature. |
| ALG-08 | Composite planner/runtime plus declarative token/tree passes | Use dependency analysis to select admitted regions, execute generated token normalization and structural passes, and retain an unspecialized semantic control. Do not cross an unresolved choice, recursion or context boundary merely because two engines have matching APIs. |

GLL, a correctly bounded GLR variant, adaptive LL(*), regex derivatives and
scannerless generalized parsing remain explicit literature/reference
comparators in the final support matrix. Their capabilities, costs and unique
use cases must be assessed. They are not all mandatory additional native ports:
Earley supplies the first general CFG control and canonical LR supplies the
deterministic one. Add a further native engine only if a concrete unresolved
question requires it, recording the reason and acceptance impact here.

**Initial support hypothesis, not the verdict:** a native DFA backend, an
LR(1)-capable backend and a PEG backend cover the likely core; LL(1)/Pratt can
be internal specializations, and a general CFG engine can supply an explicit
fallback/ambiguity service. Composite planning coordinates these. Pika and
Squirrel must earn their place through the same correctness, usability and
cost criteria. An engine's restricted but reliable profile can be worth
supporting even when a universal claim fails.

### Mathematics to establish

**M-01 — Meanings before transformations.** Define recognition, full-input
acceptance, tree/capture selection and source ownership for regular rules,
ordinary PEG, the selected recursive-PEG extension and CFG rules. Distinguish
language equivalence from tree equivalence and diagnostic behavior. Preserve
named production identity through normalization. Decide which differences
are allowed observations before measuring them. Side effects during speculative
recognition are excluded from the proved core; semantic construction is pure
or delayed until selection.

**M-02 — Conservative static analysis.** Compute nullability, possible first
tokens, progress, reachability, recursive dependency components and relevant
lookahead/context use with sound approximations. Exact LL(1)/LR(1) table
checks and regular automata analysis can settle useful cases. General PEG
equivalence/nonemptiness and general CFG equivalence are undecidable; arbitrary
host callbacks also defeat universal cost/termination checking. Therefore use
`supported with certificate`, `supported with stated conditional bound`,
`unsupported`, and `not proved` as distinct results. A strict requested profile
rejects missing proof at build time; that does not assert that the language is
inherently unparsable. Construction-budget exhaustion is another distinct error.

**M-03 — Automatic grammar manipulation.** Investigate EBNF expansion,
left factoring, guarded left-recursion/operator lowering, automaton
minimization and regular-region extraction. Every rewrite has preconditions,
a local proof argument and an AST/source reconstruction map. A CFG rewrite
cannot be assumed correct for PEG priority. An ordinary conflict is not itself
proof of language ambiguity. Synthesize a witness where possible and otherwise
retain the unresolved obligation. Use exhaustive short-input and generated
counterexample searches as tests of proofs, not substitutes for them.

**M-04 — Composite parsing and fixups.** Classify the existing stages:

- Finite-state token transductions for suitable continuation/terminator rules;
  stack-aware rules for nesting; explicit finite contextual modes where valid.
- Syntax-directed tree construction, grouping and provenance propagation.
- Acyclic attribute dependencies with a generated evaluation order.
- Monotone equations over a stated finite-height lattice, evaluated by a
  dependency worklist; derive the height/edge cost rather than merely cap loops.
- Rewrites with a decreasing measure and deterministic order, or a justified
  confluence argument when order independence is promised.
- Name/type resolution, arbitrary exits and other semantic effects that need
  declared host services and separate bounds.

Prove selected passes preserve intended source meaning and compose. The engine
can derive a schedule from declared rules; it cannot infer arbitrary intended
fixups from an incomplete grammar. For hybrid calls, specify entry/exit,
consumption, context, result multiplicity and error ownership. CFG callees may
need multiple end positions; replacing those with a PEG's first success is
not a valid interface conversion. Memo keys must include every relevant mode
or context identity. Changing context cannot reuse stale `(rule, position)`
results.

**M-05 — Lex-first equivalence.** Extend the pilot's boundary theorem to the
chosen tree and trivia contract. Preserve lexical priority, contextual words,
case policy, byte/codepoint mapping, comments, whitespace/abuttal and raw
malformed-input behavior. Test recognizer predicates that inspect inside or
across a proposed token. When a unique tokenization cannot be certified,
retain scannerless parsing, parser-controlled lexing or an explicit token
lattice; account for that alternative's cost. Do not call ASCII-only results
Unicode qualification or normalize source implicitly.

**M-06 — Cost certificates.** Use input bytes/scalars `n`, token count `t`,
normalized grammar size `g`, lexer-state count `q`, recursion expansion work
`e`, produced tree/forest size `a` and repair budget `d`. Separate generation
cost, parsing work, peak/live retained memory, and repeated-request retention.

`T_total = T_decode + T_lex + T_parse + T_tree + T_passes + T_recovery`.

For a suitable fixed deterministic grammar, seek `O(t + a)` parsing. For
normalized pure non-LR packrat, justify `O(g*t + a)` with the actual
memoization granularity and terminal costs. For Earley retain a conservative
polynomial bound with grammar dependence explicit; forest construction and
enumerating all trees are different operations. For LR extensions account
for every expansion/re-evaluation and all retained match nodes: `O(g*t)` memo
keys alone proves neither linear work nor linear live memory. A length bound
per start position can sum to a quadratic total.

For lexers, analyze bounded backup or implement a linear maximal-munch
construction following Reps; include its grammar-dependent storage/time.
DFA construction can be exponential in grammar size even when scanning is
linear in input. For recovery, report the dependence on `d`, ambiguity and
candidate widths. A runtime fuel limit guarantees a bounded run, not successful
recognition, and must yield `resource exhausted`, never `syntax invalid`.

Certificates must name the backend/version, grammar hash, semantic profile,
input-unit assumptions, output costs and proof obligations. Seek decidable
sufficient conditions, including candidate disjoint-expansion conditions, not
a complete algorithm for classifying every possible grammar's complexity.

**M-07 — Audit recursive PEG and repair claims.** Specify the semantic target
before judging Pika/Squirrel correct. Ordered choice and predicates need not
be monotone under ordering results solely by matched length. A grow-only
iteration is not automatically Kleene iteration of a monotone function. As a
simple sanity check, for `A <- A 'b' / 'a'` on `ab`, the rule body evaluated
with a one-character seed grows to two, but evaluated with the two-character
seed falls back to one. Keeping the longer seed is an additional operational
policy whose meaning needs stating. Compare formal LR-PEG treatments and test
mixed, indirect, nullable, predicate and equal-length/different-tree cases.

For recovery, define what is minimized: token edits, character edits, annotated
missing constructs, structural loss or a lexicographic ranking. Prove that
discarding one candidate at an equal end position is safe for every later
composition before treating it as dynamic programming. Keep multiple candidates
when needed. Audit the updated Squirrel manuscript's hardness and optimality
arguments with grammar size and edit budget explicit. Small exact search can
refute a claim; passing it is not a universal proof. No metric can establish
the programmer's intended repair uniquely in every case.

**M-08 — Trees, transformations and proof ownership.** Define the generic
services needed for eventual AST migration: node/production identity, ordered
children, optional sharing, source/token links, trivia, error nodes, traversal,
construction, transformation and owned lifetime. Keep lossless source trees,
syntactic ASTs and later semantic IR distinguishable rather than forcing them
into one physical record or one set of invariants. Describe how richer typed
nodes and consumer attributes attach without dependence on compiler globals.

For representative transformations, state preconditions and postconditions
for structural integrity, source provenance and meaning preservation. Track
which facts/certificates survive a mutation and which must be invalidated or
rechecked, using stable identities and explicit revisions where needed. Keep
runtime invariant checks, bounded exhaustive validation and mathematical
proofs distinct. Prototype a compiler-like builder, visitor and grouping or
lowering rewrite on the reusable structures. The migration outline must cover
later symbol/type/control/ownership facts and their consumers; passing this
prototype does not prove the existing compiler's entire semantic pipeline.

### Corpus and usability protocol

Use three complementary groups, selected and frozen before comparison:

1. **Useful consumers.** Existing authored TinyExpr and Unicode rule-file
   lexer examples, RexxScript expressions/statements, Mike's Pascal/PLM plus
   contrasting BASIC, Lisp and Prolog constructs, and representative B/G/C
   frontend fragments from owning tests. Inventory all eight audited frontends
   and record why selected fragments cover or miss their requirements. At
   least one complete small language/format must reach source-to-tree output;
   the programme is not restricted to fragment recognition.
   Include a complete configuration/data format, a small embedded-language
   boundary example, a standalone lexer and a parser fed by an independently
   supplied token stream. Include one tree visitor/transform that produces
   useful output, preserving or deliberately mapping original source/trivia.
   These are coequal use cases, not demonstrations built on a cREXX-specific
   frontend abstraction. Assess streaming, incremental/editor reuse, grammar
   modules and captures as support dimensions; qualify only the modes actually
   implemented in this research and give explicit errors for unavailable ones.
2. **Rexx-specific semantics.** Soft keywords and assignment/label precedence,
   abuttal versus blank concatenation, nested comments, comma continuations,
   clause boundaries, Classic IF/THEN/ELSE and PARSE templates, B/G procedure
   grouping/declarations, source spans and malformed/incomplete text. Separate
   authored syntax trees from later semantic canonicalization. Do not claim
   full B/G/C conformance from this representative prototype corpus.
3. **Adversaries and damage.** Existing Pika/munch counterexamples, overlapping
   LR expansion, nullable cycles, predicates, prefix choices, ambiguity,
   deep nesting, long lexical atoms and repeated calls. Generate deletion,
   insertion, substitution and truncation cases; retain a held-out set to
   reduce tuning to known examples. Include damage in lexing as well as parsing.

Differentially compare only engines that implement the same declared semantics.
When two profiles intentionally differ, retain the distinguishing input and
explanation rather than count the difference as a bug. Compare trees/spans as
well as acceptance. Use pinned independent oracles and canonical tree adapters,
so engines sharing a mistaken lowering are not their own only validation.

Two different diagnostic audiences need evidence:

- **Grammar author:** source rule names/locations, readable witness input,
  conflicting alternatives or recursion path, explanation of the backend
  limitation, and a semantics-preserving suggestion when established. Mark a
  behavior-changing suggestion explicitly. Show before/after trees for a
  proposed rewrite. A LALR merge conflict should be distinguished from genuine
  ambiguity and from a rule-order choice that hides a valid alternative.
- **Program author:** unexpected input, a short contextual expectation,
  source line/caret, related opening delimiter, stable code, and explicit
  suggested edits or missing/skipped nodes. Recovery must preserve later
  correct structure and must not present a repaired parse as clean acceptance.

Compare ordinary farthest-failure reporting, PEG labeled failures, Pika
islands/explanation, Squirrel/c14 and LR expected-token plus CPCT+-style local
repair. Separate diagnostic quality from repair speed. Retain local versus
global optimality and chosen cost model. A repair search can time out honestly.
Grammar-build diagnostics must not require reading automaton state dumps;
those remain optional detail. Prepare concrete Rexx-style examples for Adrian
to judge. Without a user study, report authoring assessments as assessments,
not a proved universal claim that PEG is more natural.

### Expanded acceptance criteria — current status in the audit below

12. **AC-12 — Shared semantics and build admission.** A documented common
    model preserves the existing lexer surface and distinguishes parser
    semantics explicitly. At least two backend pairs consume the same authored
    rules on their shared domain. Tests show located build-time rejection of
    unsupported constructs, unresolved proof requirements and budget exhaustion.
13. **AC-13 — Current frontend and logical-grammar inventory.** Map relevant
    B/G/C lexer/glue/parser/tree/semantic responsibilities to current sources
    and tests. Reconcile the historical logical grammar and new surface;
    record production integration obligations and uncovered cases explicitly.
14. **AC-14 — Required native portfolio.** ALG-01 through ALG-08 execute with
    documented inputs, trees or forests, supported subsets and failure paths.
    Pika and Squirrel are identifiable ports plus separately named experiments;
    reference wrappers or the pilot subset cannot satisfy those native ports.
15. **AC-15 — Independent semantic evidence.** Retain pinned reference runs,
    exhaustive bounded/differential cases and minimized disagreements for
    recognition, precedence, recursive choices, trees, spans and errors.
    Distinguish definition differences from implementation faults.
16. **AC-16 — Generated stages and hybrid proof.** Execute at least one token
    normalization, contextual-word classification and structural AST pass from
    declarations, plus one parser-region specialization. Compare with explicit
    reference passes. Retain preconditions, provenance maps, termination/cost
    arguments and a rejected unsafe composition.
17. **AC-17 — Lexer and lex-first obligations.** Retain scannerless/tokenized
    parity on a stated useful domain, a checked sufficient boundary condition,
    counterexamples outside it and the maximal-munch improvement/bound. UTF-8,
    byte spans, malformed raw input and binary/NUL capability are either
    exercised or rejected explicitly per backend. Existing wider contracts
    cannot silently become an ASCII contract.
18. **AC-18 — Mathematics and executable certificates.** Deliver M-01 through
    M-08 with theorem/assumption/counterexample/open-conjecture labels. A
    cREXX checker emits certificates for at least regular lexing, deterministic
    parsing and normalized non-LR PEG, plus a conservative general-CFG bound;
    the recursive-PEG/composite analyses may soundly return unknown. Include
    certificate-validation failures and adversarial scaling checks.
19. **AC-19 — Grammar-author explanation.** Executable examples explain an
    unreachable lexical rule, empty/nonprogressing rule, recursion cycle,
    LL conflict, LALR-only conflict, genuine CFG ambiguity and PEG priority
    surprise in source terms. A bounded witness search reports failure to find
    a witness honestly; it cannot prove ambiguity by exhibiting unrelated
    continuations. Retain a rule-repair walkthrough understandable without a
    Lemon state report.
20. **AC-20 — Input diagnostics and recovery.** Compare the named recovery
    families on frozen and held-out damage corpora with a stated quality rubric,
    explicit repair costs, a small exact oracle and useful subsequent structure.
    Retain cases of bad recovery, not only successes. Recovered results and
    resource failures remain distinct from valid or syntactically invalid input.
21. **AC-21 — Useful frontend prototypes.** Deliver a complete small-language
    source-to-tree example and representative B/G/C initial-AST comparisons,
    including the Rexx-specific cases above. Report evidence for choosing PEG,
    CFG or a composite for each dialect without claiming full replacement.
22. **AC-22 — End-to-end cost and lifecycle.** Freeze inputs and compare
    generation/initialization, decoding/lexing, recognition, tree creation,
    transformations and recovery separately and together. Retain raw serial
    measurements and counters for work, retained objects and peak/storage
    behavior, including repeated small-after-large and interleaved owned
    contexts. State stack/resource limits and actual platform scope.
23. **AC-23 — Toolchain and reproducibility.** Native modules use rxc, rxas,
    rxlink and both concrete VMs; focused optimized/no-opt parity covers new
    semantics. Check deterministic generation and negative exits. Preserve
    exact source/build/input hashes and runnable scripts; apply performance
    governance before new measurements and reuse unchanged pilot evidence.
24. **AC-24 — Decision report and reviewable syntax.** Publish locally a support
    matrix for every candidate: semantics, proven/conditional/measured bounds,
    grammar restrictions, diagnostics, code maturity and maintenance cost.
    Recommend supported core, specialist/experimental and rejected options,
    with concrete Level L declarations and B/G/C frontend diagrams/examples
    for final approval. Explain what remains unknown and the cost of resolving
    it. Every required experiment above must be dispositioned against its own
    criterion; merely choosing a favorite engine does not complete the plan.
25. **AC-25 — General language-engineering applicability.** Run the standalone
    lexer, existing-token parser, complete format, embedded-language boundary
    and source-tree transformation examples described above. They share the
    common model and reusable cREXX libraries without depending on compiler
    ASTs or B/G/C semantics. The final matrix assesses generation/library use,
    grammar reuse, captures/trivia, binary/Unicode, streaming, incremental
    operation and embedding; distinguish executed capability from recommended
    later product work. cREXX frontend success alone cannot satisfy this gate.
26. **AC-26 — AST/invariant migration feasibility.** Execute a representative
    builder, traversal and rewrite using the general Level L tree/contract
    infrastructure, with positive and deliberately violated invariant cases,
    provenance preservation and certificate invalidation/revalidation. Compare
    the observable result to a current cREXX case through a documented adapter.
    Deliver a staged migration map for compiler AST and proof/invariant
    consumers, including lifecycle and semantic gaps. Keep full migration open
    for the later production programme; generic parsing alone is insufficient.

### Current acceptance audit — completed representative gates

Historical checkpoints below describe their state at the time. This audit
supersedes their blanket “all expanded criteria open” statements without
removing any requirement. Overall research remains in progress.

| Criterion | Current status and evidence |
|---|---|
| AC-12 | **Verified for the research contract.** SEMANTICS/SURFACE; unchanged established lexer frontend; llfront ordered packrat/tabulation and contextfree LR/Earley pairs; located capability, unknown-proof and construction failures in toolkit/front; tranches 1 and 5. Provisional syntax is not product adoption. |
| AC-13 | **Verified inventory.** SOURCE-AUDIT frontend responsibility/migration table and owning source paths; SURFACE historical-convention reconciliation; uncovered semantic/integration obligations remain explicit. The broader executable corpus is assessed separately under AC-21 below. |
| AC-16 | **Verified representative generated mechanisms.** llstage normalization/context classification and independent pass oracle (1,555 generated cases); llgroup declaration grouping versus three B/G snapshots; llregional Pratt-in-LR versus whole CFG (789 cases plus multi-region and unsafe composition controls). MATH STAGE/TREE/COMP arguments, provenance and tranches 5–6. |
| AC-26 | **Verified migration-feasibility slice.** llcore builder/contracts, llgroup traversal/rewrite, origin classifications, positive/invalid spans and mutation/revalidation, independent B/G compiler adapter in cases/migration-reference.py; tranche 6. SOURCE-AUDIT states the adapter's exact projection and staged semantic/lifecycle migration gaps. Full migration remains a later programme. |
| AC-19 | **Verified representative author explanations.** Tranche 8; exact lexer winning-rule proof, bounded same-input CFG/PEG witnesses, actual cycle path, nonprogress/LL failures. Earlier LR controls distinguish LALR-only conflicts from ambiguity. SURFACE walkthrough avoids state dumps and identifies language-changing edits. |
| AC-17 | **Verified stated-domain gate.** Tranche 10 retains 46 faithful Pika character/token cases with named spans, trivia ownership, NUL and UTF-8 ingress refusal; llboundary issues identity-bound CFG regular-substitution certificates for immediate-stop boundaries, rejects unsafe adjacency/predicates/implicit trivia and separates construction exhaustion. LEX-04 supplies the sufficient proof. Earlier reverse-munch bounds, byte/span and Unicode capability refusals remain in force; this does not certify arbitrary tokenization or Unicode parsing. |
| AC-21 | **Verified representative frontend gate.** C02 complete config source/tree/visitor (tranche12); C06 actual RexxScript value/error control and48 native cases (tranche14); C08 twenty B/G/C initial-tree observations plus earlier procedure grouping (tranches15/6). CONSUMERS gives dialect-specific algorithm evidence, exact projection/subset and source-span limits. Full frontend replacement remains outside this research gate. |
| AC-20 | **Verified comparative research gate.** Frozen array and lexical-damage panels (tranches11/16), held-out splits, independent byte-edit oracle, exact native checks, bad label/local-repair cases, source excerpts and later-island observations. RECOVERY.md states incomparable objectives and Q020 equal-cost tree change. This is not a user study or globally optimal recovery claim. |
| AC-14, AC-15, AC-18, AC-22–25 | **Open for their complete obligations.** Implemented subsets and historical evidence remain valid; finish certificate audit, lifecycle/cost and final support report. |

### Expanded implementation steps and gates

8. **STEP-08 (AC-12, AC-13, AC-24, AC-25, AC-26) — Approve strategy; freeze questions.**
   Present this plan now. After approval, inventory source semantics and
   consumers, pin reference versions, propose the common IR and provisional
   declarations, and freeze the hypothesis/corpus manifest. Approval permits
   these research surfaces, not a final product-language change. No new parser
   implementation precedes approval of this programme.
9. **STEP-09 (AC-12, AC-15, AC-18, AC-23) — Build semantic controls and analysis.**
   Implement the shared source/tree model, capability checks, progress/dependency
   analysis and canonical comparison tools. Use a plain PEG control and
   independent CFG/reference results. Begin the proof/counterexample ledger.
   Depends on STEP-08; avoid timing engines with incompatible semantics.
10. **STEP-10 (AC-14, AC-15, AC-17, AC-18, AC-23) — Implement traditional
    controls and lexers.** Extend native lexical backends; implement LL/Pratt,
    LR(1)/LALR and Earley/forest engines. Establish end-to-end tree correctness,
    UTF-8/profile boundaries and certificates before optimization. Reuse
    unchanged re2c/native evidence, extending only for new obligations.
11. **STEP-11 (AC-14, AC-15, AC-18, AC-20, AC-23) — Audit and port Pika and
    Squirrel.** Execute pinned upstream references, including Squirrel c14;
    reproduce positive and adversarial cases. Port algorithms with explicit
    semantic correspondence, then investigate sparse/token scheduling, shared
    trees and safe grammar transformations as separate variants. Report the
    first substantive feasibility findings before polishing implementations.
12. **STEP-12 (AC-13, AC-16, AC-17, AC-21, AC-25, AC-26) — Generate composite language tools.**
    Build the selected transducer/attribute/rewrite mechanisms and at least one
    safe regional specialization. Compare staged and more direct grammars on
    useful language and B/G/C AST cases. Depends on semantic and engine controls;
    later type/import/exit behavior remains an explicit service obligation.
    Execute the independent lexer/token/format/embedding/transformation consumers
    so the common architecture is tested outside compiler construction.
    Prove the representative tree/contract migration slice and record the
    remaining compiler-internal migration requirements.
13. **STEP-13 (AC-19, AC-20, AC-21) — Explain rules and damaged programs.**
    Implement grammar witnesses and input recovery controls, exercise the frozen
    and held-out corpora, and produce authoring walkthroughs. Include lexer
    failures and context recovery. Use exact short-input repair to challenge
    optimality and pruning claims; preserve valid-input semantics.
14. **STEP-14 (AC-18, AC-22, AC-23) — Freeze, measure and challenge bounds.**
    Correctness-gate each candidate, freeze its source/build/input identity,
    then run serial research measurements and increasing adversarial sizes.
    Measure both cold service cost and reuse where relevant. Validate owned
    context teardown and memory retention. No production performance edit is
    authorized; if proposed later, its separate first-Release-verdict gate
    applies before broad closeout work.
15. **STEP-15 (AC-12 through AC-26) — Reconcile and recommend.** Write the
    general toolkit support matrix, proofs/limits and executable examples;
    include B/G/C as one consumer recommendation and a staged AST/invariant
    migration route onto the generic infrastructure.
    Keep unmet criteria open. Ask Adrian to decide final syntax, supported
    algorithms and any production frontend migration using those concrete
    results. Local research completion and production adoption are different
    milestones.

The steps are dependency order, not a requirement to delay all feedback until
the end. Report meaningful counterexamples and capability decisions as they
appear. A failed universal claim narrows a proposed supported profile only
with an explicit record; it must not erase the requested investigation.

### Primary research routes

These sources guide the investigation; cited claims still need matching
implementation assumptions and local evidence. No new runtime experiments
were performed merely to prepare this approval plan.

- [Ford: PEG semantics and decidability](https://bford.info/pub/lang/peg.pdf).
- [Medeiros et al.: formal left-recursive PEG extension](https://arxiv.org/abs/1207.0443)
  and [Tratt: direct left-recursive PEG analysis](https://tratt.net/laurie/research/pubs/html/tratt__direct_left_recursive_parsing_expression_grammars/).
- [Pika paper](https://arxiv.org/html/2005.06444) and
  [pinned reference](https://github.com/lukehutch/pikaparser/tree/8050ceaf70c79a04416fdbdc374e92f9478de7b0).
- [Squirrel current repository manuscript](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/paper/squirrel_parser.tex),
  [packaged parser](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/dart/lib/src/parser/parser.dart)
  and [recovery-engine status](https://github.com/lukehutch/squirrelparser/blob/bb1ae84701060462636c07c814286ac2875e0568/dart/experiments/recovery/README.md).
  Keep the [older arXiv presentation](https://arxiv.org/html/2601.05012)
  distinct from the newer source/manuscript.
- [Reps: linear maximal-munch tokenization](https://research.cs.wisc.edu/wpis/abstracts/toplas98b.abs.html).
- [Bison: LR table families](https://www.gnu.org/software/bison/manual/html_node/LR-Table-Construction.html)
  and [grammar counterexamples](https://www.gnu.org/software/bison/manual/html_node/Counterexamples.html).
- [Scott/Johnstone: GLL](https://www.cs.rhul.ac.uk/research/languages/csle/GLLparsers.html)
  and [generalized LR research](https://ir.cwi.nl/pub/14233).
- [Parr/Harwell/Fisher: adaptive LL(*)](https://www.antlr.org/papers/allstar-techreport.pdf).
- [JastAdd: attributes, rewrites and finite-height fixed points](https://jastadd.cs.lth.se/web/documentation/concept-overview.php).
- [CPCT+: LR error recovery](https://soft-dev.org/pubs/html/diekmann_tratt__dont_panic/),
  [PEG labeled-failure recovery](https://arxiv.org/abs/1806.11150)
  and [automatic PEG recovery annotations](https://arxiv.org/abs/1905.02145).

### Historical continuity note — before the implementation checkpoints

The overall expanded programme is **approved and in progress**. AC-01 through
AC-11 remain verified for the pilot; AC-12 through AC-26 and STEP-08 through
STEP-15 are open. The current action is STEP-08's semantics/source
inventory and corpus manifest, followed by STEP-09. Do not restart the pilot,
discard its counterexamples or silently promote provisional syntax. The user's
"Approved" authorizes the programme above, including the general toolkit and
eventual AST/invariant migration direction. Production adoption and publication
remain separate decisions. STEP-08 is active; all new criteria remain open
until their complete evidence is recorded.


### Implementation checkpoint — shared analysis and backend contracts

STEP-08/09 are active. `SEMANTICS.md` and `CORPUS.json` record the initial
research contract and workload purposes. `llcore` now supplies owned grammar,
binary/token source and revision-checked tree arenas. `llcheck` computes
backend-independent facts; `llcap` dispatches a provider's admission check and
separates support, missing proof, malformed grammar and construction limits.
This incorporates Adrian's question about plugin ownership without changing
the accepted semantics or selecting a production plugin ABI.

`llpeg` executes ordinary packrat, a predictive control and a preliminary
Squirrel seed-growth mode. The latter is not yet a faithful full port: reference
mismatch trees and rule-body memo identity require separate correspondence.
Packrat repetition tails are memoized explicitly; an iterative suffix scan
would not justify the claimed normalized-clause bound. `run-toolkit.sh opt`
and `run-toolkit.sh noopt` passed on both concrete VMs, including grammar
identity/revision checks, source/tree invariants, backend-specific rejection,
unknown cost versus required bound, construction limits, PEG priority,
independent requests and direct seed growth. Outputs are currently in ignored
`build/toolkit-{opt,noopt}`; freeze durable provenance with the next expanded
conformance tranche. These focused results do not close any whole AC-12..26.

Next: run the pinned Dart core and c14 recovery references, retain canonical
success/failure trees, and implement the full native correspondence. Continue
the other required algorithms and corpus obligations; no portfolio entry is
removed by this checkpoint.


### Implementation checkpoint — independent Squirrel core and LR controls

`llsquirrel` is now the separate reference-corresponding core port. It memoizes
rule **bodies**, passes failed references through without wrapping, retains
mismatch frontiers/children and follows the pinned expansion/version protocol.
`llhistory` keeps failure evidence separate from selected syntax-tree invariants.
The shared IR has literal, optional, plus and set clauses so the reference's
shape/frontier semantics need not be lost through an unverified rewrite.
`llpeg`'s earlier Squirrel mode remains a preliminary control, not this port.

The pinned Dart core and c14 execute under local SDK 3.13.5. Reference records
are `evidence/squirrel-reference.jsonl`, `squirrel-expected.txt` and
`squirrel-runtime.txt`; the probe and native comparator are versioned. All 486
core cases agree in opt/no-opt on both VMs, including exhaustive length-0..4
inputs over two small alphabets, direct/indirect/mixed recursion, shared bodies,
full failure trees, nullable repetition, predicates, sets and NUL. The c14
reference handles six clean/damaged array cases; its native port remains open.
The native history arena retains all allocated attempts until disposal, unlike
Dart GC; selected live memory and compaction remain research obligations.

`llcfg` supplies pure CFG productions. `lllr` constructs canonical LR(1) tables,
merges equal cores for LALR, reports conflicts and executes source-tree reductions.
Focused opt/no-opt runs on both VMs pass for left-recursive arithmetic, syntax
failure, a grammar that is canonical LR(1) but not LALR, four distinct contexts,
and an ambiguous concatenation conflict. The message names A/B and source
lines and states that LALR merging introduced the conflict. This is not yet
an ambiguity witness generator or a complete production-quality diagnostic.
The system Bison is 2.3; a modern canonical/IELR independent control is pending.

STEP-09/10/11 are active; whole expanded acceptance criteria remain open.
Next: Earley with packed forests and cyclic-derivation handling, faithful Pika,
c14 port, declarative composites, the general consumer corpus, remaining maths
and measurements. Do not reduce the portfolio to engines already implemented.

### Implementation checkpoint — Earley, reverse lexing and independent CFG controls

`llearley` and `llforest` execute Earley items with binary packed families,
nullable completion in both orders and explicit cyclic derivations. Focused
opt/no-opt tests pass on both VMs: finite Catalan ambiguity, nullable placement,
empty input, infinite nullable derivations, invalid input and resource limits.
The dense family index has a conservative cubic-in-input space bound for fixed
grammar; it is deliberately not advertised as an ordinary quadratic-space
Earley implementation. `MATH.md` records the implementation-specific argument.

`llmunch` owns scanner requests and implements ordinary maximal munch plus a
reverse-tabulation alternative consuming the unchanged generated DFA tables.
It uses two state columns plus one root result per input position. The backward
induction argument and q*n transition bound are recorded as LEX-02 in MATH.md.
Both build modes/VMs pass token/span/status comparisons over the existing
useful lexical fixture, malformed strings, byte probes, source priority,
skipped trivia, explicit legacy high-byte rejection and generic binary NUL.
The rollback adversary takes 8,256 ordinary versus 512 reverse transitions at
n=128 with q=4. These are operation counts, not timing/product verdicts. The
complete-buffer requirement is explicit; streaming remains a separate profile.

Local Bison 3.8.2 was built under ignored `build/bison-runtime` and executed by
`run-cfg-reference.sh`. Canonical LR and IELR accept all four distinguishing
contexts; LALR has two reduction conflicts, agreeing with the native family
comparison. A bounded GLR set-of-trees oracle agrees on 1,1,2,5,14 distinct
concatenation trees at input lengths 1..5. The first additive `%merge` control
returned 16 at length 5 and was rejected as a distinct-tree-count oracle;
its source and failed output are retained as `glr-additive-control` rather than
silently discarded. This does not establish a Bison recognizer defect.

The existing `evidence/toolkit-provenance.json` is a frozen checkpoint for the
shared-model/Squirrel/LR tranche. Subsequent Earley/lexer sources are not
retroactively covered by it; retain an additional exact tranche receipt after
those focused checks. All expanded AC remain open for their complete portfolio,
consumer, diagnostics, proofs and migration obligations.

### Implementation checkpoint — Pika port and pruning refinement

Adrian's follow-up asks whether Pika needs algorithmic or implementation
refinement and whether safer pruning/scoring is feasible. This steers STEP-11:
preserve the faithful reference control, then test dependency invalidation and
candidate frontiers separately. It does not authorize redefining PEG choice
or adopting a production algorithm. `MATH.md` PRUNE-01 records the semantic
gate and hypotheses; all original portfolio/consumer obligations remain open.

`llpika` is now a native operational evaluator of explicit ranked Pika normal
forms. `PikaCoreProbe.java` exports upstream-normalized graphs and independent
raw-tree/counter expectations. All 508 cases agree in opt/no-opt on both VMs,
including recursion, predicates, precedence-expanded input, empty behavior,
ASCII case folding and binary bytes. Java supplies normalization only for these
conformance fixtures; native rule/precedence/AST-label lowering and optimized
variants remain open. Start's empty-placeholder defect is reproduced, and
`MATH.md` PIKA-03 gives a conservative conditional quadratic scheduling bound
with an executable acyclic-unconditional-edge checker. This is not full PEG
semantic qualification or a linear bound.

`lltabpeg` is a separate right-to-left, settled-dependency experiment. The
acyclic same-position profile has an induction proof and a linear input bound;
154 acceptance/tree cases agree with the ordinary packrat control. Extended
clauses, EOF, binary input, trailing-trivia provenance and resource/build errors
are exercised too. Recursive candidate refinement is still open.

The packrat control gained a minimal regression for cached failure first visited
under negative lookahead. A memo-based diagnostic traversal now explains the
later required token; recognition caching no longer suppresses that evidence.
Returned packrat/LR tree arenas now retain original bytes independently.

`llc14cost` ports c14's reading totals and seven-key comparator, not yet its
recovery evaluator. The pinned Dart algebra and native code reproduce a rank
reversal after a common continuation (REC-03). This exposes a proof obligation
for best-per-end pruning; grammar-level reachability and global recovery impact were initially open;
the subsequent checkpoint below resolves that particular counterexample. Do not confuse this repair ranking with Pika recognition.

Next required work still includes native Pika lowering/refinement, the full c14
port, Pratt, exact repair oracle, generated composites and invariants, authored
parser surface, useful general/B/G/C consumers, diagnostics, measurement and
the final support/migration report. None of AC-12..26 is closed by this tranche.

### Implementation checkpoint — operator specialization and generic tree pass

`llpratt` now implements ALG-02's explicit operator specialization and emits a
shared CFG from the same declaration. Native LR consumes that CFG independently.
`llproject` executes a declarative named-node/arity projection, retaining source
ownership and origin handles. Seven arithmetic/tree examples and 781 generated
inputs agree between Pratt and LR in opt/no-opt on both VMs. Malformed input,
mixed associativity, stale specialization identity, tree contract violation and
work/map limits have distinct negative checks. OP-01 records the equivalence,
termination and cost arguments. This is a useful existing-token consumer and
part of the generic transformation foundation; it does not yet close the full
regional composite, authored surface or cREXX AST-adapter criteria.

Remaining next work: native Pika lowering/refinement, full c14 evaluator and
repair oracle, token/context stages and regional composite execution, complete
general-language/format/embedding examples, B/G/C and RexxScript comparisons,
grammar witnesses, detailed measurement and the final support/migration report.
All expanded acceptance criteria remain open until their full evidence exists.

### Implementation checkpoint — reachable recovery-pruning counterexample

REC-03 now has a concrete grammar and input, executed by the pinned c14 Dart
engine: two engine-produced readings reach the same endpoint, the locally
preferred reading is selected, and appending the same final token makes the
pruned reading rank better by c14's own comparator. The retained output is
`evidence/c14-pruning-reference.out`; this is a ranking counterexample, not a
claim about human repair intent or Pika recognition.

The native `llc14cost` experiment now has a conservative componentwise dominance
frontier and 2,016 continuation checks, passing opt/no-opt on both VMs. REC-04
proves its algebraic common-continuation property and identifies the remaining
whole-search and tree-tie obligations. The full native c14 evaluator remains
required; implement it faithfully before separately experimenting with pruning.
All AC-12..26 remain open for their complete agreed obligations.

### Implementation checkpoint — complete native c14 search and independent oracle

`llc14` now ports the complete c14 search over explicit n-ary byte-domain
reference graphs: plain rule memoization, repair cells and seed clocks,
budget-zero diversion, best-per-end folds, literal repair, delete-ahead and
restricted substitution, evidence/preference gates, repetition, EOF charging
and selected error-tree materialization. It deliberately retains c14's
ranking and pruning. The reference-normalized n-ary graph is kept explicit;
regrouping a sequence is not assumed recovery-equivalent. Shared authored-IR
lowering, general Unicode and optimized storage remain separate obligations.

All 237 reference cases agree on status, cost and complete raw error tree in
opt/no-opt on both concrete VMs. These include mixed/indirect/right recursion,
predicates, literals, nullable repetition, binary bytes, a deterministic held-out
array mutation set and REC-03's pruning failure. This establishes the executed
byte-profile port, not global optimality or general termination. Fuel and memo
construction limits have distinct negative tests. Request-owned temporary
arenas retain more history than the Dart GC reference; lifecycle measurements remain open. Candidate-frontier integration is recorded
in the subsequent checkpoint.

The first optimized port exposed a compiler-dependent failure at the nested
indexed assignment `_plain[key] = one(cleanreading(...)) + 1`. Splitting the
mutating calls into statements yields conformance without disabling optimization.
The failing source/input/output are retained under `evidence/c14-first-opt-failure`;
a production compiler repair is not claimed. Continue reducing the reproducer
before proposing a core change; do not edit the concurrently active develop
checkout as part of this research.

`llrepair` now executes as a separate exact bounded weighted byte-edit oracle,
using Dijkstra search and Earley acceptance. Its cost model is deliberately
explicit and differs from c14's missing-construct/evidence ranking. Completion
of this checkpoint does not close the remaining authored surfaces, composite
passes, useful consumers, usability, migration or measurement criteria.


### Implementation checkpoint — pruning controls and exact byte repair

The c14 `frontier-experiment` now runs inside the complete native evaluator.
On all 237 retained inputs it gives one better ranking (REC-03), zero worse,
236 ties and zero resource failures; all 42 clean trees remain identical.
The largest measured operation/allocation ratios in this panel are 1.264/1.311.
These are counters, not timing or general frontier-width bounds. REC-04 proves
only common-continuation dominance, not global search optimality.

The separate weighted byte-edit oracle passes 480 independent finite-language
Wagner-Fischer comparisons, ambiguous-repair forest, NUL, no-repair-within-bound,
invalid-cost and resource cases in both modes/VMs. Its objective differs from
c14's ranking; matching numerical costs is not asserted.

Pika `longest-choice-experiment` repairs the mixed-operator example but reverses
ordinary PEG acceptance in both directions on two nonrecursive controls.
The faithful default still passes all 508 reference cases in all four cells.
PRUNE-02 rejects local longest-choice as a semantics-preserving repair; recursive
dependency refinement remains open. Retained evidence is tranche-4.

Next, STEP-12/AC-12/16/25 adds an explicitly provisional parser declaration
frontend in the accepted Rexx style (`name: kind ...`). It will lower named
rules to the common IR, preserve declaration locations, explicitly select ordered
or contextfree meaning, and exercise two admitted backend pairs. The unchanged
established lexer frontend remains the lexer contract. This is within the
approved research spelling experiments, not adoption of production syntax.
Generated token/context/tree stages and regional composition follow. All
AC-12..26 remain open until their full obligations are verified.

### Implementation checkpoint — authored rules, generated passes and regions

`llfront` now executes provisional parser declarations in the accepted Rexx
style. Named rules and source locations lower into the shared basic graph;
ordered and contextfree alternatives are explicit profiles. Two backend pairs
consume the same authored declarations: packrat/settled tabulation for a complete
TinyConfig format, and LR/Earley for an independently supplied token list.
The unchanged established lexer bootstrap remains the lexical contract.
Undefined names, unsupported input units, lexical errors, source nesting and
construction limits have distinct located failures. Parser spelling remains
provisional under the programme's existing approval.

A handwritten independent TinyConfig reader supplies 84 valid/mutated inputs
and named source-tree expectations. These join other controls for 125 frontend
cases. Configuration strings, escaped quotes/backslashes, nested arrays, numbers,
booleans and malformed/NUL/non-ASCII contents are exercised. This is a complete
small defined format, not JSON or general Unicode qualification.

`llstage` executes declared finite-context token transductions. The staged
fragment generates comma/newline deletion, terminator mapping and clause-start
SAY classification with an assignment guard. Original bytes and composed input
maps survive. It agrees with an independent direct pass over 1,555 generated
sequences plus useful positive/negative examples. Authored tree declarations
project assignment/command subtrees with checked arity and source ownership.
This is not yet the complete B/G/C context or structural migration comparison.

`llregional` supplies an actual checked composite: disjoint delimiter-bounded
operator regions run through Pratt inside an LR outer grammar. The planner also
generates the unspecialized whole CFG. Their semantic trees/spans agree on 789
inputs; a repeated grammar exercises two independently owned regions. Shared
boundary alphabets and reserved virtual ingress reject explicitly; fuel tests
include every budget below a completed example's charged work. COMP-01 gives
the interface and local substitution proof. General ambiguous/contextual hybrid
calls remain outside this admitted profile.

The authoring/stage/regional groups pass in optimized and no-opt builds on both
VMs with identical outputs. Durable source/tool/image/output hashes are retained
in `evidence/tranche-5/provenance.json`. No whole
expanded criterion closes here: the broader syntax/corpus, B/G/C AST/invariant
adapter, attribute/rewrite scheduling, Pika lowering/refinement, diagnostics and
recovery comparisons, lifecycle measurements and final support report remain
required. Next work proceeds against those obligations, without discarding the
completed pilot or promoting research syntax to production.

### Implementation checkpoint — representative AST grouping migration

`llgroup` now reconstructs declared marker/member regions over shared owned
source trees. A frozen compiler control supplies B/G raw/fixup snapshots for two
explicit procedures and an implicit-main variant. The native builder, visitor
and grouping rewrite reproduce selected node/value/order observations. Exact,
composite and synthetic origins remain distinguishable; deliberately invalid
spans fail, mutation invalidates certificates, and repaired spans require fresh
certification. Compiler exits are disabled for these fixtures and no callback,
full type, source-span or whole-frontend equivalence is inferred.

The group passes in optimized and no-opt builds on both VMs, including
provenance classification and fresh revalidation after restoring a bad span.
TREE-02 records the local partition/provenance proof and bounds. SOURCE-AUDIT.md
now maps current stage responsibilities and the later source-tree, sidecar,
semantic-IR and proof-consumer migration route. SURFACE.md inventories historical
logical-grammar conventions that need an explicit semantics decision, including
non-greedy star, quote case policy, trivia/abuttal and arbitrary tree rewrites.
These additions do not adopt a production syntax or change the current compiler.

The broader B/G/C initial-AST corpus, automatic attribute/rewrite scheduling,
Pika normalization/refinement, diagnostics/recovery comparison, lifecycle/cost
measurements and support report remain open under their original acceptance
criteria. Review AC-16/26 against the complete retained record after this
verification; do not infer closure of the whole expanded programme.

### Implementation checkpoint — automatic attribute schedules

`llschedule` executes declared pure Boolean-join equations. It generates a
static topological schedule for acyclic dependencies and can automatically
select a finite-height worklist for cycles. A strict DAG request returns an
actual named dependency-cycle witness. Unsupported negation rejects, partial
resource-limited evaluation has no certificate, and graph mutation invalidates
previous facts. A source-tree consumer synthesizes a declared descendant-kind
attribute without importing compiler internals.

All 4,096 three-variable graph/seed combinations agree with independent
relational closure in optimized and no-opt builds on both VMs. Exact migration
and schedule inputs, tools, images and outputs are retained in
`evidence/tranche-6/provenance.json`. ATTR-01 gives the least-fixed-point/termination/cost proof. The equation
language is intentionally explicit and restricted. It cannot establish
termination of arbitrary host callbacks, type inference or compiler exits.
The broader programme remains open; these results satisfy representative
mechanisms rather than silently reducing the requested portfolio or corpus.


### Implementation checkpoint — native Pika lowering and expressive-power control

`llpikalower` now translates the common binary IR, removes named references,
interns finite expressions, expands EBNF and emits deterministic clause ranks.
An explicit `pika` profile prevents this operational engine being silently
substituted for ordinary PEG. Upstream HashSet root/cycle ordering is not a
specified semantic contract; the native profile uses stable terminal-first
source-order DFS. Pika's documented counterexamples remain in the faithful core.

The current pass includes explicit authored precedence tiers and a selected
source-tree projection with owned bytes and original-history origins. Interned
rule aliases are exposed together rather than pretending distinct wrappers
survived. The upstream precedence-number shorthand and arbitrary edge-label
syntax are not the common surface; wider capture declarations remain an
explicit product-design obligation. This does not claim every ordinary PEG
normalization is valid under recursive Pika operational semantics.

The independent a^n b^n c^n oracle has 707 inputs (block counts 0..6 and all
abc words through length 5). A separate Java probe runs the unmodified upstream
clause evaluator with native-produced ranks, comparing full raw trees, zero
flags and allocation/update counts. Its first 977-case panel passes optimized
and no-opt builds on both VMs. POWER-01
proves the language example independently of execution. POWER-02 distinguishes
new external separation claims and their trust assumptions from local proofs.

A new complemented-set test exposed a double inversion in `lltabpeg`; membership
is now used once. All 256 byte values plus EOF are checked directly, alongside
the prior 154 PEG acceptance/tree comparisons. All four cells, 125 authored
frontend regressions and the unchanged 508-case reference behavior pass.
Exact provenance is frozen in `evidence/tranche-7/provenance.json`. No existing
historical receipt is retroactively relabelled.


### Implementation checkpoint — grammar-author witnesses

`lllexplain` proves winning-rule reachability on complete generated DFA tables
with distinct rule IDs and supplies shortest positive examples. `llwitness`
finds CFG ambiguity only from multiple complete derivations of the same word,
and supplies a bounded PEG-versus-unordered priority counterexample. It also
explains a real conservative same-position cycle through source rule names.
No witness within a finite bound remains inconclusive; resource exhaustion is
separate. All focused checks pass in four cells, retained in tranche 8.
DIAG-01 gives the arguments, and SURFACE supplies a concrete repair walkthrough.
Together with the retained LR(1)-versus-LALR evidence this verifies AC-19's
representative gate, not universal diagnostic quality or a human user study.

The next active work is the distinct LR local-recovery control and comparison
against the same frozen/held-out damage corpus. Remaining useful-language,
Rexx-specific and lifecycle/cost obligations are still open. CORPUS C02's
original section/comment requirements must also be covered explicitly; the
current TinyConfig source handles assignments and nested arrays, not sections.


### Implementation checkpoint — substantive LR local-repair control

`lllrrepair` now performs cost-bucket search over actual LR stacks, including
reduction-only and single-shift successors, unit insert/delete edits, complete
minimum-cost local candidate sets and forward-progress ranking. It reproduces
the published six-repair `2 3 +` example, with integer tokens represented by a.
The implementation is a CPCT+-style control, not a Rust port or speed claim.

All four cells pass 121 independent finite-language edit distances, eight array
cases, the six-repair reference example, local-versus-full acceptance, token
origins and resource/invalid/no-repair paths. Tranche 9 freezes exact evidence;
REC-08 documents the actual shortest-path domain and quadratic bookkeeping.
The `[aa]` example retains both atom deletion and comma insertion: equally
cheap repairs need not preserve the author's intended content equally well.
Original bytes and untouched later tokens are retained. A local repair does
not silently become clean acceptance, and no global optimum is asserted.

AC-20 remains open for the shared frozen/held-out comparison including PEG
labels, Pika explanations and Squirrel/c14. This component does not close the
broader useful-language/Rexx corpus or lifecycle/cost programme.

### Implementation checkpoint — Pika dependency pruning and lex-first proof

The separately named `reachable-reference` policy preserves all 508 reference
selected histories in four cells while excluding clauses outside the requested
root's dependency closure. PIKA-04 proves the projected event trace argument;
this is not a heuristic length/repair-score pruning claim. Whole-chart island
consumers must retain the rules they need. The original reference policy and
known recursive/priority counterexamples remain intact.

Forty-six character/token assignment fixtures now pass all four cells, including
long names/numbers, comments, damage, NUL and an explicit UTF-8 DFA refusal.
Named name/integer/assignment spans agree; both trees own the original trivia.
The projection was repaired to omit zero-width recognition machinery from its
consumed-syntax view, while full histories remain available. The initial failure
is retained separately. Counters include lexer transitions; no new wall-time
speed verdict is claimed. Source projection and Pika lowering regressions also
pass. Tranche 10 retains exact tools, inputs, output and image identities.

`llboundary` checks a sufficient immediate-stop condition for pure token CFGs
expanded by the DFA's winning languages. It certifies compact assignments,
locates the unsafe identifier/number boundary (`a` plus `1` becomes `a1`), and
refuses to erase implicit trivia or predicates. Its identity check rejects
changed grammars/DFAs. LEX-04 states the proof and construction/storage bounds.
AC-17's stated-domain gate is verified; the wider useful-language, recovery,
lifecycle and report obligations remain open. Active work is now the labeled
PEG control and the frozen/held-out cross-family recovery comparison.

### Implementation checkpoint — frozen cross-family recovery comparison

Tranche 11 retains 40 common inputs (27 development,13 held out), an independent
recognizer/edit BFS, three native exact-oracle cross-checks, and identical
complete output in all four mode/VM cells. The native labeled PEG control
covers throw propagation, predicate suppression, explicit recovery and a
misplaced-label clean-language counterexample. A separate positive-clause Pika
chart explainer reports anchored expectations without changing recognition.

The panel keeps negative results: labels recover 22/33 damaged inputs; local
LR reaches a full edited parse on31/33; two local successes leave later errors.
Pika retains all29 designated later arrays as islands, and LR/c14 retain them
in their respective recovered observations. Local LR costs3 for a globally
cost1 repair; the c14 frontier adds work without improving this panel. README,
MATH REC-09/10 and evidence/recovery-summary.json preserve the distinctions.
No held-out grammar or ranking tuning was performed.

Exact presentation parity exposed an additional frozen optimized-tool finding.
It is retained in evidence/recovery-diagnostic-opt-failure with a runnable
reproducer and negative reductions. The parser's full label message is correct;
the renderer now presents that message with a stable category. No core compiler
repair or current-develop reproduction is claimed. This is separate from the
older c14 indexed-call finding until a common cause is established.

AC-20 remains open for the lexical-damage/consumer assessment, rather than
silently treating this small byte-array panel as all language diagnostics.
The complete section/comment configuration consumer is under qualification;
its original C02 requirements remain in scope. Wider Mike/RexxScript/B/G/C
fragments, lifecycle measurements, certificate audit and support report remain
open under the existing criteria.

### Implementation checkpoint — complete section/comment configuration consumer

C02 now includes its originally requested nested sections, assignments, quoted
values and comments, plus nested arrays and booleans. `config-sections.levell`
is an ordinary PEG consumer; `config-sections-tokens.levell` is an independently
lexed CFG consumer, with keyword-like names admitted explicitly. The native
lexer, packrat, settled tabulator and canonical LR run the same63 independent
Python-oracle fixtures in all four cells. They agree on acceptance and a useful
source-tree visitor that emits dotted paths and original value spellings,
including comments inside array values. Each tree retains the original source.

Tranche12 records exact provenance and full output parity. Cases cover section
nesting, malformed comments/strings/delimiters, escaped quotes, keyword names,
NUL in comments, explicit rejection outside the7-bit source profile and
truncations/mutations. This is a complete stated small format, not a claim of
Unicode or another existing configuration standard. C02's earlier coverage gap
is closed. Other C06–C08 consumer fragments, lexical recovery assessment,
lifecycle/cost work and final support/certificate audit remain open.

### Implementation checkpoint — five contrasting language consumers

C07 now executes selected pinned-source fragments for Pascal, PL/M, BASIC,
Lisp and Prolog. Tranche13 freezes347 cases with independent lexical and
recursive-reader controls. Native generated scanners, token stages, packrat,
reverse tabulation and canonical LR agree on acceptance, raw/staged token
spans and named consuming-syntax extents in all four optimized/no-opt VM cells.
`CONSUMERS.md` records the exact subset and exclusions for every language.

The retained BASIC LR conflict is resolved by a declared leading-line-number
normalizer, preserving raw positioned tokens and source. A shared raw nullable
span convention was exposed by trailing comment trivia; the explicitly stated
selected-tree adapter uses the hull of consuming leaves. Raw trees are retained
and validated, not claimed to have identical minimal spans. Prolog clause-dot
boundary validation is an explicit consumer condition. These findings feed the
composite and source-tree recommendations; no external language is refactored.

C06 actual RexxScript value/error observations and C08 broader B/G/C comparisons
are next. Their full obligations, lexical recovery assessment, lifecycle/cost
and final certificate/support audit remain open.

### Implementation checkpoint — actual RexxScript observations

C06 now has50 frozen observations from the owning evaluator, compiled with the
frozen tools and explicitly hashed existing rxfnsc/classlib dependencies. Three
read-only probe getters expose existing diagnostic fields; implementation bodies
remain unchanged. Four reference outputs agree. The Level L assignment/SAY,
integer/string and LENGTH/SUBSTR subset compares48 of those cases through a
native lexer, contextual classification, packrat/tabulation/LR and a delayed
tree visitor. Four complete native outputs agree on recognition, values, output
and selected semantic-error codes. IF/SIGNAL observations remain reference-only
here and do not close C08's broader initial-AST requirement.

Tranche14 and CONSUMERS record the observed left-to-right arithmetic, equal blank
and abuttal concatenation, omitted arguments, error coordinate differences,
factoring needed for LR admission and the retained unterminated-quote host
signal from comment stripping. This is representative consumer evidence, not
full RexxScript semantic/sandbox compatibility or a production change. C08,
lexical recovery assessment, cost/lifecycle and final proof/support audit remain
open under their existing criteria.

### Implementation checkpoint — representative B/G/C initial trees

Tranche15 freezes20 actual raw compiler observations:16 syntax-clean trees and
four explicit syntax errors. The generated lexer, parameterized comment counter,
declared token stages and packrat/reverse-tabulation source-to-tree controls
match the closed-corpus kind/value/order/structure projection in four identical
outputs. Soft assignment/label priority, concatenation, nested comments,
continuations, final clauses, IF/ELSE and PARSE templates join the earlier
procedure/declaration grouping evidence. CONSUMERS states the exact restricted
profile and frontend recommendation; MATH STAGE-02 gives masking invariants and
its conservative copying-cost limit.

The raw compiler dumps lack locations and Classic runs lack runtime imports
needed by later semantic checks. Those limitations and the resulting post-parse
errors remain explicit; no full compiler/runtime or general span-equivalence
claim is made. Native source/span invariants pass independently. An attempted
empty synthetic source token was rejected; final-clause closure is now explicit
in the grammar without weakening the common stream contract.

AC-21 is verified for its stated representative gate. Next: finish the frozen
lexical-damage recovery comparison, then lifecycle/cost measurements and the
certificate/portfolio/support audit. AC-14/15/18/20/22–25 remain open in the
current audit; production syntax/adoption/migration is still a later decision.

### Implementation checkpoint — lexical damage and recovery boundary

Tranche16 freezes34 quoted-record/comment cases, including13 held out. All four
outputs agree after correcting the independent oracle to count UTF-8 bytes;
original evidence and the unit mismatch are retained separately. Fixtures,
splits, grammars and ranking policies did not change. Twenty-eight damaged
inputs include24 that stop the standalone lexer. Raw-input LR repair reaches
a full repaired parse on28; labels on12; Pika islands and c14 histories retain
all20 designated later records, versus8 for label recovery. These are different
observations and objectives, not a single quality score.

The frontier changes Q020's tree at equal cost, with more work; neither intended
repair quality nor global optimality follows. RECOVERY.md assesses all required
quality dimensions and retains bad outcomes. AC-20's comparative gate is
verified. STEP-14 now measures the frozen native portfolio's request lifecycle,
fused recognition/tree cost, selected-tree extraction, passes and recovery,
without a product-performance claim. AC-22 and final audits remain open.

### Implementation checkpoint — lifecycle measurement boundary review

Tranche17 completed four correctness cells and serial timing/RSS runs, but
review found an outer timer that included release of the preceding candidate's
owned observation. Both host snapshots also show unrelated compiler activity.
Timings remain inconclusive; all samples are retained. The approximately2.9GB
whole-process peak needs attribution, not a per-engine or leak-free claim.
COST.md records the interpretation limits.

Corrected measuredrequest releases its own context and returns scalar metrics.
Tranche18 retains four passing correctness cells, unchanged counter observations,
four synthetic summary checks and21 negative exits. No engine/library changed.
The Level B summary now covers generation, lexer and specialist phases as well
as requests. Uncontended corrected timing and final cost/certificate/report
reconciliation remain open under STEP-14/15. Preserve other active builds;
do not run a knowingly contended timing block or silently close AC-22.


### Product checkpoint — generated grammar and first interpreter run

RS-STEP01–03 are implemented locally and under qualification; all RS acceptance
criteria remain open until the expanded matrix and integration checks finish.
The shared core now lives in `lib/rxfnsl/rexx` with research-source symlinks.
The tool emits DFA/LR images and reconstructible common grammar metadata from
`rexxscript/grammar/rexxscript.levell`; the modern interpreter is explicitly
selected through `rexxscript1` / `rexxscript1run` and `rexxscript1_cli`.

The first complete non-optimized native control admits 112 DFA states and 1,609
LR states. Independent Bison canonical-LR construction of the exact common
parser graph has no conflicts (its extra final EOF state gives 1,610 states).
A false native conflict was traced to retained scratch arrays: bare `.int[]`
declarations are not executable resets. Explicit empty-array value assignments
repair suffix-FIRST/active-symbol scratch and authored-rule attribution. A
nullable-list fixture and existing LR/LALR separation controls exercise this.
Unsupported recursion is diagnosed as a backend limitation, without incorrectly
claiming the language is mathematically nonregular. Rejection messages identify
named rules and lines, nullable tokens, conflicts and required semantics.

The first 34 interpreter cases now pass on both concrete VMs with non-optimized
Level L/interpreter modules. Initial float-literal and UTF-8 BIF configuration
failures were corrected. The example executes and prints `total: 46`, `café`,
`ok`. An immutable generated definition is initialized once per module instance;
programs own parsed source/tree and runs own mutable pools. A Level G compiler
control confirms logical AND/OR short-circuiting; the interpreter implements
that behavior and an expanded failure/lifecycle corpus is running.

CMake integration and optimized full-grammar construction are still being
checked. The latter is materially slower than the non-optimized control and
must not yet be described as qualified. Tool output now uses larger binary
literal chunks to avoid thousands of needless generated AST statements.
Build-generation uses the supported threaded VM when available, while the
normal product VM selection is unchanged. Construction is still the native
cREXX Level L implementation, not a foreign parser generator.

Next: finish the expanded cases and optimizer/VM parity, qualify the CMake
runner and another generated consumer, retain exact input hashes and receipts,
and complete product docs/feature limits. Host registration/exposure/write-back
and the seven research criteria remain open under the existing agreed scope.

### Product output decision — packed RXAS, 10 October

Adrian suggests RXAS emission and points out the packed tables already exist;
he cautions that direct RXBIN emission would be a step too far. Under RS-STEP04,
add an RXAS artifact path using the existing DFA/LR packed images and public
library loader calls. RXAS retains grammar names/locations and the same exported
`language()` contract. The assembler continues to own RXBIN serialization.
Keep generated Level B source as a differential control during review. Verify
image equality, source-tree/diagnostic behavior, compiler imports, assembler
optimization, linking and both VMs before selecting RXAS for the product build.
This changes artifact transport, not grammar semantics or admission decisions.

### Product checkpoint — RXAS and table ownership

The RXAS path assembles, imports through rxc, links and passes the first55
interpreter cases on both VMs with optimized/no-opt assembler output. The
original DFA image is21,200 bytes and LR image291,580 bytes; their bytes match
Level B emission exactly (`evidence/product-rxas/initial-image-parity.json`).
This first receipt predates the later scalar-literal/operator conformance edits.

Optimized construction was isolated to action-table population: inlined getters
retained copies of the entire receiver across writes. Direct internal reads
remove those copies without changing LR decisions; image emission similarly
avoids repeated receiver copies. New loaded tables retain immutable packed
images instead of expanding every cell to a boxed integer. LR lookup uses a
16-byte/state directory and scoped `reference .binary`/`dereference` views;
no weak reference escapes its owning table. DFA lookups read encoded offsets.
The existing portable little-endian image format stays distinct from host-native
`<packed..int>` storage. RXAS remains the serialization boundary; direct RXBIN
emission is not part of this work. Qualification of these ownership changes
and the complete current product matrix remains open.

Ownership correction: ordinary VM binary/object value copies duplicate storage;
they are not copy-on-write. Earlier conversational/checkpoint COW descriptions
were incorrect. See `docs/ai-context/RXVM_INTERPRETER.md`, value-transfer paths.
Scoped references avoid payload copies only while their owner remains alive.
Public owned results may still copy deliberately. Audit the actual emitted RXAS
and complete lifecycle checks before claiming request-level table sharing.

The first per-lookup weak-reference version was functionally progressing but
unacceptably expensive: a native sample attributed the hot path to MKREF's
lifetime-owner tree search. The final private lookup helper uses synchronous
`arg expose image, expose directory` borrowing; its body is read-only and no
alias escapes. Both images remain owned by the loaded table. This avoids the
per-lookup weak-reference creation as well as binary copies. With this change,
57 interpreter cases and4,724 second-language/image/ownership assertions pass
in all four optimizer/VM cells. A later signed-minimum scalar repair and its
expanded matrix are still under qualification. There is no general zero-copy
claim for public owned program/tree values or all initialization paths.
