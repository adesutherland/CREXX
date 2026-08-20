# Commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`. Verbose build output was
redirected to `/tmp/crexx-rcc5-release-build.log`.

## Profiling-off Release build

```sh
cmake --build cmake-build-release --parallel 10 --target \
  rxbvm rxtvm float library \
  benchmark_awfy_nbody_opt_artifact \
  benchmark_awfy_nbody_noopt_artifact \
  benchmark_awfy_cd_opt_artifact \
  benchmark_awfy_cd_noopt_artifact \
  benchmark_runner_opt_artifact
```

`cmake-build-release/CMakeCache.txt` recorded `CMAKE_BUILD_TYPE=Release` and
`CREXX_VM_PROFILING=OFF`.

## Artifact identity

```sh
shasum -a 256 -c /tmp/crexx-rcc5-baseline.kjE8Pt/SHA256SUMS
shasum -a 256 \
  cmake-build-release/bin/rxbvm \
  cmake-build-release/bin/rxtvm \
  cmake-build-release/bin/rxfloat.rxplugin \
  cmake-build-release/bin/library.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_awfy_nbody_opt.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_awfy_nbody_noopt.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_awfy_cd_opt.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_awfy_cd_noopt.rxbin
```

Key SHA-256 identities:

| Artifact | SHA-256 |
|---|---|
| control `rxbvm` | `9da37ce71ac9b3ac902710113b117620feecc4c6f2bdae4d411e2598b5a2cd2c` |
| candidate `rxbvm` | `4b8c6aac80134da2747e2ea890d3b6249cc2aefafd4a40c5b0a00f314f467604` |
| control `rxtvm` | `5432c1bd16003806a7d3b05ce5af986ff631f332d51b2c55e7741564dceb6801` |
| candidate `rxtvm` | `bfb3af5d6d5addc359e7a1cd4e4fcbd9d533cf0a2f8cedf78da5639012afa8f0` |
| control `rx_rxmath.rxplugin` | `c1d47f5ce87228368cef81cf094a871e993ed021a472cd3c44ec5f7ccd496148` |
| candidate `rxfloat.rxplugin` | `cdfe786d36f63bbba66ecf794aa11249e68eb3a6e556cf66d3f98366c8fe7496` |
| shared `library.rxbin` | `2257bb1b09bdd1283c4a2480a18af124c845231871ea99f5f2676b77d3f1fbd1` |

All exact identities used by the accepted run are retained in
`artifact-sha256.txt`; `manifest.txt` retains the corresponding command cells.

## Balanced capture

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-20-rcc5b-rxfloat-first-release-verdict/manifest.txt \
  --output-dir CAPTURE_DIR \
  --measurement timing --warmups 1 --runs 12
```

The checked-in `samples.csv`, `outputs.csv`, `cell-summary.csv`, and
`capture-manifest.json` are the resulting clean-host capture.

## Pair reduction

```sh
cmake-build-release/bin/crexx \
  performance/evidence/2026-08-20-rcc5b-rxfloat-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-20-rcc5b-rxfloat-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-20-rcc5b-rxfloat-first-release-verdict/samples.csv
```

For each round, the reducer computes `(candidate - control) / control * 100`.
The interval is the paired mean plus or minus Student t(11)=2.200985 times the
sample standard error. No sample is removed.
