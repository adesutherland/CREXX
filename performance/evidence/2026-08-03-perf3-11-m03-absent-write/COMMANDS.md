# PERF3-11 M03 validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` on macOS ARM64
(`Darwin 25.5.0`, Apple M5), using Apple Clang 21.0.0, CMake 4.3.2 and Ninja
1.13.2.  Verbose output was redirected to temporary logs.

## Focused correctness

```sh
/usr/bin/cc -Icmake-build-debug/generated -Iassembler \
  -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
  -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
  -Wsign-conversion -fsyntax-only \
  assembler/rxas_flow.c assembler/rxas_flow_proof.c \
  assembler/rxas_flow_ssa.c binutils/rxopmeta.c

cmake --build cmake-build-debug --target \
  rxas test_rxas_flow_graph --parallel 10
cmake-build-debug/tests/test_rxas_flow_graph

ctest --test-dir cmake-build-debug --output-on-failure \
  -R 'rxas_flow_graph_contract|rxas_optimizer_(whole_procedure_panel|redundant_constant_flow|redundant_absent_flow)'
```

## Ordinary Release gate

```sh
cp cmake-build-release/bin/rxas FROZEN_M02_RXAS
cmake --build cmake-build-release --target \
  rxas rxdas test_rxas_flow_graph --parallel 10
cmake-build-release/tests/test_rxas_flow_graph

FROZEN_M02_RXAS -d -o OLD \
  tests/rxas_optimizer/redundant_absent_flow.rxas
cmake-build-release/bin/rxas -d -o NEW \
  tests/rxas_optimizer/redundant_absent_flow.rxas
cmake-build-release/bin/rxas -n -o NOOPT \
  tests/rxas_optimizer/redundant_absent_flow.rxas
```

For Richards, Towers and RexxCPS, the frozen and current assemblers consumed
the exact retained Stage 0 input through the same absolute source pathname.
Each pair was checked with `cmp` and SHA-256.

## Broad closeout

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
git diff --check
```
