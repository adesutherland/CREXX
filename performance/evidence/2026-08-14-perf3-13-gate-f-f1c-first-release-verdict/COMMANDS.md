# Gate F F1c verdict and closeout commands

The timing directories contain the complete raw process transcript and sample
CSV for both unchanged runs. Each used one warmup and twelve balanced recorded
rounds across both concrete VMs, the F1c channel benchmark, and the retained
persistent-executor controls. `timing/combined-paired-summary.txt` is the
24-pair aggregation.

Focused Debug full-toolchain closure:

```sh
ctest --test-dir cmake-build-debug --parallel 5 --output-on-failure \
  -R '^(gate_f_channel_contract|gate_f_levelb_contract|rxvmchannel_provider_registry|gate_f_channel_local_(rxbvm|rxtvm)|persistent_worker_executor-(rxvml|rxbvml|rxtvml)|inline_test_binary_class_attr_assign_run_(noopt|opt)|gate_f_signal_names_run_(noopt|opt)|ts_signal_(noopt|opt)|gate_f_levelb_bridge_inspection|testConcurrency_(noopt|opt))$'
```

The exact expanded 18-test selection and results are retained in
`qa/focused-debug.log`.

Final structural inliner regression:

```sh
ctest --test-dir cmake-build-debug --parallel 5 --output-on-failure \
  -R '^(inline_test_binary_class_attr_assign_(shape|run_noopt|run_opt)|object_reference_regression_(opt|run_noopt|run_opt)|inline_test_block_expr_live_sibling_(opt|run_noopt|run_opt))$'
```

Full Debug closeout:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

Apple ASan closeout through the repository runner:

```sh
tools/asan-run.sh --phase build --build-leaks off --build-jobs 10 \
  --build-target all

tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_channel_contract|gate_f_levelb_contract|rxvmchannel_provider_registry|gate_f_channel_local_(rxbvm|rxtvm)|persistent_worker_executor-(rxbvml|rxtvml)(-doorbell-stress|-sparse-owner|-sparse-owner-stress|-sparse-progress|-native-return)?|rxvmmemory_allocator|rxvmworker_lifecycle|rxvmactive_isolation|inline_test_binary_class_attr_assign_(shape|run_noopt|run_opt)|object_reference_regression_(opt|run_noopt|run_opt)|inline_test_block_expr_live_sibling_(opt|run_noopt|run_opt)|gate_f_signal_names_run_(noopt|opt)|ts_signal_(noopt|opt)|gate_f_levelb_bridge_inspection|testConcurrency_(noopt|opt))$'
```

The exact expanded 36-test command and environment are retained in
`qa/asan-ctest.log`. `qa/asan-leak-detection-unsupported.log` is the preceding
leak-enabled platform proof.

Final ordinary profiling-off Release build and focused matrix:

```sh
cmake --build cmake-build-release --parallel 10

ctest --test-dir cmake-build-release --parallel 5 --output-on-failure \
  -R '^(gate_f_channel_contract|gate_f_levelb_contract|rxvmchannel_provider_registry|gate_f_channel_local_rxbvm|gate_f_channel_local_rxtvm|persistent_worker_executor-(rxbvml|rxtvml)|inline_test_binary_class_attr_assign_(shape|run_noopt|run_opt)|object_reference_regression_opt|inline_test_block_expr_live_sibling_opt|gate_f_signal_names_run_(noopt|opt)|ts_signal_(noopt|opt)|gate_f_levelb_bridge_inspection|testConcurrency_(noopt|opt))$'
```

Repeated lifecycle/provider/channel checks, on both Debug and Release:

```sh
ctest --test-dir cmake-build-debug --parallel 4 --repeat until-fail:100 \
  --output-on-failure \
  -R '^(rxvmchannel_provider_registry|gate_f_channel_local_(rxbvm|rxtvm)|testConcurrency_(noopt|opt))$'

ctest --test-dir cmake-build-release --parallel 4 --repeat until-fail:100 \
  --output-on-failure \
  -R '^(rxvmchannel_provider_registry|gate_f_channel_local_(rxbvm|rxtvm)|testConcurrency_(noopt|opt))$'
```
