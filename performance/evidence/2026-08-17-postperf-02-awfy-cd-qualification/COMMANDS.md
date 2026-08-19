# Replay commands

Build CD and the maintained runner in ordinary profiling-off Release:

```sh
cmake --build cmake-build-release --parallel 10 --target \
  benchmark_awfy_cd_opt_artifact \
  benchmark_awfy_cd_noopt_artifact \
  benchmark_runner_opt_artifact \
  benchmark_runner_noopt_artifact
```

Run the bounded correctness matrix:

```sh
VM cmake-build-release/tests/benchmarks/benchmark_awfy_cd_MODE.rxbin \
  rx_rxmath cmake-build-release/bin/library.rxbin -a 10
```

`MODE` is `opt` or `noopt`; `VM` is `rxvm`, `rxtvm` or `rxbvm`. Repeat with
the Debug tree for the Debug half of the 12-cell matrix.

Run the complete no-opt reference matrix with aircraft counts
`2 10 100 200 250 500 1000`. Optimized reference checks used 2, 10 and 100.

The maintained runner cells used:

```sh
cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_runner_MODE.rxbin \
  cmake-build-release/bin/library.rxbin -a \
  --build-dir cmake-build-release --vm VM --mode MODE \
  --warmups 1 --runs 5 --benchmark cd \
  --output summary.csv --samples-output samples.csv
```

Generated source RXAS used the established instruction rule:

```sh
awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' IMAGE.rxas
```

Linked images were produced with `rxlink -s`, disassembled with `rxdas`, and
counted through their retained `* 0xADDRESS:OPCODE` annotations.

Focused qualification selection:

```sh
ctest --test-dir BUILD --output-on-failure \
  -R '^benchmark_awfy_cd_(noopt|opt)$'
```

Broad Debug closeout:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```
