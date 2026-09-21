# CMS portability integration review and host QA

## Vision and intended outcomes

Produce a reviewable, host-QA-qualified integration candidate for the existing
eleven CMS portability topics, preserving desktop behavior, UTF-8 semantics,
64-bit Rexx integers, RXBIN 007 and compiler exits. Constrained capabilities
remain opt-in and use existing explicit error/signal contracts. The primary
target route is maintained GCC 16.2, cross-built and run on macOS. Legacy
GCCCMS/GCCMVS compatibility must not impose new cREXX restrictions.

The 21 September clarification makes clean upstream cREXX the governing
mandate: options for a minimal C99-based VM/CE build using GCC 16.2 are welcome;
legacy compiler support is acceptable only where it does not compromise cREXX.
This reinforces the existing design/approval boundary rather than adding a
legacy portability acceptance gate.

The subsequent clarification is nuanced: legacy compatibility is welcome in
cREXX when clean and justified; it is not automatically excluded or required
to live separately. Uncertain changes or trade-offs require Adrian's approval.
The explicit approval requirement for additional email-derived changes still
applies. Keep their provenance and authorization distinct from the agreed QA
repairs, without treating that distinction as a blanket architecture rule.

This plan owns upstream integration review and host QA. The earlier
`cms-rxas-platform.md` records the initial platform increment; Mainframe Lab's
CPR/TP plans continue to own guest application, memory and package acceptance.
Historical RXC `-X`/RXAS/RXVM results and small 31-bit runtime/text fixtures are
distinct retained evidence. Actual 31-bit applications, compiler-exit memory
fit and release packaging remain open unless specifically tested.

## Source and authority

- Fetched cREXX `origin/develop` and original base:
  `5de72ae57df4e9fa4939d5a3f9412aa3b0569108`.
- Original candidate: `e68681cd1dd0203088f8a4ad73d9d9a80700084d`, preserved
  on `temp/cms-release-poc`; its untracked builds/evidence are untouched.
- QA branch: `temp/cms-portability-qa-20260921`, sibling worktree
  `CREXX-cms-portability-qa`, starts at the candidate. Fetch found no newer
  develop commits; `git cherry` identifies all eleven topics as unapplied.
  Therefore preserve their exact history without a redundant replay.
- Mainframe Lab starting HEAD/main:
  `6b1795d71884e0fa22544fdd66381928f180294c`.
  The later fetch found HEAD/main at
  `7d737174eb8300085273df58c0ce17a057485c32`, integrating VM/CE uplink and
  production glue. Reviewed the intervening log, instructions/profile changes
  and combined-integration report. Maintained GCC/checker sources, all six
  requested CMS documents and the eleven recovery patches are unchanged.
  All eleven recovery patch SHA-256 checks pass; no cREXX base change follows.
- Main cREXX checkout has unrelated edits and is not used for implementation.
- Authorized: necessary fixes and regression coverage within the existing
  candidate design, local reviewable commits and documentation. No release,
  push, correspondence, new language/architecture decision or email-derived
  implementation is authorized by this task. Correspondence review and
  proposals are retained privately outside public commits.
- Intended C baseline is C99. Retained target recipes use GNU99; host checks
  do not establish complete strict-C99, C90, CMS or release qualification.

## Acceptance criteria

- [x] **AC-01 — source reconciliation:** fetch remotes, identify exact bases,
  inspect live dirty state, verify eleven-topic history and recovery hashes,
  and isolate QA without rewriting preserved candidates or unrelated edits.
- [x] **AC-02 — code/interface review:** review all eleven topics for option
  ownership, default behavior, error paths, lifecycle and documentation;
  record topic dispositions and repair only in-scope defects.
- [x] **AC-03 — desktop correctness:** ordinary core product build and the
  appropriate normal Debug correctness suite pass, including affected compiler,
  assembler, linker, UTF-8, import and environment regressions. Retain commands,
  source/build identities and failures as well as successful results.
- [x] **AC-04 — constrained VM:** `single_vm_state`, `single_vm_embedding` and
  `single_vm_parity` pass; verify unavailable services, repeated/nested lifecycle,
  allocator defaults/alternative/invalid configurations and unchanged integer/
  RXBIN behavior. NTHREADED alone must not mean no workers.
- [x] **AC-05 — constrained compiler:** `check_single_rxc` passes with exits
  enabled in MinSizeRel, Debug and targeted sanitizer builds, comparing
  optimized/unoptimized assembly, bytecode, execution and invalid-input errors.
  Use generated/native inputs from the same qualified source.
