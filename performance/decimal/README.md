# Decimal backend performance workspace

This directory is the independent control plane for decimal-provider
correctness and performance engineering. It does not extend the completed
[`NUMERIC-01`](../NUMERIC-01-WORKLIST.md) typed-BIF/RexxCPS work and it does
not borrow implementation authority from the live PERF3 queue.

The approved engineering contract is
[`DECIMAL-01-ENGINEERING-PLAN.md`](DECIMAL-01-ENGINEERING-PLAN.md), and live
Gate 0 progress is recorded in
[`DECIMAL-01-WORKLIST.md`](DECIMAL-01-WORKLIST.md). Gate 0 validates the current
provider contract and the individual/combined RXAS numeric-context
instructions. No candidate backend, hybrid value representation, plugin ABI
change, or production edit is selected by this approval.

The preliminary source-attributed review that will inform the later extended
panel is [`PUBLIC-EVIDENCE-ORIENTATION.md`](PUBLIC-EVIDENCE-ORIENTATION.md).
Its external results are hypotheses and experiment-shape inputs, never CREXX
performance evidence.

The first Gate 0 correctness result is retained at
[`2026-08-05-decimal-01-rxas-numctx-opening`](../evidence/2026-08-05-decimal-01-rxas-numctx-opening/).
It proves the basic individual/combined RXAS state path and reproduces current
`FUZZ` plus `db_decimal` form/case/Common-rounding defects without fixing them.

Adrian subsequently authorized repair before further work and accepted the
focused verdict on 2026-08-05. It is retained at
[`2026-08-05-decimal-01-numctx-repair-verdict`](../evidence/2026-08-05-decimal-01-numctx-repair-verdict/):
Debug and ordinary profiling-off Release each pass 9/9 focused tests and 6/6
observable VM/provider cells. It contains no timing.

The 2026-08-18 current-provider capture is retained at
[`2026-08-18-decimal-01-gate1-current-provider`](../evidence/2026-08-18-decimal-01-gate1-current-provider/).
The isolated libmpdec 4.0.1 candidate and implementation double-check are at
[`2026-08-18-decimal-01-libmpdec-screen`](../evidence/2026-08-18-decimal-01-libmpdec-screen/).
Libmpdec passed the focused semantic and raw-copy lifecycle proof but failed
the L1 progression gate: arithmetic and conversion were materially slower and
only comparison improved by at least 15%. It therefore did not proceed to
L2/L3 and did not change the production provider or ABI.

The remaining D2 tuned-decNumber and D3 fixed-34 decQuad calibration is at
[`2026-08-18-decimal-01-stage3-calibration`](../evidence/2026-08-18-decimal-01-stage3-calibration/).
All 48 D2 builds passed their provider contracts, but no complete build showed
credible timing headroom. D3 passed its admitted 9/18 semantic and lifecycle
boundary but was materially slower in adapter and direct-core arithmetic.
Stage 3 therefore closes as no change: retain the current 8/64/64
`mc_decimal`; no production provider, plugin ABI or hybrid design is selected.
The maintained Level B decision-table builder is
[`summarize_stage3_calibration.crexx`](summarize_stage3_calibration.crexx).
The enduring no-repeat lessons and explicit reopening triggers are recorded in
[`performance/DECISIONS.md`](../DECISIONS.md).

The qualified Gate 1 boundary and exact checksums are recorded in
[`GATE1-CELL-MATRIX.md`](GATE1-CELL-MATRIX.md). Those independently authored
kernels form the arithmetic core of the draft publishable
[`CREXX Decimal Benchmark (CDB-1)`](CREXX-DECIMAL-BENCHMARK.md). CDB-1 uses
standard decimal semantics as its correctness authority but is not represented
as an IEEE, ANSI or General Decimal Arithmetic performance standard.

The permanent boundary is:

- plans, worklists, candidate dispositions and evidence indexes live here;
- approved Level B workloads and implementation comparisons live under a
  separately named `tests/performance/decimal/` subtree;
- maintained orchestration remains Level B cREXX under `performance/tools/`;
- third-party source, disposable native comparators and candidate builds stay
  in an external scratch root until a candidate passes the selection gate; and
- production provider code remains under
  `interpreter/rxvmplugin/rxvmplugins/` and is changed only after explicit
  approval.

The Mac is a shared performance host. Correctness builds may proceed, but any
new timing session requires a fresh clear-host confirmation; the retained
2026-08-18 sessions record Adrian's confirmation and pre/post host state.
