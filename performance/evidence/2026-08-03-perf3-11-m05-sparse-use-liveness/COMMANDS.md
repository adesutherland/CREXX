# PERF3-11 M05 validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` on macOS ARM64
(`Darwin 25.5.0`, Apple M5, 10 logical CPUs), using Apple Clang 21.0.0,
CMake 4.3.2 and Ninja 1.13.2. Verbose output was redirected to temporary logs.

## Focused correctness

    /usr/bin/cc -Icmake-build-debug/generated -Iassembler \
      -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
      -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
      -Wsign-conversion -fsyntax-only \
      assembler/rxas_flow.c assembler/rxas_flow_analysis.c \
      assembler/rxas_flow_proof.c assembler/rxas_flow_ssa.c \
      assembler/rxas_flow_use.c binutils/rxopmeta.c

    cmake --build cmake-build-debug --target \
      rxas test_rxop_metadata test_rxas_flow_graph --parallel 10

    ctest --test-dir cmake-build-debug --output-on-failure \
      -R '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_whole_procedure_flow|rxas_optimizer_whole_procedure_flow_noopt|rxas_optimizer_whole_procedure_panel|rxas_optimizer_whole_procedure_panel_noopt)$'

## Ordinary Release gate

    shasum -a 256 /tmp/perf3-m05-baseline.YawcI3/rxas-m04

    cmake --build cmake-build-release --target \
      rxas rxdas test_rxop_metadata test_rxas_flow_graph --parallel 10

    ctest --test-dir cmake-build-release --output-on-failure \
      -R '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_whole_procedure_flow|rxas_optimizer_whole_procedure_flow_noopt|rxas_optimizer_whole_procedure_panel|rxas_optimizer_whole_procedure_panel_noopt)$'

The frozen and current assemblers consumed the same absolute input path for
each focused or canonical comparison:

    FROZEN_M04_RXAS -o OLD EXACT_ABSOLUTE_INPUT
    cmake-build-release/bin/rxas -o NEW EXACT_ABSOLUTE_INPUT
    cmp OLD.rxbin NEW.rxbin
    shasum -a 256 OLD.rxbin NEW.rxbin

The non-formal scale screen used three serial paired rounds per canonical
input, alternating tool order:

    /usr/bin/time -lp FROZEN_M04_RXAS -o OLD EXACT_ABSOLUTE_INPUT
    /usr/bin/time -lp cmake-build-release/bin/rxas -o NEW EXACT_ABSOLUTE_INPUT

The final diagnostic scale/counter screen was redirected before extracting the
bounded summary rows:

    /usr/bin/time -lp cmake-build-release/bin/rxas -d \
      -o DIAGNOSTIC_REXXCPS \
      /Users/adrian/CLionProjects/CREXX/cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas \
      >DIAGNOSTIC.stdout 2>DIAGNOSTIC.log

## Broad closeout

    cmake --build cmake-build-debug --parallel 10
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    git diff --check
