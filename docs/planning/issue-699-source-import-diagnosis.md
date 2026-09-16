# Issue #699: source-import compiler diagnosis

## Vision and intended outcomes

Assess and diagnose [#699](https://github.com/adesutherland/CREXX/issues/699)
on the persistent `hotfix` checkout. Establish why adding `closefile` to the
downstream operator-output cleanup was associated with a symbol convergence
failure in `ragcommand` and a provider type mismatch in `ragprocess`. Separate
reproduced causes from hypotheses and preserve an actionable repair proposal.
The initial assessment authorized investigation and scratch reproducers. On
16 September Adrian authorized repair through hotfix and publication to develop
when green. Preserve working source/binary imports, inline metadata and legitimate
namespace extensions while fixing the proved trigger; no new language/API or
architecture is authorized. GitHub comments remain unauthorized. Do not modify the
main CREXX/llama checkout or the downstream RAG checkout; no model workloads.

## Acceptance criteria

- **AC-01 — baseline:** record current issue, compiler revision, source/history
  relationship and provenance of recovered downstream inputs.
- **AC-02 — reproduction:** independently check both reported diagnostics with
  source imports; compare the `lineout` cleanup and built-interface controls.
  Label reconstructed inputs and any unverified original reproduction plainly.
- **AC-03 — diagnosis:** reduce reproduced failures where practical and identify
  the responsible compiler stage/mechanism with retained evidence. Distinguish
  the two diagnostics unless evidence establishes a shared cause.
- **AC-04 — recommendation:** record the narrow repair/regression scope, remaining
  uncertainties, commands/logs/results, and assessment status. Avoid unnecessary
  broad testing; elapsed delay must buy specific diagnostic evidence.
- **AC-05 — repair:** both reduced failures and reconstructed original modules
  compile successfully in opt/noopt. Ordinary source and binary import behavior,
  actual extension dependencies and invalid-interface diagnostics remain correct.
- **AC-06 — regression/qualification:** permanent tests fail on the defective
  baseline and pass with the repair. Measure new aggregate tests in normal Debug
  and maintained ASan; run the full normal correctness suite (`qa-comprehensive`).
  Adrian explicitly requested full regression testing during implementation.
  No overnight/deep gate unless an actual finding requires it.
- **AC-07 — delivery:** commit/push the qualified correction on hotfix, promote
  to develop, retain exact revisions/logs and check normal automatic publication
  workflows to honest terminal outcomes. No tag/release or local installation.

## Steps and status

- **STEP-01 (AC-01):** read instructions, issue and retained downstream evidence;
  inspect current hotfix/history. Complete: clean hotfix and both fetched origin
  branches at `17e844441ed87e1f6e0d5f1f0d3bb4bee8db6187`. Compiler implementation
  and rxfnsb source unchanged since reported `037e7939bc29eb91b29ed41e9b1b8debdef6353d`;
  compiler test registration changed. Installed clean Release matches hotfix.
- **STEP-02 (AC-02):** reconstruct failing cleanup in scratch inputs, run bounded
  direct compiler checks and controls. Complete: both original diagnostics
  reproduced on current hotfix in opt/noopt; lineout, extension exclusion and
  retained binary-interface controls pass.
- **STEP-03 (AC-03):** reduce and trace the failure, inspect stage ownership.
  Complete: independent three-file convergence and five-file provider mismatch
  reproducers; both fail on the reported and current installed compilers.
  Reported flag `0x0002` is `FLAG_VAL_SYM`, not type validation. Recursive source
  loading exposes unfinished contracts; the array shape oscillates and a class
  is exported with its unresolved interface relationship missing.
- **STEP-04 (AC-04):** retain diagnosis/evidence and propose next repair scope.
  Complete: [assessment, commands, limits and repair recommendation](../qa/issue-699-20260916/README.md).
  No broad build/test or hosted qualification was needed for assessment.

**Assessment status:** AC-01 through AC-04 complete. Repair/delivery now authorized;
AC-05 through AC-07 complete. The historical assessment evidence remains valid.

- **STEP-05 (AC-05):** inspect exact inline-dependency and namespace visibility
  handling; implement the smallest correction consistent with existing contracts.
  Complete: distinguish actual program imports from materialized dependency
  scopes for source discovery. No new import architecture. Both reduced cases
  and both reconstructed original modules compile in opt/noopt; positive and
  negative extension/interface controls pass.
- **STEP-06 (AC-05, AC-06):** register both reduced regressions with opt/noopt and
  extension/invalid-contract controls; run focused Debug and ASan, then full normal
  correctness qualification and recompile both reconstructed original modules.
  Complete. Full sanitizer/deep matrices remain overnight assurance unless
  an actual first-party finding makes them necessary.
  Focused maintained Apple ASan is green (7/7). Full normal correctness is green
  (2,294/2,294, 792.65 s). Frozen implementation/test inputs still match.
- **STEP-07 (AC-07):** commit/push qualified hotfix, promote to develop, then check normal
  automatic publication gates and reconcile post-promotion outcomes. Complete.
  Repair `f786b15d86a66b52c8ab1ce631054afc7a5937e8` was pushed to hotfix and
  fast-forwarded to develop. Automatic [Build CREXX 35120521324](https://github.com/adesutherland/CREXX/actions/runs/35120521324)
  and [CodeQL 35120521311](https://github.com/adesutherland/CREXX/actions/runs/35120521311)
  both completed successfully for that exact revision. Release product/smoke/package
  jobs passed on Linux x64, Windows MinGW x64 and both macOS architectures;
  Linux optimizer parity passed 742/742. Terminal JSON and job logs are retained
  in the [repair report](../qa/issue-699-20260916/repair.md). No extra hosted matrix
  was dispatched. The evidence-only closeout retains identical qualified inputs.

## Initial evidence

The issue's original triggering working tree was uncommitted; its reported base
`dd96144bb3689ad4da4a9a43c8df914555402877` alone is not the reproducer. The later
downstream commit `ab7943d42d459c7d8fc6e146f7709dfac7a2d827` retains the `lineout`
workaround and the reported `ragprocess` failing call at line 588. Recovering that
tree and replacing only the cleanup call is a reconstruction, not a claim of
byte-identical recovery. The original failed retry is retained at
`/Users/adrian/CLionProjects/crexx-rag/docs/qa/t7-10-controller-diagnosis-20260915/compiler-closefile-retry.log`.

`closefile` is a small wrapper around `_close`; `lineout` without a line also
calls `_close` and clears line-input pending state. The issue already records
successful isolated runtime close tests. This investigation concerns compiler
import/validation behavior, not evidence of a runtime close defect.
