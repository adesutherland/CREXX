# PERF3-11 M01 validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.  Verbose build,
assembly and test output was redirected to temporary logs.

```sh
/usr/bin/cc -Icmake-build-debug/generated -Iassembler \
  -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
  -Ibinutils/include -std=gnu90 -Wall -Wextra -Wconversion \
  -Wsign-conversion -fsyntax-only assembler/rxas_flow.c

cmake --build cmake-build-debug --target \
  rxas rxvm rxbvm test_rxop_metadata test_rxas_flow_graph \
  mc_decimal_full_tests db_decimal_tests signal_contract_runtime --parallel 10

cmake-build-debug/tests/test_rxop_metadata
cmake-build-debug/tests/test_rxas_flow_graph
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  -R '^rxas_optimizer_'

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

The Release build and direct dual-dispatch semantic matrix used the ordinary
profiling-off product:

```sh
cmake --build cmake-build-release --target \
  rxas rxvm rxbvm test_rxop_metadata mc_decimal_full_tests db_decimal_tests \
  signal_contract_runtime --parallel 10

cd cmake-build-release/tests
../bin/rxvm signal_contract_runtime_rxvm_noopt
cmake -P signal_contract_runtime_rxvm_opt_linked_runtime.cmake
../bin/rxbvm signal_contract_runtime_rxbvm_noopt
cmake -P signal_contract_runtime_rxbvm_opt_linked_runtime.cmake
```

Canonical output and scale used the exact retained Stage 0 inputs and frozen
Stage 6 assembler:

```sh
/tmp/perf3-stage10-legacy-baseline.gJS4Fg/rxas-stage6 -o OLD EXACT_INPUT
cmake-build-release/bin/rxas -d -o NEW EXACT_INPUT
/usr/bin/time -l cmake-build-release/bin/rxas -d -o NEW EXACT_REXXCPS_INPUT
```
