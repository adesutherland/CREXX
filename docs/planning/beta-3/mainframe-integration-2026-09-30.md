# Mainframe repair integration and beta 3 preparation

30 September 2026. Authorised by Adrian's request to review the Lab I/O repair,
integrate it into develop, QA, commit and push, then prepare formal beta 3.

## Vision and intended outcomes

Bring the supplied mainframe changes into the maintained cREXX source, preserving
the previously approved core/platform/runtime boundary, UTF-8 text, exact binary
bytes, desktop behaviour and accurate failure diagnostics. Publish one reviewed
development revision with build/regression evidence. Prepare that version for
the formal beta 3 release with accurate release notes, candidate identity and
explicit remaining platform/package gates. Preparation does not claim that a
tag or formal release exists. Unrelated dirty work in the primary checkout and
the supplied Lab tree must remain intact.

The repair's existing plan is `docs/planning/mainframe-io-repair-20260930.md`;
the approved architecture is `core-baseline-2026-09-28.md` and
`text-boundary-2026-09-28.md`. This record owns this integration/delivery request
and does not reopen cancelled scope or replace their historical criteria.
The Release 1 plan owns release cadence and gates. Lab qualification remains
separate evidence; SDK WAIT, heap and stack changes are Lab runtime inputs.

## Numbered acceptance criteria

1. **INT-AC-01 — PASS:** Every supplied delta has a reviewed disposition against
   current origin/develop. Inspect complete committed and working-tree diffs,
   permanent regression quality and architecture; report defects/uncertainty.
2. **INT-AC-02 — PASS:** The integrated core builds and focused I/O, allocation,
   native-platform and normal correctness checks pass. Retain exact source/test
   identity, commands and logs; measure changed nested CMS aggregate in Debug
   and maintained sanitizer builds before accepting its scheduling properties.
   Use focused sanitizer checks for changed allocation/error paths. Reuse valid
   unchanged evidence; broad overnight assurance is not an ordinary push gate.
3. **INT-AC-03 — PASS:** Reviewed commits reach origin/develop and applicable
   automatic Build/optimizer-parity/CodeQL results are terminal and inspected.
   Preserve the primary checkout's unrelated edits and identify publication SHA.
4. **INT-AC-04 — PASS (PREPARATION):** Beta 3 candidate preparation records the exact revision,
   updated notes, actual build/package evidence and remaining formal gates.
   Verify tags, README, VERSION and notes together. No wider guest, sanitizer,
   signing or install claim beyond retained evidence; no tag/release publication
   is inferred from the request to prepare the version.

5. **INT-AC-05 — PASS:** Default SAY/SAYX and UTF console FWRITE/FWRITECDPT
   raise existing UNICODE_ERROR for conversion failure and NOTREADY for native
   output/flush failure. Valid accents remain native bytes, custom SAY exits
   retain their void callback ABI, BYTE/binary operations remain raw. Verify
   the real interpreter's terminal and catchable signal behavior with a native
   console host fixture, plus Debug and maintained ASan regressions.

6. **INT-AC-06 — PASS:** Current public documentation is the final beta 3
   publication copy: README, release notes, installation/security guidance,
   documentation entry points and packaging examples agree with `VERSION`
   `1.0.0-beta.3`. No second post-tag wording edit is required. Verify current
   version consumers, local links, release filename conventions and the diff;
   preserve historical release/evidence records and unchanged qualified inputs.

## Numbered implementation steps

1. **INT-STEP-01 — DONE (AC-01):** Inspect repository instructions,
   authoritative plans, supplied source and retained Lab receipts. Use a clean
   worktree from origin/develop; preserve snapshots of supplied/primary status.
2. **INT-STEP-02 — DONE (AC-01/02):** Integrate reviewed causal changes and
   regressions, updating docs. Pause for any new language or architecture decision;
   routine repairs inside the approved design are already authorised.
3. **INT-STEP-02.1 — DONE (AC-05/02):** Use the existing execution-local
   raise_signal path in the default SAY callback and SET_SIGNAL_MSG in the
   console write handlers. Add positive/negative real-VM coverage and update
   docs; no new signal, callback ABI or architecture. Rebuild affected targets,
   repeat affected focused checks, then qualify the final combined product.

4. **INT-STEP-03 — DONE (AC-02):** Freeze product/test inputs, build core, run
   focused normal and sanitizer proof and the normal correctness suite once.
   Register actual first-party sanitizer findings in the canonical SAN worklist.
5. **INT-STEP-04 — DONE (AC-03):** Review final diff and commit separate causes,
   promote to develop without rewriting unrelated history and push. Inspect
   automatic workflows on the exact published revision.
6. **INT-STEP-05 — DONE (PREPARATION) (AC-04):** Prepare curated beta 3 notes and candidate
   handoff, retaining open formal release gates and profile limits.

