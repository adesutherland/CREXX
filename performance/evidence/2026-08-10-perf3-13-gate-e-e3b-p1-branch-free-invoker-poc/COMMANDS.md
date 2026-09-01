# E3b-P1 branch-free invoker retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`.

## Build and correctness

The disposable proof source is retained under
poc/e3b_rxpa_invoker_ceiling.c. During the isolated gate it was temporarily
wired into tests/performance/CMakeLists.txt with the target and CTest below;
that production-tree wiring was removed after the integrated design passed.

```sh
cmake --build cmake-build-debug \
  --target e3b_rxpa_invoker_ceiling --parallel 10
ctest --test-dir cmake-build-debug \
  -R '^e3b_rxpa_invoker_ceiling_selftest$' --output-on-failure

cmake --build cmake-build-release \
  --target e3b_rxpa_invoker_ceiling --parallel 10
ctest --test-dir cmake-build-release \
  -R '^e3b_rxpa_invoker_ceiling_selftest$' --output-on-failure
```

The Release cache records `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF` and `CREXX_VM_HANDLER_PANEL=profile-20`.

## Assembly proof

```sh
nm -nm cmake-build-release/tests/performance/e3b_rxpa_invoker_ceiling
otool -tvV cmake-build-release/tests/performance/e3b_rxpa_invoker_ceiling
```

## Formal timing and reduction

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-invoker-poc/manifest.txt \
  --output-dir performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-invoker-poc/timing \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-invoker-poc/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-invoker-poc/paired-summary.csv \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-branch-free-invoker-poc/timing/samples.csv
```