- [x] **AC-06 — platform and sequential input:** verify historical/CMS20/host
  selection, optional directory interface, seekable/sequential/empty/error
  source input and scanner sentinels with enduring automated coverage.
- [x] **AC-07 — text boundary:** test selection before lexical analysis,
  source imports, RXC output and RXAS input through the optional hook; binary
  files remain unchanged; conversion, write and close failures are reported.
  Preserve UTF-8 defaults and explicit unsupported-encoding errors.
- [x] **AC-08 — targeted sanitizer evidence:** run relevant focused platform,
  source/text and constrained lifecycle/parity checks using `tools/asan-run.sh`;
  record instrumentation and Apple LSan limits. Register actual first-party
  findings immediately in SANITIZER-WORKLIST and follow its closure rules.
- [x] **AC-09 — separate correspondence review:** inspect E03/E04 and live
  relevant threads/newer messages where accessible; record dates, limitations,
  exact toolchain attribution, topic overlap, GCC16 relevance, proposed changes,
  risks and smallest validation privately. Additional implementation awaits
  explicit authorization. Do not treat drafts as received correspondence.
- [x] **AC-10 — handoff:** focused reviewable commits, enduring cREXX guides,
  source/evidence report, remaining gaps, and recovery/follow-up instructions;
  no generated artifacts/build directories or private correspondence committed.

## Implementation steps

1. **STEP-01 / AC-01:** read instructions, plans and architectural/QA references;
   inspect/fetch sources and preserve recovery identities. Source reconciliation
   is complete; establish isolated QA and retained local evidence directory.
2. **STEP-02 / AC-02,06,07:** inspect all changed interfaces and consumers;
   reproduce concrete defects, add decisive regression coverage and repair
   within the agreed design. New design decisions require approval.
3. **STEP-03 / AC-09:** review private correspondence independently. Preserve
   concrete proposals privately; no dependent changes without authorization.
4. **STEP-04 / AC-03,04,05,06,07:** build normal product and constrained variants,
   exercise focused tests then normal correctness suite. Reuse unchanged valid
   evidence; do not dispatch overnight or release matrices automatically.
5. **STEP-05 / AC-08:** run focused sanitizer commands on the same test shapes.
   Findings trigger repository SAN disposition, not suppression or silent deferral.
6. **STEP-06 / AC-10:** freeze code/test/build inputs, document exact results
   and limitations, commit reviewable changes, refresh recovery material or give
   exact regeneration instructions, and recheck remote drift before handoff.

## Evidence and progress

Local logs/builds are retained outside the tracked source under
`/Users/adrian/CLionProjects/crexx-cms-qa-evidence-20260921/`.
The final report will identify exact inputs and accepted logs. No broad rerun
is required after documentation-only commits when qualified inputs are unchanged.

Review and focused QA completed on 21 September:

- STEP-02 complete: all eleven topics reviewed; fixed unchecked input closes
  in main/header/import/RXAS reads and premature debug-success output. Added
  independent CMS test working directories, QA preparation/tier membership,
  optional directory and injected text-boundary regression coverage.
- STEP-03 complete: source snapshots and live relevant threads reviewed;
  dates, limitations and approval proposals remain in the private local record.
  No correspondence content is part of this repository, and no additional
  email-derived change was implemented.
- STEP-04 complete: ordinary Debug and Release core builds pass; the normal
  correctness selection passes 2,350/2,350 with 0 failures in 993.40 seconds.
  Constrained Debug and MinSizeRel state/embedding/parity pass 3/3 each;
  compiler parity passes with exits enabled in both. MinSizeRel uses alternate
  8 KiB/4 KiB pools; four valid and six invalid allocator compile controls pass.
- STEP-05 complete: maintained macOS ASan passes all four CMS CTests, including
  69 text controls. Constrained ASan/UBSan passes 3/3 plus compiler parity.
  No first-party sanitizer finding occurred. Apple LeakSanitizer is unsupported;
  no leak or cross-platform sanitizer qualification is claimed.
- STEP-06 complete: code/test repair commit `edc4f3378e3bb4c867f7943d2c3ca6d52d855b37`,
  documentation/report closeout and generated recovery series. The external
  `qualified-code-inputs.json` matches the committed inputs; documentation-only
  changes retain that proof. Recovery tree and final candidate identities are
  recorded in the external `recovery/` verification and handoff manifest.

All acceptance criteria for this host-QA integration scope are satisfied; the
[qualification report](../qa/cms-portability-2026-09-21.md) records topic review,
commands, results and the remaining Mainframe Lab/application/release gates.
Private approval proposals are outside this repository. No new design or
email-derived implementation was included. No push, release or message was sent.
