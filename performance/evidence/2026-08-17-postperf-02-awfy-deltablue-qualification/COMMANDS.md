# Replay commands

Build the benchmark and maintained runner in ordinary profiling-off Release:

```sh
cmake --build cmake-build-release --parallel 10 --target \
  benchmark_awfy_deltablue_opt_artifact \
  benchmark_awfy_deltablue_noopt_artifact \
  benchmark_runner_opt_artifact \
  benchmark_runner_noopt_artifact
```

Run the direct correctness matrix after linking each benchmark image with the
same Release library:

```sh
cmake-build-release/bin/rxlink -s -o IMAGE-linked \
  cmake-build-release/tests/benchmarks/benchmark_awfy_deltablue_MODE.rxbin \
  cmake-build-release/bin/library.rxbin
VM IMAGE-linked.rxbin -a 10
```

`MODE` is `opt` or `noopt`; `VM` is `rxvm`, `rxtvm` or `rxbvm`.

The maintained runner cells used:

```sh
cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_runner_MODE.rxbin \
  cmake-build-release/bin/library.rxbin -a \
  --build-dir cmake-build-release --vm VM --mode MODE \
  --warmups 1 --runs 5 --benchmark deltablue \
  --output summary.csv --samples-output samples.csv
```

Generated source RXAS used the established instruction rule:

```sh
awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' IMAGE.rxas
```

Linked images were disassembled with `rxdas`; executable lines were counted by
their retained `* 0xADDRESS:OPCODE` annotation.

Focused closeout selection:

```sh
ctest --test-dir BUILD --parallel 5 --output-on-failure \
  -R '^(inline_indexed_attr_factory_lifetime_(run_noopt|run_opt|shape)|benchmark_awfy_deltablue_(noopt|opt)|benchmark_runner_(noopt|opt))$'
```

Broad Debug closeout:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```
