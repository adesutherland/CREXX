# Gate E E1 retained commands

All commands ran from `/Users/adrian/CLionProjects/CREXX` on the cleared Darwin
arm64 host. Large build and CTest streams were redirected to the retained logs
in this evidence directory.

## Focused Debug and Release

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|rxvm_product_entry_point|rxvmmemory_allocator|rxvmworker_lifecycle|rxassignalstests|rxasbasictests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|dynamic_interface_load_driver|ts_address_crexx_noopt|ts_address_crexx_opt|crexx_headerless_simple_smoke)$'

cmake --build cmake-build-release \
  --target rxvm rxbvm rxtvm rxvml test_rxvmworker test_reentrancy \
  --parallel 10

ctest --test-dir cmake-build-release --output-on-failure \
  -R '^(rxvm_product_entry_point|rxvmworker_lifecycle|rxasbasictests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check)$'
```

## First Release verdict

The control was built from an exact `git archive` export of
`19802842e0655b2f2ae011f911e909a2ded7233b`, outside the repository and without
creating another branch or worktree. Both products were ordinary profiling-off
Release builds.

```sh
/private/tmp/crexx-rxvm-inline.yvLywZ/builds/baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest core-four-manifest.txt \
  --output-dir timing \
  --measurement timing \
  --warmups 1 \
  --runs 12
```

The exact absolute control/candidate commands and retained RXBIN inputs are in
`core-four-manifest.txt`; raw samples and output checks are under `timing/`.

## AddressSanitizer

The existing ASan tree was refreshed because it predated the new target:

```sh
cmake -S . -B cmake-build-debugasan
```

The first runner build retained the platform LSan rejection. Repeating with
only leak detection disabled built the E1 targets before the broad linked
fixture reached the separately classified RXAS use-after-free:

```sh
tools/asan-run.sh --phase build \
  --build-target test_rxvmworker \
  --build-target test_rxvmmemory \
  --build-target test_reentrancy \
  --build-target linked_opt_runtime_artifacts \
  --build-target crexx \
  --build-jobs 10 \
  --build-leaks off

tools/asan-run.sh --phase ctest \
  --regex '^(rxvmworker_lifecycle|rxvmmemory_allocator|reentrancy_check)$' \
  --leaks off \
  --test-jobs 1
```

## Full closeout

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release \
  --target linked_opt_runtime_artifacts test_rxvmworker test_rxvmmemory \
           test_reentrancy test_db_decimal_archive_link_bin \
           rxbvm rxtvm rxvml \
  --parallel 10

ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -R '^(linked_opt_runtime_artifacts_build|rxvm_product_entry_point|rxvmmemory_allocator|rxvmworker_lifecycle|rxassignalstests|rxasbasictests|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|db_decimal_archive_link|dynamic_interface_load_driver|ts_address_crexx_noopt|ts_address_crexx_opt|crexx_headerless_simple_smoke)$'
```
