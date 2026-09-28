# Beta 3 core baseline: architecture and execution proposal

Date: 28 September 2026. Status: **approved by Adrian; execution starting**.
Approval: Adrian replied “Approved” after the metadata-only SOURCELINE revision.
AD-01–08, the conditional deferral and the explicit sanitizer closeout are accepted.

## Vision and intended outcome

Establish one coherent cREXX core baseline before closing the beta 3 product
definition and selecting the built, distributed and archived component/example
sets. Incorporate the useful mainframe-discovered changes where they belong in
cREXX, preserve a sensible boundary with platform-specific source and generic
newlib/native runtime services, assess and if sensible implement SOURCELINE
from retained metadata, and repair scheduled sanitizer assurance. The result is an integrated, reviewed, tested development
baseline with exact source and QA evidence, ready for the lab to consume in its
CMS31/TSO31/TSO64 PoC/alpha release packages.

This is the execution plan for that batch, subordinate to `docs/ROADMAP.md` and
`docs/release-1-plan.md`. It implements the next action selected by Adrian after
the [product-definition review](reports/product-definition-review-2026-09-28.md).
It supersedes that review's proposed automatic transfer of #602 to Beta 4.
Adrian then narrowed SOURCELINE: attempt reconstruction from existing metadata;
unretained comments can be blank; defer #602 until after beta 3 if this is
unfeasible or not sensible. This conditional scope is explicit user direction,
not an implementation-time weakening. Other R1 design proposals remain unapproved.

Final mainframe installation/package qualification remains owned by
`mainframe-lab/docs/BETA3-MAINFRAME-PLAN.md`, B3-01–06. Completion here is not
completion of that alpha release or of beta 3. Preserve those outstanding gates
and the later product-definition, catalogue, example and distribution work.

## Inspected inputs

- Published cREXX `develop`: `143921e11e4d573909fcc4def28da5dceadba9d9`.
- Scheduled/default branch `master`: `48ebc1f610a948a39972348379fd02fff4b156ad`.
- Primary checkout: `47168a1f16365d6c2aaa54de770889c0e8dce6a4`, with unrelated
  source and documentation changes. Preserve this checkout and use an isolated
  current-develop workspace after approval.
- Mainframe Lab published `main`: `1742740df60c0858421f196993e55da3edcb681b`;
  read the current lab plan again before consuming later changes.
- Lab active `patches/crexx-cms-release-poc/linux-topics.series`: topics 0013–0017.
  Older `patches/crexx/` topics 0010–0013 contain additional TSO adaptations,
  overlapping sentinel work, and compiler/VM allocation repairs.
- cREXX references: `docs/ai-context/CREXX_ARCHITECTURE.md` (import discovery),
  `RXVM_INTERPRETER.md`, `RXAS_ASSEMBLER.md`, `RXLINK_LINKER.md`,
  `RXBIN_007_SEMANTIC_GRAPH.md`, `RXPP_PREPROCESSOR.md`, `CREXX_ASAN_TESTING.md`,
  published `ports/single-threaded/README.md` and `CMS-TEXT.md`,
  `compiler/docs/levelc_classic_bifs.md`, and the current shared Classic BIF and
  RexxScript developer guides. Read the relevant authoring guide before Rexx edits.

## Approved architecture decisions

### AD-01 — One shared language and toolchain core

The compiler owns namespace/header matching, provider selection, ordered source
root precedence, diagnostics and source semantics. The binary tools own portable
RXBIN representation and validation. The VM/core own values, allocation checks,
execution/source context and lifecycle. These rules are common to desktop and
mainframe builds. General repairs remain ordinary shared code, even when a
mainframe workload first exposed them.

Do not require filenames to equal namespaces, omit providers to fit a native
directory limitation, change Rexx integer widths, create a mainframe RXBIN
dialect, or add OS conditionals to parser/resolver semantics. Preserve multiple
providers in the first selected root and the documented source/binary root split.

