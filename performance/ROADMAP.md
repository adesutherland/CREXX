# cREXX Performance Roadmap

Status: live performance companion, refreshed 2026-09-04.

Project-wide priority and release ordering belongs in
[`docs/ROADMAP.md`](../docs/ROADMAP.md). This file records only current
performance closeout, evidence-gated performance candidates, and their
selection state. It is not a second product roadmap.

The completed PERF3 activity and idea register is preserved in
[`PERF3-PROGRAMME-LEDGER-2026-08-17.md`](PERF3-PROGRAMME-LEDGER-2026-08-17.md).
Current measured results belong in [`RESULTS.md`](RESULTS.md), enduring
measurement rules in
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md), and accepted or
rejected mechanism decisions in [`DECISIONS.md`](DECISIONS.md).

## Current Position

The completed Apple portfolio-v3 scorecard is strong overall: cREXX is well
ahead of ooRexx on the common five, ahead of genuine NetRexx on its comparable
common four, and split seven wins each with CPython across the fourteen Python
controls. The weaker rows are useful product-shape evidence, especially around
object ownership, nested containers, and complex graph workloads; they do not
authorize another general optimization programme.

Beta 3 now targets 2026-09-30 after the extended performance programme. Work
before that cut is limited to the existing closeout obligations, KeyAccess
decisions, and defect-driven qualification; no new broad performance stage
belongs in beta 3. The feature-bearing beta train ends at beta 6 on 2027-03-31,
so Release 1 performance must be refreshed against the exact April RC1/May 2027
release candidate rather than inferred from the retained August 2026 scorecard.

`POSTPERF-01` through `POSTPERF-05` are complete. No `POSTPERF-06` exists and
no new production compiler, RXAS, VM, language, RXBIN, ABI, or architecture
performance edit is automatically authorized. New production work requires a
separately selected gate and the first ordinary profiling-off Release verdict
defined in [`AGENTS.md`](AGENTS.md).

## Activity Register

`CHANNEL-LIFETIME-01`: explicit completed-request release and bounded bookkeeping.
Adrian accepted the first Release verdict and 2.51 MiB retained-request RSS
tradeoff on 2026-09-09. Documentation and full local Debug/Apple-ASan correctness
qualification pass (2,298 checks each). Adrian accepted the later-platform handoff
to Hotfix release QA and authorized develop publication/local install on 2026-09-09;
see [the selected design and scope](../concurrency/CHANNEL-REQUEST-LIFETIME.md).
This is a channel lifecycle defect repair, not a new broad performance stage.

| ID | Status | Work | Exit or decision |
| --- | --- | --- | --- |
| PERF-CLOSEOUT-01 | required beta 3 closeout | Resolve the remaining formal Linux QA-C obligation for the frozen Apple Stage 5 evidence, and run the named exact-SHA hosted gates for the 2026-09-30 beta 3 candidate. Do not relabel an unmatched newer run as the missing counterpart to the retained `81f159186` scorecard. | The retained scorecard has an explicit Linux disposition, and the exact beta 3 candidate has the required cross-platform evidence. |
| PERF-CLOSEOUT-02 | awaiting maintainer review | Accept, revise, or reject the completed [`KEYACCESS-01`](KEYACCESS-01-WORKLIST.md) and [`KEYACCESS-02`](KEYACCESS-02-WORKLIST.md) first Release verdicts. The selected fixes report roughly 187x improvement for 50,000-key insertion and reduce the 500,000-key missing/present ratio from 17.97x to 0.95x while preserving full-key comparison and error handling. | Adrian records the product decision; any requested rework receives its own bounded validation. |
| PERF-RELEASE1-01 | planned release verification | After the 2027-03-31 feature freeze, qualify the frozen portfolio and run the smallest decisive exact-candidate scorecard needed for Release 1. This is verification of the accumulated product, not automatic optimization authority. | The 2027-04-15 RC1 and 2027-05-01 Release 1 candidate have correctness-gated, platform-labelled throughput/lifecycle/RSS/artifact evidence and explicit dispositions for material regressions. |
| PERF-REVIEW-01 | evidence-gated; no automatic change | Review CD, DeltaBlue, Towers, and Havlak only for a safe, general, material mechanism. Preserve workload equivalence and treat an inconclusive result as a retained finding. | Select one bounded mechanism with a first Release verdict, or defer the row without production change. |
| PERF-NEXT-01 | proposed next-release product work | Use Storage/List and related graph evidence to inform the Level G ownership and nested-container workstream in `docs/ROADMAP.md`. This is a product-capability question, not permission for a benchmark-specific speed patch. | A separately approved ownership/container contract and equivalent benchmark control exist before implementation timing is compared. |
| PERF-NEXT-02 | later evidence queue | Retain NBody and Permute as product questions, not assumed optimizer defects. Re-profile against the then-current product before selecting work. | Current evidence identifies a general mechanism and passes the normal selection gate, or the item remains deferred. |

