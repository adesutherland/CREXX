# NR-15 D2-hybrid first Release verdict commands

The frozen ordinary product was built with `CMAKE_BUILD_TYPE=Release`,
`CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG` and `CREXX_VM_PROFILING=OFF`:

```sh
cmake --build cmake-build-release --target rxc rxas rxlink rxvm rxbvm library \
  test_rxop_metadata performance_nr15_stem_semantics \
  performance_nr15_stem_access benchmark_rexxcps_levelb_opt_artifact \
  benchmark_rexxcps_levelb_noopt_artifact --parallel 10
```

The candidate modules were stripped-linked with their exact new library using
the same Release `rxlink`. The focused Release matrix passed 10/10 plus the
opcode metadata executable before timing.

The formal timing block uses the checked-in Level B matrix driver, one warmup
and 12 recorded rounds. Each workload group contains Candidate A and D2-hybrid
for both VMs; the starting position rotates each round, and every process runs
serially:

```sh
cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest performance/evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/input-manifest.txt \
  --output-dir performance/evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/timing \
  --measurement timing --warmups 1 --runs 12
```

The same driver captures a 12-pair process-inclusive load/64-entry
construction/first-access lifecycle diagnostic, and the policy minimum three
canonical peak-RSS observations per image/VM:

```sh
cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args --manifest performance/evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/lifecycle-manifest.txt \
  --output-dir performance/evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/lifecycle \
  --measurement timing --warmups 1 --runs 12
cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args --manifest performance/evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/rss-manifest.txt \
  --output-dir performance/evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/rss \
  --measurement rss --warmups 0 --runs 3
```

The lifecycle `rxvm` absolute min/max span exceeded 10%, so the policy-required
ten unchanged serial observations were captured under `lifecycle-append/` and
merged under `lifecycle-combined/`. No sample was removed. Schema-4 profiles
used the exact linked images with the current profiling VMs and the access
arguments `get-hit|histogram 64 1000 ascii`; retained RexxCPS profiles used the
fixed canonical default `100 x 100` work.

The discarded qualification pilots were not formal samples. Their only role
was to choose equal work counts that make the fastest timed cell at least one
second and to detect that the generated module required explicit linking.
