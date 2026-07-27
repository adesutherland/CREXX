# Reproduction commands

The paths below record the executed scratch layout. Replays may substitute
paths but must preserve the exact base, candidate logic, profiling-off Release
configuration, inputs, work counts and balanced ordering.

```sh
cmake -S /private/tmp/crexx-perf2-06-c3.BeGrb4/worktree \
  -B /private/tmp/crexx-perf2-06-c3.BeGrb4/release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build /private/tmp/crexx-perf2-06-c3.BeGrb4/release \
  --target rxvm rxbvm --parallel 10

caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest /private/tmp/crexx-perf2-06-c3.BeGrb4/manifest.txt \
  --output-dir /private/tmp/crexx-perf2-06-c3.BeGrb4/timing-12 \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-07-27-perf2-06-vm-c2-reset-poc/tools/filter_vm_c2_pairs.crexx \
  --nokeep --args \
  /private/tmp/crexx-perf2-06-c3.BeGrb4/timing-12/samples.csv \
  /private/tmp/crexx-perf2-06-c3.BeGrb4/timing-12/c3-paired-samples.csv \
  changed-only-numeric-sync

cmake-build-release/bin/crexx \
  performance/evidence/2026-07-26-perf2-06-vm-audit/summarize_paired.crexx \
  --nokeep --args \
  /private/tmp/crexx-perf2-06-c3.BeGrb4/timing-12/c3-paired-summary.csv \
  /private/tmp/crexx-perf2-06-c3.BeGrb4/timing-12/c3-paired-samples.csv
```
