# Reproduction commands

Apply the combined patch to detached
`b08611179db5ff4257c3be3103f3aeab55ea5b50`:

```sh
git apply performance/evidence/2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/v1r01-r1-plus-v3.patch
```

Configure independent products:

```sh
cmake -S SOURCE -B FOCUSED_DEBUG -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=OFF
cmake -S SOURCE -B COUNTS_RELEASE -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=ON
cmake -S SOURCE -B ORDINARY_RELEASE -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCREXX_VM_PROFILING=OFF
cmake --build ORDINARY_RELEASE --parallel 10
```

Focused gate:

```sh
ctest --test-dir FOCUSED_DEBUG \
  -R '^(linked_opt_runtime_artifacts_build|perf2_07_v3_representation|inline_local_member_scalar_opt|inline_receiver_nested_this_direct_placement_opt|inline_receiver_reference_alias_opt|inline_test_computed_receiver_copyback_opt_rewrites|inline_test_computed_receiver_copyback_run_noopt|inline_test_computed_receiver_copyback_run_opt|inline_test_mutating_method_scalar_attr_return_run_noopt|inline_test_mutating_method_scalar_attr_return_run_opt)$' \
  --output-on-failure
```

Formal initial block; use the same command with `--warmups 0 --runs 12` and
the `timing/append-01` and `timing/append-02` output directories for governed
appends:

```sh
caffeinate -i CONTROL_RELEASE/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/manifests/first-verdict.txt \
  --output-dir performance/evidence/2026-07-27-perf2-06-07-v1r01-r1-first-release-verdict/timing/formal-12 \
  --measurement timing --warmups 1 --runs 12
```

The evidence-local Level B reducer accepts all three `samples.csv` paths and
writes `paired-summary-36.csv` plus `common-geomean-36.csv`. Raw exact counts
use the independently built counts VM with `--profile=counts`; execute current
and candidate images under the same counts binary and load the matching library.
