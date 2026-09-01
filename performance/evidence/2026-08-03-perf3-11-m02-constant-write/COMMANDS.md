# PERF3-11 M02 validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` on macOS ARM64
(`Darwin 25.5.0`, Apple M5), using Apple Clang 21.0.0, CMake 4.3.2 and Ninja
1.13.2.  Verbose output was redirected to temporary logs.

## Build and focused correctness

```sh
/usr/bin/cc -Icmake-build-debug/generated -Iassembler \
  -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
  -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
  -Wsign-conversion -fsyntax-only \
  assembler/rxas_flow.c assembler/rxas_flow_proof.c \
  assembler/rxas_flow_ssa.c binutils/rxopmeta.c

cmake --build cmake-build-debug --target \
  rxas test_rxop_metadata test_rxas_flow_graph --parallel 10
cmake-build-debug/tests/test_rxop_metadata
cmake-build-debug/tests/test_rxas_flow_graph

cmake -DRXAS="$PWD/cmake-build-debug/bin/rxas" \
  -DRXDAS="$PWD/cmake-build-debug/bin/rxdas" \
  -DSOURCE="$PWD/tests/rxas_optimizer/redundant_constant_flow.rxas" \
  -DWORKING_DIRECTORY=WORK -DCASE=redundant_constant_flow \
  -DRXAS_FLAGS=-d -P tests/rxas_optimizer/check_optimizer.cmake
cmake -DRXAS="$PWD/cmake-build-debug/bin/rxas" \
  -DRXDAS="$PWD/cmake-build-debug/bin/rxdas" \
  -DSOURCE="$PWD/tests/rxas_optimizer/redundant_constant_flow.rxas" \
  -DWORKING_DIRECTORY=WORK -DCASE=redundant_constant_flow_noopt \
  -DRXAS_FLAGS=-n -P tests/rxas_optimizer/check_optimizer.cmake
```

## Ordinary Release gate

```sh
cmake --build cmake-build-release --target \
  rxas rxvm rxbvm test_rxop_metadata test_rxas_flow_graph \
  signal_contract_runtime --parallel 10

cmake-build-release/tests/test_rxop_metadata
cmake-build-release/tests/test_rxas_flow_graph

cd cmake-build-release/tests
../bin/rxvm signal_contract_runtime_rxvm_noopt
cmake -P signal_contract_runtime_rxvm_opt_linked_runtime.cmake
../bin/rxbvm signal_contract_runtime_rxbvm_noopt
cmake -P signal_contract_runtime_rxbvm_opt_linked_runtime.cmake
```

The focused old/new/no-opt replay used the final M02 source:

```sh
/tmp/perf3-m02-m01-baseline.0O7fNN/rxas-m01 -d \
  -o OLD tests/rxas_optimizer/redundant_constant_flow.rxas
cmake-build-release/bin/rxas -d \
  -o NEW tests/rxas_optimizer/redundant_constant_flow.rxas
cmake-build-release/bin/rxas -n \
  -o NOOPT tests/rxas_optimizer/redundant_constant_flow.rxas
cmake-build-release/bin/rxdas -o DISASSEMBLY IMAGE.rxbin
```

Each canonical comparison assembled the exact retained Stage 0 input with the
frozen M01 and current M02 tools, then used `cmp` and SHA-256:

```sh
/tmp/perf3-m02-m01-baseline.0O7fNN/rxas-m01 -o OLD EXACT_INPUT
cmake-build-release/bin/rxas -o NEW EXACT_INPUT
cmp OLD.rxbin NEW.rxbin
shasum -a 256 OLD.rxbin NEW.rxbin
```

## Scale and broad closeout

The scale rows were collected serially with `/usr/bin/time -l`, three recorded
samples per tool/mode and no warmup:

```sh
/usr/bin/time -l FROZEN_M01_RXAS -o OLD EXACT_REXXCPS_INPUT
/usr/bin/time -l M02_RXAS -o NEW EXACT_REXXCPS_INPUT
/usr/bin/time -l FROZEN_M01_RXAS -d -o OLD_DIAG EXACT_REXXCPS_INPUT
/usr/bin/time -l M02_RXAS -d -o NEW_DIAG EXACT_REXXCPS_INPUT
```

The accepted broad closeout command was:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```
