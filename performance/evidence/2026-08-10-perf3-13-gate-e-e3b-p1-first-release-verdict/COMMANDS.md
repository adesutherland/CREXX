# E3b-P1 retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX` unless noted.

## Frozen control and candidate

```sh
shasum -a 256 cmake-build-release/bin/rxtvm cmake-build-release/bin/rxbvm
control_dir=$(mktemp -d /tmp/crexx-e3b-control.XXXXXX)
cp cmake-build-release/bin/rxtvm "$control_dir/rxtvm-e3a-29ef1975e"
cp cmake-build-release/bin/rxbvm "$control_dir/rxbvm-e3a-29ef1975e"

cmake --build cmake-build-release \
  --target rxtvm rxbvm test_rxpa_concurrency _rxpa_dynlink \
  _rxpa_invalid_manifest _rxpa_bench_reentrant _rxpa_bench_legacy \
  rxc rxas library --parallel 10
```

The copied control binaries retained the exact accepted E3a hashes. The
Release cache recorded `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF` and `CREXX_VM_HANDLER_PANEL=profile-20`.

## Focused Release correctness

```sh
ctest --test-dir cmake-build-release \
  -R '^(rxpa_static_catalogue_replay|rxpa_legacy_call_serialization|rxpa_process_reentrant_overlap|rxpa_legacy_recursive_reentry|rxpa_process_reentrant_manifest|rxpa_invalid_manifest_is_legacy|rxpa_dynamic_context_ownership)$' \
  --output-on-failure
```

## Optimized native-call images

From `cmake-build-release/tests/performance`:

```sh
../../bin/rxc -x \
  -i "../rxpa;../../bin" \
  -o e3b_rxpa_call_reentrant \
  ../../../tests/performance/rxpa_call_reentrant.crexx
../../bin/rxas -o e3b_rxpa_call_reentrant e3b_rxpa_call_reentrant

../../bin/rxc -x \
  -i "../rxpa;../../bin" \
  -o e3b_rxpa_call_legacy \
  ../../../tests/performance/rxpa_call_legacy.crexx
../../bin/rxas -o e3b_rxpa_call_legacy e3b_rxpa_call_legacy
```

The manifest contains the resolved absolute paths used by the capture.

## Formal timing and reduction

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-first-release-verdict/timing/samples.csv
```

## Host and binary inspection

```sh
uname -a
sysctl -n machdep.cpu.brand_string
sysctl -n hw.logicalcpu
pmset -g batt
pmset -g custom
pmset -g therm
uptime

size -m CONTROL_OR_CANDIDATE_VM
nm -n CONTROL_OR_CANDIDATE_VM
otool -tvV CONTROL_OR_CANDIDATE_VM
```
