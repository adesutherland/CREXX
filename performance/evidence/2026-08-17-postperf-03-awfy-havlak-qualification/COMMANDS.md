# Replay commands

Build Havlak and the maintained runner in ordinary profiling-off Release:

```sh
cmake --build cmake-build-release --parallel 10 --target \
  benchmark_awfy_havlak_opt_artifact \
  benchmark_awfy_havlak_noopt_artifact \
  benchmark_runner_opt_artifact \
  benchmark_runner_noopt_artifact
```

Run the bounded correctness matrix:

```sh
VM cmake-build-release/tests/benchmarks/benchmark_awfy_havlak_MODE.rxbin \
  cmake-build-release/bin/library.rxbin -a 1 0
```

`MODE` is `opt` or `noopt`; `VM` is `rxvm`, `rxtvm` or `rxbvm`.

Run the current-source executed reference scales:

```sh
cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_awfy_havlak_noopt.rxbin \
  cmake-build-release/bin/library.rxbin -a DUMMY_LOOPS 0
```

`DUMMY_LOOPS` was `1`, `15`, `150` and `1500`. The `15000` attempt was
manually interrupted at the recorded resource bound and is not an executed
pass. Its published result is retained through the exact audited relation
`1602 + 3 * DUMMY_LOOPS`.

Run the maintained lifecycle cell:

```sh
cmake-build-release/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_runner_MODE.rxbin \
  cmake-build-release/bin/library.rxbin -a \
  --build-dir cmake-build-release --vm rxvm --mode MODE \
  --warmups 0 --runs 1 --benchmark havlak \
  --output summary.csv --samples-output samples.csv
```

The explicit runner lane uses `1 1`: one persistent simple-graph recognition,
the fixed persistent large-graph recognition and one discarded full-graph
recognition.

Generated source RXAS used the established instruction rule:

```sh
awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' IMAGE.rxas
```

Linked images were produced with `rxlink -s`, disassembled with `rxdas`, and
counted through their retained `* 0xADDRESS:OPCODE` annotations.

Focused qualification selections:

```sh
ctest --test-dir cmake-build-debug --output-on-failure \
  -R '^benchmark_awfy_havlak_(noopt|opt)$'
ctest --test-dir cmake-build-release --output-on-failure \
  -R '^benchmark_awfy_havlak_(noopt|opt)$'
```

Broad Debug closeout:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```
