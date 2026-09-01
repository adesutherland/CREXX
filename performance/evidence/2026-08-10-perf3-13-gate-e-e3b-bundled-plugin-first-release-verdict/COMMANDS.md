# Retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`.

## Focused correctness

```sh
cmake --build cmake-build-debug --parallel 10 --target test_rxpa_concurrency
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^rxpa_(static_catalogue_replay|legacy_call_serialization|process_reentrant_overlap|legacy_recursive_reentry|branch_free_load_binding|bound_legacy_serialization|legacy_transition_quiescence|bundled_.*_concurrency)$'

cmake --build cmake-build-release --parallel 10 \
  --target rxvm rxbvm rxtvm test_rxpa_concurrency \
  _cipher _cipher_static _stack _stack_static _strings _getpi _id _id_static
ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -R '^rxpa_(static_catalogue_replay|legacy_call_serialization|process_reentrant_overlap|legacy_recursive_reentry|branch_free_load_binding|bound_legacy_serialization|legacy_transition_quiescence|bundled_.*_concurrency)$'
```

Both focused panels passed 12/12.

## Timing

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-10-perf3-13-gate-e-e3b-bundled-plugin-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-10-perf3-13-gate-e-e3b-bundled-plugin-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12
```

The paired mean and 95% interval use each recorded round's candidate/control
elapsed ratio, Student t critical value 2.200985 for 11 degrees of freedom,
and no outlier removal.
