# PERF3-02-C2E2-P1A first ordinary Release verdict

Status: **A1 correct and byte-identical; performance neutral on Richards;
rework-or-accept decision required**

Date: 2026-08-01

Source baseline: local `develop` at
`4a3940395980dc40ea45917d71d99caa080e89bb`, with the candidate source diff
retained in `source.diff`. The worktree also contained the roadmap/worklist
records and five protected untracked lifecycle RXBIN files; none of those
RXBIN files was read, changed or staged.

## Candidate

A1 retains typed normal/signal-skip/signal-retry edges and the complete bounded
graph-owned storage service, but attaches the storage environment on demand.
The current debug identity report requests it; the ordinary non-debug product
has no rewrite consumer and previously built then immediately freed it.

The candidate changes no optimizer rule, public RXAS/RXBIN/ABI surface,
runtime path, signal contract or diagnostic semantics.

## Correctness and identity

- Debug and ordinary Release `rxas` build successfully.
- Focused tactical, whole-procedure, storage and optimized/no-opt dual-VM
  runtime matrix: 24/24 pass.
- Locked-P1 and candidate NR27 identity/flow/rejection summaries are exact for
  24 Richards procedures, 13 Towers procedures and 14 focused-fixture
  procedures.
- Locked-P1, candidate and pre-P1 ordinary RXBINs are byte-identical for both
  target inputs.
- Exact results are in `diagnostic-equivalence.csv`, `diagnostics/`, `logs/`
  and `artifacts.csv`.

## Measurement

The ordinary profiling-off Release binaries assembled the exact committed
C1abc Richards and Towers RXAS inputs. Each variant received two warmups and 30
recorded observations per workload. The three variants used all six order
permutations five times, serially. Elapsed time includes process startup and
complete assembly. No outlier was removed.

The host was an Apple M5 running Darwin 25.5.0/macOS 26.5.2, on AC with low
power mode off and no thermal or performance warning. Pre/post load state is
retained; the six-order matched schedule is the authority for before/after
interpretation.

## Verdict

| Workload | Comparison | Paired median | Mean | 95% mean interval |
| --- | --- | ---: | ---: | ---: |
| Richards | locked P1 vs pre-P1 | +2.201% | +3.973% | -1.259% to +9.205% |
| Richards | candidate vs pre-P1 | +3.679% | +3.603% | -1.779% to +8.986% |
| Richards | candidate vs locked P1 | -0.141% | +0.180% | -4.021% to +4.381% |
| Towers | locked P1 vs pre-P1 | -0.371% | +0.522% | -3.754% to +4.798% |
| Towers | candidate vs pre-P1 | -1.725% | -2.895% | -6.039% to +0.249% |
| Towers | candidate vs locked P1 | -0.945% | -2.791% | -6.048% to +0.466% |

A1 does not recover the material Richards cost and misses the stated within-1%
pre-P1 target. Its Towers direction is favorable but the interval includes
zero. This is a neutral first Release verdict, not an accepted improvement.

The result disproves eager final storage attachment as the dominant retained
Richards overhead. A2 compressed eager state and A3 demand-gated typed CFG
construction remain untested. The production edit stays provisional and the
programme stops here for a rework-or-accept decision; PERF3-05 has not begun.

## Evidence map

- `raw/samples.csv`: all 180 recorded observations;
- `summary.csv`: absolute descriptive statistics;
- `paired-effects.csv`: paired deltas and two-sided Student-t 95% intervals;
- `pre-state.txt`, `post-state.txt`: host/load/power/thermal snapshots;
- `diagnostics/`, `diagnostic-equivalence.csv`: exact P1/candidate summaries;
- `logs/`: build and 24/24 focused test logs;
- `source.diff`, `artifacts.csv`: candidate and binary/input identities;
- `COMMANDS.md`: exact build, proof and measurement schedule; and
- `checksums.sha256`: recursive integrity for the bundle.
