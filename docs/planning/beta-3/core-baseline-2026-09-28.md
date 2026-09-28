# Beta 3 core baseline: architecture and execution proposal

Date: 28 September 2026. Status: **approved by Adrian; integration and qualification in progress**.
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

**28 September native-integration clarification:** Adrian explicitly accepts
minor later changes to `platform_fopen()` by the mainframe agent once native
details are settled. Its platform implementation and supporting header/backend
mechanics may be reconciled within AD-02/03; current draft signatures are not
an immutable ABI. Keep native flags inside the platform boundary and bring
the adjustments upstream with focused QA. Preserve cREXX conversion ownership,
byte-exact binary I/O and explicit record/line handling. This permission does
not close unverified AC-04/AC-11 or the lab's native/package gates.

### AD-03 — newlib and native runtime remain application-independent

Generic newlib/C runtime responsibilities include C allocation/stdio behavior,
errno, byte/record stream services, native DD/DSN access, directory primitives,
exit/flush, and ABI/native service linkage. The runtime/OS adapters own native
low-address buffers/control blocks, entry modes, stack setup, register preservation
and cleanup. Mainframe Lab owns their source/build/package qualification.

No cREXX namespace policy, source-extension precedence, RXBIN knowledge or BIF
semantics belongs in generic newlib. The subsequent [text-boundary clarification](text-boundary-2026-09-28.md)
supersedes the tentative runtime-transcoder allocation: common cREXX codec
code owns conversion and the cREXX platform layer selects and applies it.
The cREXX route requires raw native byte/record services with no implicit
transcoding. Native record boundaries and logical text lines remain distinct.
Each stream is converted exactly once;
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

Adrian approved this plan and initially selected one Astra Extra High implementation
subagent in an isolated current-develop worktree. On 28 September he authorized
switching that worker to GPT-6 Sol Extra High at an appropriate checkpoint.
The outgoing worker saved the linked handoff after its current focused ASan
operation completed, stopped editing and preserved the same checkout and all
test evidence. The replacement continues with Sol Extra High; the coordinator
retains independent architectural review. Only one implementation worker edits
the checkout at a time. The worker integrates the
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

Architectural approval is recorded above; criterion status and evidence are
maintained in the current handoff and dated execution receipts below. Every
closure must cite the qualified source revision, command/result and retained
artifact/log. Unverified outcomes remain open.

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
| AC-11 | Every maintained CMS31/TSO31/TSO64 text boundary is coherent UTF-8 internally with explicit external encoding, exactly-once conversion, strict failures and unchanged binary bytes. Includes source/imports, assembler/linker/disassembler, VM stdio/arguments/diagnostics, file/stream APIs, tools/driver/RXPP where available, native names and environment. Source/file selectors include UTF8, ASCII, Latin1, Windows-1252, IBM437, IBM850 and IBM1047 using existing authoritative maps. Classic BYTE/UTF8 and replacement-codec policies remain unchanged. | Complete component/profile/selector capability matrix, native runtime and core host fixtures for non-ASCII, chunk/record/close/error boundaries and binary high bytes. No blanket parity claim from source smoke; actual unavailable components explicit. Native package B3 proof remains separate. |
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

**28 September replacement worker checkpoint:** the complete
[takeover handoff](worker-handoff-2026-09-28.md) records every AC/step and the
earlier source/test boundary. The Sol Extra High worker continued in the same
checkout; latest implementation commit is `2df36e28e`, followed by local
documentation-only audit/handoff commits. The later current-status receipts
below supersede the outgoing worker's draft inventory. Draft PR #709 later
published the reviewed cREXX candidate; no final sanitizer workflow dispatch
or develop integration occurred.

