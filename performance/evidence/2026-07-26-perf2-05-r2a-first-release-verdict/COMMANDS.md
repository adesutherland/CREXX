# Commands

## Fresh ordinary Release product

```sh
cmake -S . -B "$r2a_root/release" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  '-DCMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG' \
  -DCREXX_VM_PROFILING=OFF -DBUILD_TESTING=ON
cmake --build "$r2a_root/release" \
  --target rxvm rxbvm run_tests_perf2_partial_reference_materialization \
  --parallel 10
```

## Focused correctness

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxasperf2partialreferencetests|rxasperf2partialreferencetests-rxbvm|rxasperf2referenceguardtests|rxasperf2referenceguardtests-rxbvm|rxasreferencetests|rxasreferencetests-rxbvm|rxasbreakpointtraceeventtests|rxasbreakpointtraceeventtests-rxbvm|rxassignalmaskbreakpointignoretests|rxassignalmaskbreakpointignoretests-rxbvm)$'

ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(perf2_05_partial_call|object_reference_regression_(noopt|opt|run_noopt|run_opt)|reference_source_(explicit|contract|iterator|dereference_live|inline_lifetime|block_lifetime)_(rxvm|rxbvm)_(noopt|opt)|inline_receiver_reference_alias_opt|inline_reference_member_accessors_opt|inline_reference_accessors_import_opt|reference_iterator_(compare|system|perf)_(rxvm|rxbvm)_(noopt|opt)|reference_generated_contract_(rxvm|rxbvm)_(noopt|opt))$'
```

The new Release fixture was run with both integrated VMs and both retained Q0
VMs. The retained optimized List image and library were then run with both
integrated VMs at work 10 before measurement.

## Balanced captures

The maintained Level B runner self-test passed first. The initial work-10 guard
used `manifests/list-q0-r2a.txt` and is retained under
`timing/list-work10-guard/`. Rechecking the accepted PoC showed that its exact
decision work was 100, so the authoritative capture was:

```sh
caffeinate -i \
  /tmp/crexx-perf2-05-sa1.MzdvHT/baseline-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-05-r2a-first-release-verdict/manifests/list-q0-r2a-work100.txt \
  --output-dir performance/evidence/2026-07-26-perf2-05-r2a-first-release-verdict/timing/list-formal \
  --measurement timing --warmups 1 --runs 12

/tmp/crexx-perf2-05-sa1.MzdvHT/baseline-release/bin/crexx \
  performance/evidence/2026-07-26-perf2-05-r2a-first-release-verdict/summarize_paired.crexx \
  --nokeep --args \
  performance/evidence/2026-07-26-perf2-05-r2a-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-07-26-perf2-05-r2a-first-release-verdict/timing/list-formal/samples.csv
```

## Accepted closeout

After Adrian accepted the first Release verdict, the required broad gates were
run sequentially so build and test activity did not overlap in either tree:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --parallel 30 --output-on-failure
```

Both full builds pass. Debug CTest passes 1,922/1,922 in 167.16 seconds; ordinary
profiling-off Release CTest passes 1,922/1,922 in 64.49 seconds.
