# Replay commands

All repository-relative commands were run from
`/Users/adrian/CLionProjects/CREXX`. The retained current product was
`/tmp/crexx-perf3-05.9DY131/build-baseline`; the disposable review root was
`/tmp/crexx-perf3-03.TsP0ej`.

## Helper controls

The C helper was compiled against `binutils/include/rxnumparse.h` and
`platform/rxinteger.c`; `from_chars_control.cpp` supplied the libc++ bounded
control. Contract runs used `C` and `de_DE.UTF-8`. Twelve raw one-million-call
rounds are retained in `controls/helper-timing/`.

The locale-aware classifier oracle was built with:

```sh
cc -O3 -DNDEBUG -std=c99 -I binutils/include \
  controls/prefilter3_contract_probe.c -o /tmp/prefilter3_contract_probe

locale -a | while IFS= read -r locale_name; do
  /tmp/prefilter3_contract_probe "$locale_name"
done
```

## Disposable VM products

Each candidate used a detached HEAD worktree, the corresponding patch under
`diagnostics/patches/`, and an isolated build:

```sh
cmake -S SOURCE -B BUILD -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DBUILD_TESTING=OFF
cmake --build BUILD --parallel 10 --target rxvm rxbvm
```

The diagnostic patch records loose-comparison outcome and input-shape counters
at exit. It was run against the exact current workload images and library.

## Formal matrices

Every formal block used the checked-in Level B driver:

```sh
/tmp/crexx-perf3-05.9DY131/build-baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest MANIFEST --output-dir OUTPUT \
  --measurement timing --warmups WARMUPS --runs RUNS
```

Initial blocks use one warmup and 12 recorded balanced rounds. Absolute-noise
extensions use zero warmups and ten recorded rounds. Remaining interval
extensions use zero warmups and 12 recorded rounds. Final absolute summaries
were regenerated with `--summary-only` and every applicable `samples.csv`.

The common-layout initial block captured 156 passing executions. Its first
manifest had `aggregate=yes`, so the driver returned 1 only after capture when
it could not find an ooRexx aggregate reference. The retained samples were not
rerun. `prefilter3-common-zero-nonaggregate-v2.txt` changes only that metadata,
and a `--summary-only` replay succeeded over the original `samples.csv`.
Governed zero-warmup append manifests then added 120 executions for the five
intervals crossing zero and 72 executions for the three intervals still
crossing zero; every execution passed. The final summary combines all three
retained sample files, with individual comparisons stopping at 12, 24 or the
36-pair cap as their intervals resolved.

Paired percentage change is `(candidate/current - 1) * 100` for RexxCPS rate
and `(current/candidate - 1) * 100` for elapsed-time workloads. Quartiles use
R-7 interpolation; the interval is the two-sided 95% Student-t interval around
the arithmetic mean. No sample was removed.
