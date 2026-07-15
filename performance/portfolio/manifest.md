# Seed performance portfolio manifest

Manifest version: 1

Status: active seed, not yet an approved Tier A/Tier B Release 1 portfolio

This manifest versions the initial correctness-gated language workloads used by
NR-01. Source, license and detailed provenance remain beside the programs in
`tests/benchmarks/README.md` and `tests/benchmarks/THIRD_PARTY_NOTICES.md`.

## Measurement contract

- Build with CMake `Release`; record the actual compiler and flags.
- Build the `language_benchmarks` target and pass the `benchmark` CTest smoke
  gate before formal timing.
- Execute recorded samples serially. Default formal seed runs use two
  unrecorded process warmups and ten recorded runs per workload.
- Retain every warmup and recorded sample with `--samples-output`; summary
  min/median/mean/max values are secondary derived evidence.
- For a self-reporting benchmark such as RexxCPS, retain its native metric and
  unit separately. Do not infer clauses per second from startup-inclusive
  process elapsed time.
- The current runner's `process` lifecycle starts a new VM and loads the
  workload plus `library.rxbin` for every sample. It includes process/module
  startup and is not a steady-state kernel measurement.
- The runner executes separate workload and library images, so source/TRACE
  metadata is retained. A stripped linked image is a different mode and must
  have a separate manifest entry/evidence series.
- Every child must exit successfully and emit its deterministic `PASS:` line.
  Timing without correctness is invalid.
- Report VM (`rxvm`/`rxbvm`), bytecode mode (`opt`/`noopt`), arguments, commit,
  dirty state, host/toolchain/build provenance and raw samples.

## Workloads

| ID | Source and built target | Lineage / pressure | Formal seed arguments | Correctness signal |
| --- | --- | --- | --- | --- |
| `sieve` | `tests/benchmarks/awfy_sieve.crexx`; `benchmark_awfy_sieve_{opt,noopt}.rxbin` | Are We Fast Yet?/SOM; integer arrays, indexing and nested loops | `50` | `PASS: AWFY Sieve` |
| `permute` | `tests/benchmarks/awfy_permute.crexx`; `benchmark_awfy_permute_{opt,noopt}.rxbin` | Are We Fast Yet?/SOM; recursion, method calls and array mutation | `50` | `PASS: AWFY Permute` |
| `mandelbrot` | `tests/benchmarks/awfy_mandelbrot.crexx`; `benchmark_awfy_mandelbrot_{opt,noopt}.rxbin` | AWFY/Benchmarks Game; floating point, branches and bit operations | `500` | `PASS: AWFY Mandelbrot` |
| `towers` | `tests/benchmarks/awfy_towers.crexx`; `benchmark_awfy_towers_{opt,noopt}.rxbin` | Are We Fast Yet?/SOM; allocation, attributes and recursion | `10` | `PASS: AWFY Towers` |
| `rexxcps` | `tests/benchmarks/rexxcps_levelb.crexx`; `benchmark_rexxcps_levelb_{opt,noopt}.rxbin` | RexxCPS 2.2c for cREXX; Classic clause mix through Level B surfaces | canonical defaults (`100 x 100` and self-calibration) | `PASS: RexxCPS 2.2c cREXX port` |

## Runner contract

The versioned runner is `tests/benchmarks/run_benchmarks.crexx`:

```sh
cmake-build-release/bin/crexx --nokeep \
  tests/benchmarks/run_benchmarks.crexx --args \
  --build-dir cmake-build-release --vm rxvm --mode opt \
  --warmups 2 --runs 10 \
  --output summary.csv --samples-output samples.csv
```

`summary.csv` contains one aggregate row per workload. `samples.csv` preserves
one row per warmup and recorded run, in execution order, with lifecycle and
phase labels. The two files must come from the same runner invocation.

## Known coverage gaps

This seed is intentionally incomplete. NR-01 remains in progress until the
programme selects broader Classic Rexx, parser/text, calls/recursion,
allocation/object, binary, collections, startup and application workloads and
provides a distinct steady-state measurement mode. Tier assignment and
portfolio aggregation policy belong to NR-11. Cross-runtime coverage, port
status and the proposed portfolio-size range are maintained in
[`cross-runtime-plan.md`](cross-runtime-plan.md).
