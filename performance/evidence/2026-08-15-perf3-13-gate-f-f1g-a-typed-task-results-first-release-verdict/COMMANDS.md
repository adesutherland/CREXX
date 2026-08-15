# F1g-A qualification commands

The retained logs are the command-output authority. Principal commands were
run from the repository root on `develop`.

```sh
cmake --build cmake-build-debug \
  --target rxc rxas rxlink rxbvm rxtvm classlib test_rxgraph \
  gate_f_task_method_import_matrix --parallel 10
ctest --test-dir cmake-build-debug --output-on-failure --parallel 10 \
  -R '^(rxgraph_unit|rxlink_format_check|gate_f_.*task.*|gate_f_.*result.*)$'

cmake --build cmake-build-release \
  --target rxc rxas rxlink rxbvm rxtvm rxbvml classlib --parallel 10
cmake --build cmake-build-release \
  --target test_gate_f_channel_benchmark_bin --parallel 10
ctest --test-dir cmake-build-release -L gate_f \
  --output-on-failure --parallel 10

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*)$'

# The control directory was reconstructed by compiling rxvmexecutor.c and
# rxbin007.c from a5a99be15 with the recorded Release command lines and linking
# them with the otherwise unchanged current Release objects.
GF_CONTROL_DIR=/tmp/crexx-gate-f-f1g-control.yXl0ee \
GF_ACCEPTED_IMAGE=/Users/adrian/CLionProjects/CREXX/cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
GF_CANDIDATE_IMAGE=/Users/adrian/CLionProjects/CREXX/cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
performance/evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/capture-task-cache-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-a-typed-task-results-first-release-verdict/timing/incremental

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-a-typed-task-results-first-release-verdict/timing/incremental/summary.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-a-typed-task-results-first-release-verdict/timing/incremental/samples.csv
```