### AD-02 — cREXX platform source owns cREXX-to-host adaptation

The `platform/` boundary owns cREXX logical filename/type/root mapping to native
names, directory/member discovery, executable/path services, text-encoding
selection and routing, and declared unavailable host capabilities. For example,
mapping cREXX `name.type` plus a library root to a TSO dataset/member belongs
here. Native APIs must expose actual directory/member entries; the compiler then
applies its normal selection rules.

Retain the existing `openfile`, `fileexists`, `dirfstfl`, `dirnxtfl`, `dirclose`
and text-adapter contracts where suitable. Add or regularize only the minimal
hooks needed by the current ports, with explicit iterator ownership, error/EOF
behavior and cleanup. Put CMS/TSO-specific implementation behind those contracts
in cREXX platform source. The cREXX-specific parts of lab `runtime/tso/platform.c`
and `files.c` must be classified and brought across or factored behind generic
runtime services; exporting them as generic newlib functionality is unsuitable.

Select CMS/TSO and capabilities explicitly. Toolchain-defined Linux macros must
not accidentally select Linux services for a non-Linux target. Use product-owned
platform configuration, accepting old lab defines only as documented transitional
build aliases if needed. Avoid a new general plugin framework or a wholesale
platform-directory reorganization for this batch.

### AD-03 — newlib and native runtime remain application-independent

Generic newlib/C runtime responsibilities include C allocation/stdio behavior,
errno, byte/record stream services, native DD/DSN access, directory primitives,
exit/flush, and ABI/native service linkage. The runtime/OS adapters own native
low-address buffers/control blocks, entry modes, stack setup, register preservation
and cleanup. Mainframe Lab owns their source/build/package qualification.

No cREXX namespace policy, source-extension precedence, RXBIN knowledge or BIF
semantics belongs in generic newlib. General native I/O/transcoding helpers may
remain reusable runtime facilities. cREXX's platform layer selects the encoding
and applies its logical naming policy. Each stream is converted exactly once;
binary input/output is never transcoded. Preserve the existing non-seekable
source-stream and deferred close-error contracts. A remaining runtime dependency
must be a documented service with ownership and tests, not an invisible source
rewrite in a packaging script. No generic-newlib fork is selected by this plan.

### AD-04 — Integrate causes, not historical patch stacks

| Change family | Owning layer and integration rule |
| --- | --- |
| RXBIN compression workspace | Shared binutils: checked per-call heap workspace, unchanged algorithm/format, cleanup on every failure. Preserve byte-equivalence evidence and measure constrained-build memory impact. |
| Native discovery and TSO naming | cREXX platform mapping/enumeration backed by generic runtime services. Reconcile the newer common hook and older TSO-specific hooks into one maintained route. |
| First-root namespace precedence | Shared compiler resolver; same behavior and regression on every platform. |
| RXBIN import diagnostics | Shared compiler/binary loader; retain the useful cause and nonzero failure status. |
| Imported-procedure sentinel | Shared RXBIN reader/writer: canonical portable sentinel, accept documented older ILP32 import spelling, preserve checks for overflowing real addresses. Combine the older complete repair/tests and newer pipeline fix. |
| Symbol and value factory allocation failures | Shared compiler/VM: check allocation before use and follow the existing panic policy. Cover both worker-owned and standalone value factories. |
| Exhaustion diagnostics | Shared bounded panic path plus platform output service where required. Prove that reporting exhausted memory does not itself require fresh heap allocation; integer formatting alone is not proof. |

Reproduce the compiler-exit load/false-success case against current code. If the
shared path can omit a required exit after a load failure, repair the failure
propagation and add a distinguishing regression in this batch. If it no longer
reproduces, record the current evidence and exact disposition. Do not hide it by
reducing the tested language surface or enlarging a heap without fixing status.

Already-upstream CMS foundation changes are retained, not reapplied. Keep
separate reviewable commits for distinct causes while using combined final QA.

### AD-05 — SOURCELINE uses existing retained metadata, conditionally

