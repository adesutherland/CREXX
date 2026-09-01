# Provenance

## Source and build

- source revision: `298f412dc0e40ef12b4957df4b5f8b57a8a14d9f`
- branch: `develop`
- source state: dirty working tree containing the provisional RCC-5D/RCC-5E
  implementation plus pre-existing user work
- build: ordinary `Release`, `CREXX_VM_PROFILING=OFF`, Ninja 1.13.2,
  CMake 4.3.2, Apple clang 21.0.0
- baseline: exact pre-edit Release artifacts preserved before rebuilding, at
  the same source revision
- historical statistics provider: compiled at `-O3 -DNDEBUG` from
  `git show f95f906de^:lib/plugins/rxmath/rxmath.c` with provider ID
  `rx_rxmath`

## Host

- host: `Mac17,3`, Apple M5, 10 logical CPUs, arm64
- OS: macOS 26.5.2 (25F84), Darwin 25.5.0
- authoritative capture: 2026-08-21 12:50-12:54 UTC
- power: AC attached, low-power mode 0, battery 80%
- thermal: no thermal, performance, or CPU-power warning recorded
- load average: 2.20/2.14/2.20 before; 1.98/2.11/2.16 after
- competing cREXX build/test/VM processes: none

The user cleared the host immediately before the authoritative run.  An earlier
same-session block was superseded and was not retained in this repository
bundle.

## Accepted RCC-5D rework capture

- authoritative capture: 2026-08-21 13:31:05-13:31:44 UTC
- source/build boundary: same dirty source revision and ordinary profiling-off
  Release tree; only `rxstats`, `rxbvm`, and `rxtvm` were rebuilt after the
  accepted accumulator rework and numeric-signal length repair
- host: user-cleared, AC attached, battery 80%
- thermal: no thermal, performance, or CPU-power warning before or after
- load average: 2.52/2.14/2.24 before; 2.32/2.14/2.23 after
- competing cREXX build/test/VM processes: none
- execution: two warmups plus twelve paired balanced recorded rounds for the
  ordinary and `1e12` statistics cells on both concrete VMs; 112/112 passed

Accepted rework artifact SHA-256 values:

| Artifact | SHA-256 |
|---|---|
| candidate `rxstats.rxplugin` | `eb442f57feeee2f15c1667369e355e26c6309e1c7e31b199d3cb4f212107e0be` |
| candidate `rxbvm` | `28570ba94566c470afd343b43ccd5a49398268de0b87e1a5e72412ad7e1e95f6` |
| candidate `rxtvm` | `40d1c52fe9e3b69e324e55c27521ea8c6a999965f83a1b3a973a96037878f056` |
| rework manifest | `3d70ff08dc3a6c7ddab7c566830d033f9eae0cd5f0445c1bf043ba8a7abc2d5e` |
| paired reducer | `9046d780a4fbc9efe51dcafa87cfa9228cba58defedde37c5209e9753914110a` |

The accepted rework command was:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-21-rcc5de-provider-split-first-release-verdict/rework-input-manifest.txt \
  --output-dir performance/evidence/2026-08-21-rcc5de-provider-split-first-release-verdict/rework-timing \
  --measurement timing --warmups 2 --runs 12
```

## Artifact SHA-256

| Artifact | SHA-256 |
|---|---|
| baseline `crexx` | `a40eae65c1e0d912e090eeca858c715a857e9dc4b7c5a290d174d197dd913aa4` |
| baseline `rxbvm` | `c7723b6057e837cf589a2e0e1bd5c07f021c8e5c40bcaa20e25298214e81bf54` |
| baseline `rxtvm` | `03ac694944c7aa82e298abd72091b0b8e11849a4d9e59858b0b2cd970d3c0550` |
| baseline `rx_hash.rxplugin` | `f044b469e67cf30f9846646d85b31f091fe97d1a49c67586870589629441d16b` |
| historical `rx_rxmath.rxplugin` | `c1d47f5ce87228368cef81cf094a871e993ed021a472cd3c44ec5f7ccd496148` |
| candidate `crexx` | `006311ef1ac97334de5272b408e713ff7ef19ae513dab69f4b26c55329e32762` |
| candidate `rxbvm` | `ff67c454c38aec1476f88a8b241945bf32ac68893a1b0accdf828c60c035c629` |
| candidate `rxtvm` | `64ed4682edfe71f41b9f628cab01cbaf08b759afa4d194b1b5a7e16b49ecfc8e` |
| candidate `rx_hash.rxplugin` | `66bf1580e828f96b6cde7f3e236261b6077525e2b85db501fd88d5da5094fa0a` |
| candidate `rxstats.rxplugin` | `4f74a37b8e77af2ede3305fc564fb4e6f8ab199ab59aa46c0e58d1e03df6d856` |
| candidate statistics RXBIN | `7218c9531511daaef95c6c095807dcbcd699178f763461aa406308ec2c504916` |
| historical statistics RXBIN | `d814adc44c1d64b49da627af664e0cc12435baee3a4052a0b7dd11107ea94ba1` |
| balanced matrix runner | `51a4c7982c3a4d31a00b36e991d069a411a61db1d49d51bf0d4c6e2811fa1633` |
| paired reducer | `fd5f806406d2bab813eb0caeb4d510099e1ee64c013c570824d1efa26f7ae4a1` |

## Commands

The initial authoritative matrix used two warmups and twelve balanced recorded
rounds:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-21-rcc5de-provider-split-first-release-verdict/input-manifest.txt \
  --output-dir performance/evidence/2026-08-21-rcc5de-provider-split-first-release-verdict/clear-timing \
  --measurement timing --warmups 2 --runs 12
```

The absolute-noise append used `clear-noise-append-manifest.txt`, zero warmups,
and ten rounds.  The full paired extension reused `input-manifest.txt` with zero
warmups and twelve rounds.  The final SHA extension used
`paired-append-2-manifest.txt` with zero warmups and twelve rounds.  The Level B
reducer combined those four retained `samples.csv` files into
`clear-paired-summary.csv`.
