# PERF3-11 K02/K03 validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`. Verbose assembler,
build and broad-test output was redirected to temporary logs and only focused
slices were inspected.

## Syntax and focused correctness

    /usr/bin/cc -Icmake-build-debug/generated -Iassembler \
      -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
      -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
      -Wsign-conversion -fsyntax-only \
      assembler/rxas_flow.c assembler/rxas_flow_analysis.c \
      assembler/rxas_flow_graph.c assembler/rxas_flow_proof.c \
      assembler/rxas_flow_signal.c assembler/rxas_flow_ssa.c \
      assembler/rxas_flow_use.c binutils/rxopmeta.c

    cmake --build BUILD --target \
      rxas test_rxas_flow_graph test_rxop_metadata --parallel 10

    BUILD/tests/test_rxas_flow_graph
    BUILD/tests/test_rxop_metadata

    ctest --test-dir BUILD --output-on-failure \
      -R '^(rxas_optimizer_(nr09_class1|whole_procedure_flow|whole_procedure_panel|nr18_flow_harvest|compare_branch|duplicate_link.*|duplicate_linkattr.*|redundant_self_copy_flow)|rxas_flow_graph_contract)$'

The build/executable/test sequence was run against `cmake-build-debug` and
`cmake-build-release`. The K02/K03-only subset is 21 tests; the combined
flow/K02/K03/K04/M04/M05/M06 expression is 28 tests.

## Frozen focused and canonical output

The frozen K04 and current Release assemblers consumed the same unchanged
input path including the `.rxas` suffix. Focused images were assembled with
the relative `tests/rxas_optimizer/NAME.rxas` path used for the frozen audit.

    RXAS -d -o OUTPUT EXACT_INPUT.rxas >STDOUT 2>STDERR
    shasum -a 256 OUTPUT.rxbin
    cmp FROZEN.rxbin CURRENT.rxbin

The canonical inputs were:

    cmake-build-release/tests/benchmarks/benchmark_awfy_richards_opt.rxas
    cmake-build-release/tests/benchmarks/benchmark_awfy_towers_opt.rxas
    cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas

## Broad closeout

    cmake --build cmake-build-debug --parallel 10
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    git diff --check