Adrian clarified that SOURCELINE should attempt to reconstruct source from the
metadata already retained; comments may be blank and only retained code source
need be returned. Do not add complete-source storage to deliver this beta feature.
If this approach is infeasible or produces an incoherent language/API contract,
report the evidence and leave #602 open for post-beta-3 work. Do this feasibility
check early, before investing in a larger implementation.

Approved bounded behavior:

- Resolve the calling program's module and source-file identity from its existing
  source metadata. Do not inspect the BIF helper's own source or mix different
  imported files whose line numbers happen to match. Existing RXPP/source-map
  identities govern which original source the retained record identifies.
- `SOURCELINE(n)` returns the retained whole-line text for one-based line n,
  without its terminator. A gap within the known range returns an empty string;
  this includes unretained comments, blanks and other unretained lines. Do not
  invent the text of a continuation or a line removed from the metadata.
- Approved no-argument result: the highest retained line number for that source
  identity, or zero if no source metadata is available. This is the known line
  extent, not proof of the original file's full length; trailing unretained
  comments cannot be counted. This qualification must be documented. If the
  existing metadata cannot identify even this extent reliably, defer the feature.
- Validate at most one optional positive whole-number argument using shared
  Classic argument checks. Beyond the known range uses 40.34; malformed,
  nonpositive and extra arguments retain the existing appropriate errors.
  Explicitly omitted and explicitly empty arguments remain distinguishable.
- Read retained source through a single core service and pass the semantic
  caller context explicitly where needed. If proceeding, expose real compiled
  Level C use, the shared Classic helper, a typed Level B entry and the
  RexxScript adapter where equivalent source metadata exists. Do not introduce
  a new source-retention subsystem for the evaluator merely to cover it; an
  unavailable source context returns zero and is documented/tested as such.
- Approved typed Level B entry returns `.string` for both forms, with the count
  as decimal text; Classic results use `RexxValue`. This avoids introducing
  general return-type overloading. Use the existing typed error convention.
- Never reopen source files at runtime. Do not manufacture complete source from
  original files, AST pretty-printing or generated compiler implementation text.

The feasibility receipt must show real caller identity, multiple procedures,
imports, RXPP mapping, gaps/comments, conflicting duplicate anchors, unavailable
metadata and linking/stripping. Identical duplicate anchors can be coalesced;
conflicting text for the same identity must not be resolved by arbitrary order.
Explain any unresolved ambiguity and recommend deferral instead of extending
beta scope. Completion, if accepted, is this documented metadata-based behavior,
not an unqualified claim of full Classic source-retention compatibility.

### AD-06 — No new source-storage or RXBIN format project

Reuse `.srcstep` / `META_SOURCE_STEP` and the existing metadata read facilities.
The whole-line payload is useful, but sparse executable anchors are not a
complete source-file archive. A small core lookup or derived index is acceptable
if it has explicit ownership, bounded allocation and cleanup; do not add new
source-table records, RXBIN feature flags or RXAS source-table directives for
this work. There is no new format or per-activation copy of whole sources.

Existing `rxlink -s` / `STRIP SOURCE` removes the source-step data. Queries then
report unavailable source (zero count; numbered query out of range), including
older binaries without useful metadata. Linked images keep original module/file
identity. Avoid baking source text into executable constants in a way that
bypasses source stripping.

Optimization can change which anchors remain. Return only the metadata actually
retained, preserving the text/identity of common retained anchors; do not promise
that optimized and unoptimized source coverage is identical or change optimizer
policy merely to retain comments/removed code. Caller identity must still be
correct through supported inlining. If the mechanism requires a broad compiler,
VM or optimizer redesign to be sensible, take the user-authorized deferral.

### AD-07 — Repair the scheduled sanitizer workflow at its actual source

