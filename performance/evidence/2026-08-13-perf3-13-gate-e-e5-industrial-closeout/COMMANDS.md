# E5 industrial closeout commands

All build and test output was redirected to temporary logs and inspected in
bounded slices. Build and test invocations did not overlap in a build tree.

## Candidate configuration

The retained candidate cache records:

```text
CMAKE_BUILD_TYPE=Release
CREXX_VM_PROFILING=OFF
CREXX_VM_HANDLER_PANEL=profile-20
```

The accepted timing driver used the baseline and candidate persistent-worker
executables with the same RXBIN, one warmup, 12 pairwise-balanced recorded
rounds, `jobs=10000`, and `iterations=25000`. It covered `direct`,
`executor1`, and `executor2` under `rxbvml` and `rxtvml`; every invocation had
to report `E5_BENCHMARK result=PASS` and the expected checksum.

## Post-repair Debug

```sh
cmake --build /private/tmp/crexx-mthread-review.VqdaIN/build --parallel 10
ctest --test-dir /private/tmp/crexx-mthread-review.VqdaIN/build \
  --parallel 30 --output-on-failure
ctest --test-dir /private/tmp/crexx-mthread-review.VqdaIN/build \
  -R '^syntaxhighlight_trace_keywords$' --output-on-failure
```

The focused cold-route reproduction used:

```sh
ctest --test-dir /private/tmp/crexx-mthread-review.VqdaIN/build \
  --output-on-failure \
  -R '^(rxc_invalid_binary_to_string_runtime|rxdas_roundtrip_binary|rxvmactive_isolation|rxassignalstests)$'
```

## Post-repair sanitizer

The supported runner built the affected targets and ran the active-state guard
plus all industrial E5 tests with Apple leak detection disabled:

```sh
tools/asan-run.sh --phase build \
  --build-dir /private/tmp/crexx-e5-industrial-asan \
  --build-jobs 10 --build-leaks off \
  --build-target rxbvm --build-target rxtvm --build-target rxvm \
  --build-target test_rxvmactive \
  --build-target persistent_worker_executor-rxvml \
  --build-target persistent_worker_executor-rxbvml \
  --build-target persistent_worker_executor-rxtvml
tools/asan-run.sh --phase ctest \
  --build-dir /private/tmp/crexx-e5-industrial-asan \
  --test-jobs 1 --leaks off \
  --regex '^(rxvmactive_isolation|rxc_invalid_binary_to_string_runtime|persistent_worker_executor-)'
```

## Post-repair Release

```sh
cmake --build /private/tmp/crexx-e5-industrial-candidate --parallel 10
ctest --test-dir /private/tmp/crexx-e5-industrial-candidate \
  --parallel 10 --output-on-failure \
  -R '^(rxvmactive_isolation|rxc_invalid_binary_to_string_runtime|rxdas_roundtrip_binary|rxassignalstests|persistent_worker_executor-)'
```
