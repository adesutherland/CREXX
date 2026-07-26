# Commands

## Focused Debug correctness

```sh
cmake --build cmake-build-debug \
  --target run_tests_perf2_exact_relink --parallel 10

ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxasperf2exactrelinktests|rxasperf2exactrelinktests-rxbvm|rxasperf2partialreferencetests|rxasperf2partialreferencetests-rxbvm|rxasperf2referenceguardtests|rxasperf2referenceguardtests-rxbvm|rxasreferencetests|rxasreferencetests-rxbvm|rxasbreakpointtraceeventtests|rxasbreakpointtraceeventtests-rxbvm|rxassignalmaskbreakpointignoretests|rxassignalmaskbreakpointignoretests-rxbvm)$'
```

The retained compiler/import/optimized/no-opt reference matrix was then run and
passed 49/49.

## Fresh corrected ordinary Release product

```sh
cmake -S . \
  -B /tmp/crexx-perf2-05-r1a.xbBPgs/candidate-release-v2 \
  -G Ninja -DCMAKE_BUILD_TYPE=Release \
  '-DCMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG' \
  -DCREXX_VM_PROFILING=OFF -DBUILD_TESTING=ON

cmake --build /tmp/crexx-perf2-05-r1a.xbBPgs/candidate-release-v2 \
  --target rxvm rxbvm run_tests_perf2_exact_relink --parallel 10

ctest --test-dir /tmp/crexx-perf2-05-r1a.xbBPgs/candidate-release-v2 \
  -R '^rxasperf2exactrelinktests(-rxbvm)?$' --output-on-failure
```

The generated guard RXBIN was run directly with both fresh R1a VMs and both
preserved R2a VMs. The retained optimized List RXBIN and library were run with
both fresh R1a VMs at work 100 before measurement.

## Governed balanced captures

The maintained Level B runner self-test passed first:

```sh
/tmp/crexx-perf2-05-sa1.MzdvHT/baseline-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args --self-test
```

Each capture used the same corrected manifest and exact inputs:

```sh
caffeinate -i /tmp/crexx-perf2-05-sa1.MzdvHT/baseline-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest /tmp/crexx-perf2-05-r1a.xbBPgs/v2/list-r2a-r1a-work100.txt \
  --output-dir OUTPUT_DIR --measurement timing --warmups 1 --runs 12
```

`OUTPUT_DIR` was, in order, `timing/list-initial-12`,
`timing/list-append-12` and `timing/list-append-24`. The first append was
required by the initial `rxvm` paired interval and absolute spans; the final
append was required by the second capture's R2a `rxvm` absolute span. The
series then reached its 36-pair cap.

```sh
/tmp/crexx-perf2-05-sa1.MzdvHT/baseline-release/bin/crexx \
  summarize_paired.crexx --nokeep --args paired-summary.csv \
  timing/list-initial-12/samples.csv \
  timing/list-append-12/samples.csv \
  timing/list-append-24/samples.csv
```

## Accepted closeout

After Adrian accepted the verdict, the affected products and focused sets were
rebuilt and replayed before broad validation:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --parallel 30 --output-on-failure
```

Both broad configurations pass 1,924/1,924. No sanitizer, install/package,
cross-platform, expanded-portfolio, repeated-baseline or push command was run.