## Candidate Queue Below The Cut

These items remain available for future selection but are not active merely
because they appeared in the completed programme ledger:

- `PERF-COMPILER-01`: later Level L inline slices, bounded register
  finalisation, hoisting, and late-inlining consumers. Start from a current
  profile and reuse the graph/proof service; do not broaden an unsupported
  inline shape as a shortcut.
- `PERF-RUNTIME-01`: numeric value caching, string-copy fast paths, signal
  specialization, and related runtime ideas. Each needs an attributable current
  workload, semantic guards, and a regression budget before selection.
- `PERF-LINK-01`: VM/link and dead-code hygiene. Keep artifact size, lifecycle,
  throughput, and late-loaded/plugin behaviour as separate verdicts.
- `PERF-ISA-01`: file-I/O call migration and measured RXAS instruction-family
  review. These remain post-Release-1 design studies; no opcode or RXBIN change
  follows from the old ledger alone.
- `PERF-JIT-01`: MIR/JIT/LLVM-style backend research. This remains exploratory
  and outside the interpreter and bytecode release contract.

## Closed Programme Evidence

- PERF3 and the five-stage POSTPERF sequence are complete. Their activity
  history, negative results, transfers, and evidence links are in the
  [historical PERF3 ledger](PERF3-PROGRAMME-LEDGER-2026-08-17.md) and
  [`POST-PERF3-WORKLIST.md`](POST-PERF3-WORKLIST.md).
- Generic final/concrete scalar access, packed native numeric ownership, the
  exact CPU `rxvector` provider, bounded late profitability, and the reusable
  RXAS proof infrastructure have completed their governed verdicts. They are
  current implementation evidence, not open roadmap stages.
- The Apple Stage 5 scorecard is retained under
  [`evidence/2026-08-18-performance-closeout-stage5/`](evidence/2026-08-18-performance-closeout-stage5/).
  Its numbers describe that frozen candidate; release claims still require the
  release/tag and named final-candidate gates.

## Update Rules

1. Add an idea here only when it is a plausible current or future selection;
   give it a stable ID, hypothesis, affected surfaces, risks, evidence gate,
   and disposition.
2. Move durable accepted/rejected mechanism lessons to `DECISIONS.md`. Preserve
   large closed activity histories in a dated ledger or worklist rather than
   expanding this live file indefinitely.
3. An item is `active` only after Adrian selects its bounded gate. Observation,
   profiling, implementation, first verdict, acceptance, broad closeout, and
   hosted/release qualification remain distinct states.
4. Correctness and workload equivalence precede timing. A benchmark finding is
   not by itself a correctness defect, release blocker, or product-change
   authorization.
5. Do not repeat completed broad testing when code and relevant build/test
   inputs are unchanged. Exact-SHA hosted release gates remain separate.

## RXC-PROJECT-01: compiler and project-build scaling

Selected by Adrian on 2026-09-06; both bounded Release verdicts are accepted.
The [worklist](RXC-PROJECT-SCALING-WORKLIST.md) records the measured declaration
walk, binary forward-declaration and project-key mechanisms, their history and
rejected alternatives. Normal optimization and strict import/callable validation
are retained. The frozen application wave improves from 509.45 to 70.98 seconds;
the separate ADDRESS library from 156.18 to 11.53 seconds. Dependency snapshots
are automatic; real imported implementations still invalidate their consumers.
macOS Debug/Release/Apple-ASan and scratch installed/offline checks are qualified
for the local develop repair. Linux/Windows and release publication remain
separate; this is not a new broad performance stage or portfolio claim.
