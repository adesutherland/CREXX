# NR-27 formal commands

The ordinary profiling-off Release product and five common-workload artifacts
were built serially with respect to all tests and captures:

```sh
cmake --build cmake-build-release \
  --target rxas rxdas rxlink rxvm rxbvm crexx library \
  benchmark_awfy_sieve_opt_artifact \
  benchmark_awfy_permute_opt_artifact \
  benchmark_awfy_bounce_opt_artifact \
  benchmark_awfy_richards_opt_artifact \
  benchmark_awfy_list_opt_artifact \
  benchmark_awfy_towers_opt_artifact \
  --parallel 10
cmake --build cmake-build-release \
  --target benchmark_base64_roundtrip_opt_artifact --parallel 10
```

Each retained baseline or candidate module was linked with its matching exact
`library.rxbin`, without `-s`, using the same candidate `rxlink`:

```sh
cmake-build-release/bin/rxlink -o OUTPUT.rxbin MODULE.rxbin library.rxbin
```

The initial formal block used the Level B driver and 20-cell manifest:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-22-nr-27-panel-verdict/input-manifest.txt \
  --output-dir performance/evidence/2026-07-22-nr-27-panel-verdict/timing \
  --measurement timing --warmups 1 --runs 12
```

The policy-required absolute-noise append used the 16-cell Sieve, Permute,
Bounce and Base64 manifest with `--warmups 0 --runs 10`. The paired-interval
append then reused the main manifest with `--warmups 0 --runs 12`. Richards'
final 12-pair block used `richards-final-input-manifest.txt`. The final absolute
summary was regenerated from all four `samples.csv` files with `--summary-only`.

All commands ran serially. No recorded sample was removed or replaced.

After Adrian accepted the first Release verdict, the required closeout gate
used:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

The full build passed, followed by 1,885/1,885 CTests in 145.65 seconds.
