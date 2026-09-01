# E3a retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX` unless a subshell changes
directory. Verbose build output was kept out of the terminal.

## Exact control and ordinary Release builds

```sh
git worktree add --detach \
  /private/tmp/crexx-perf3-13-e3a-control.WdO0Ib/source \
  6d12cd921cdbf9cb2098df2a4c8ae6eee75e4a7f

cmake -S /private/tmp/crexx-perf3-13-e3a-control.WdO0Ib/source \
  -B /private/tmp/crexx-perf3-13-e3a-control.WdO0Ib/source/build-release \
  -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build \
  /private/tmp/crexx-perf3-13-e3a-control.WdO0Ib/source/build-release \
  --target linked_opt_runtime_artifacts rxtvm rxbvm --parallel 10

cmake --build cmake-build-release \
  --target linked_opt_runtime_artifacts rxtvm rxbvm --parallel 10
```

## Focused correctness

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^(rxvmplugin_context_ownership|rxvmactive_isolation|rxasdecimaltests(-mc|-db)?|reentrancy_check|db_decimal_archive_link|decnumber_test|mc_decimal_(test[123]|dyn_test1|manual_test1|full_tests)|db_decimal_tests)$'

(cd cmake-build-debug/interpreter && ../bin/rxtvm tests_decimal)
(cd cmake-build-debug/interpreter && ../bin/rxbvm tests_decimal)
(cd cmake-build-debug/interpreter && ../bin/rxtvm -p rxvm_mc_decimal tests_decimal)
(cd cmake-build-debug/interpreter && ../bin/rxbvm -p rxvm_mc_decimal tests_decimal)

ctest --test-dir cmake-build-release --output-on-failure \
  -R '^rxvmplugin_context_ownership$'
```

## Formal timing and reduction

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-10-perf3-13-gate-e-e3a-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-10-perf3-13-gate-e-e3a-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3a-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3a-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3a-first-release-verdict/timing/samples.csv
```

The manifest records the exact executable, shared clean library/workload image,
arguments, expected output and benchmark-rate marker for every cell.

## Accepted closeout

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release \
  --target rxtvm rxbvm test_rxvmplugin_context --parallel 10
shasum -a 256 cmake-build-release/bin/rxtvm cmake-build-release/bin/rxbvm
ctest --test-dir cmake-build-release --output-on-failure \
  -R '^rxvmplugin_context_ownership$'
```

The full build and CTest streams were redirected to temporary logs and only
their compact completion summaries were read back.
