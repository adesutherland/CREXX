# F3C1 qualification commands

The retained logs are the command-output authority. Principal commands were
run from the repository root on `develop`.

```sh
cmake --build cmake-build-debug \
  --target test_rxgraph rxbvm rxtvm test_gate_f_channel_benchmark_bin \
  test_gate_f_process_crash_replacement test_program_generation_control \
  --parallel 10
ctest --test-dir cmake-build-debug --output-on-failure --parallel 10 \
  -R '^(rxgraph_unit|gate_f_channel_contract|gate_f_local_.*|gate_f_process_crash_replacement|gate_f_process_.*|program_generation_control-.*)$'

cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release -L gate_f \
  --output-on-failure --parallel 10

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*)$'

GF_CONTROL_DIR=/tmp/crexx-gate-f-cache-control-7108a9c5f \
GF_ACCEPTED_IMAGE=cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
GF_CANDIDATE_IMAGE=cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
performance/evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/capture-task-cache-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/timing/initial

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/timing/initial/summary.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/timing/initial/samples.csv
```
