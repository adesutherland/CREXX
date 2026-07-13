# cREXX Language Performance Benchmarks

This directory adds a portable language-workload layer above the focused VM,
optimizer, collection, iterator, and library comparisons in
`tests/performance/`. The programs have deterministic reference results and no
timed I/O in their kernels.

## Included programs

| Program | Provenance | Main pressure |
| --- | --- | --- |
| `rexxcps_levelb.crexx` | Level B adaptation of Mike Cowlishaw's RexxCPS 2.2 workload | A historically representative classic Rexx clause mix |
| `awfy_sieve.crexx` | Are We Fast Yet? / SOM | integer arrays, indexed access, nested loops |
| `awfy_permute.crexx` | Are We Fast Yet? / SOM | recursion, method calls, array mutation |
| `awfy_mandelbrot.crexx` | Are We Fast Yet? / Benchmarks Game | floating-point arithmetic, branches, bit operations |
| `awfy_towers.crexx` | Are We Fast Yet? / SOM | object allocation, object attributes, recursion |

The Are We Fast Yet? suite was published with the DLS 2016 paper
“Cross-Language Compiler Benchmarking—Are We Fast Yet?” (DOI
`10.1145/2989225.2989232`). Its porting rules prioritize deterministic,
well-typed workloads and comparable code structure.

RexxCPS is different: it reconstructs a dynamic mix derived from about 2.5
million lines of traced Rexx programs and reports clauses per second. The Level
B type system and explicit stem objects require source changes, so the result
printed here is deliberately named `estimated_levelb_clauses_per_second`. It is
useful for cREXX regressions, but it must not be quoted as an official,
unchanged RexxCPS 2.2 score. For comparisons with ooRexx or Regina, run the
unmodified upstream RexxCPS 2.2 source separately.

RexxCPS measures **Rexx clauses**, not cREXX VM instructions. For VM
instructions per second, sum the `count` column of the profiler's `instruction`
rows and divide by the non-profiled elapsed time for the identical workload.
Do not use the instrumented elapsed time: per-instruction timestamps perturb
the run.

## Build and correctness checks

The CMake target builds optimized and unoptimized bytecode for every program.
CTest smoke-runs both forms and verifies reference results without asserting
machine-specific timing thresholds:

```bash
cmake --build cmake-build-release --target language_benchmarks --parallel
ctest --test-dir cmake-build-release -L benchmark --output-on-failure
```

## Repeatable timing

Use a Release build, keep the machine otherwise idle, and run enough work for
each measured process to last at least a few hundred milliseconds. The runner
does two unrecorded warmups, checks every result, and emits min/median/mean/max
wall-clock time as CSV:

```bash
cmake-build-release/bin/crexx --nokeep tests/benchmarks/run_benchmarks.crexx \
  --args --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 --output rxvm-opt.csv
```

The runner itself is Level B cREXX. It records microsecond-resolution wall
time as nanoseconds in the CSV columns, validates every child run by exit code
and `PASS:` output, and identifies the runner with `crexx_version`.

Repeat with `rxbvm` and `noopt`. Compare one change at a time on the same host,
power mode, compiler, build type, and commit. Keep raw samples; do not compare
single best-case numbers from different machines. Process-level results include
VM and module startup, which is intentional for cold-run measurements. For
steady-state kernel work, increase the matching argument in
`benchmark_command()` in `run_benchmarks.crexx` rather than subtracting startup
time.

## cREXX profiling workflow

Configure a separate profiling build because profiling hooks are compiled out
of normal builds:

```bash
cmake -S . -B cmake-build-profile -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON
cmake --build cmake-build-profile --target language_benchmarks rxseq --parallel
```

Instruction/transition timing (diagnostic only; do not use the instrumented
elapsed time as the benchmark result):

```bash
cmake-build-profile/bin/rxvm \
  --profile-output sieve-profile.csv \
  cmake-build-profile/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-profile/bin/library.rxbin -a 50
```

Dynamic three-instruction sequences, followed by offline clustering:

```bash
cmake-build-profile/bin/rxvm \
  --sequence-count=3 --sequence-output sieve.rxseq \
  cmake-build-profile/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-profile/bin/library.rxbin -a 50
cmake-build-profile/bin/rxseq sieve.rxseq \
  cmake-build-profile/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-profile/bin/library.rxbin --output sieve-sequences.csv
```

Use the timing profile to find costly opcodes and transitions, the sequence
profile to find high-volume instruction windows, and optimized `.rxas`/`rxdas`
output to map those costs back to source. Confirm every optimization with the
ordinary non-profiled Release build and the correctness tests.

## Expansion order

The next useful Are We Fast Yet? ports are `Queens`, `Bounce`, `List`, and
`Storage`, followed by the larger `Richards`, `DeltaBlue`, `Havlak`, and JSON
workloads. The larger group is especially valuable after the basic array,
object, recursion, and floating-point paths are stable.

See `THIRD_PARTY_NOTICES.md` for attribution and license terms.
