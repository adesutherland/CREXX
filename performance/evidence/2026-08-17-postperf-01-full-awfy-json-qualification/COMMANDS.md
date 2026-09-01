# Replay commands

## Build and fixture validation

```sh
cmake -S . -B cmake-build-release
cmake --build cmake-build-release \
  --target benchmark_awfy_json_opt_artifact \
           benchmark_awfy_json_noopt_artifact \
           benchmark_runner_opt_artifact \
           benchmark_runner_noopt_artifact \
  --parallel 10
wc -c tests/benchmarks/fixtures/awfy_json_rap_minified.json
shasum -a 256 tests/benchmarks/fixtures/awfy_json_rap_minified.json
jq -r '[type, (.head|type), (.operations|type), (.operations|length), .head.requestCounter] | @csv' \
  tests/benchmarks/fixtures/awfy_json_rap_minified.json
```

## Correctness matrix

The optimized image is linked with the ordinary library before execution; the
unoptimized image loads the program and library modules separately. The direct
matrix repeats this command shape for `rxvm`, `rxtvm` and `rxbvm`:

```sh
rxlink -s -o LINKED \
  cmake-build-debug/tests/benchmarks/benchmark_awfy_json_opt.rxbin \
  cmake-build-debug/bin/library.rxbin
VM LINKED.rxbin -a 1 \
  cmake-build-debug/tests/benchmarks/awfy_json_rap_minified.json
VM cmake-build-debug/tests/benchmarks/benchmark_awfy_json_noopt.rxbin \
  cmake-build-debug/bin/library.rxbin -a 1 \
  cmake-build-debug/tests/benchmarks/awfy_json_rap_minified.json
```

The focused CTest paths were:

```sh
ctest --test-dir cmake-build-debug \
  -R '^benchmark_awfy_json_(noopt|opt)$' --output-on-failure
ctest --test-dir cmake-build-release \
  -R '^(inline_binary_formal_string_conversion_(run_args_noopt|run_args_opt|shape)|benchmark_awfy_json_(noopt|opt))$' \
  --output-on-failure
```

## Bounded process pilot

The Release tree was explicitly reconfigured before capture so the embedded
version identifies `110e298af` plus the controlled dirty scope. The exact
command was run once for `rxvm` and once for the distinct `rxtvm` control:

```sh
cmake-build-release/bin/crexx --nokeep \
  tests/benchmarks/run_benchmarks.crexx --args \
  --build-dir cmake-build-release --vm VM --mode opt \
  --warmups 1 --runs 5 --benchmark awfy-json \
  --output SUMMARY.csv --samples-output SAMPLES.csv
```

Each process performs 50 complete parse/verify iterations. The runner measures
process elapsed time, including VM startup, module loading and the one fixture
load. Samples were serial. The pilot is orientation only and is excluded from
all aggregates.

## Linked-image inspection

```sh
rxlink -s -o benchmark_awfy_json_opt_linked \
  cmake-build-release/tests/benchmarks/benchmark_awfy_json_opt.rxbin \
  cmake-build-release/bin/library.rxbin
rxdas -o benchmark_awfy_json_opt_linked.rxas \
  benchmark_awfy_json_opt_linked.rxbin
```

The linked product is reproducible from the versioned source, fixture and
recorded tool/library hashes; it is not duplicated in this evidence directory.
