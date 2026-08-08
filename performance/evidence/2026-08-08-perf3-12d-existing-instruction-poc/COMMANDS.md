# PERF3-12D command record

All repository commands ran from
`/Users/adrian/CLionProjects/CREXX-parse-planning-poc` on branch
`codex/peter-parse-planning-poc`, based on
`06fe132872b1ef51409cb071cb907da9f3714632` from `origin/develop`.

## Focused build and tests

```sh
cmake --build cmake-build-release --parallel 10 \
  --target exit_parse_bin test_parse_direct_bin test_parse_pattern_poc_bin

ctest --test-dir cmake-build-release --parallel 10 --output-on-failure -R \
  '^(nr14_frozen_parse_contract|test_parse_noopt|test_parse_opt|test_parse_direct_|test_parse_pattern_poc_|ts_parse|benchmark_parse_frozen)'
```

The selected test matrix passed 19/19.

## Compile and assemble

E0 was compiled before the PoC edit; E1 was compiled after it. Both used the
same source and ordinary profiling-off Release tools:

```sh
cmake-build-release/bin/rxc -i cmake-build-release/bin \
  -o WORK/pattern_dynamic_eN raw/pattern_dynamic_benchmark.crexx
cmake-build-release/bin/rxas \
  -o WORK/pattern_dynamic_eN WORK/pattern_dynamic_eN
cmake-build-release/bin/rxdas WORK/pattern_dynamic_eN.rxbin \
  > WORK/pattern_dynamic_eN.rxdas
```

## Timing

One warm-up per VM/variant was discarded. Eight recorded rounds alternated E0,
E1 and E1,E0 order:

```sh
/usr/bin/time -p -o TIME \
  cmake-build-release/bin/VM cmake-build-release/bin/library.rxbin \
  WORK/pattern_dynamic_eN.rxbin -a 1000000 / left/right
```

## Counts

```sh
cmake-build-profile-parse/bin/VM \
  --profile=counts --profile-output=COUNTS.csv \
  cmake-build-release/bin/library.rxbin \
  WORK/pattern_dynamic_eN.rxbin -a 100000 / left/right
```

The profiling build had `CREXX_VM_PROFILING=ON`; product timing used the
ordinary profiling-off Release build.

## Canonical RexxCPS score

The CMake benchmark artifact was first inspected and rejected as stale because
it still contained four `parseExec` calls. The unchanged canonical source was
compiled into a fresh temporary artifact:

```sh
cmake-build-release/bin/rxc -i cmake-build-release/bin \
  -o SCRATCH/rexxcps_e1 tests/benchmarks/rexxcps_levelb.crexx
cmake-build-release/bin/rxas \
  -o SCRATCH/rexxcps_e1 SCRATCH/rexxcps_e1
```

One warm-up per VM was discarded. Eight runs per VM were recorded in alternating
VM order with no benchmark arguments:

```sh
/usr/bin/time -p -o TIME \
  cmake-build-release/bin/VM cmake-build-release/bin/library.rxbin \
  SCRATCH/rexxcps_e1.rxbin -a
```
