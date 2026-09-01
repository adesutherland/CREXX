# PERF2-06/07 V1R01 first ordinary Release verdict

Status: **guard hit; stop for Adrian's rework/revert/accept decision**.

Adrian selected `PERF2-06-07-V1R01` after the complete Apple candidate panel.
The isolated candidate extends compiler-owned inlined-method receiver placement
to a proved nested `§this` linear leaf. It is not installed in the main source
tree and has not been committed.

The candidate passed the focused semantic gate (10/10) and proved its intended
machine ceiling before timing:

- Richards removed 16,404,842 copy operations and 130,345,888 copy bytes per
  governed run in each VM (-22.378%/-22.393%);
- Permute removed 10,077,000 copy operations and 72,554,400 copy bytes in each
  VM (-98.220%/-98.030%).

The first ordinary profiling-off Release block then ran one warmup and 12
balanced/interleaved recorded rounds for current and candidate products on both
VMs across Sieve, Permute, Bounce, Richards, Base64 and RexxCPS. All 312
executions passed correctness. Richards and Permute were strongly favorable,
but Bounce was clearly adverse in every pair: paired median **+75.743%** on
`rxvm` and **+67.267%** on `rxbvm` (elapsed time, lower is better). Both exceed
the 3% comparable Tier A guard.

The causal count review is exact. The new leaf restrictions were accidentally
applied to the pre-existing ordinary direct-object receiver path as well as the
new nested-`§this` case. That disables accepted direct receiver sharing for
branch-rich `ball.bounce()`, inserts a copy-in/copyback pair, executes exactly
1,000,000 additional `COPY_REG_REG` instructions, adds 6,000,019 value
operations and 40,000,120 value bytes, and explains the timing regression in
both VMs. This is a candidate placement error, not a VM-mode discrepancy.

The five common-workload median geometric mean remains favorable at 1.111675x
(`rxvm`) and 1.105427x (`rxbvm`), but governance does not allow that aggregate
gain to waive Bounce's individual guard. Artifact size does not hit its guard.
Lifecycle and RSS were not run: the Stage 4 ladder reserves them for an
accepted candidate, and the decisive throughput guard ended this first verdict.

No rework, broad QA, sanitizer/package pass, production edit, commit, or
PERF2-08/09 work follows from this evidence without Adrian's next exact choice.

## Contents

- `VERDICT.md`: complete paired table, guard disposition and causal review.
- `PROVENANCE.md`: source, product, host, build and replay identities.
- `COMMANDS.md`: reproducible candidate, count and formal timing commands.
- `paired-summary-12.csv` and `common-geomean.csv`: Level B paired reductions.
- `exact-count-summary.csv` and `bounce-causal-summary.csv`: exact mechanism
  proof and guard-hit diagnosis.
- `artifact-summary.csv`: separately governed deterministic size dimension.
- `freeze/`: exact candidate workloads, RXAS, pre-link RXBIN, linked images and
  compiled Level B matrix RXAS/RXBIN.
- `timing/formal-12/`: all formal raw samples, outputs and harness summaries.
- `timing/invalidated-missing-library/`: retained pre-formal manifest-construction
  failure; it is not timing evidence.
- `counts/` and `diagnostics/`: raw deterministic dual-VM count profiles.
- `v3-correctness-base.patch` and `v1r01-linear-leaf.patch`: independently
  replayed source identity.
- `SHA256SUMS`: checksum closure for this evidence directory.
