# CONC-16 HTTP rationalisation retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`. The control directory
was frozen before the production edit at
`/tmp/crexx-http-rationalization-control-d024b9738`.

## Candidate build

```sh
cmake --build cmake-build-release --parallel 10
```

The Release cache recorded `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF`, and `CREXX_VM_HANDLER_PANEL=profile-20`.

## First paired run

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/timing/single-thread \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/timing/single-thread/samples.csv
```

## Identical confirmation run

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/timing/single-thread-confirmation \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/paired-summary-confirmation.csv \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/timing/single-thread-confirmation/samples.csv
```

## Focused QA

```sh
ctest --test-dir cmake-build-release \
  -R '^ts_(llm_(ollama|providers)|(http_(pooled|policy|streaming|crexx_rag|codec))_.*)_(noopt|opt)$' \
  --parallel 10 --output-on-failure

cmake --build cmake-build-debug --target ts_http_pooled --parallel 10
ctest --test-dir cmake-build-debug \
  -R '^ts_(llm_(ollama|providers)|(http_(pooled|policy|streaming|crexx_rag|codec))_.*)_(noopt|opt)$' \
  --parallel 10 --output-on-failure
```

## Artifact inspection

```sh
uname -srm
sysctl -n machdep.cpu.brand_string
shasum -a 256 \
  cmake-build-release/bin/rxtvm cmake-build-release/bin/rxbvm \
  cmake-build-release/bin/library.rxbin cmake-build-release/bin/rxfnsg.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  /tmp/crexx-http-rationalization-control-d024b9738/bin/rxtvm \
  /tmp/crexx-http-rationalization-control-d024b9738/bin/rxbvm \
  /tmp/crexx-http-rationalization-control-d024b9738/bin/library.rxbin \
  /tmp/crexx-http-rationalization-control-d024b9738/bin/rxfnsg.rxbin
```
