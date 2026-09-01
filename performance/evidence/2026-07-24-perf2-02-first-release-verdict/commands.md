# PERF2-02 first Release verdict commands

The formal comparison uses the maintained Level B matrix driver:

```sh
caffeinate -i \
  /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/crexx \
  /Users/adrian/CLionProjects/CREXX/performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/manifests/bounce-q0-production.txt \
  --output-dir /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-formal \
  --measurement timing --warmups 1 --runs 12
```

The maintained matrix driver's summary-only mode combines the unchanged
absolute cells, and the evidence-local Level B reducer reports the governed
balanced-pair distribution and mean Student-t interval:

```sh
/private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/crexx \
  /Users/adrian/CLionProjects/CREXX/performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/manifests/bounce-q0-production.txt \
  --output-dir /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-combined \
  --measurement timing --summary-only \
  --samples /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-formal/samples.csv \
  --samples /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-rxbvm-noise-append/samples.csv

/private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/crexx \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/summarize_paired.crexx \
  --nokeep --args \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/paired-summary.csv \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-formal/samples.csv \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-rxbvm-noise-append/samples.csv
```

Samples are serial and position-balanced within each VM workload.

The initial `rxbvm` cells crossed the programme's 10% span rule. The required
ten-sample append keeps the artifacts, image, library, work and serial sampling
unchanged and omits a second warmup after the immediately preceding campaign:

```sh
caffeinate -i \
  /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/crexx \
  /Users/adrian/CLionProjects/CREXX/performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/manifests/bounce-rxbvm-noise-append.txt \
  --output-dir /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-24-perf2-02-first-release-verdict/timing/bounce-rxbvm-noise-append \
  --measurement timing --warmups 0 --runs 10
```

## Post-acceptance QA

After Adrian accepted the first Release verdict, the broad QA commands were:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --parallel 30 --output-on-failure

tools/asan-run.sh --phase full --test-jobs 8 \
  --build-leaks off --leaks off --no-live-tail

cmake --install cmake-build-release --prefix "$isolated_prefix"
CREXX_HOME="$isolated_prefix" \
  "$isolated_prefix/bin/crexx" hello.crexx -native -nokeep
./hello

"$isolated_prefix/bin/rxvm" \
  /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/tests/benchmarks/benchmark_awfy_bounce_opt.rxbin \
  /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/library.rxbin \
  -a 1
"$isolated_prefix/bin/rxbvm" \
  /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/tests/benchmarks/benchmark_awfy_bounce_opt.rxbin \
  /private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin/library.rxbin \
  -a 1
```

The first sanitizer invocation used the runner defaults and stopped before
testing because Apple AddressSanitizer rejects `detect_leaks`. The rerun above
explicitly disabled the unsupported leak option while retaining AddressSanitizer
for the full build and test sweep.
