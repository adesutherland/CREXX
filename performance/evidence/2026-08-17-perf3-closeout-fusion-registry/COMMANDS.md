# Commands

The generated disassembly directory is build-tree evidence and is not tracked.
The retained CSV is produced from it.

```sh
cmake --build cmake-build-release \
  --target perf3_fusion_inventory_selftest_opt_artifact \
           perf3_fusion_inventory_selftest_noopt_artifact \
  --parallel 10

mkdir -p cmake-build-release/tests/benchmarks/fusion-census-disassembly

# For each row in performance/manifests/perf3-closeout-fusion-census-v2.txt:
cmake-build-release/bin/rxdas \
  -o cmake-build-release/tests/benchmarks/fusion-census-disassembly/NAME.rxas \
  cmake-build-release/tests/benchmarks/NAME.rxbin

cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/perf3_fusion_inventory_selftest_opt \
  cmake-build-release/bin/library -a \
  --manifest performance/manifests/perf3-closeout-fusion-census-v2.txt \
  --output fusion-census.csv

ctest --test-dir cmake-build-debug \
  -R '^perf3_fusion_inventory_selftest_(opt|noopt)$' \
  --output-on-failure
```

Focused Debug self-test result: 3/3 including the linked artifact fixture.
