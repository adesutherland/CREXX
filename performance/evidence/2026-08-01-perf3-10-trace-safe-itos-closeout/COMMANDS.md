# PERF3-10 reproduction commands

Run from `/Users/adrian/CLionProjects/CREXX`.

## Ordinary Release candidate

```sh
cmake -S . -B cmake-build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-release --parallel 10
```

The exact retained C0 and frozen C1 paths and hashes are recorded in
`timing/initial-manifest.txt` and `artifact-hashes.csv`.

## Accepted paired verdict

```sh
cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest /tmp/crexx-perf3-10-first-verdict-manifest.txt \
  --output-dir /tmp/crexx-perf3-10-first-verdict.WXxEhm \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest /tmp/crexx-perf3-10-rxvm-noise-append-manifest.txt \
  --output-dir /tmp/crexx-perf3-10-rxvm-append.ZXJEBQ \
  --measurement timing --warmups 0 --runs 10
```

Samples were serial within each rotated workload group.  No sample was
removed.  `timing/final-pairs.csv` and `paired-summary.csv` combine the initial
and declared append blocks.

## Equal-work diagnostic counts

```sh
cmake-build-profile/bin/rxvm --profile=counts \
  --profile-output /tmp/crexx-perf3-10-profile-equal200.b4R3eu/c0.csv \
  /tmp/crexx-perf3-10-baseline.pkyQyD/rexxcps.rxbin \
  /tmp/crexx-perf3-10-baseline.pkyQyD/library.rxbin \
  -a --smoke-count 200

cmake-build-profile/bin/rxvm --profile=counts \
  --profile-output /tmp/crexx-perf3-10-profile-equal200.b4R3eu/c1.csv \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  cmake-build-release/bin/library.rxbin \
  -a --smoke-count 200

awk -F, '$1=="instruction" { total += $5 } \
  $1=="instruction" && $2=="ITOS_REG" { itos = $5 } \
  END { print total, itos }' profile.csv
```

This is deliberately labelled counts-only and noncanonical.  Both retained
stdout files prove `effective_count=200`, `averaging=100`, `calibrated=0` and
the correctness marker.

## Static image proof

```sh
cmake-build-release/bin/rxdas -o c0.rxas \
  /tmp/crexx-perf3-10-baseline.pkyQyD/rexxcps.rxbin
cmake-build-release/bin/rxdas -o c1.rxas \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin
rg -c '^\\s*itos\\b' c0.rxas c1.rxas
```

## Closeout correctness

```sh
cmake --build cmake-build-debug --parallel 30

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  -R '^(test_trace_|rxas_optimizer_(metadata|whole_procedure_flow|whole_procedure_panel|storage_identity_flow|redundant_itos_flow)|storage_identity_runtime_|redundant_itos_runtime_|benchmark_rexxcps_levelb_(noopt|opt)$)'

ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^trace_stem_sugar$'

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```
