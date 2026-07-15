# cREXX clean-build O3 rerun — 2026-07-15

Status: diagnostic confirmation after clean CLion Release/Debug rebuilds; not
an NR-10 formal baseline

## Decision

The current cREXX source has not reproduced the apparent RexxCPS slowdown.
The clean Release build's canonical recorded median is **857,561 CPS**, 1.7%
above the seed median of 843,110 CPS and 62.5% above the later 527,607-CPS
pilot. Opaque A and B also recovered to 840,065 and 836,741 CPS respectively.

This rules out a persistent loss of optimization in the current rebuilt
artifact. It cannot retrospectively distinguish transient host load from a
stale pre-rebuild artifact, so the earlier 500k observations remain retained
rather than rewritten.

## Source, build and host

- Branch: `develop`
- Commit: `44dd4dbf3da624e2d8e79eccf696f78f023d436e`
- Worktree: dirty with the in-progress performance programme and benchmark
  work; no VM, compiler, assembler or measured benchmark source changed
  between the old and new measurements
- Host: `Darwin 25.5.0 arm64`, Apple M5, 10 logical CPUs
- Generator: Ninja 1.13.2 through CMake 4.3.2
- C compiler: `/usr/bin/cc`, Apple clang 21.0.0
- CMake build type: `Release`
- Effective VM compile and link flags: `-O3 -DNDEBUG -arch arm64`
- `CREXX_VM_PROFILING=OFF`
- Tool version:
  `crexx-1.0.0-beta.3+local.g44dd4dbf3da6.dirty 20260715`
- VM/image mode: `rxvm`, optimized RXBIN, `library.rxbin`, source/TRACE
  metadata retained
- Lifecycle: one fresh VM/module load per sample; all samples serial

The session began on battery at 73%, with no thermal or performance warning
and one-minute load average 1.82. Power changed to AC before the session ended;
the four-workload isolated repeat and Towers repeat were captured entirely on
AC, again with no warning. The machine remained an interactive desktop with
load averages around 2–3, so this bundle is diagnostic evidence rather than a
quiet-machine formal baseline.

## Correctness and commands

The rebuilt Release correctness gate passed the linked-artifact fixture and
both optimized RexxCPS tests (3/3):

```sh
ctest --test-dir cmake-build-release \
  -R '^benchmark_rexxcps_levelb(_opaque)?_opt$' --output-on-failure
```

RexxCPS canonical and opaque A/B used the Level B cREXX capture tool with one
warmup and three recorded full-default runs. The canonical command shape was:

```sh
cmake-build-release/bin/crexx performance/tools/run_cross_runtime.crexx \
  --nokeep --args --workload rexxcps \
  --runtime crexx-release-clang-o3 --variant 2.2c-canonical-opt \
  --warmups 1 --runs 3 --expect 'PASS: RexxCPS 2.2c cREXX port' \
  --metric-prefix 'Performance:' \
  --output-dir performance/evidence/2026-07-15-crexx-rexxcps-o3-rerun/canonical-opt -- \
  cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  cmake-build-release/bin/library.rxbin
```

Opaque A/B used `benchmark_rexxcps_levelb_opaque_opt.rxbin -a A|B` and the
matching diagnostic correctness line. Each manifest retains the absolute
argv, cREXX version, timestamps and result; every stdout/stderr stream is
retained under the corresponding `raw/` directory.

The first all-five repeat used the Level B portfolio runner with two warmups
and ten recorded runs:

```sh
cmake-build-release/bin/crexx tests/benchmarks/run_benchmarks.crexx \
  --nokeep --args --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 \
  --output performance/evidence/2026-07-15-crexx-rexxcps-o3-rerun/portfolio-summary.csv \
  --samples-output performance/evidence/2026-07-15-crexx-rexxcps-o3-rerun/portfolio-samples.csv
```

Because the power source changed during the wider session, Sieve, Permute,
Mandelbrot and Towers were then recaptured individually on AC through
`run_cross_runtime.crexx`, with two warmups and ten recorded runs. Towers was
repeated once more because its longer process times were visibly bimodal.

## RexxCPS native-rate results

These are benchmark-native clause rates, not harness wall-clock rates.

| Variant | Recorded CPS values | Median CPS | Versus earlier matching pilot |
| --- | --- | ---: | ---: |
| Canonical | 857,561; 861,908; 857,288 | **857,561** | +62.5% |
| Opaque A | 852,392; 840,065; 836,771 | **840,065** | +56.9% |
| Opaque B | 843,094; 836,741; 830,635 | **836,741** | +51.2% |

Canonical is 1.7% above the seed median. The three rebuilt variants occupy a
narrow 2.5% band, unlike the common roughly 500k slowdown in the earlier
qualification pilots.

## Portfolio process-time results

Lower is better. The all-five runner pass is retained even though the wider
session crossed a power-source change. The AC-only rows are separate captures,
not replacements.

| Workload/run | Recorded n | Median ms | Versus seed median |
| --- | ---: | ---: | ---: |
| Sieve / all-five runner | 10 | 23.194 | -4.4% |
| Permute / all-five runner | 10 | 98.056 | -1.8% |
| Mandelbrot / all-five runner | 10 | 185.480 | -0.4% |
| Towers / all-five runner | 10 | 688.274 | -1.5% |
| RexxCPS / all-five runner | 10 | 12,732.693 | +8.6% |
| Sieve / AC isolated | 10 | 23.482 | -3.2% |
| Permute / AC isolated | 10 | 97.120 | -2.7% |
| Mandelbrot / AC isolated | 10 | 188.084 | +1.0% |
| Towers / AC isolated | 10 | 765.709 | +9.5% |
| Towers / AC repeat | 10 | 770.930 | +10.3% |

Sieve, Permute and Mandelbrot show no material slowdown. Towers is noisy: the
same rebuilt optimized artifact produced a 688-ms median in the all-five pass,
then 766-ms and 771-ms medians in the busier AC-only passes. All passing values
are retained. The faster series proves the rebuilt artifact can still achieve
the seed band, while the spread shows that a quiet-machine NR-10 run is needed
before treating Towers as a regression.

The RexxCPS process-duration row includes startup and a self-calibrating amount
of benchmark work. Its benchmark-native CPS rows above are the appropriate
answer to the optimization-loss question.

## Retained files

- `canonical-opt/`, `opaque-a-opt/`, `opaque-b-opt/`: manifests, serial
  samples and every raw stream
- `portfolio-summary.csv`, `portfolio-samples.csv`: first all-five Level B
  runner pass
- `portfolio-ac/*/`: AC-only manifests, samples and raw streams, including the
  additional Towers repeat
