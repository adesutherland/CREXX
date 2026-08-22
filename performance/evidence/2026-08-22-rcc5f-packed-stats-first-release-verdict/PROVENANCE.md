# Provenance

## Source and build

- source revision at capture: `585dd2823c5a02fcf84ebde383c41254d2ff608f`
- branch: `develop`
- source state: dirty working tree containing the provisional RCC-5F
  implementation and accepted BINARY-01 baseline
- build: ordinary `Release`, `CREXX_VM_PROFILING=OFF`, Ninja 1.13.2,
  CMake 4.3.2, Apple clang 21.0.0
- workload: 256 packed or boxed float observations; 4,000 iterations of mean,
  sample standard deviation, sample covariance, and correlation (16,000
  provider calls per recorded process)
- direct control: the same accumulation kernel as production, invoked through
  one test-only RXPA procedure without the public object/type handling

## Host and capture

- host: `Mac17,3`, Apple M5, 10 logical CPUs, arm64
- OS: macOS 26.5.2 (25F84), Darwin 25.5.0
- authoritative matrix: 2026-08-22 08:49:01-08:49:27 UTC
- permitted unchanged `rxtvm` rerun: 2026-08-22 08:50:27-08:50:40 UTC
- power: AC attached; battery 80%
- thermal: no thermal, performance, or CPU-power warning recorded
- host state: user-declared clear for the performance run

The original matrix used two warmups and twelve balanced recorded rounds for
all twelve ordinary/offset and boxed/packed/direct cells. The unchanged rerun
used the same warmup/recorded counts for the six `rxtvm` cells. The final
boxed-versus-packed `rxtvm` verdict combines all 24 retained recorded pairs per
comparison. Direct-control reach uses the accepted final-rerun median rates.

## Artifact SHA-256

| Artifact | SHA-256 |
|---|---|
| `rxbvm` | `294701d6afbfb5ada7d084ef7c1f201b3fac2a2113b974eabce2e2a082768b04` |
| `rxtvm` | `bb2c678154bd1187f69fa395b10674ebe67fabe24302d387645bb34a9b0b4b78` |
| production `rxstats.rxplugin` | `557e607c0f8606256ec573bdee7e64049cc888e0cf1ccb33b3f54989b437ee15` |
| boxed-oracle plugin | `f21a1ca52298e1a31ab2c3e12950e8cd859f419e1ac8b3a18f2d3b669ccc3dbc` |
| direct-control plugin | `fb45e505ac0e4893f5b8078cc9cff63e9ae4144e1f1edcbcd61cf246393638a1` |
| packed workload RXBIN | `b5cb8d35d1a77f959c42bd6542c38d0c3be4204d0c62f4e5bd1c8ea5045d6d1d` |
| boxed workload RXBIN | `8d322acce6ff0565fb948a547969636acd155432c8e57fabe1064289be7ae4be` |
| direct workload RXBIN | `d71c1cb8a207c975fac6f8c143e084617be2b9d787c0d50e6056ca910c1282bb` |
| balanced matrix runner | `51a4c7982c3a4d31a00b36e991d069a411a61db1d49d51bf0d4c6e2811fa1633` |
| original manifest | `fd02cea35b1746da3c670459487d09f60c50680bfb52e9584c97ed84d045be87` |
| rerun manifest | `bedca37ab626c9fa86627b4290954c38c6248f27ffcc87b71f5c336c900faa32` |
| paired reducer | `40abfac52e6a69dd5565111c0c151e56caee52bfb88ad32989b7ab789d4aca8e` |

## Commands

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-22-rcc5f-packed-stats-first-release-verdict/input-manifest.txt \
  --output-dir performance/evidence/2026-08-22-rcc5f-packed-stats-first-release-verdict/timing-final \
  --measurement timing --warmups 2 --runs 12
```

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-22-rcc5f-packed-stats-first-release-verdict/rxtvm-final-rerun-manifest.txt \
  --output-dir performance/evidence/2026-08-22-rcc5f-packed-stats-first-release-verdict/rxtvm-final-rerun \
  --measurement timing --warmups 2 --runs 12
```

The Level B `summarize_paired.crexx` reducer combined the two retained sample
files into `paired-summary.csv`. It does not remove outliers.
