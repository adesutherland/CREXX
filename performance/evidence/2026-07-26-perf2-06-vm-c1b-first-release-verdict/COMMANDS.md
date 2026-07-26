# Reproduction commands

The original scratch roots are recorded in `manifests/first-verdict.txt`.
For a fresh reproduction, create a detached worktree at
`e7090198e45002a6a73b654f6d98b9eb91d2e5cb`, apply the current two-file VM-C1b
diff and configure an isolated ordinary Release product:

```sh
cmake -S CANDIDATE_SOURCE -B CANDIDATE_BUILD -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCREXX_VM_PROFILING=OFF
cmake --build CANDIDATE_BUILD --parallel 10 --target rxvm rxbvm
```

Run the checked-in Level B balanced matrix on the exact manifest:

```sh
caffeinate -i BASELINE_BUILD/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest manifests/first-verdict.txt \
  --output-dir timing/initial-12 \
  --measurement timing --warmups 1 --runs 12

BASELINE_BUILD/bin/crexx summarize_paired.crexx --nokeep --args \
  paired-summary-12.csv timing/initial-12/samples.csv
```

Capture `pmset -g batt`, `pmset -g therm` and `uptime` immediately before and
after. Do not overlap builds, tests or other measurement processes. The
clear-adverse Sieve `rxbvm` result ended the governed first-verdict block. It
was not appended; the later diagnostic cells under `diagnostics/` are named
controls, not extensions of that timing block.

After Adrian's explicit acceptance, closeout used the ordinary project build
trees and the shortest required broad validation:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --parallel 30 --output-on-failure
```

Both CTest runs passed 1,924/1,924. VM-C2 begins only after the accepted slice
and exact debt anchor are committed, in a separate clean worktree/branch.
