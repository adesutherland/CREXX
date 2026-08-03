# PERF3-11 K01 validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`. Verbose assembler,
build and broad-test output was redirected to temporary logs and only focused
slices were inspected.

## Syntax, unit and focused correctness

    /usr/bin/cc -Icmake-build-debug/generated -Iassembler \
      -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
      -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
      -Wsign-conversion -fsyntax-only \
      assembler/rxas_flow.c assembler/rxas_flow_analysis.c \
      assembler/rxas_flow_graph.c assembler/rxas_flow_proof.c \
      assembler/rxas_flow_signal.c assembler/rxas_flow_ssa.c \
      assembler/rxas_flow_use.c binutils/rxopmeta.c

    cmake --build BUILD --target \
      rxas rxdas test_rxas_flow_graph --parallel 10

    BUILD/tests/test_rxas_flow_graph

    ctest --test-dir BUILD --output-on-failure \
      -R 'rxas_optimizer_(swap_round_trip_flow|storage_identity_flow|implicit_inc0_(relevant|unrelated)|loadint_(relevant|unrelated)|linkarg_(relevant|unrelated))'

The build/unit/focused sequence was run against `cmake-build-debug` and
`cmake-build-release`. The optimizer expression is nine tests including the
new optimized/no-opt K01 panel.

## Shared proof-consumer regression

    ctest --test-dir BUILD --parallel 10 --output-on-failure \
      -R '^(rxas_optimizer_(nr09_class1|whole_procedure_flow|whole_procedure_panel|nr18_flow_harvest|compare_branch|duplicate_link.*|duplicate_linkattr.*|redundant_self_copy_flow|storage_identity_flow|swap_round_trip_flow.*|implicit_inc0_(relevant|unrelated)|loadint_(relevant|unrelated)|linkarg_(relevant|unrelated))|rxas_flow_graph_contract)$'

This 37-test expression was run in Debug and ordinary profiling-off Release.

## Frozen focused and canonical outputs

The committed `45e027685` Release assembler and current Release assembler
consumed the same unchanged focused inputs and the retained same/reversed
orientation source.

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
