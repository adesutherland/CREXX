# F1e qualification commands

The retained logs are the command output authority. The principal commands
were run from the repository root.

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^(rxvmchannel_process_crash_replacement|gate_f_channel_process_(rxbvm|rxtvm)|gate_f_channel_local_(rxbvm|rxtvm)|program_generation_control-(rxvml|rxbvml|rxtvml)|testConcurrency_(noopt|opt))$' \
  --repeat until-fail:100

ctest --test-dir cmake-build-debug --parallel 3 --output-on-failure \
  -R '^(rxvmchannel_process_crash_replacement|gate_f_channel_process_(rxbvm|rxtvm))$' \
  --repeat until-fail:500

# Repeated unchanged after adding running-sibling fail-fast coverage.
ctest --test-dir cmake-build-debug --parallel 3 --output-on-failure \
  -R '^(rxvmchannel_process_crash_replacement|gate_f_channel_process_(rxbvm|rxtvm))$' \
  --repeat until-fail:100

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*)$'

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --output-on-failure --parallel 1 \
  -R '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*)$'

GF_CONTROL_DIR=/tmp/crexx-f1e-control.gsJewy \
performance/evidence/2026-08-15-perf3-13-gate-f-f1e-first-release-verdict/capture-channel-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1e-first-release-verdict/timing/initial

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1e-first-release-verdict/timing/initial/summary.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1e-first-release-verdict/timing/initial/samples.csv
```

The same frozen binaries and commands produced `timing/confirmation`. The
`pre-archive-fix`, `pre-sigpipe-fix` and `pre-failfast` directories and QA logs
are retained as diagnostic evidence, not as the final verdict.
