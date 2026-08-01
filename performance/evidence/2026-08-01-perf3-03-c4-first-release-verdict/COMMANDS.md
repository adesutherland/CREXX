# Replay commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

## Minimum Debug gate

```sh
cmake --build cmake-build-debug --parallel 10 --target rxvm rxbvm
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(06_logic_(noopt|opt)|rxaslogicflowtests(-rxbvm)?|rxasconversiontests(-rxbvm)?)$'
```

## Ordinary Release product

```sh
cmake -S . -B BUILD -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DBUILD_TESTING=OFF
cmake --build BUILD --parallel 10 --target rxvm rxbvm
```

## First verdict matrices

Every block used the checked-in Level B runner with the retained current
product, workload images and library:

```sh
/tmp/crexx-perf3-05.9DY131/build-baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest MANIFEST --output-dir OUTPUT \
  --measurement timing --warmups WARMUPS --runs RUNS
```

The initial block used one warmup and 12 recorded pairs. Append blocks used
zero warmups and 10, 12 and 10 recorded pairs according to the retained
absolute-noise and interval rules. The final absolute summary was regenerated
with `--summary-only` and all four `samples.csv` inputs.

Paired percentage change is `(candidate/current - 1) * 100` for RexxCPS rate
and `(current/candidate - 1) * 100` for Base64 elapsed time. Quartiles use R-7
interpolation; intervals are two-sided 95% Student-t intervals around the
arithmetic mean. No sample was removed.
