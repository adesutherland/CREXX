# Replay commands

The current ordinary Release products and optimized artifacts were refreshed
without changing the source tree:

```sh
cmake --build cmake-build-release \
  --target linked_opt_runtime_artifacts rxtvm rxbvm --parallel 10
```

The initial formal absolute block used the maintained Level B runner:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest "$BASELINE_ROOT/manifest.txt" \
  --output-dir "$BASELINE_ROOT/timing" \
  --measurement timing --warmups 2 --runs 10
```

The four cells selected mechanically by the noise rule received the only
permitted append:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest "$BASELINE_ROOT/noise-append-manifest.txt" \
  --output-dir "$BASELINE_ROOT/noise-append" \
  --measurement timing --warmups 0 --runs 10
```

The initial and append samples were merged without removing an observation:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest "$BASELINE_ROOT/manifest.txt" \
  --output-dir "$BASELINE_ROOT/merged" \
  --measurement timing --summary-only \
  --samples "$BASELINE_ROOT/timing/samples.csv" \
  --samples "$BASELINE_ROOT/noise-append/samples.csv"
```

Pre/post state captured `git status`, source commit, compiler/CMake/Ninja and
CMake-cache identities, `uname`, CPU/cache topology, AC/low-power/thermal/load
state, overlapping build/test/VM processes, product symlink selection and
SHA-256 hashes for both VMs, the library, all seven workload images, the
manifest and the matrix runner.