7. **INT-STEP-06 — DONE (AC-06):** Review current version labels and
   public documentation, replace staging/WIP prose with final beta 3 copy,
   validate consistency without repeating unchanged product QA, then commit
   and push the documentation follow-up to develop.

## Publication documentation follow-up

Adrian requested the README/version review, then clarified: "This is the
version that will be published ... we won't update them again." The intended
outcome is final beta 3 documentation in the source used for publication,
including versioned download links and installation examples. This supersedes
the earlier WIP wording requirement for current public docs. It does not claim
that the formal tag already exists or waive any remaining release/platform
gate in the [formal candidate handoff](formal-candidate-2026-09-30.md).

## Initial inputs

- origin/develop: `e7ac9edacd1ec053c4df2b4c082961bae084ff69`.
- Supplied source: `/Users/adrian/CLionProjects/mainframe-lab/crexx-release/work/io-repair/source`,
  HEAD `db87e9ac3b329dbc5d6a20d93da442bdcca9601a` plus uncommitted repair.
- Clean execution checkout: `/Users/adrian/.codex/worktrees/mainframe-beta3-review/CREXX`.
- Primary develop checkout: `47168a1f1`, with unrelated source/docs edits.
- Latest observed formal beta tag: `v1.0.0-beta.2`; beta 3 is WIP.
- Lab CMS31 and TSO31 reports are bounded guest proof. TSO31 binaries predate
  the two final allocation guards; CMS31 includes their normal and OOM checks.

## Review and execution evidence

Adrian approved "Raise a signal" after the actual FWRITE euro-sign reproducer.
This adds INT-AC-05 and INT-STEP-02.1 before the final frozen-input QA.

The supplied-delta Debug build and focused 24/24 and Apple ASan 7/7 pass.
Normal correctness was already running when the behavior approval arrived;
retain its result as pre-signal-repair evidence, not final-input qualification.

## Final local qualification and pre-publication handoff

Reviewed source changes are in three causal commits:

- `7768d0b78`: supplied native text/diagnostic I/O and codec tests.
- `9a0b09f08`: supplied record-table and AST allocation failure guards.
- `6a09f7786b79c025981bda5d7a891982f8304dc0`: approved console signal propagation and real-VM tests.

INT-AC-01/02/05 and INT-STEP-01/02/02.1/03 pass. Final normal correctness is
2,293/2,293 (766.24 s); focused maintained Apple ASan is 8/8 (23.26 s).
No sanitizer diagnostic appeared. Apple LSan is unavailable; this is focused
host proof, not broad/native release qualification. Product inputs and permanent
logs are retained in [the review](../../qa/beta3-mainframe-2026-09-30/README.md).

INT-AC-03 and STEP-04 await the fast-forward push to origin/develop and terminal
Build CREXX/optimizer-parity/CodeQL inspection on the pushed revision. The final
preparation commit is documentation/evidence only and preserves the qualified
product input hashes. Do not repeat broad local testing for that metadata.

INT-AC-04 and STEP-05 have prepared [beta 3 notes](../../releases/v1.0.0-beta.3.md)
and [the formal candidate handoff](formal-candidate-2026-09-30.md). Attach the
automatic Release/package receipts when available. Remaining exact-tag platform,
sanitizer, signing/assets and manual native gates remain visibly open there.
Do not cut a tag or call the formal release complete from this preparation.

## Terminal closeout

INT-AC-03/04 and INT-STEP-04/05 now pass for the requested publication and
preparation. The reviewed delivery `e15392da705c5e9e0e1e57b70586b53fc8cb64ed`
is on origin/develop. Build CREXX 36749864309, optimizer parity and CodeQL
36749863689 pass on that exact SHA; CodeQL has no new alert (nine existing
alerts retained). The dev snapshot publishes 30 assets and points at it.
[Publication/package receipts](../../qa/beta3-mainframe-2026-09-30/publication.json)
include real core-package smoke and archive identities for all four platforms.

During final CI the Lab completed its independent PDOS kernel repair and
recorded frozen HIGH guest closure, without changing any of the 44 imported
cREXX source files. The notes/handoff now reflect that result while retaining
exact upstream-package, manual/native and modern z/OS HIGH limits.

This closeout commit changes documentation/evidence only. All qualified
product/test/build hashes are unchanged, so valid local/hosted proof is reused
and a duplicate development CI matrix is skipped. Formal tag workflows and
release/platform requirements remain as configured. The integration/preparation
request is complete; formal beta 3 release completion remains open.

Final documentation review: VERSION and all three CMake version channels agree
with `1.0.0-beta.3`; release display is `crexx-1.0.0-beta.3`. Retained Debug
`rxc -v`, `rxas -v`, `rxvm -v` and `crexx --version` report the same base
version. All newly added local links resolve; `git diff --check` passes. All
42 frozen product/test/build inputs are unchanged, so the existing qualified
product evidence is reused. Only Markdown files change. The versioned download
links intentionally target the formal beta 3 tag for publication; that tag and
its remaining qualification gates are still tracked in the candidate handoff.
