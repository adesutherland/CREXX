# Structured exit condition cleanup

## Vision and intended outcome

Repair the live IF-condition link left behind when ITERATE bypasses its normal
cleanup. Register reuse must not mutate the previous condition's object field.
Preserve scalar-accessor inlining and the supported optimized code shape. Retain
a permanent regression and publish the qualified hotfix to develop so native
Windows UI qualification can resume. Windows qualification remains with Adrian
and the Windows agent; this change is compiler source and macOS correctness proof.

The baseline is `6ab5a7815df751e806696c8d196c6ddf7affcb69`, synchronized in the
existing `CREXX-hotfix` checkout. The independent main `hotfix2` checkout contains
unrelated work and is excluded from this repair.

## Acceptance criteria

- **AC-01 — PASS:** Crossed structured exits copy already-live IF-condition
  cleanup, retain normal fall-through cleanup, exclude conditions still being
  evaluated, and preserve the target loop. Verify focused AST emission coverage
  and related structured-exit runtime regressions.
- **AC-02 — PASS:** A permanent reduced modal-loop regression fails on the
  baseline and passes all four compiler/assembler opt/noopt combinations after
  repair, using the linker and VM. Optimized RXAS retains inlined counter
  accessors; noopt retains ordinary accessor calls.
  Preserve the result of imported inlined IF-guard returns when crossed cleanup
  releases condition links; verify `inline_receiver_detached_guard_opt`.
- **AC-03 — PASS:** The original engineering Lotto Save/confirmation/write chain
  passes with all modules optimized, alongside its control and scoped-write
  tests. Use the unchanged diagnostic snapshot and matching product libraries.
- **AC-04 — PASS:** The core product builds successfully and the relevant normal
  compiler correctness suite passes. Retain exact input identities, commands,
  and results in a qualification receipt. No known sanitizer finding is being
  waived; ordinary automatic publication/overnight assurance follows AGENTS.md.
- **AC-05 — READY FOR PUBLICATION:** Commit code, regression, documentation and qualification
  receipt together on hotfix, promote the same qualified product inputs to
  develop without disturbing unrelated work, and verify remote branch tips.
  Normal automatic publication checks are expected to pass; their observed
  status can be reported without a status-only commit.

## Implementation steps

1. **STEP-01 — DONE (AC-01/02):** Synchronize the requested hotfix checkout and
   inspect cleanup ownership, emission ordering and existing tests. Reuse the
   independently reproduced modal-loop failure as the minimal reproducer.
2. **STEP-02 — DONE (AC-01/02):** Track the crossed child while walking ancestors;
   copy IF-condition cleanup only when exiting an already-entered branch.
   Extend the existing early block-result reservation to crossed IF branches,
   preserving final scalar-load donation. Add focused AST ownership/path checks
   and a permanent runtime matrix. Refresh the seven reviewed assembly goldens
   whose dedicated result-register allocation changes; replay those expectations
   in isolation, retaining the rest of the completed normal sweep.
3. **STEP-03 — DONE (AC-02/03):** Build the repaired tools, run the focused matrix
   and inlining assertions, then rerun the unchanged engineering UI snapshot.
4. **STEP-04 — DONE (AC-04):** Complete the core build and normal compiler
   correctness checks; retain the smallest decisive evidence and update docs.
5. **STEP-05 — READY (AC-05):** Record final receipts and expected publication
   status before committing; publish hotfix and fast-forward develop where
   possible, reconciling any concurrent remote changes before promotion.

## Diagnostic evidence

`/Users/adrian/Documents/Franz-UI-Mac-Investigation-20261012/DIAGNOSIS.txt`
records the baseline failure and private emitted-RXAS unlink proof. The standalone
`repro/modal_loop_counter.crexx` fails with both compiler modes; adding the missing
unlink before the modal ITERATE makes each pass. The engineering snapshot archive
SHA-256 is `461503d840c7f7849b6d1c2646f5e6b1ea3de3d4159ff7aa9250fe6fa41df48f`.

The first normal compiler sweep exposed a related result-register alias in
`inline_receiver_detached_guard_opt`: an imported inline return stored its value
in the IF array-condition helper register, which the new exit cleanup then
unlinked. The repair therefore includes the existing block-result ownership
reservation rule, rather than disabling inlining or omitting required cleanup.

Final focused evidence: `docs/qa/iterate-condition-cleanup-2026-10-12/` retains
15/15 normal Debug focused checks (2.46 s), the four failing baseline controls,
qualified input/tool hashes, and all three passing optimized engineering UI
tests. The snapshot's 32 input checksums remained unchanged. The final core
product/QA-prep build passed without compiler warnings or errors. The normal
1,837-case compiler sweep passed 1,830 and found only seven reviewed stale
assembly goldens; their isolated replay and final smoke/parity registrations
passed 11/11. All paired runtime tests passed. Final registered test commands
remain unchanged; the four regressions now run in smoke and optimizer-parity CI.

The [qualification receipt](../../docs/qa/iterate-condition-cleanup-2026-10-12/README.md)
and exact product/test input hashes accompany the first substantive commit/push.
Expected final publication state: the qualified hotfix commit is fast-forwarded
to remote hotfix and develop, with local develop synchronized and the unrelated
hotfix2 checkout preserved. Normal automatic publication checks are expected to
pass. Push/ref verification is reported live in the conversation; it does not
require a CI-status-only follow-up commit. AC-05 is verified by those final refs.
