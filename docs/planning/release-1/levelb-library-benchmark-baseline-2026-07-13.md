# Level B library programme benchmark baseline — 2026-07-13

This is programme step -1 for the high-priority Level B library work list. It
is a coarse, process-level sanity baseline, not a selector microbenchmark suite
or a general performance-management framework. The existing five-program
language suite was used unchanged. The RexxCPS Level B adaptation is the
closest current foundation-library workload in that suite.

## Source and host

- Timestamp: `2026-07-13T15:08:33+01:00`
- Branch: `develop`
- Commit: `3fee485cdc741124c39934c2bbe2a8ff7f6a30c9`
- Worktree before the formal run: one untracked planning-only file,
  `docs/planning/release-1/levelb-library-temporary-worklist.md`; no source or
  benchmark changes
- Host: `Darwin 25.5.0 arm64`, macOS `26.5.1` (`25F80`)
- CPU: Apple M5, 10 logical CPUs
- CMake: `4.3.2`
- Generator: Ninja `1.13.2`
- C compiler: `/usr/bin/cc`, Apple clang `21.0.0`
- Build: `Release`, C flags `-O3 -DNDEBUG`
- Measured VM: `rxvm`
- Bytecode mode: `opt`
- Tool version: `crexx-1.0.0-beta.3+local.g3fee485cdc74`

The pre-existing Release tree initially contained native tools from ancestor
commit `eebe9817930a`. Those exploratory results were discarded. The tree was
reconfigured and cleaned, then the benchmark artifacts, `crexx`, and `rxvm`
were rebuilt. Both `rxc` and `crexx --version` reported the current
`g3fee485cdc74` build before the formal run.

## Commands

From the repository root:

```sh
cmake -S . -B cmake-build-release
cmake --build cmake-build-release --target clean
cmake --build cmake-build-release --target language_benchmarks --parallel 10
cmake --build cmake-build-release --target crexx rxvm --parallel 10
ctest --test-dir cmake-build-release -L benchmark \
  --output-on-failure --parallel 10
cmake-build-release/bin/crexx --nokeep \
  tests/benchmarks/run_benchmarks.crexx --args \
  --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 --output <results.csv>
```

The benchmark smoke gate passed all 13 tests: the linked-artifact fixture plus
optimized and unoptimized smoke runs for the five programs and runner.

## Formal results

Each workload had two unrecorded warmups and ten recorded runs. Times are
process-level wall-clock nanoseconds and intentionally include VM/module
startup. Every child run passed its deterministic correctness check.

| Workload | Minimum (ns) | Median (ns) | Mean (ns) | Maximum (ns) | Median (ms) |
|---|---:|---:|---:|---:|---:|
| Sieve | 32,573,000 | 35,035,500 | 34,883,400 | 35,741,000 | 35.036 |
| Permute | 122,821,000 | 129,424,500 | 132,343,900 | 146,989,000 | 129.425 |
| Mandelbrot | 195,695,000 | 227,606,500 | 249,329,300 | 342,092,000 | 227.607 |
| Towers | 940,991,000 | 980,985,500 | 977,532,100 | 1,014,329,000 | 980.986 |
| RexxCPS Level B adaptation | 2,383,463,000 | 2,428,091,500 | 2,445,497,200 | 2,515,603,000 | 2,428.092 |

## Interpretation and step 8 rule

These figures are only a broad regression signal. They do not isolate a
particular Level B selector, and an aggregate change must not be attributed to
one implementation without focused evidence. The Mandelbrot range also shows
that machine noise can be material for a single ten-run sample.

Programme step 8 must rerun the same five workloads with `rxvm`, optimized
bytecode, two warmups, ten recorded runs, and the same workload arguments. It
will report per-workload absolute results and percentage changes, without
hiding a regression behind improvement elsewhere.

## Programme step 8 results — 2026-07-14

The Release tree was incrementally refreshed at
`2026-07-14T01:20+01:00`. Stale generated aggregate images were removed after
the first incremental attempt exposed an old ABBREV signature; source and
member images were not cleaned. The benchmark runner then used the same
`rxvm`/optimized-bytecode settings, two warmups, ten recorded runs, and
unchanged workload arguments. Every child correctness check passed.

| Workload | Minimum (ns) | Median (ns) | Mean (ns) | Maximum (ns) | Baseline median (ns) | Median change (ns) | Median change |
|---|---:|---:|---:|---:|---:|---:|---:|
| Sieve | 22,352,000 | 22,719,500 | 22,839,800 | 24,554,000 | 35,035,500 | -12,316,000 | -35.15% |
| Permute | 91,134,000 | 93,987,500 | 93,804,200 | 96,717,000 | 129,424,500 | -35,437,000 | -27.38% |
| Mandelbrot | 147,709,000 | 154,303,500 | 165,026,300 | 271,727,000 | 227,606,500 | -73,303,000 | -32.21% |
| Towers | 683,276,000 | 708,681,000 | 706,280,900 | 725,088,000 | 980,985,500 | -272,304,500 | -27.76% |
| RexxCPS Level B adaptation | 1,465,897,000 | 1,495,438,500 | 1,497,435,000 | 1,526,455,000 | 2,428,091,500 | -932,653,000 | -38.41% |

The result is a healthy aggregate sanity signal, not evidence that any single
selector caused a particular improvement. The Mandelbrot maximum again shows
why this programme deliberately does not treat this suite as a performance
management framework.
