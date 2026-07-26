# PERF2-06 reproduction commands

The retained manifests record the exact paths used by this capture. The
scratch root for this run was `/private/tmp/crexx-perf2-06.Pn88uJ`; use a new
`mktemp -d` root for a fresh reproduction and substitute its absolute paths in
the manifests.

## Isolated products

```sh
audit_root=$(mktemp -d)
git worktree add --detach "$audit_root/baseline-src" \
  e7090198e45002a6a73b654f6d98b9eb91d2e5cb
git worktree add --detach "$audit_root/cow-src" \
  e7090198e45002a6a73b654f6d98b9eb91d2e5cb

cmake -S "$audit_root/baseline-src" -B "$audit_root/baseline-build" \
  -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF
cmake --build "$audit_root/baseline-build" --parallel 10 --target \
  rxvm rxbvm crexx language_benchmarks

git -C "$audit_root/cow-src" apply \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-07-26-perf2-06-vm-audit/poc/cow-interrupt-table.patch
cmake -S "$audit_root/cow-src" -B "$audit_root/cow-build" \
  -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF
cmake --build "$audit_root/cow-build" --parallel 10 --target \
  rxvm rxbvm language_benchmarks
```

The diagnostic builds use independent build directories and
`-DCREXX_VM_PROFILING=ON`; they are never timing products.

## Current profile census

```sh
"$audit_root/baseline-build/bin/crexx" \
  performance/tools/run_evidence_bundle.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-06-vm-audit/manifests/current-head-profile-capture.txt \
  --release-build "$audit_root/baseline-build" \
  --profile-build "$audit_root/diagnostic-build" \
  --vm rxvm --profile-mode counts --output-dir "$audit_root/current-rxvm"

"$audit_root/baseline-build/bin/crexx" \
  performance/tools/run_evidence_bundle.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-06-vm-audit/manifests/current-head-profile-capture.txt \
  --release-build "$audit_root/baseline-build" \
  --profile-build "$audit_root/diagnostic-build" \
  --vm rxbvm --profile-mode counts --output-dir "$audit_root/current-rxbvm"

"$audit_root/baseline-build/bin/crexx" \
  performance/tools/summarize_perf2_profiles.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-06-vm-audit/manifests/current-profile-summary-inputs.txt \
  --output-dir "$audit_root/profile-census"
```

The checked-in summary-input manifests intentionally retain the executed
absolute paths. A reproduction must update paths only; workload forms, work
counts and hashes remain unchanged.

## COW timing and RSS

```sh
caffeinate -i "$audit_root/baseline-build/bin/crexx" \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-06-vm-audit/manifests/cow-canonical-timing.txt \
  --output-dir "$audit_root/cow-timing-12" \
  --measurement timing --warmups 1 --runs 12

caffeinate -i "$audit_root/baseline-build/bin/crexx" \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-06-vm-audit/manifests/cow-canonical-timing.txt \
  --output-dir "$audit_root/cow-timing-append-24" \
  --measurement timing --warmups 1 --runs 24

"$audit_root/baseline-build/bin/crexx" \
  performance/evidence/2026-07-26-perf2-06-vm-audit/summarize_paired.crexx \
  --nokeep --args "$audit_root/cow-paired-36.csv" \
  "$audit_root/cow-timing-12/samples.csv" \
  "$audit_root/cow-timing-append-24/samples.csv"

caffeinate -i "$audit_root/baseline-build/bin/crexx" \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-26-perf2-06-vm-audit/manifests/cow-canonical-timing.txt \
  --output-dir "$audit_root/cow-rss-5" \
  --measurement rss --warmups 0 --runs 5
```

Capture `pmset -g batt`, `pmset -g therm` and `uptime` before and after each
timing block. Do not overlap builds, tests or measurement processes.

## Native sample and size checks

```sh
"$audit_root/baseline-build/bin/rxvm" \
  "$audit_root/baseline-build/tests/benchmarks/benchmark_awfy_list_opt.rxbin" \
  "$audit_root/baseline-build/bin/library.rxbin" -a 6000 >/tmp/list.out 2>&1 &
sample_pid=$!
/usr/bin/sample "$sample_pid" 2 1 -file /tmp/list-rxvm.sample
wait "$sample_pid"

otool -l "$audit_root/baseline-build/bin/rxvm"
nm -n "$audit_root/baseline-build/bin/rxvm"
```

Repeat the sample for baseline/COW and `rxvm`/`rxbvm`; keep each workload and
output separately named. Use the `__text` section size from `otool` and the
distance from `_run` to the next text symbol from sorted `nm` output.

## Cross-platform completion protocol

On Linux ARM64 and Linux x86-64, configure independent profiling-off Release
products for supported GCC and Clang, update only absolute manifest paths, and
run the same canonical matrix. For at least Permute, List, Base64 and Sieve,
retain:

```sh
perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses \
  -- VM IMAGE LIBRARY -a WORK
perf record -- VM IMAGE LIBRARY -a WORK
perf report
```

Add supported L1 instruction-cache and iTLB events reported by `perf list`;
record unavailable events rather than substituting invented values. On Windows
x86-64 use the repository workflow-supported MSYS2 GCC/Ninja configuration,
the same paired timing manifest, focused semantic tests, peak RSS and text
sizes. Windows hardware-counter evidence is optional only when the supported
host tooling cannot provide it; the blocker must be explicit.

## Package verification

```sh
"$audit_root/baseline-build/bin/crexx" \
  performance/tools/inventory_performance_artifacts.crexx --nokeep --args \
  --checksums-root \
  performance/evidence/2026-07-26-perf2-06-vm-audit
```
