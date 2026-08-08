# E1-P1 retained commands

All commands ran from `/Users/adrian/CLionProjects/CREXX` on Adrian's cleared
Mac host. Large output was redirected while running and only compact results
are retained in this bundle.

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|rxvm_product_entry_point|rxvmmemory_allocator|rxvmworker_lifecycle|rxassignalstests|rxasbasictests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|dynamic_interface_load_driver|ts_address_crexx_noopt|ts_address_crexx_opt|crexx_headerless_simple_smoke)$'

cmake --build cmake-build-release \
  --target rxvm rxbvm rxtvm rxvml test_rxvmworker test_reentrancy \
  --parallel 10

ctest --test-dir cmake-build-release --output-on-failure \
  -R '^(rxvm_product_entry_point|rxvmworker_lifecycle|rxasbasictests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check)$'
```

The first verdict used an exact source export of `19802842e` and the retained
manifest in this directory:

```sh
/private/tmp/crexx-rxvm-inline.yvLywZ/builds/baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest core-four-manifest.txt \
  --output-dir timing \
  --measurement timing \
  --warmups 1 \
  --runs 20
```

Post-acceptance QA:

```sh
tools/asan-run.sh --phase build \
  --build-target test_rxvmworker \
  --build-target test_rxvmmemory \
  --build-target test_reentrancy \
  --build-jobs 10 \
  --build-leaks off

tools/asan-run.sh --phase ctest \
  --regex '^(rxvmworker_lifecycle|rxvmmemory_allocator|reentrancy_check)$' \
  --leaks off \
  --test-jobs 1

cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release \
  --target linked_opt_runtime_artifacts test_rxvmworker test_rxvmmemory \
           test_reentrancy test_db_decimal_archive_link_bin \
           rxbvm rxtvm rxvml \
  --parallel 10

ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|rxvm_product_entry_point|rxvmmemory_allocator|rxvmworker_lifecycle|rxassignalstests|rxasbasictests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|db_decimal_archive_link|dynamic_interface_load_driver|ts_address_crexx_noopt|ts_address_crexx_opt|crexx_headerless_simple_smoke)$'
```
