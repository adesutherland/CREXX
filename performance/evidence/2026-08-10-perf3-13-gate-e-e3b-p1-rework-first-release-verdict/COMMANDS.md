# E3b-P1 inline-rework retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`.

## Focused build and correctness

```sh
cmake --build cmake-build-debug \
  --target test_rxpa_concurrency _rxpa_dynlink _rxpa_invalid_manifest \
  rxtvm rxbvm --parallel 10
ctest --test-dir cmake-build-debug \
  -R '^(rxpa_static_catalogue_replay|rxpa_legacy_call_serialization|rxpa_process_reentrant_overlap|rxpa_legacy_recursive_reentry|rxpa_process_reentrant_manifest|rxpa_invalid_manifest_is_legacy|rxpa_dynamic_context_ownership)$' \
  --output-on-failure

cmake --build cmake-build-release \
  --target rxtvm rxbvm test_rxpa_concurrency _rxpa_dynlink \
  _rxpa_invalid_manifest _rxpa_bench_reentrant _rxpa_bench_legacy \
  --parallel 10
ctest --test-dir cmake-build-release \
  -R '^(rxpa_static_catalogue_replay|rxpa_legacy_call_serialization|rxpa_process_reentrant_overlap|rxpa_legacy_recursive_reentry|rxpa_process_reentrant_manifest|rxpa_invalid_manifest_is_legacy|rxpa_dynamic_context_ownership)$' \
  --output-on-failure
```

The Release cache recorded `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF` and `CREXX_VM_HANDLER_PANEL=profile-20`.

## Assembly proof

```sh
nm -n cmake-build-release/bin/rxbvm
nm -n cmake-build-release/bin/rxtvm
otool -tvV cmake-build-release/bin/rxbvm
```

The binaries contain direct `rxvm_callfunc_direct` and legacy
`rxvm_callfunc` targets, no `rxvm_call_native_procedure` symbol, and a
556-byte direct adapter matching the control adapter size.

## Formal timing and reduction

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-rework-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-rework-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-rework-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-rework-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-rework-first-release-verdict/timing/samples.csv
```

The manifest retains every resolved executable, RXBIN, plugin, argument and
expected output.
