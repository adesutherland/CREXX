# Gate F F1d verdict and closeout commands

Final focused Debug full-toolchain matrix:

```sh
ctest --test-dir cmake-build-debug --output-on-failure --parallel 1 \
  -R '^(gate_f_channel_contract|gate_f_levelb_contract|gate_f_levelb_bridge_inspection|rxvmchannel_provider_registry|rxvmbyteendpoint_substrate|rxvmchannel_byte_provider|gate_f_channel_local_(rxbvm|rxtvm)|retired_process_opcode_(466|467|468|469|470|471)_(rxbvm|rxtvm)|inline_nested_binary_block_owner_(shape_opt|run_noopt|run_opt)|testConcurrency_(noopt|opt)|address_crexx_flush_visible_run_(noopt|opt)|address_exit_extended_parse|address_inline_then_parse|address_bridge|address_expose_(noopt|opt)|test_address_direct_(noopt|opt)|ts_address_(protocol|crexx|capture|argv|array_lifecycle|spawn_transfer)_(noopt|opt)|ts_address_(capture|argv|array_lifecycle|spawn_transfer)_rxbvm_(noopt|opt)|ts_address_spawn_transfer_rxtvm_(noopt|opt)|rxvmworker_lifecycle|rxvmactive_isolation)$'
```

Complete Debug closeout:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

The final complete run contains 2,112 tests. The earlier nine-test endpoint,
provider and ADDRESS stress selection used `--repeat until-fail:100` and is
retained in `qa/debug-stress-repeat100.log`.

Apple ASan closeout through the repository runner:

```sh
tools/asan-run.sh --phase build --build-leaks off --build-jobs 10 \
  --build-target all

tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_channel_contract|gate_f_levelb_contract|gate_f_levelb_bridge_inspection|rxvmchannel_provider_registry|rxvmbyteendpoint_substrate|rxvmchannel_byte_provider|gate_f_channel_local_(rxbvm|rxtvm)|retired_process_opcode_(466|467|468|469|470|471)_(rxbvm|rxtvm)|inline_nested_binary_block_owner_(shape_opt|run_noopt|run_opt)|testConcurrency_(noopt|opt)|address_crexx_flush_visible_run_(noopt|opt)|address_exit_extended_parse|address_inline_then_parse|address_bridge|address_expose_(noopt|opt)|test_address_direct_(noopt|opt)|ts_address_(protocol|crexx|capture|argv|array_lifecycle|spawn_transfer)_(noopt|opt)|ts_address_(capture|argv|array_lifecycle|spawn_transfer)_rxbvm_(noopt|opt)|ts_address_spawn_transfer_rxtvm_(noopt|opt)|rxvmworker_lifecycle|rxvmactive_isolation)$'
```

Final ordinary profiling-off Release build and focused matrix:

```sh
cmake --build cmake-build-release --parallel 10

ctest --test-dir cmake-build-release --output-on-failure --parallel 1 \
  -R '^(gate_f_channel_contract|gate_f_levelb_contract|gate_f_levelb_bridge_inspection|rxvmchannel_provider_registry|rxvmbyteendpoint_substrate|rxvmchannel_byte_provider|gate_f_channel_local_(rxbvm|rxtvm)|retired_process_opcode_(466|467|468|469|470|471)_(rxbvm|rxtvm)|inline_nested_binary_block_owner_(shape_opt|run_noopt|run_opt)|testConcurrency_(noopt|opt)|address_crexx_flush_visible_run_(noopt|opt)|address_exit_extended_parse|address_inline_then_parse|address_bridge|address_expose_(noopt|opt)|test_address_direct_(noopt|opt)|ts_address_(protocol|crexx|capture|argv|array_lifecycle|spawn_transfer)_(noopt|opt)|ts_address_(capture|argv|array_lifecycle|spawn_transfer)_rxbvm_(noopt|opt)|ts_address_spawn_transfer_rxtvm_(noopt|opt)|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-(rxvml|rxbvml|rxtvml))$'
```

Balanced first verdict and unchanged confirmation against the exact retained
F1c products:

```sh
GF_CONTROL_DIR=/tmp/crexx-f1d-accepted.WICztV zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/capture-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/timing/initial

GF_CONTROL_DIR=/tmp/crexx-f1d-accepted.WICztV zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/capture-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/timing/confirmation
```

The paired reducer is authored in cREXX Level B and was executed through the
complete product toolchain for each sample set:

```sh
cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1d-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args OUTPUT_SUMMARY SAMPLES
```
