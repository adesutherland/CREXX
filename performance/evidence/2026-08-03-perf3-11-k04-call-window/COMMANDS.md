# PERF3-11 K04 call-window validation commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` on the ordinary
profiling-off macOS ARM64 Debug and Release build trees. Verbose output was
redirected to temporary logs.

## Focused correctness

    /usr/bin/cc -Icmake-build-debug/generated -Iassembler \
      -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
      -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
      -Wsign-conversion -fsyntax-only \
      assembler/rxas_flow.c assembler/rxas_flow_analysis.c \
      assembler/rxas_flow_proof.c assembler/rxas_flow_ssa.c \
      assembler/rxas_flow_use.c binutils/rxopmeta.c

    cmake --build cmake-build-debug --target rxas test_rxop_metadata --parallel 10
    cmake --build cmake-build-release --target rxas test_rxop_metadata --parallel 10

    ctest --test-dir BUILD --output-on-failure \
      -R '^(rxas_optimizer_metadata|rxas_optimizer_compare_branch|rxas_optimizer_compare_branch_noopt|rxas_optimizer_barrier_igtbr|rxas_optimizer_barrier_fgtbr)$'

The five-test expression was run against both `cmake-build-debug` and
`cmake-build-release`.

## Output-neutral and broad gates

    shasum -a 256 cmake-build-release/bin/rxas
    cmp K04B_CANONICAL.rxbin K04A_CANONICAL.rxbin
    cmake --build cmake-build-debug --parallel 10
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    git diff --check

Temporary K04c diagnostics printed the exact call bounds and first dependent
`ValueId` witness for the five canonical rejections. They were removed before
the final build, hash, focused and broad validation.

## K04d1 retirement and K04d2 focused gates

The following focused matrix was run against both `cmake-build-debug` and the
ordinary profiling-off `cmake-build-release` tree and passed 14/14 in each:

    cmake --build BUILD --target \
      rxas rxdas test_rxop_metadata test_rxas_flow_graph \
      run_tests_signal_retry_retired ts_signal \
      rxpa_native_unwind_funcs_dynamic \
      rxpa_native_unwind_funcs_static --parallel 10

    ctest --test-dir BUILD --output-on-failure \
      -R '^(rxassignalretryretiredtests(-rxbvm)?|ts_signal_(noopt|opt)|test_signal_retry_removed_diagnostic|rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_compare_branch(_noopt)?|rxpa_native_unwind_funcs_(dynamic|static)_(noopt|opt))$'

Strict GNU90 syntax was rerun over the changed RXAS flow and metadata sources;
it passed with the pre-existing unused `graph` parameter warning in
`rxas_flow.c`.

The frozen K04d3 candidate was assembled and disassembled with the ordinary
Release tools. Instruction lines were counted by their RXBIN-address comments
and TRACE records by `.traceevent` directives. It contains 1,222 VM
instructions and 1,249 TRACE events. The retained M06 runtime image contains
1,241 and 1,252 respectively. Formal timing was not started while
`avconferenced` was consuming roughly one CPU core.

## K04d3 first ordinary Release verdict

After the interfering process was stopped, the host was verified on AC with
low-power mode off. The four-cell manifest retained at
`timing/k04d3-manifest.txt` uses the same current Release VM and library for the
retained M06 and K04d images. The Level B runner was invoked as follows:

    caffeinate -i cmake-build-release/bin/crexx \
      performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
      --manifest MANIFEST --output-dir OUTPUT \
      --measurement timing --warmups 1 --runs 12

The runner uses serial, workload-rotated balanced sampling. Raw samples,
benchmark stdout, summary and capture metadata are retained under `timing/`.
All 52 executions pass. No sample is removed; the round-12 `rxvm` candidate
low sample remains in the evidence and triggers the runner's rerun flag.

## K04d4 closeout

After Adrian accepted the neutral verdict without an append:

    cmake --build cmake-build-debug --parallel 10
    ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
    git diff --check

The complete Debug build passes. Broad Debug passes 1,998/1,998 in 297.92
seconds and the diff check passes. A repository search confirms that no retry
enum, VM continuation, CFG edge, retry-only loop classification or public
factory remains in production code.