The reviewed cREXX candidate and local AC-07 acceptance were recorded at
`30d723054`. Draft [PR #709](https://github.com/adesutherland/CREXX/pull/709)
now publishes `a4a39dc3b9c63fff9c726842f1c8ea43ded9326e`, including the
portable source-root fixture rename (`369cbeb7b`), parser-mode fixture
repairs (`1366df3c1`, `00d42cb3a`) and coordinator acceptance. The new
ordinary Build `36440639967` and CodeQL `36440638876` were in progress at
15:22 UTC on 28 September; all Linux/Windows/macOS core jobs and Linux
optimizer parity had passed, while plugin jobs and CodeQL were still running.
`develop` remains at `143921e11`. Product implementation
inputs remain those at `ce4a9273f`; no later production code changed.

STEP-01 complete, STEP-02/03 in progress, STEP-04 complete by authorized
SOURCELINE deferral, STEP-05 local QA accepted, STEP-06 review/draft publication
in progress, STEP-07–08 open. AC-05 satisfied by evidence; AC-06 not applicable;
AC-07 satisfied for the local candidate by coordinator-reviewed combined and
focused repair evidence at `30d723054`. The coordinator subsequently accepted
AC-01 patch disposition, AC-02 and AC-03 on the unchanged published product
inputs at `a4a39dc3b`, after reviewing the completed register and actual
hosted ILP32 evidence; see the shared-core acceptance receipt below. All
remaining criteria stay open/partial
as detailed below. The earlier hosted Build on `30d723054` found two fixture
causes and did not pass; its focused repair evidence is in the
[hosted fixture receipt](../../qa/beta3-core-baseline/hosted-fixture-repair-2026-09-28.md).
Shared cREXX conversion and raw native services are authoritative; the
tentative runtime map-setter draft was removed before the platform commit.

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
  No worker workflow edit or dispatch. The later AC-07 local QA acceptance is
  recorded below; this dated setup observation predates it.

### Patch reconciliation register

| Lab candidate | Disposition / owning layer | Implementation evidence |
| --- | --- | --- |
| CMS release-PoC topics 0001–0010 | Upstream foundation retained, no blind replay | Already in base `143921e11`: `e05066b01` through `20e3de226` cover CMS ELF/C99/sequential RXAS input, environment/process separation, compact VM/allocator/RXC embedding, CMS directory services and platform selection. Later core commits retain these paths. |
| CMS release-PoC topic 0011 | Retained converted-text compatibility in the base; superseded for the proposed cREXX raw route once its backend is integrated | Base `e68681cd1`; feature-gated cREXX adapter `177c077aa`. Existing lab packages may still use the old route until cutover. Do not stack converted newlib text under cREXX decoding; native backend/package proof open. |
| CMS release-PoC topic 0012 | Reject filename-equals-namespace fallback when enumeration is unavailable; assign real enumeration to native runtime | The patch explicitly misses providers whose filename differs from namespace. Shared first-root/multiple-provider resolver `7e5771d2a` and platform raw directory adapter `177c077aa` retain the correct model; actual CMS iterator/backend remains an AC-04/11 dependency. |
| Active 0013 compression workspace | Shared binutils, checked heap table and failure tests | Committed `4ce9de623`; LP64 local proof and actual Linux `gcc -m32` ILP32 check passed in PR Build job `108975377026` at published head `30d723054`. |
| Active 0014 native directory hook | cREXX platform adapter backed by raw native member service | Compiler error propagation committed `7e5771d2a`; CMS/TSO adapter committed `177c077aa` with sequential-input follow-up `e287d5f2f`. Host mocks and constrained host build pass; actual raw backend/package proof open. |
| Active 0015 RXBIN import diagnostics | Shared compiler, retain binary cause and nonzero failure | Committed `7e5771d2a`; real malformed-import regression passes Debug and Apple ASan. |
| Active 0016 + older 0011 sentinel | Canonical portable writer and legacy ILP32 spelling reader | Committed `4ce9de623`; actual Linux `gcc -m32` ILP32 reader/writer passed in PR Build job `108975377026` at `30d723054`. |
| Active 0017 namespace source root | Shared compiler first-root rule, same-root providers retained | Committed `7e5771d2a`; source-root and private-dependency normal regressions pass. |
| Older 0010 TSO platform | Explicit product profile, logical mapping and raw native service route | Profile/name/directory/file adapters and console/process-input wrappers committed `177c077aa` and pass CMS/TSO host mocks; core CLI selector follow-up `61fdba645` passes. Product startup/stdio routing and actual backend/package gates open. |
| Native VM environment crossing | Shared VM `GETENV` uses platform-decoded raw environment service on CMS/TSO, while desktop lookup stays unchanged | Committed `2df36e28e`; CMS/TSO host mocks and optimized/unoptimized VM getenv pass matching Debug and Apple ASan tests. Other process-input crossings and actual backend/package gates open. |
| Older 0012 symbol allocation | Shared compiler terminal allocation checks | Committed `34cc3a725`; fault CTest passes Debug and Apple ASan. |
| Older 0013 value factories | Shared VM worker and standalone allocation checks | Committed `34cc3a725`; fault CTest passes Debug and Apple ASan. |
| TSO integer-only exhaustion formatting | Bounded heap-independent diagnostic path | Shared OOM/UTF-8 and native raw emergency output committed `177c077aa`; host mocks pass Debug and Apple ASan. Actual backend and bounded emergency-record capacity remain open. |
| Compiler-exit false success | Required `library` load fails, optional exit warning/fallback remains | Committed `7e5771d2a`; real PARSE/ADDRESS regression and CTest pass Debug and Apple ASan. |
| SOURCELINE #602 | Feasibility disposition; user-authorized deferral | Complete for this batch; #602 OPEN and AC-06 not applicable. |

### SOURCELINE scope clarification and disposition

Adrian further clarified during execution: metadata-only SOURCELINE is not a
beta 3 requirement; update #602 accordingly. A future Level C design may use an
explicit source-file-available solution. No general Level B/G commitment is
selected, and no source-file fallback is to be implemented in this batch.
This explicit scope amendment supersedes conditional implementation in AD-05/06.

**AC-05 satisfied as a feasibility disposition; AC-06 not applicable. STEP-04
complete by authorized deferral; #602 remains OPEN.** Real baseline compilation
of separate `alpha/unit.crexx` and `beta/unit.crexx`, with namespaces alpha and
beta, followed by binary imports and normal inlining into main, emits conflicting
`unit.crexx:6` anchors (`return value + 11` versus `return value + 29`) in the
same physical module. META_SOURCE_STEP has no original module identity. A lookup
cannot distinguish these sources by the approved module/file/line key; choosing
one text arbitrarily is forbidden. Fixing source identity transport is a future
compiler/inliner provenance design, not a comment-retention tweak. Existing
source-step stripping does not repair this collision.

The compact [probe log](../../qa/beta3-core-baseline/sourceline-probe.log)
and the [actual caller RXAS](../../qa/beta3-core-baseline/sourceline-main.rxas)
retain commands and conflicting source anchors from product revision
`143921e11e4d573909fcc4def28da5dceadba9d9`. Providers are retained beside them.
Coordinator independently inspected this evidence. No further SOURCELINE QA or
implementation is required by the clarified beta 3 scope; the coordinator owns
the #602 issue update.

### Expanded text boundary outcome

Adrian explicitly confirmed all maintained components must have coherent
EBCDIC/UTF8 conversion both ways, then included classic ASCII and existing
Windows/legacy pages. AC-11 records the clarified outcome and remains OPEN.
STEP-02/03/05 now include a complete text-boundary audit and host verification.
Bootstrap/native paths reuse the retained `rxunicode` mapping files, not a
running Rexx codec or divergent hand-coded tables. No UTF16/32 tool-stream
selector or new codepage policy is selected. Source/API bytecode stays binary.

Adrian then explicitly clarified that EBCDIC/code-page conversion belongs in
cREXX so the lab can supply the correct newlib services. The authoritative
[text boundary](text-boundary-2026-09-28.md) supersedes the tentative runtime
map-setter design: shared common cREXX codecs; cREXX platform selection,
conversion and logical line adaptation; raw byte/record/native services below.
Passing cREXX codec maps to newlib is NOT the selected architecture.

The coordinator's isolated lab checkout
`/Users/adrian/CLionProjects/mainframe-lab-beta3-text-runtime`, branch
`temp/beta3-text-runtime`, base `1742740df60c0858421f196993e55da3edcb681b`,
contains only a superseded unpublished runtime draft. It is not a delivered
baseline dependency. Adrian is handing the raw-runtime contract to the
mainframe agent. Tentative uncommitted worker map-setter declarations/routing
are being reconciled and must not be published. AC-04/AC-11 remain OPEN until
raw service signatures, cREXX adapters and combined host evidence are complete;
actual native packages still require the separate B3 gates. No guest operation
is authorized here.

### Outgoing worker checkpoint (28 September, 11:26 local)

- Commits: `27614024d` approved plan/review; `4ce9de623` RXBIN compression and
  portable sentinel repair plus focused Linux ILP32 normal-CI command.
- Normal Debug baseline `all` build passed (log `/tmp/beta3-baseline-build.0pyDsF`).
- Five focused normal cases passed: compression workspace, compact format,
  symbol allocation, value factory allocation and both source-root orders;
  `/tmp/beta3-core-focused.SG3Jq7`. The new source-root fixture fails against
  unchanged baseline RXC (`38` where first-root expectation is `35`).
- Real mandatory-exit regression passes after repair: absent/corrupt library
  and missing certified exits fail, working PARSE executes `before/hello/after`,
  optional exit load keeps its existing warning/fallback, malformed RXBIN
  import fails with retained cause. Baseline PARSE/ADDRESS silently disappeared
  with status zero; compact receipt retained under `docs/qa/beta3-core-baseline`.
- At this earlier checkpoint, compiler root/load/allocation and native platform
  edits remained uncommitted. Shadowed source providers
  now have a separate flag so dependency manifests do not hash unused bodies.
- The outgoing worker's final focused Apple ASan build completed PASS in
  `cmake-build-debugasan/asan-logs/20260928-113702-build`. Apple LeakSanitizer
  is unsupported; no broad local or hosted sanitizer matrix was launched.

### Replacement worker checkpoint (28 September, 10:55 UTC)

- Local, unpushed HEAD `7e5771d2a` follows `34cc3a725` allocation repair and
  `4ce9de623` binutils repair. The compiler commit adds first-root namespace
  selection, required-exit/RXBIN failure status, directory enumeration/close
  failure propagation and explicit rediscovery reset. No primary or lab source
  checkout was modified.
- Debug command `cmake --build cmake-build-debug --target
  test_import_directory_errors check_compiler_load_failures --parallel 8`
  passed (`/tmp/beta3-takeover-build.log`). The matching seven focused CTests,
  including the private-dependency convergence test, passed 7/7
  (`/tmp/beta3-takeover-focused.log`).
- Apple ASan focused build passed under `tools/asan-run.sh` at
  `cmake-build-debugasan/asan-logs/20260928-114734-build`; focused
  `symbol_allocation_failure`, `value_factory_allocation_failure`,
  `import_directory_errors`, `source_root_namespace_order` and
  `compiler_load_failures` passed 5/5 at
  `cmake-build-debugasan/asan-logs/20260928-115011-ctest`. No sanitizer
  diagnostic was observed. Apple LSan remains a capability limit.
- The **uncommitted** shared codec and bounded OOM drafts at this checkpoint build and pass
  `text_codec`/`oom_diagnostic` 2/2 in Debug (`/tmp/beta3-codec-oom-build.log`,
  `/tmp/beta3-codec-oom-test.log`) and Apple ASan
  (`cmake-build-debugasan/asan-logs/20260928-115149-build`,
  `20260928-115157-ctest`). The codec regression covers all seven selectors,
  full byte round trips, CP1252 C1 policy, malformed UTF-8, mapping failures
  and chunk boundaries. These results **do not** qualify the unimplemented
  native raw stream/console/name adapters or the final combined tree.
- `ports/single-threaded/CMakeLists.txt` now lists extracted `platform/oom.c`;
  a constrained standalone build was still due at this checkpoint. The lab raw
  backend and actual CMS31/TSO31/TSO64 package gates are external dependencies.
- AC-01/02/03/07 remain partial, AC-04/11 and product AC-09/10 remain open,
  AC-08 awaits final combined Linux/macOS sanitizer assurance. STEP-02/03/05
  continue; STEP-06–08 remain open. Freeze combined inputs and run the normal
  correctness suite once after the platform work, then coordinator review,
  ordinary publication and the one approved hosted sanitizer matrix.

### Current implementation status (28 September, 11:36 UTC)

- Latest local implementation commit is `61fdba64573f923327a40bb50901e268b0f45825`,
  following reviewable platform commit `177c077aa` and sequential-input
  regression commit `e287d5f2f`. Superseded native map setters and converted TSO
  file/directory calls have been removed from its raw route. `platform_fopen`
  still owns native open mechanics; codec and physical byte/record storage are
  independent. IBM1047 defaults to native records, the six exchange selectors
  to explicit-LF byte streams, and binary modes remain raw bytes. Adrian
  permits small later platform/backend signature adjustments as native details
  settle; combined native qualification remains open.
- CMS/TSO host mocks now cover `platform.c` file routing, raw name/directory
  adaptation, read/write/flush/close errors, short writes, record capacity,
  binary high bytes, emergency stderr, owned IBM1047 standard-stream wrappers,
  and length-aware argument/environment conversion. Debug build/test passed at
  `/tmp/beta3-raw-errors-build.log` and `/tmp/beta3-raw-errors-test.log` (2/2).
  Matching Apple ASan build/test passed through `tools/asan-run.sh` at
  `cmake-build-debugasan/asan-logs/20260928-121803-build` and
  `20260928-121808-ctest` (2/2), without a sanitizer diagnostic. Apple LSan is
  unsupported. These adapters are not yet attached to all product startup,
  stdio/SAY/TRACE, environment and ancillary-tool crossings.
- The constrained host build now passes: configure
  `/tmp/beta3-single-config.log`, `rxbvm_single`, `test_single_state` and
  `test_single_embed` build `/tmp/beta3-single-build.log`, and focused
  `single_vm_state` `/tmp/beta3-single-test.log`. This is not a CMS/TSO native
  compiler/package proof. The detailed crossing inventory is
  [text-capability-matrix.md](../../qa/beta3-core-baseline/text-capability-matrix.md).
- The complete Debug build passed on the platform candidate before the later
  narrow existence-probe and selector edits at
  `/tmp/beta3-platform-all-build2.log` (1,564 actions after the focused
  test-target include-path repair). `platform_cms_text` initially could not
  start because its `EXCLUDE_FROM_ALL` harness was not built; after building
  `rxc_cms_text_harness` and `rxas_cms_text_harness`, the unchanged test passed
  at `/tmp/beta3-cms-text-replay.log`. Nine relevant file/linker/disassembler/
  VM/platform CTests pass at `/tmp/beta3-platform-crossing-test.log`. The
  generated codec table matches its authoritative inputs with
  `python3 platform/generate_text_codecs.py --check`
  (`/tmp/beta3-codec-generation-check.log`). These focused receipts do not
  replace the one final combined normal correctness suite.
- A subsequent narrow CMS `fileexists` correction routes existence probes
  through the raw binary open, avoiding the old converted libc path. Targeted
  product/adapter build `/tmp/beta3-fileexists-build.log`, seven related
  CTests `/tmp/beta3-fileexists-test.log`, and matching Apple ASan raw tests
  `cmake-build-debugasan/asan-logs/20260928-122744-build` and
  `20260928-122754-ctest` pass. The earlier full-build receipt predates only
  this narrow edit; final combined correctness remains due on frozen inputs.
- After local platform commit `177c077aae39c610ba6d557c229f85a611e7906f`,
  the raw host fixture gained explicit `file2buf` coverage for nonseekable
  record input, scanner sentinels and injected read failure. Its two Debug
  tests pass at `/tmp/beta3-sequential-test2.log` and matching Apple ASan
  tests at `cmake-build-debugasan/asan-logs/20260928-123014-ctest`.
- The existing `-E` external-text selector now reaches linker control/maps,
  disassembler RXAS output and VM application text files as well as compiler
  and assembler text. A new CLI regression was timed in isolation (Debug
  0.13 s; Apple ASan runner 0.49 s), registered `RUN_SERIAL` with a 300 s hang
  guard, and passes in Debug `/tmp/beta3-selector-final-ctest.log` and Apple
  ASan `cmake-build-debugasan/asan-logs/20260928-123436-ctest`. It verifies
  supported UTF8/unsupported-selector behavior on desktop, not native page
  conversion or all component text crossings.
- AC-01/02/03/07 remain partial; AC-04/11, product AC-09/10 and final combined
  AC-08 remain open. STEP-02/03/05 continue; STEP-06–08 remain open. Needed
  lab input is the actual raw backend, CMS iterator payload shape/capacity and
  standard/argument/environment service behavior. No lab checkout was edited
  or guest run performed. One frozen-input combined normal correctness suite,
  independent review, normal publication and one approved hosted sanitizer
  matrix remain in sequence.

### VM environment crossing (28 September, 12:49 local)

- Local commit `2df36e28e` routes the VM's `GETENV` through the cREXX-owned
  native environment decoder only on raw CMS/TSO builds. It preserves the
  existing desktop lookup, distinguishes absent from unsupported/error, rejects
  embedded-zero native names, and signals `NOTREADY` on service failure.
  CMS/TSO host mocks exercise present, absent, embedded-zero and unavailable
  services. This does not route every tool configuration lookup or startup
  argument, and it does not qualify a lab raw backend.
- Matching Debug build `cmake --build cmake-build-debug --target rxbvm
  test_native_raw_cms test_native_raw_tso --parallel 8` passed at
  `/tmp/beta3-getenv-final-build.log`; `ctest --test-dir cmake-build-debug -R
  '^(ts_getenv_(noopt|opt)|native_raw_(cms|tso))$' --parallel 1
  --output-on-failure` passed 4/4 at `/tmp/beta3-getenv-final-test.log`.
  Apple ASan runner builds passed at
  `cmake-build-debugasan/asan-logs/20260928-124517-build` (the generated
  `ts_getenv` fixture) and `20260928-124747-build` (current VM/mock objects);
  the same four CTests passed at `20260928-124751-ctest`. An earlier attempt
  at `20260928-124328-ctest` found no generated `ts_getenv` module in that
  build tree; this was a missing fixture, with no sanitizer diagnostic, and
  the targeted fixture build/replay resolved it. Apple LSan is unsupported.
- Console/stdout integration remains an explicit open dependency. Existing
  standalone RXC/RXAS/RXVM entrypoints (and any later packaged RXLINK/RXDAS)
  use standard C streams for diagnostics/SAY/TRACE. A platform-local startup
  attachment of owned IBM1047 wrappers has been proposed to the coordinator;
  it requires the lab's safe newlib bind/restore and exit-flush ownership
  detail before modifying FILE slots. Embedding and ordinary C clients retain
  their host-owned standard streams. The lab also needs to settle the CMS
  iterator payload shape and emergency stderr capacity. AC-04/AC-11 and the
  frozen combined correctness/native gates remain open.

### Frozen combined normal QA (28 September)

At clean local product/test/build input `ce4a9273fc5752fd045170661140e322cdf192c2`,
the normal Debug `all` build and `qa-prep-comprehensive` passed. The one
combined essential/smoke/comprehensive CTest run finished **2283/2286 passed**
in 820.57 seconds, without a timeout. The three failed test entries have two
causes: `source_import_srcmap_factory` lacks the required library search path
in its isolated fixture, and both RXC/RXPP diagnostic-catalog tests find the
same two new keys absent from German and Dutch catalogues. Exact commands,
tree/cache/log hashes, excerpts, passing serialized aggregate receipts and
repair limits are in the [combined normal QA receipt](../../qa/beta3-core-baseline/combined-normal-qa-2026-09-28.md).
At this point, the coordinator was reviewing both causes. No production source,
test fixture or catalogue had yet changed; AC-07 and STEP-05 remained OPEN
pending focused repair qualification. Preserve the other 2283 passing results
unless a changed input or distinct failure justifies broader repetition.
AC-04/11 raw native and text crossings, AC-08 final hosted product sanitizer,
AC-09 product publication and AC-10 final handoff also remain OPEN.

**STEP-05 focused repair actions, completed after coordinator cause review:**

1. **STEP-05.1 — Fixture dependency.** `64ba2ef0d` passes the configured
   build `bin` library path to all three source-map RXC calls and registers
   `library` with `rxc` as the fixture's prep targets. It preserves mandatory
   compiler-exit loading, `--no-exe-import` and every srcmap/error assertion.
2. **STEP-05.2 — Catalogue completeness.** `d454750bd` adds the two missing
   import-failure messages in German and Dutch with unchanged placeholder
   names. No English message or product logic changed.
3. **STEP-05.3 — Focused qualification.** The regenerated Debug
   `qa-prep-comprehensive` passed; the exact three previously failed CTests
   passed 3/3; a focused Apple ASan build of `rxc` and `library` and the
   matching source-map CTest passed 1/1 without sanitizer diagnostics. Exact
   commands, input tree `088c61f77fda5eef407cf0fd1a54f0d13b55ef8d`,
   local paths and hashes are in the [combined normal QA receipt](../../qa/beta3-core-baseline/combined-normal-qa-2026-09-28.md).

The 2,283 unchanged passing combined results are retained and the three
failed entries are resolved by focused rerun. The coordinator accepted this
composite local correctness evidence in `30d723054`; AC-07 is satisfied for
the current local cREXX candidate. It is not a new single-run 2,286/2,286
result. STEP-05 local QA is accepted; final integrated inputs and their
applicable native/sanitizer QA are still unsettled. AC-04/11
native raw backend and text crossings, AC-08 final Linux/macOS sanitizer
matrix, AC-09 product publication and AC-10 handoff remain OPEN.

### Draft PR hosted fixture follow-up

Draft PR #709 published `30d723054`; `develop` did not move. Its ordinary
Build run `36436367998` found two independent fixture causes. MSVC/MinGW
could not check out the Windows-reserved `second/aux.crexx` basename. Local
commit `369cbeb7b` renames only that fixture to `second_extra.crexx`; its
first-root/multiple-provider regression passes focused Debug and Apple ASan.
Linux x64, macOS ARM64 and macOS Intel each completed 175/176 smoke tests;
their only failure was parser-only `source_semantics` loading its required
`library` from the wrong working directory. The original local Debug/ASan
combined builds had `ENABLE_PARSER_MODE=OFF`, so their accepted AC-07 proof
did not cover this test. A fresh parser-enabled Debug build reproduced the
failure, then passed the original test after setting its CTest working
directory to build `bin` and declaring `test_source_semantics`, `library` and
`compiler_exit_bin` prep targets in `1366df3c1`. The matching focused Apple
ASan parser-mode test passes 1/1. Exact commands, local and hosted log hashes,
the read-only DSL-Syntax-Highlighter input identity and the open hosted retry
are in the [fixture repair receipt](../../qa/beta3-core-baseline/hosted-fixture-repair-2026-09-28.md).
An adjacent parser-mode sandbox audit found the same missing-module setup in
the comprehensive `highlight_cache` test; commit `00d42cb3a` makes its first
sandbox stage the required library and compiler exits, with explicit fixture
prep. Its original
cache assertions pass focused Debug and Apple ASan 1/1 each. The other direct
parser-mode editor test passed unchanged, and no further isolated parser
sandbox was found. The coordinator accepted these test-only inputs at
`e98c4542d` for the next PR publication; no
production behavior was changed.
The Linux hosted `gcc -m32` RXBIN cross-width check and optimizer parity job
passed on published head; retain those valid results. No broad QA rerun was
made for these test-only/fixture-only changes.

The separate [console flush probe](../../qa/beta3-core-baseline/console-flush-probe-2026-09-28.md)
demonstrates that `fflush` can report success while a partial prompt remains
invisible and a deferred raw error reaches only close. This is an additional
AC-04/11 native standard-stream contract requirement, not a sanitizer finding.
Startup binding, CMS iterator shape, emergency stderr capacity and the actual
raw backend/package proof remain open. The coordinator reviewed and published
the compatible local fixture/docs commits in PR #709 at `a4a39dc3b`; the
fresh automatic Build and CodeQL checks remain in progress. The
[native flush alternatives](native-raw-services.md#standard-stream-flush-reconciliation-proposal-only)
compare a cREXX-owned `platform_fflush` route with a generic per-stream
explicit-flush delivery mechanism. The ordinary C `fflush(FILE *)` gap is
still open; the comparison selects no new runtime ABI or generic-newlib fork.

### Shared-core acceptance and exact-revision handoff (28 September)

Published PR #709 head `a4a39dc3b9c63fff9c726842f1c8ea43ded9326e`
contains the last production implementation at `2df36e28e` and the later
fixture-only repairs. The local follow-up after that head changes documentation
only. Coordinator-reviewed composite local correctness at `30d723054`
retains 2,283 passing combined Debug tests plus three focused repairs; the
parser-mode hosted fixtures then passed focused Debug/Apple ASan at the exact
inputs in the [fixture receipt](../../qa/beta3-core-baseline/hosted-fixture-repair-2026-09-28.md).
No broad local QA was repeated for fixture or documentation changes.

The patch register above now disposes every supplied CMS release-PoC topic:
0001–0010 were already in the base, 0011's converted-text compatibility is
superseded for the new raw cREXX route, 0012's filename/namespace shortcut is rejected
in favor of real enumeration, 0013/0015/0016/0017 are integrated in shared
core, and 0014 plus older TSO native mechanics are assigned to the raw
platform/runtime boundary. Older portable-sentinel, allocation and bounded
diagnostic causes are integrated in their owning shared layers. No historical
patch stack is being replayed. The present codec, source/import and RXBIN
behavior is reviewable without an implemented lab raw backend.

For coordinator acceptance, **AC-02 and AC-03 are ready at the shared-core
boundary**. AC-02 is supported by the first-root/same-root-provider,
private-dependency, directory-error, malformed-import and real mandatory/
optional compiler-exit regressions in Debug, with matching focused Apple ASan
for the relevant ownership/error paths, plus the
combined normal proof. AC-03 is supported by the deterministic compressor
byte/digest and fault cases, canonical/legacy imported sentinel and real
overflow controls, compiler/VM allocation and bounded OOM checks, LP64 local
proof, and an actual Linux `gcc -m32` reader/writer pass. AC-01's patch
disposition is ready for coordinator acceptance; actual native interface/
package behavior remains explicitly assigned to AC-04/11. AC-05 is satisfied
by the approved #602 feasibility disposition, AC-06 is not applicable, and
AC-07's local composite correctness proof has already been accepted. AC-10
has current guides, matrix and a consumable candidate identity, but remains
OPEN until the final reviewed baseline/native dependency handoff.

The repaired-head ordinary Build run `36440639967` is on exact
`a4a39dc3b`. At the 15:22 UTC read-only status snapshot, Linux core job
`108990065887` passed **176/176** smoke tests (including parser-mode
`source_semantics`) and `gcc -m32` RXBIN, Linux optimizer parity
`108990065642` passed, MSVC `108990065685` passed **162/162** (parser test
absent in that configuration), and MinGW `108990129985`, macOS ARM64
`108990065730` and macOS Intel `108990065704` passed. The Build run itself
remained in progress on four plugin jobs; CodeQL run `36440638876`, C++ job
`108989891074`, remained in progress on the same head. Retained Linux log
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-linux-retry.n2Iuq6YKeU`
has SHA-256 `7b4bc33100119f61279f2e7adee99e7365bf8ff41b0104f869b05eb79960c908`;
MSVC log
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-msvc-retry.C6pziKqSlE`
has SHA-256 `0a6c750017da215e1901b4692844d5015b22601829019eb4e529028b71aa1ae5`.

The reviewed shared-core repairs and feature-gated native adapters are ready
for **ordinary develop publication once this PR's automatic Build and CodeQL
checks finish successfully**. This is phase readiness, not native acceptance
or beta release qualification. `develop` remains `143921e11`. AC-04/11 remain
OPEN for the lab's raw backend, actual CMS iterator shape, console bind/flush,
emergency stderr capacity, native argument/configuration text crossings,
applicable RXPP/ancillary availability and CMS31/TSO31/TSO64 package evidence.
The cREXX side can reconcile `platform_fopen` details and independently route
supported callers once the matching raw service semantics are concrete; it
must not invent a runtime flush or stdio ABI. AC-08 remains OPEN for the
final combined Linux ASan/LSan plus macOS ASan matrix without open first-party
SAN findings; AC-09 remains OPEN until develop integration and its automatic
publication checks; AC-10 and STEP-08 remain OPEN for the final baseline and
lab handoff. No native guest, broad sanitizer or new product QA is inferred
from the ordinary PR results.

### Coordinator shared-core acceptance (28 September, 15:29 UTC)

The coordinator reviewed documentation-only follow-up `35495c941` against
published product/test head `a4a39dc3b`, checked the upstream CMS foundation
identities and the rejected filename/namespace shortcut, and accepted the
complete patch disposition under AC-01. Native mechanisms are assigned explicit
runtime dependencies; this disposition does not qualify those mechanisms.
Previously reviewed compiler/error and compressor/sentinel/allocation changes,
their retained normal/focused sanitizer regressions, and the independently
inspected actual Linux ILP32 job support acceptance of AC-02 and AC-03.
All five ordinary platform core jobs and optimizer parity have passed on
`a4a39dc3b`. Plugin jobs and current CodeQL remain running at this snapshot.
The local follow-up contains only documentation and passes `git diff --check`;
no code or test/build input invalidates the retained results.

Publish this final review/handoff documentation with the compatible reviewed
phase and make PR #709 ready for review. Ordinary automatic checks on the
published revision remain separate publication evidence. AC-04/11 native text
and backend completion, AC-08 final sanitizer assurance, AC-09 develop
integration/checks and AC-10 final baseline handoff remain OPEN. Neither native
flush proposal selects a newlib fork or adds an approved runtime ABI. This is
ordinary development integration readiness, not overall baseline acceptance.

### Coordinator workflow publication receipt

Master workflow-only commit `2d24ae989fdb530942c73d81301d6affd243a671` has
completed automatic Build `36407284141` SUCCESS and CodeQL `36407284095` SUCCESS,
as independently reported by the coordinator. This verifies that narrow
publication only. AC-08 still requires the final combined product sanitizer
matrix; AC-09 product integration remains OPEN.
