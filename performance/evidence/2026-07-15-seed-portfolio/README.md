# Seed portfolio process baseline — 2026-07-15

Status: initial NR-01 evidence slice; not a Release 1 portfolio baseline

This bundle exercises manifest version 1 in
`performance/portfolio/manifest.md`. It proves the first runner/evidence path
and preserves serial samples; it does not complete NR-01 or establish a
regression threshold.

The cross-bundle median index is maintained in
`performance/evidence/benchmark-median-summary.md`; keep this bundle's raw
rows unchanged and add future runs as separate master-table rows.

## Source and host

- Capture started: `2026-07-15T14:01:07+01:00`
- Capture completed: `2026-07-15T14:05:06+01:00`
- RexxCPS CPS supplement completed: `2026-07-15T15:58:48+01:00`
- Branch: `develop`
- Commit: `44dd4dbf3da624e2d8e79eccf696f78f023d436e`
- Worktree: dirty only with the new performance workspace, root guidance,
  runner raw-sample support and matching benchmark documentation
- Host: `Darwin 25.5.0 arm64`, macOS `26.5.2` (`25F84`)
- CPU: Apple M5, 10 logical CPUs
- CMake: `4.3.2`
- Generator: Ninja `1.13.2`
- C compiler: `/usr/bin/cc`, Apple clang `21.0.0`
- Build: clean targeted `Release`, C flags `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`
- Tool version: `crexx-1.0.0-beta.3+local.g44dd4dbf3da6.dirty 20260715`
- VM and bytecode: `rxvm`, optimized (`opt`)
- Lifecycle: `process`; a new VM/module load per sample
- Image mode: workload RXBIN plus `library.rxbin`, source/TRACE metadata retained
- Sampling: serial by workload; two unrecorded warmups then ten recorded runs

## Commands

From the repository root:

```sh
cmake -S . -B cmake-build-release
cmake --build cmake-build-release --target clean
cmake --build cmake-build-release \
  --target language_benchmarks crexx rxvm --parallel 10
ctest --test-dir cmake-build-release -L benchmark \
  --output-on-failure --parallel 10
cmake-build-release/bin/crexx --nokeep \
  tests/benchmarks/run_benchmarks.crexx --args \
  --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 \
  --output performance/evidence/2026-07-15-seed-portfolio/summary.csv \
  --samples-output performance/evidence/2026-07-15-seed-portfolio/samples.csv
```

The benchmark-native RexxCPS rate was then captured three times through the
same standalone VM and image set used by the runner:

```sh
cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  cmake-build-release/bin/library.rxbin
```

## Correctness gate

The clean Release benchmark gate passed all 13 tests: the linked-artifact
fixture, optimized and unoptimized smoke runs for all five workloads, and both
runner images. Every timed child invocation must also pass its exit-code and
`PASS:`-line check or the runner fails without producing valid evidence.

## Results

Results are retained in `summary.csv`; every warmup and recorded observation is
retained in execution order in `samples.csv`.

| Workload | Minimum (ns) | Median (ns) | Mean (ns) | Maximum (ns) | Median (ms) |
| --- | ---: | ---: | ---: | ---: | ---: |
| Sieve | 23,945,000 | 24,266,000 | 24,292,900 | 24,795,000 | 24.266 |
| Permute | 96,556,000 | 99,816,500 | 107,093,900 | 147,757,000 | 99.817 |
| Mandelbrot | 185,373,000 | 186,261,500 | 188,632,900 | 209,118,000 | 186.262 |
| Towers | 694,024,000 | 699,026,000 | 698,852,800 | 703,376,000 | 699.026 |
| RexxCPS 2.2c for cREXX | 11,593,419,000 | 11,722,164,000 | 11,730,635,400 | 12,002,027,000 | 11,722.164 |

The sample stream contains exactly 60 observations: two warmups and ten
recorded runs for each of five workloads. Recomputing count, minimum, median,
mean and maximum from the recorded raw rows matched every summary row exactly.
The Permute maximum is visibly above the other recorded values; it is retained
as machine noise/evidence rather than discarded by an undocumented outlier
rule.

### RexxCPS benchmark-native clause rate

The process table above answers “how long did the complete child process
take?” RexxCPS also reports its own internally timed historical metric. The
three direct `rxvm` observations are retained in `rexxcps-cps.csv`:

| Observation | Reported internal time | cREXX clauses per second |
| ---: | ---: | ---: |
| 1 | 11.9 s | 843,110 |
| 2 | 11.7 s | 853,668 |
| 3 | 11.9 s | 840,723 |
| **Mean** | — | **845,834** |
| **Median** | — | **843,110** |

This is **RexxCPS 2.2c for cREXX** on the standalone Release `rxvm`, optimized
RXBIN, metadata-retained image set. It measures nominal Rexx clauses per second,
not VM instructions per second. It must not be reconstructed from the runner's
wall-clock column, and no refreshed ooRexx/Regina/NetRexx ratio is claimed from
this bundle.

## Interpretation boundary

These are process-startup-inclusive, metadata-retained seed measurements from
one dirty beta 3 WIP source state on one Apple M5 host. They validate the
NR-01 capture slice and can seed later paired runs made with the same contract.
They are not steady-state timings, cross-language results, released beta 3
results, an approved portfolio score, or evidence for a particular
optimization.
