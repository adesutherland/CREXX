# PERF3-05-R2 instruction-handler panel checkpoint

This bundle retains the approved Apple ARM64 checkpoint for the internal VM
instruction-handler placement framework. It compares the two concrete engines,
`rxtvm` and `rxbvm`; `rxvm` is a product alias and is not a third engine.

## Provenance and method

- Source baseline: clean `origin/develop` at
  `6a65b9c685b3776da211bcd209af14fcf23be445`.
- Branch: `codex/perf3-05-r2-handler-panel` in an isolated worktree.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 128 KiB L1I and
  6 MiB L2; Apple clang 21.0.0.
- Ordinary profiling-off Release: `-O3 -DNDEBUG`.
- Timing launch: AC power, no thermal/performance warning, no concurrent
  build/test process.
- Matrix: seven workloads, three handler shapes and both concrete engines;
  two warmups plus twelve retained recorded rounds per cell, serial and
  pairwise balanced within each workload.
- Correctness: all 588 timing executions passed their exact output oracle.
  All 504 recorded samples are retained; no sample was removed.

The all-inline, all-outline and profile-30 products all passed the complete
2,002-test Release suite. The all-outline setting additionally received a
fresh complete Debug build and suite; see `test-summary.txt`.

## Handler population and heat

The starting ISA has 650 public opcode slots: 62 reserved and 588
non-reserved. The two private execution-image handlers are also migrated but
are excluded from the percentage denominator. All 651 public/sentinel/private
handler implementations live once in the grouped definition files.

The common profile-selected panel places 176 of 588 non-reserved public
handlers inline (29.93%) and compiles 475 remaining public/private handlers as
force-noinline functions. Equal-workload profile aggregation found 184
instruction forms in the 22 frozen count profiles. Cumulative dynamic share
was 75.07% at 29 handlers, 91.35% at 59, 99.767% at 118 and
99.9999969% at 176. The leading forms were `BRF_ID_REG`, `UNLINK_REG`,
`LINKATTR1_REG_REG_INT`, `BR_ID`, `IEQ_REG_REG_INT`, `LOAD_REG_INT`,
`BRT_ID_REG`, `ICOPY_REG_REG`, `IADD_REG_REG_INT` and `IGT_REG_REG_REG`.

## Result

The framework makes owner size highly tunable. All-outline reduces `run()`
from about 530 KiB to about 32 KiB, below this host's 128 KiB L1I size, but
increases total `__text` because it preserves 651 callable bodies. Profile-30
reduces the owner to about 200 KiB, still larger than L1I, and emits 475
callable bodies. `static-shape.csv` contains the exact values.

Neither smaller owner is a production performance candidate as measured.
Relative to the mechanically equivalent all-inline control, profile-30 loses
9.347% geometric-mean normalized throughput on `rxtvm` and 12.082% on
`rxbvm`; all-outline loses 40.021% and 34.243%, respectively. The detailed
medians are in `performance-deltas.csv` and the raw harness output is retained.

Most importantly, the profile-30 loss is not handler-call overhead in these
workloads. Six of seven workload profiles execute no outlined handler at all;
RexxCPS executes only eight outlined instructions in roughly 23 million.
Despite that, profile-30 materially changes performance. The experiment
therefore directly confirms that changing the monolithic owner's population
changes compiler optimization, layout, register allocation and/or branch
placement even when the dynamic instruction stream never crosses the new call
boundary. Frequency ranking identifies the truly executed handlers, but it
does not by itself choose a fast owner shape.

This is the approved stop checkpoint. The internal framework is retained for
controlled follow-on panels, but no production/default policy is selected and
no further layout optimization is included here.

## Bundle map

- `baseline-manifest.txt`, `three-shape-manifest.txt`: exact commands/cells.
- `timing-capture-manifest.json`: capture identity and completion state.
- `host-state.txt`: toolchain, cache, power, thermal and load observations.
- `measured-products.sha256`: exact untouched and three-shape VM identities.
- `timing-samples.csv`, `timing-summary.csv`, `timing-outputs.csv`: raw and
  summarized formal timing/correctness evidence.
- `profile-ranking.txt`, `profile30-inline-names.txt`: frozen common ranking.
- `profiles/`: all 22 exact workload/engine count profiles used to build the
  ranking; the fourteen governed timing-workload profiles also support the
  executed-outline audit.
- `static-shape.csv`, `performance-deltas.csv`,
  `workload-outline-call-share.csv`: compact derived scorecards.
- `SHA256SUMS`: checksum closure for every other retained file.
