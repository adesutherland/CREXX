# PERF3-11 Stage 1 contract-lock commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

```sh
cmake --build cmake-build-debug --target \
  test_rxop_metadata rxas rxc rxvm rxbvm \
  mc_decimal_full_tests db_decimal_tests --parallel 10

cmake --build cmake-build-debug --target signal_contract_runtime --parallel 10

ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxassignal.*|rxas_optimizer_(metadata|barrier_signal|storage_identity_flow|redundant_itos_flow|redundant_itos_flow_noopt)|storage_identity_runtime_.*|redundant_itos_runtime_.*|signal_contract_runtime_.*|mc_decimal_full_tests|db_decimal_tests)$'

cmake --build cmake-build-release \
  --target rxas rxc rxlink rxvm rxbvm --parallel 10
cmake --build cmake-build-release --parallel 1 --target \
  tests/benchmarks/benchmark_awfy_richards_opt.rxbin \
  tests/benchmarks/benchmark_awfy_towers_opt.rxbin \
  tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin
shasum -a 256 \
  cmake-build-release/tests/benchmarks/benchmark_awfy_richards_opt.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_awfy_towers_opt.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin
```

The maintained `performance/tools/build_sequence_ledger.crexx` was compiled
and assembled with the Debug product, then run through `rxvm` with
`library.rxbin` and `StringTreeMap.rxbin` against the retained NR-09 manifest.
Its schema-2 run reported 650 effect and 650 signal rows and completed normally.
