# Retained commands

Commands ran from the clean `mthread` worktree on the Apple arm64 host. The
paths below use descriptive build-directory names; `manifest.txt` retains the
exact paths used for the formal timing capture.

## Focused Debug and Release

```sh
cmake -S . -B cmake-build-mthread-debug \
  -DCMAKE_BUILD_TYPE=Debug -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build cmake-build-mthread-debug \
  --target persistent_worker_executor-rxvml \
           persistent_worker_executor-rxbvml \
           persistent_worker_executor-rxtvml --parallel 10
ctest --test-dir cmake-build-mthread-debug \
  -R '^persistent_worker_executor-' --repeat until-fail:20 \
  --output-on-failure --timeout 120

cmake -S . -B cmake-build-mthread-release \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF \
  -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build cmake-build-mthread-release \
  --target rxbvm rxtvm persistent_worker_executor-rxvml \
           persistent_worker_executor-rxbvml \
           persistent_worker_executor-rxtvml \
           benchmark_awfy_sieve_opt_artifact \
           benchmark_awfy_permute_opt_artifact \
           benchmark_rexxcps_levelb_opt_artifact library --parallel 10
ctest --test-dir cmake-build-mthread-release \
  -R '^persistent_worker_executor-' --output-on-failure --timeout 120
```

## Doorbell latency

```sh
CREXX_VM_POC_DOORBELL=posix \
  persistent_worker_executor-rxbvml \
  --doorbell-latency test_persistent_worker_executor.rxbin 1000
CREXX_VM_POC_DOORBELL=posix \
  persistent_worker_executor-rxtvml \
  --doorbell-latency test_persistent_worker_executor.rxbin 1000
```

## Formal Release comparison

```sh
caffeinate -i QUALIFIED_CREXX performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-12-perf3-13-gate-e-e5-macos-doorbell-poc/manifest.txt \
  --output-dir performance/evidence/2026-08-12-perf3-13-gate-e-e5-macos-doorbell-poc/timing \
  --measurement timing --warmups 1 --runs 12

QUALIFIED_CREXX \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-macos-doorbell-poc/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-macos-doorbell-poc/paired-summary.csv \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-macos-doorbell-poc/timing/samples.csv
```

## Broad Debug and supported Apple sanitizer

```sh
cmake --build cmake-build-mthread-debug --parallel 10
ctest --test-dir cmake-build-mthread-debug \
  --parallel 30 --output-on-failure

cmake -S . -B cmake-build-mthread-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  '-DCMAKE_C_FLAGS=-fsanitize=address -fno-omit-frame-pointer' \
  '-DCMAKE_CXX_FLAGS=-fsanitize=address -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address \
  -DCMAKE_SHARED_LINKER_FLAGS=-fsanitize=address \
  -DCREXX_VM_PROFILING=OFF -DCREXX_VM_HANDLER_PANEL=profile-20
tools/asan-run.sh --build-dir cmake-build-mthread-asan \
  --phase build --build-target persistent_worker_executor-rxvml \
  --build-target persistent_worker_executor-rxbvml \
  --build-target persistent_worker_executor-rxtvml \
  --build-leaks off --leaks off
tools/asan-run.sh --build-dir cmake-build-mthread-asan \
  --phase ctest --regex '^persistent_worker_executor-' \
  --leaks off --test-jobs 1
```