Schedules execute workflow configuration from the default branch, even though
the gate checks out develop's resolved target. `master` still has the old
120-minute job backstop; develop has 240 minutes, Python setup and the explicitly
selected first-party `ENABLE_LLAMA=OFF` boundary. Synchronize the relevant
sanitizer workflow configuration onto the default branch using a narrowly scoped
change. Do not merge all unreleased product work into master for this repair.

Preserve the existing supported Linux ASan/LSan and macOS ASan lanes, runner,
test scheduling, artifact retention and success-marker semantics. Record both
workflow revision and actual tested product revision. No test deletion,
suppression, supported leak-disable or relaxed success criterion is authorized.

One completed Linux/macOS sanitizer matrix is the approved explicit baseline
closeout gate because current scheduled assurance was cancelled and the batch
changes allocation, binary loading and source ownership. Reuse a scheduled run
covering the final candidate if available; approval of this plan authorizes one
targeted manual dispatch if needed to verify the repaired pipeline. Publish
normally after the required focused/normal correctness checks; this closeout
gate need not delay ordinary develop integration. Repeat only for changed inputs
or a concrete failing gate. Actual first-party findings follow SAN worklist rules.

### AD-08 — One integration worker, independent coordinator review

Adrian approved this plan. Start one Astra Extra High implementation
subagent in an isolated current-develop worktree. The worker integrates the
approved cREXX changes, writes permanent regressions, performs QA, maintains
this plan's evidence/status and delivers reviewable commits. It may adapt patch
mechanics within these decisions; changing the boundary, metadata-query/stripping
contract or acceptance scope requires returning for a decision.

The coordinator independently inspects the diffs, reproducer/test quality,
source/artifact identity and decisive results before accepting integration.
Reuse unchanged worker evidence instead of repeating complete suites. Approved
delivery is develop integration plus the narrowly scoped default-branch QA
repair and normal automatic CI verification; no beta tag or release publication.
Attach any task PRs. No new user-owned chat or lab guest operation is implied.

## Numbered acceptance criteria

All criteria are **OPEN** pending execution; architectural approval is recorded above. Every closure must
cite the qualified source revision, command/result and retained artifact/log.

| ID | Observable pass condition | Required evidence |
| --- | --- | --- |
| AC-01 | Every candidate change is integrated, superseded, rejected with reason, or assigned an explicit runtime dependency; the core/platform/newlib boundary matches AD-01–04. | Patch-to-commit/disposition matrix, interface docs and coordinator diff review; no invisible build-script source rewriting. |
| AC-02 | Imports preserve namespace independence, root ordering, multiple same-root providers and normal desktop semantics. Missing/malformed input and required compiler-exit load failures return failure without stale success output. | Permanent positive/negative reproductions using normal compiler paths and injected platform enumeration/I/O; current false-success disposition. |
| AC-03 | RXBIN compression and cross-width import representation are correct; all added allocation/error paths clean up or follow the existing terminal panic contract without null dereference. | Distinguishing compressor/sentinel/OOM regressions, byte-equivalence/round trips, ILP32 and LP64 coverage, forced allocation/output failures. |
| AC-04 | Platform selection, logical/native naming, enumeration, conversion and resource ownership obey the documented contract. Binary bytes remain exact and conversion occurs once. | Desktop plus explicit CMS/TSO platform contract tests, non-seekable/short reads, read/write/close failures, repeat/cleanup tests and supported constrained-build compile checks. Identify actual toolchain availability; no guest proof inferred from host tests. |
| AC-05 | SOURCELINE receives an evidence-backed feasibility disposition. If feasible, real compiled Level B/Level C use and the shared service obey the approved metadata-only contract; otherwise #602 stays open with a post-beta-3 handoff. | Real caller/module/file and sparse-line/extent proof. If implemented: count, gaps/comments, Unicode, arguments, imported/RXPP identities and missing source files; document RexxScript availability without adding a source store. A helper-only pass does not prove compiled use. |
| AC-06 | If SOURCELINE proceeds, its lookup preserves retained module/file/text identity and lifecycle through supported optimization, procedure/import/linking and nested contexts; stripping/no-source behavior follows AD-06. No new source table or format extension is introduced. | `rxc`→`rxas`→`rxlink`→VM tests in both optimizer modes, duplicate/conflicting-anchor controls, cross-module/inlining, lookup lifecycle, stripped/no-source controls. If AD-05 is deferred, record this conditional implementation criterion as not applicable with the feasibility evidence and #602 still open. |
| AC-07 | The batch passes a core product build, focused normal regressions and the relevant normal correctness suite; focused sanitizer checks pass where the changed ownership/error paths warrant them. | Exact-input local receipts; use `tools/asan-run.sh` for sanitizer commands. Run broader correctness once for the combined implementation and reuse valid results. |
| AC-08 | Scheduled/default-branch QA is repaired, and the final combined baseline has completed Linux ASan/LSan and macOS ASan assurance without unresolved first-party SAN findings. | Narrow workflow diff; terminal hosted jobs/logs, actual checkout SHA and workflow SHA, no false success marker from cancellation. Distinguish Apple LSan capability limit. |
| AC-09 | Reviewed commits are integrated into develop and the workflow repair into the default branch; relevant automatic publication checks pass. | Commit/PR links, Build/CodeQL results and any affected ordinary packaging/optimizer checks. No beta release claim. |
| AC-10 | Guides, #602 evidence and handoff match the implemented contract; lab can consume one exact baseline and sees remaining native/package work. | Updated human/agent docs, matching tool/library manifest, patch reconciliation, #602 closed only for the accepted implemented contract after AC-05/06 and QA pass, or visibly open with its authorized post-beta-3 disposition, final report naming remaining B3 gates and product-definition phase. |

