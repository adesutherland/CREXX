# Commands and reduction contract

All relative paths are from `/Users/adrian/CLionProjects/CREXX`. Verbose build
output was redirected to a temporary log.

```sh
cmake --build cmake-build-release --parallel 10 \
  > /tmp/nr26-panel-release-build.XXXXXX.log 2>&1
```

The candidate cache was checked for `Release`, `-O3 -DNDEBUG` and
`CREXX_VM_PROFILING=OFF`. The retained accepted baseline was the existing
`cmake-build-nr14-candidate` product. Its `rxc` hash was checked against the
accepted NR-14 evidence before reuse.

Each optimized workload module was linked with its matching product and
library. The following is the exact reproducible form; `name/short` pairs were
`awfy_mandelbrot/mandelbrot`, `awfy_permute/permute`,
`awfy_richards/richards`, `awfy_towers/towers`,
`base64_roundtrip/base64`, and `rexxcps_levelb/rexxcps`.

```sh
cmake-build-nr14-candidate/bin/rxlink -s \
  -o /tmp/nr26-panel-verdict.XXXXXX/baseline-SHORT.rxbin \
  cmake-build-nr14-candidate/tests/benchmarks/benchmark_NAME_opt.rxbin \
  cmake-build-nr14-candidate/bin/library.rxbin

cmake-build-release/bin/rxlink -s \
  -o /tmp/nr26-panel-verdict.XXXXXX/candidate-SHORT.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_NAME_opt.rxbin \
  cmake-build-release/bin/library.rxbin
```

The image inventory was reproduced after capture from those inputs with exact
hash equality. Both variants then ran under the same current Release VMs.

Initial block:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/input-manifest.txt \
  --output-dir performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12
```

The paired intervals crossed zero, so governance required two unchanged
append blocks, each using the same command with `--warmups 0 --runs 12` and
output directories `timing-append-01` and `timing-append-02`.

The repository Level B runner consolidated all retained samples:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/input-manifest.txt \
  --output-dir performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/timing-consolidated-36 \
  --measurement timing --summary-only \
  --samples performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/timing/samples.csv \
  --samples performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/timing-append-01/samples.csv \
  --samples performance/evidence/2026-07-21-nr-26-panel-first-release-verdict/timing-append-02/samples.csv
```

Paired reduction uses all 36 block-qualified rounds for each workload/VM,
R-7 quartiles, per-pair `(candidate / baseline - 1) * 100`, and a two-sided
Student-t interval around the arithmetic mean with critical value 2.030108 for
35 degrees of freedom. Native RexxCPS rate uses the same ratio with positive
as favorable. No sample was removed or replaced.
