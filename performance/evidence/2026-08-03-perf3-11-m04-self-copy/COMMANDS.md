# PERF3-11 M04 validation commands

Commands were run from '/Users/adrian/CLionProjects/CREXX' on macOS ARM64
('Darwin 25.5.0', Apple M5), using Apple Clang 21.0.0, CMake 4.3.2 and Ninja
1.13.2. Verbose output was redirected to temporary logs.

## Focused correctness

    /usr/bin/cc -Icmake-build-debug/generated -Iassembler \
      -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
      -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
      -Wsign-conversion -fsyntax-only \
      assembler/rxas_flow.c assembler/rxas_flow_proof.c \
      assembler/rxas_flow_ssa.c binutils/rxopmeta.c

    cmake --build cmake-build-debug --target \
      rxas rxdas test_rxop_metadata test_rxas_flow_graph --parallel 10

    ctest --test-dir cmake-build-debug --output-on-failure \
      -R '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_redundant_self_copy_flow|rxas_optimizer_redundant_self_copy_flow_noopt|rxas_optimizer_whole_procedure_panel|rxas_optimizer_whole_procedure_panel_noopt)$'

## Ordinary Release gate

    cp cmake-build-release/bin/rxas FROZEN_M03_RXAS
    shasum -a 256 FROZEN_M03_RXAS

    cmake --build cmake-build-release --target \
      rxas rxdas test_rxop_metadata test_rxas_flow_graph --parallel 10

    ctest --test-dir cmake-build-release --output-on-failure \
      -R '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_redundant_self_copy_flow|rxas_optimizer_redundant_self_copy_flow_noopt|rxas_optimizer_whole_procedure_panel|rxas_optimizer_whole_procedure_panel_noopt)$'

The frozen and current assemblers consumed the exact same absolute input path
for the focused fixture and each canonical input:

    FROZEN_M03_RXAS -o OLD EXACT_INPUT
    cmake-build-release/bin/rxas -o NEW EXACT_INPUT
    cmp OLD.rxbin NEW.rxbin
    shasum -a 256 OLD.rxbin NEW.rxbin

The non-formal scale screen used three serial paired rounds per canonical
input:

    /usr/bin/time -lp FROZEN_M03_RXAS -o OLD EXACT_INPUT
    /usr/bin/time -lp cmake-build-release/bin/rxas -o NEW EXACT_INPUT

## Broad closeout

    cmake --build cmake-build-debug --parallel 10
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    git diff --check