## Numbered implementation steps

1. **STEP-01 — Approval and workspace.** Serves AC-01/09/10. Obtain approval of
   AD-01–08, record it here, inspect attached worktrees and reuse a suitable free
   one or create an isolated current-develop checkout. Preserve all primary edits.
   Copy/link this plan and exact lab inputs into the execution handoff; start the
   approved implementation subagent only after this gate.
2. **STEP-02 — Reconcile architecture and reproduce.** Serves AC-01–04. Map the
   active and older lab patches to current source, document minimal platform
   interfaces within the approved design and retain small distinguishing failure
   cases. Evaluate the metadata-only SOURCELINE contract early and report a
   proceed/defer recommendation. Report any material design
   conflict before editing production behavior.
3. **STEP-03 — Integrate portability and correctness repairs.** Serves AC-01–04/07.
   Implement shared binutils/compiler/VM fixes and bounded platform adapters in
   separate causal commits. Preserve published foundation work. Run focused
   tests as each change lands; stop on unintended regressions.
4. **STEP-04 — Implement bounded SOURCELINE or retain its issue.** Serves
   AC-05/06/07. On a sensible feasibility result, reuse existing source metadata
   for lookup/context binding and real Classic/typed entry points; document the
   evaluator boundary and known-line extent. Test retained identity, gaps, errors,
   links, stripping and lifecycle. If the contract is infeasible or not sensible,
   retain #602 open after beta 3, record the reason and continue the other repairs.
5. **STEP-05 — Repair workflow and run combined local QA.** Serves AC-07/08.
   Prepare the minimal default-branch sanitizer update and validate its target
   selection/configuration. Run the justified combined normal suite and focused
   ownership sanitizer checks once on frozen inputs. No unrelated performance
   programme or mainframe OS/runtime redesign is included.
6. **STEP-06 — Independent review and integration.** Serves AC-01–07/09. Worker
   supplies the patch matrix, commit IDs, complete AC statuses and receipts.
   Coordinator checks architecture, actual behavior and evidence. Integrate
   reviewed changes into develop, deliver the narrow default-branch workflow
   repair, and inspect the normal automatic publication checks.
