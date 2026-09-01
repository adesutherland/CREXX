# PERF3-11 M06 validation commands

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
      -R '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_whole_procedure_flow|rxas_optimizer_whole_procedure_flow_noopt|rxas_optimizer_whole_procedure_panel|rxas_optimizer_whole_procedure_panel_noopt|rxas_optimizer_nr18_flow_harvest|rxas_optimizer_nr18_flow_harvest_noopt)$'

The same target build and eight-test expression were run against
`cmake-build-release`.

## Frozen/current output and Release gate

    shasum -a 256 /tmp/perf3-m06-baseline.XgOb84/rxas-m05

For each focused or canonical input, both assemblers consumed the same absolute
path and their output was compared directly:

    FROZEN_M05_RXAS -o OLD EXACT_ABSOLUTE_INPUT
    cmake-build-release/bin/rxas -o NEW EXACT_ABSOLUTE_INPUT
    cmp OLD.rxbin NEW.rxbin
    shasum -a 256 OLD.rxbin NEW.rxbin

Three serial paired RexxCPS assembly rounds used `/usr/bin/time -lp`, with old
and new order balanced across the rounds.

## Broad closeout

    cmake --build cmake-build-debug --parallel 10
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    ctest --test-dir cmake-build-debug --output-on-failure \
      -R '^rxas_optimizer_nr09_class1$'
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    git diff --check
