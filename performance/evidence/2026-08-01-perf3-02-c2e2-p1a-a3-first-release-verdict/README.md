# PERF3-02-C2E2-P1A A3 ordinary Release verdict

Status: **correct and byte-identical; no demonstrated A3 performance gain;
revert-or-accept decision required**

Date: 2026-08-01

Source baseline: local `develop` at
`4a3940395980dc40ea45917d71d99caa080e89bb`, with the complete provisional
A1+A3 source diff retained in `source.diff`. The worktree also contained the
P1/P1A control records, the immutable A1 verdict bundle and five protected
untracked lifecycle RXBIN files. None of those lifecycle files was read,
changed or staged.

## Candidate

A3 keeps A1 demand-driven storage attachment and makes typed signal-edge
construction explicit. Existing fixed-point rewrite graphs request normal
successors only; the final debug/storage graph requests normal,
signal-skip and signal-retry successors. A graph records its mode and
`flow_storage_attach()` fails closed when typed signal continuations were not
requested.

The candidate changes no optimizer rule, public RXAS/RXBIN/ABI surface,
runtime path, signal contract or diagnostic semantics.

## Correctness and identity

- Debug and ordinary profiling-off Release `rxas` build successfully.
- Focused tactical, whole-procedure, storage and optimized/no-opt dual-VM
  runtime matrix: 24/24 pass.
- Locked-P1 and A3 NR27 identity/flow/rejection summaries are exact for 24
  Richards procedures, 13 Towers procedures and 14 focused-fixture
  procedures.
- Pre-P1, locked-P1, A1 and A3 ordinary RXBINs are byte-identical for Richards
  and Towers. Locked-P1, A1 and A3 are byte-identical for the focused fixture.
- The canonical output hashes remain `bb7832ff...` for Richards,
  `c29785fa...` for Towers and `a2a0ea00...` for the fixture.

## Measurement

The ordinary Release binaries assembled the exact committed C1abc Richards
and Towers RXAS inputs. Each variant received two warmups. The first block used
all 24 four-variant order permutations once. Because paired intervals crossed
zero and the guard, the governed noise rule appended 12 balanced even
permutations, for 36 recorded paired rounds per workload and variant. Each
variant occupied every order position nine times and every pair preceded the
other 18 times. No outlier was removed.

Elapsed time includes process startup and complete assembly. The host was an
Apple M5 running Darwin 25.5.0/macOS 26.5.2, on AC with low-power mode off and
no thermal or performance warning. Pre/post host and load state is retained;
the balanced same-session schedule is the authority for interpretation.

## Verdict

| Workload | Comparison | Paired median | Mean | 95% mean interval |
| --- | --- | ---: | ---: | ---: |
| Richards | locked P1 vs pre-P1 | +1.562% | +1.767% | -1.978% to +5.512% |
| Richards | A1 vs pre-P1 | +0.154% | +1.274% | -1.951% to +4.499% |
| Richards | A3 vs pre-P1 | +2.556% | +1.401% | -1.615% to +4.417% |
| Richards | A3 vs locked P1 | -0.447% | +0.097% | -2.452% to +2.647% |
| Richards | A3 vs A1 | +1.460% | +0.509% | -2.287% to +3.305% |
| Towers | locked P1 vs pre-P1 | +1.003% | +2.283% | -0.947% to +5.512% |
| Towers | A1 vs pre-P1 | +0.523% | +0.545% | -2.485% to +3.576% |
| Towers | A3 vs pre-P1 | +0.509% | +0.140% | -2.942% to +3.222% |
| Towers | A3 vs locked P1 | -0.885% | -1.620% | -4.888% to +1.649% |
| Towers | A3 vs A1 | +0.250% | -0.144% | -2.710% to +2.422% |

A3 does not demonstrate recovered assembler cost. Its primary Richards median
misses the within-1% pre-P1 target and is 1.460% slower than A1; Towers is
neutral. Every interval remains ambiguous at the governed 36-pair maximum, so
this is `noisy/inconclusive`, not a selectable improvement.

The same low-interference session changes the earlier attribution: A1 itself is
only +0.154% median versus pre-P1 on Richards and +0.523% on Towers, though its
intervals also remain ambiguous. This supports dropping A3 rather than paying
for a second graph mode, while leaving acceptance of A1/P1 infrastructure to
Adrian's decision. A3 remains provisional and PERF3-05 has not begun.

## Evidence map

- `raw/samples.csv`: all 288 recorded observations;
- `summary.csv`: absolute descriptive statistics and noise measures;
- `paired-effects.csv`: paired quartiles, favorable counts, deltas and
  two-sided Student-t 95% intervals;
- `pre-state.txt`, `post-state.txt`, `append-pre-state.txt` and
  `append-post-state.txt`: host/load/power/thermal snapshots;
- `diagnostics/` and `diagnostic-equivalence.csv`: exact locked-P1/A3 proof;
- `logs/`: build and 24/24 focused test logs;
- `source.diff` and `artifacts.csv`: candidate and artifact identities;
- `COMMANDS.md`: reproduction and schedule; and
- `checksums.sha256`: recursive integrity for the bundle.
