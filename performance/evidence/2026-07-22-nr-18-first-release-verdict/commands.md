# NR-18 first Release evidence commands

The accepted-NR-27 reference assembler was built from detached commit
`65ea6b9e292b5335565ec63d86c6a40e5f35853b`. The candidate is the frozen NR-18
worktree. Both assembled the same retained Release-generated RXAS sources.

The profiling-off candidate product was built with:

```sh
cmake --build cmake-build-release \
  --target rxas rxdas rxlink rxvm rxbvm crexx library \
  benchmark_awfy_sieve_opt_artifact \
  benchmark_awfy_permute_opt_artifact \
  benchmark_awfy_bounce_opt_artifact \
  benchmark_awfy_richards_opt_artifact \
  benchmark_base64_roundtrip_opt_artifact \
  nr03_evidence_tool_selftest_opt_artifact \
  nr10_cross_runtime_matrix_selftest_opt_artifact \
  nr10_lifecycle_tool_selftest_opt_artifact \
  nr10_artifact_inventory_selftest_opt_artifact \
  --parallel 10
```

Each baseline/candidate changed path was linked with its matching exact library
using the same candidate `rxlink -s`. Ordinary end-to-end checks used the
profiling-off Release `rxvm` and `rxbvm`; instruction counts used the same
linked images with the existing `cmake-build-profile` VMs and
`--profile-output`. Counts are the sum of schema-4 `instruction` rows.

The benchmark-runner path used one Sieve dispatch:

```sh
RUNNER IMAGE -a --build-dir cmake-build-release --vm rxvm \
  --mode opt --warmups 0 --runs 1 --benchmark sieve
```

The five common workload pairs were linked before hashing. Every pair was
byte-identical, so no wall-clock sampling block was run: identical executable
input cannot discriminate NR-18 and already supplies an exact no-regression
result for those retained products.
