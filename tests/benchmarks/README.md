# cREXX Language Performance Benchmarks

This directory adds a portable language-workload layer above the focused VM,
optimizer, collection, iterator, and library comparisons in
`tests/performance/`. The programs have deterministic reference results and no
timed I/O in their kernels.

## Included programs

| Program | Provenance | Main pressure |
| --- | --- | --- |
| `rexxcps_levelb.crexx` | RexxCPS 2.2c, the cREXX Level B port of Mike Cowlishaw's RexxCPS 2.2 | A historically representative classic Rexx clause mix |
| `awfy_sieve.crexx` | Are We Fast Yet? / SOM | integer arrays, indexed access, nested loops |
| `awfy_permute.crexx` | Are We Fast Yet? / SOM | recursion, method calls, array mutation |
| `awfy_mandelbrot.crexx` | Are We Fast Yet? / Benchmarks Game | floating-point arithmetic, branches, bit operations |
| `awfy_towers.crexx` | Are We Fast Yet? / SOM | object allocation, object attributes, recursion |

The Are We Fast Yet? suite was published with the DLS 2016 paper
“Cross-Language Compiler Benchmarking—Are We Fast Yet?” (DOI
`10.1145/2989225.2989232`). Its porting rules prioritize deterministic,
well-typed workloads and comparable code structure.

RexxCPS is different: it reconstructs a dynamic mix derived from about 2.5
million lines of traced Rexx programs and reports clauses per second. Version
2.2c is a source port rather than a newly designed workload: its control flow,
1,000-clause dynamic mix, calibration, defaults, reporting shape, and stem-
default assignment follow upstream 2.2. The Level B type system still requires
the substitutions audited below. Quote its result as **RexxCPS 2.2c for
cREXX**, never as an unchanged RexxCPS 2.2 result.

Run the canonical benchmark without arguments so it uses the upstream `100 x
100` defaults and one-second self-calibration:

```bash
cmake-build-release/bin/crexx --nokeep \
  -lcmake-build-release/bin/library.rxbin \
  tests/benchmarks/rexxcps_levelb.crexx
```

The optional `count` and `averaging` arguments exist only for CTest and
development diagnostics. Results produced with overrides should report those
values and should not be compared with default community runs.

The direct `crexx` command and `run_benchmarks.crexx` execute the program and
library modules separately, retaining source/TRACE metadata. Optimized CTest
smoke tests instead create a linked image with `rxlink -s`. These are different
lifecycle modes: source stripping removes diagnostic metadata and currently
also avoids an accidental runtime metadata-scan cost in type/ADDRESS paths.
Published timing must state whether metadata was retained or stripped and must
not mix samples from the two modes.

### RexxCPS 2.2 timed-kernel equivalence audit

The execution counts below are for one outer iteration labelled “1,000
clauses” by RexxCPS. Counts come from the unchanged loop/select paths, not from
wall-clock inference. “Preserved” means the source clause count and intended
value effect are the same; it does not claim identical VM instructions.

| Upstream RexxCPS 2.2 | cREXX 2.2c port | Executions | Assessment |
| --- | --- | ---: | --- |
| `flag=0` | `flag = "0"` | 1 | Preserved; the explicit string spelling fixes the Level B type. |
| `do loop=1 to 14` | `do lvar = 1 to 14` | 1 loop / 14 bodies | Preserved; `loop` is a Level B statement keyword, so only the identifier changes. |
| Implicit conversions in comparisons and arithmetic | Local `as .decimal` / `as .string` conversions in the same clauses | 28 inner-loop bodies; 27 stem multiplications | Clause counts and value effects preserved; typed conversion work is necessarily explicit. |
| `avar.=1.0''loop` | `avar. = "1.0" \|\| lvar` | 14 | Preserved, including the Classic operation that replaces existing tail values and establishes the new default. `.stem` implements this reset in constant time using generations. |
| `when flag` | `when flag <> "0"` | 28 predicate checks / 27 true paths | Preserved truth test; Level B requires a Boolean expression. |
| `trace value trace()` | `trace value _trace_current_mode()` | 14 | Preserved query/set clause; uses the Level B trace-state surface. |
| `address value address()` | `address value addressenv().environment_name()` | 14 | Preserved query/set clause; uses the Level B ADDRESS environment object. |
| Two-argument call plus `parse upper arg a1 a2 a3 ., a4` | Four typed call arguments plus one typed `arg` clause | 14 calls / 14 argument clauses | Clause count and resulting argument values preserved. Argument splitting and uppercasing are expressed at the typed call boundary, so internal work is not instruction-identical. |
| Four assignments to `rc` | Four assignments to `text` | 56 | Preserved; Level B reserves typed `RC` for host-command status. |
| Failure-path `say` clauses | The same `say` clauses | 0 on a valid run | Preserved; CTest treats any `Failed...` output as a test failure. |

**Net timed source-clause change: zero.** The score therefore retains the
upstream numerator of exactly 1,000 nominal RexxCPS clauses per outer
iteration; it is not an estimate derived from cREXX instructions or trace-line
counts.

This table makes the comparability decision reviewable: rows marked preserved
have the same dynamic source-clause count and intended result; the typed
argument and conversion rows are disclosed differences. Cross-implementation
scores are therefore meaningful community comparisons of the recognizable
RexxCPS workload, but not proof that the implementations perform identical
lower-level work per clause.

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
