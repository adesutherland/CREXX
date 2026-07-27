# Reproduction commands

Start from detached `b08611179db5ff4257c3be3103f3aeab55ea5b50` and apply the
prerequisite and isolated candidate patches in order:

```sh
git apply performance/evidence/2026-07-27-perf2-06-07-v1r01-first-release-verdict/v3-correctness-base.patch
git apply performance/evidence/2026-07-27-perf2-06-07-v1r01-first-release-verdict/v1r01-linear-leaf.patch
```

The independently named builds were configured as follows (replace the source
and build placeholders with isolated absolute paths):

```sh
cmake -S CANDIDATE_SOURCE -B FOCUSED_DEBUG -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=OFF
cmake -S CANDIDATE_SOURCE -B COUNTS_RELEASE -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=ON
cmake -S CANDIDATE_SOURCE -B ORDINARY_RELEASE -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=OFF
cmake --build ORDINARY_RELEASE --parallel 10
```

The final focused semantic gate was:

```sh
ctest --test-dir FOCUSED_DEBUG \
  -R '^(linked_opt_runtime_artifacts_build|perf2_07_v3_representation|inline_local_member_scalar_opt|inline_receiver_nested_this_direct_placement_opt|inline_receiver_reference_alias_opt|inline_test_computed_receiver_copyback_opt_rewrites|inline_test_computed_receiver_copyback_run_noopt|inline_test_computed_receiver_copyback_run_opt|inline_test_mutating_method_scalar_attr_return_run_noopt|inline_test_mutating_method_scalar_attr_return_run_opt)$' \
  --output-on-failure
```

The ordinary Release comparison used the checked-in Level B matrix:

```sh
caffeinate -i BASELINE_BUILD/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-27-perf2-06-07-v1r01-first-release-verdict/manifests/first-verdict.txt \
  --output-dir FORMAL_OUTPUT --measurement timing --warmups 1 --runs 12

BASELINE_BUILD/bin/crexx \
  performance/evidence/2026-07-27-perf2-06-07-v1r01-first-release-verdict/summarize_paired.crexx \
  --nokeep --args PAIRED_SUMMARY COMMON_GEOMEAN FORMAL_OUTPUT/samples.csv
```

The Bounce causal count used one candidate counts VM binary for both source
products, changing only the linked image and matching library:

```sh
COUNTS_RELEASE/bin/rxvm --profile=counts --profile-output current.csv \
  CURRENT_BOUNCE.rxbin CURRENT_LIBRARY.rxbin -a 100
COUNTS_RELEASE/bin/rxvm --profile=counts --profile-output candidate.csv \
  CANDIDATE_BOUNCE.rxbin CANDIDATE_LIBRARY.rxbin -a 100
```

Repeat the last two commands with `rxbvm`. The exact manifest contains the
fully resolved paths and arguments for all formal cells.
