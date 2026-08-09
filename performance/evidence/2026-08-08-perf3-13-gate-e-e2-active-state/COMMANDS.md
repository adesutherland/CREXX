# E2 retained commands

All commands ran from `/Users/adrian/CLionProjects/CREXX` on Adrian's cleared
Mac host. Large output was redirected while running; compact results are
retained here.

First-verdict timing used the retained manifest and exact exported E1-P1
control:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest sieve-rexxcps-manifest.txt \
  --output-dir timing \
  --measurement timing \
  --warmups 1 \
  --runs 12
```
RXAS sanitizer reproduction and repair checks:

```sh
cmake --build cmake-build-debug \
  --target test_rxas_flow_graph nr15_stem_semantics_rxvm_opt_artifact \
  --parallel 8
cmake-build-debug/tests/test_rxas_flow_graph

tools/asan-run.sh --phase build \
  --build-target test_rxas_flow_graph \
  --build-target nr15_stem_semantics_rxvm_opt_artifact \
  --build-jobs 4 --build-leaks off --no-live-tail
tools/asan-run.sh --phase ctest --regex '^rxas_flow_graph_contract$' \
  --leaks off --test-jobs 1 --no-live-tail

tools/asan-run.sh --phase build \
  --build-target linked_opt_runtime_artifacts \
  --build-jobs 4 --build-leaks off --no-live-tail
```

The no-malloc performance diagnostic ran two balanced blocks in each order;
each block assembled the NR15 input ten times with `/usr/bin/time -p`. The
candidate and discarded malloc-control RXBIN were compared with `shasum -a 256`.

Focused E2 sanitizer with its required fixture included:

```sh
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 --no-live-tail \
  --regex '^(e2_active_context_isolation|rxvmactive_isolation|rxvmworker_lifecycle|rxvmmemory_allocator|rxpa_utf_validation|reentrancy_check|rxvml_utf_boundaries|address_callback_host|dynamic_interface_load_driver|ts_address_(crexx|capture|argv|array_lifecycle)_(noopt|opt)|ts_address_spawn_transfer(_rxbvm|_rxtvm)?_(noopt|opt)|rxpa_(dynlink|staticlink)_run_(noopt|opt)|rxpa_signal_funcs_(dynamic|static)_(noopt|opt)|crexx_(nocompile_exec|native_nocompile|native_address_crexx)_smoke)$'
```

Complete supported sanitizer and ordinary Debug closeout:

```sh
tools/asan-run.sh --phase ctest --leaks off --test-jobs 8 --no-live-tail
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

Final ordinary profiling-off Release closeout:

```sh
cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --output-on-failure --parallel 10 \
  -R '^(rxpa_(dynlink|staticlink)_(compile|run)_(noopt|opt)|rxpa_utf_validation|ts_address_spawn_transfer(_rxbvm|_rxtvm)?_(noopt|opt)|ts_address_(argv|array_lifecycle|capture|crexx)_(noopt|opt)|ts_address_(argv|array_lifecycle|capture)_rxbvm_(noopt|opt)|address_callback_host|dynamic_interface_load_rxvm|rxpa_signal_funcs_(dynamic|static)_(noopt|opt)|rxvml_utf_boundaries|reentrancy_check|rxvm_product_entry_point|rxvmworker_lifecycle|rxvmmemory_allocator|db_decimal_archive_link|e2_active_context_isolation|dynamic_interface_load_driver|rxvmactive_isolation|crexx_(native_address_crexx|native_nocompile|nocompile_exec)_smoke|rxpa_external_sdk_consumer)$'
```
