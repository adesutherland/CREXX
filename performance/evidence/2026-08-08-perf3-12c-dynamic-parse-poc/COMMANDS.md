# PERF3-12C command record

All repository commands ran from
`/Users/adrian/CLionProjects/CREXX-parse-planning-poc`.

## Isolated source

```sh
git fetch origin develop
git worktree add -b codex/peter-parse-planning-poc \
  /Users/adrian/CLionProjects/CREXX-parse-planning-poc origin/develop
git rev-parse HEAD
git rev-parse origin/develop
```

Both revisions were
`06fe132872b1ef51409cb071cb907da9f3714632` at activity start.

## Ordinary Release and focused qualification

```sh
cmake -S . -B cmake-build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DBUILD_TESTING=ON
cmake --build cmake-build-release --parallel 10

ctest --test-dir cmake-build-release --output-on-failure -R \
  'test_parse|test_parse_direct|ts_parse|ts_parseexec|benchmark_parse_frozen|benchmark_rexxcps_levelb'
```

The selected matrix completed 17/17. Exact test names and output are retained
in `raw/focused-ctest.log`.

## Disposable PoCs

The H1 image was created by applying `raw/hoisted-callframe.patch` to
`cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxas`, then:

```sh
cmake-build-release/bin/rxas \
  -o /private/tmp/crexx-parse-poc-sources/rexxcps_hoisted_callframe \
  /private/tmp/crexx-parse-poc-sources/rexxcps_hoisted_callframe
```

The H2 source was created by applying `raw/static-b-source.patch` to a temporary
copy of `tests/benchmarks/rexxcps_levelb.crexx`, then:

```sh
cmake-build-release/bin/rxc -i cmake-build-release/bin \
  -o /private/tmp/crexx-parse-poc-sources/rexxcps_static_b \
  /private/tmp/crexx-parse-poc-sources/rexxcps_static_b.crexx
cmake-build-release/bin/rxas \
  -o /private/tmp/crexx-parse-poc-sources/rexxcps_static_b \
  /private/tmp/crexx-parse-poc-sources/rexxcps_static_b
```

## Timing

For each concrete backend and variant, one warm-up was discarded. Eight
canonical-default runs followed in alternating orders `S0,H1,H2` and
`H2,H1,S0`:

```sh
/usr/bin/time -l cmake-build-release/bin/<rxtvm-or-rxbvm> \
  cmake-build-release/bin/library.rxbin <variant-image>.rxbin -a
```

No positional benchmark arguments were supplied. The raw per-process logs are
under `raw/<backend>-round<round>-<position>-<variant>.log`.

## Counts

```sh
cmake -S . -B cmake-build-profile-parse -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON \
  -DBUILD_TESTING=ON
cmake --build cmake-build-profile-parse --parallel 10 \
  --target rxtvm rxbvm library benchmark_rexxcps_levelb_opt_artifact

cmake-build-profile-parse/bin/<rxtvm-or-rxbvm> \
  --profile=counts \
  --profile-output performance/evidence/2026-08-08-perf3-12c-dynamic-parse-poc/profiles/<backend>-<variant>-counts.csv \
  cmake-build-profile-parse/bin/library.rxbin \
  <variant-image>.rxbin -a --smoke-count 200
```

`S0` and `H2` were captured under both backends. Every run reports
`effective_count=200` and `calibrated=0`.

## Correctness probes

`raw/dynamic_delimiter_semantics.crexx` was compiled with and without `-n`,
assembled in matching mode, and run with both `rxtvm` and `rxbvm`. All four
cells print `PASS dynamic delimiter semantics`.

`raw/issue667_exact.crexx` was compiled with the ordinary optimized compiler:

```sh
cmake-build-release/bin/rxc -i cmake-build-release/bin \
  -o /private/tmp/crexx-parse-poc-sources/issue667_exact_opt \
  performance/evidence/2026-08-08-perf3-12c-dynamic-parse-poc/raw/issue667_exact.crexx
```

It fails before RXAS generation as retained in `raw/issue667-opt-rxc.log`.
