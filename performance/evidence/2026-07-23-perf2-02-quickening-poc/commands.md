# PERF2-02 reproduction commands

All product timing used the maintained Level B matrix driver and absolute
manifests retained in this bundle. The decisive blocks were:

```sh
caffeinate -i /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/crexx \
  /private/tmp/crexx-perf2-02.kJ5sZT/q3-direct/performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest performance/evidence/2026-07-23-perf2-02-quickening-poc/manifests/bounce-q0-q3b-q4.txt \
  --output-dir performance/evidence/2026-07-23-perf2-02-quickening-poc/timing/bounce-q3b-formal \
  --measurement timing --warmups 2 --runs 12

caffeinate -i /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/crexx \
  /private/tmp/crexx-perf2-02.kJ5sZT/q3-direct/performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest performance/evidence/2026-07-23-perf2-02-quickening-poc/manifests/sieve-q0-q3b-q4.txt \
  --output-dir performance/evidence/2026-07-23-perf2-02-quickening-poc/timing/sieve-q3b-guard \
  --measurement timing --warmups 1 --runs 12
```

Startup used `bounce-startup-q0-q3b-q4-q7.txt`, one warmup and 12 runs. RSS
used `bounce-rss-q0-q3b-q4-q7.txt`, zero warmups and four runs. Earlier panel,
guard and Richards manifests are retained alongside their raw capture files.

Q3b was built from its detached clean worktree with:

```sh
cmake -S . -B cmake-build-perf2-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF -DBUILD_TESTING=ON
cmake --build cmake-build-perf2-release \
  --target rxvm rxbvm library --parallel 10
cmake --build cmake-build-perf2-release \
  --target run_tests_references run_tests_signal_call_unwind \
           test_dynamic_load test_reentrancy --parallel 10
ctest --test-dir cmake-build-perf2-release --output-on-failure \
  -R '^(dynamic_load|rxasreference.*|rxassignalcallunwind.*|reentrancy_check)$'
```

Exact focused logs and diagnostic profile/RXSEQ files are under `correctness/`.
The separate build-private counter reproductions are retained in
`correctness/q3b-q4-counter-diagnostics.md` and
`correctness/q7-diagnostic-reproduce.md`; their incremental source overlays are
under `prototypes/`. None of those diagnostic executables supplied product
timing.