7. **STEP-07 — Terminal sanitizer closeout.** Serves AC-08/09. Reuse a valid
   scheduled final-candidate run or perform the one explicitly agreed dispatch.
   For timeout-only failures, follow isolated scheduling diagnosis before any
   broad repeat. For real findings, create/update SAN entries, repair and retain
   the required focused and platform closure proof. Keep this criterion open
   until the actual matrix completes.
8. **STEP-08 — Accept baseline and hand off definition work.** Serves AC-10.
   Close #602 only if implemented to the accepted bounded contract; otherwise
   retain it open with the authorized post-beta-3 disposition. Publish an exact-baseline
   handoff and remaining native B3 dependencies. Then return to product policy,
   catalogue and built/shipped/archive choices, preserving useful archived assets
   and their explicit support status. Do not start that reclassification during
   this integration batch or equate it with completing the beta release.

## Current handoff

- Architecture decisions: AD-01–08 approved on 28 September 2026.
- AC-01–10: open. STEP-01 complete: isolated checkout on `temp/beta3-core-baseline` at the inspected develop revision; primary checkout preserved.
- STEP-02 in progress; STEP-03–08 open.
- Execution checkout: `/Users/adrian/.codex/worktrees/beta3-core-baseline/CREXX`.
  This execution copy is the live status/evidence record; the primary checkout
  copy is retained for review and will be synchronized after acceptance.
- Selected scope: cREXX integration, architectural reconciliation, a conditional
  metadata-only SOURCELINE attempt, sanitizer workflow repair and baseline QA;
  subsequent definition/archive work. Adrian explicitly allows blank comments
  and deferral of SOURCELINE if reconstruction is infeasible or not sensible.
- Next action: establish the isolated execution checkout, start the approved
  implementation agent, and begin patch reconciliation and SOURCELINE feasibility.
  No production source, workflow, issue or release has yet changed.

## Execution receipts

### 28 September — reconciliation in progress

- Worker reread AGENTS.md and this complete approved plan before edits. Lab HEAD
  remains `1742740df60c0858421f196993e55da3edcb681b`; B3-01/02/03/05/06 remain
  native/package gates in the lab, not acceptance claims for host tests.
- Coordinator delivered the narrow default-branch sanitizer workflow repair as
  `2d24ae989fdb530942c73d81301d6affd243a671`. Only
  `.github/workflows/sanitizers.yml` changed; it matches develop's blob
  `f3806cda1`. Coordinator reports `actionlint` and `git diff --check` passed.
  AC-08 is partial: scheduling source repaired; final candidate matrix OPEN.
  No worker workflow edit or dispatch. STEP-05 final local QA remains open.

### Patch reconciliation register

| Lab candidate | Disposition / owning layer | Implementation evidence |
| --- | --- | --- |
| Earlier CMS topics 0001–0012 | Upstream foundation retained, no blind replay | Current platform text/directory and constrained build code inspected; pending final manifest |
| Active 0013 compression workspace | Shared binutils; adapt checked heap table and add failure tests | Open |
| Active 0014 native directory hook | Reconcile into cREXX platform adapter backed by native member services | Open |
| Active 0015 RXBIN import diagnostics | Shared compiler; preserve binary cause and nonzero required failure | Open |
| Active 0016 + older 0011 sentinel | Older writer/reader/legacy spelling is complete superset of newer decoder | Open |
| Active 0017 namespace source root | Shared compiler first-root rule, multiple same-root providers retained | Open |
| Older 0010 TSO platform | Explicit product profiles and logical mapping in cREXX; generic records/stdio/encoding below | Open |
| Older 0012 symbol allocation | Shared compiler terminal allocation checks | Open |
| Older 0013 value factories | Shared VM worker and standalone allocation checks | Open |
| TSO integer-only exhaustion formatting | Supersede with bounded heap-independent diagnostic path; integer formatting alone insufficient | Open |
| Compiler-exit false success | Reproduce current `library` bridge load failure and distinguish required/optional modules | Open |
| SOURCELINE #602 | Early metadata/caller feasibility assessment under AD-05/06 | Open |
