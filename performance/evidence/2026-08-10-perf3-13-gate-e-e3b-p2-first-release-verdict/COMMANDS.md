# E3b-P2 first Release verdict commands

Commands run from `/Users/adrian/CLionProjects/CREXX`.

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p2-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p2-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p2-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p2-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p2-first-release-verdict/timing/samples.csv
```

## Accepted closeout and real-driver qualification

The dependency installation was separately approved after the first closeout
review. `brew install sqliteodbc` installed unixODBC 2.3.14 and sqliteodbc
0.99991. The successful focused qualification used:

```sh
cmake -S . -B cmake-build-debug -DENABLE_ODBC=ON
cmake --build cmake-build-debug \
  --target _odbc odbc_sqlite_prepared_runtime \
  odbc_mock_prepared_runtime test_rxpa_odbc test_rxpa_odbc_old_host \
  --parallel 10
ctest --test-dir cmake-build-debug -R odbc --output-on-failure

cmake -S . -B cmake-build-debugasan -DENABLE_ODBC=ON
tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off \
  --build-target _odbc \
  --build-target odbc_sqlite_prepared_runtime \
  --build-target odbc_mock_prepared_runtime \
  --build-target test_rxpa_odbc \
  --build-target test_rxpa_odbc_old_host --no-live-tail
tools/asan-run.sh --phase ctest --regex odbc --leaks off \
  --test-jobs 1 --no-live-tail

cmake -S . -B cmake-build-release -DENABLE_ODBC=ON
cmake --build cmake-build-release \
  --target _odbc odbc_sqlite_prepared_runtime \
  odbc_mock_prepared_runtime test_rxpa_odbc test_rxpa_odbc_old_host \
  --parallel 10
ctest --test-dir cmake-build-release -R odbc --output-on-failure
```

The successful Apple ASan logs are retained at
`cmake-build-debugasan/asan-logs/20260810-214329-build/` and
`cmake-build-debugasan/asan-logs/20260810-214337-ctest/`. Leak detection is off
because the Apple AddressSanitizer runtime does not support it; explicit
session, handle and DSO teardown assertions remain enabled.
