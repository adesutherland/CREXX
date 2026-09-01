# Provenance

## Source identity

- main repository: `/Users/adrian/CLionProjects/CREXX`
- live branch at selection: `develop`
- live HEAD: `b08611179db5ff4257c3be3103f3aeab55ea5b50`
- upstream `origin/develop`: `53f7757c5b21c15d405b17920d4cd7f6c554c46b`
- isolated candidate source: detached worktree
  `/private/tmp/crexx-perf2-0607.HOrlKC/v1r01-src`
- replay verification: detached worktree
  `/private/tmp/crexx-perf2-0607.HOrlKC/v1r01-replay`; applying the two retained
  patches to HEAD reproduced all nine changed/added candidate files byte for
  byte.
- V3 prerequisite patch SHA-256:
  `0c864c2b9d834001c83d4649b16db218a1fb0a71c0da20e8c433664102a8e04a`
- isolated V1R01 patch SHA-256:
  `eedffb6ef301cabf730ca87f6492148bdf0f3a4d7341299a31f63063f9f78c4e`

The main worktree retains the previously reported V3 correctness changes and
selection-panel documentation. V1R01 production source exists only in the
detached candidate/replay worktrees and the evidence patch. Nothing was staged
or committed.

## Builds

| Identity | Path | Configuration |
| --- | --- | --- |
| retained accepted control | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v3-fixed` | Release, `BUILD_TESTING=ON`, `CREXX_VM_PROFILING=OFF` |
| focused candidate | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v1r01-focused-debug` | Debug, `BUILD_TESTING=ON`, `CREXX_VM_PROFILING=OFF` |
| deterministic candidate counts | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v1r01-counts` | Release, `BUILD_TESTING=ON`, `CREXX_VM_PROFILING=ON` |
| ordinary candidate authority | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v1r01-release` | Release, `BUILD_TESTING=ON`, `CREXX_VM_PROFILING=OFF` |

All builds use Ninja, `/usr/bin/cc` and `/usr/bin/c++` (AppleClang
21.0.0.21000101), native ARM64, TLS `NETWORK`, legacy socket plugin off,
SQLite ADDRESS demo on, default Release flags `-O3 -DNDEBUG`, and no explicit
`CMAKE_OSX_ARCHITECTURES`. Configure logs are retained under `logs/`.

The formal comparison uses the same retained accepted `rxvm`/`rxbvm` binary on
both sides to isolate the compiler-generated image and library product. Each
cell loads its matching current or candidate linked image plus matching
`library.rxbin`. Exact hashes are in `baseline-products.sha256`,
`candidate-products.sha256`, `candidate-images.sha256`, and the checksummed
selection-panel freeze.

## Host and formal session

- MacBook Air `Mac17,3`, Apple M5, 10 logical cores (4 performance, 6
  efficiency), 24 GB;
- macOS 26.5.2 build 25F84, Darwin ARM64;
- Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2;
- AC attached, 80% battery, low-power mode off, no thermal or performance
  warning before or after;
- load averages: 2.30/3.21/3.58 before and 2.49/3.01/3.45 after.

The formal block ran serially from 2026-07-27T13:20:40Z to 13:22:35Z. It used
one warmup and 12 balanced/interleaved recorded rounds per 24 cells. All 24
warmups and 288 recorded executions passed. Raw host snapshots are retained in
`host-pre.txt`, `host-post.txt` and the redacted `host-freeze.txt`.

## Tools, inputs and output identity

- maintained Level B matrix source SHA-256:
  `77cfd1d1b4532b545d19f8313ac74567211834ca4fd7e2537a493c8f40bbceab`
- compiled matrix RXAS SHA-256:
  `78c63ea27f115dedfefe66221901ec240fccf9c781d365542379333608cccfbf`
- compiled matrix RXBIN SHA-256:
  `ce37beef18cd969ed78c8ddfe8ffb91a045a93c3b6d1f9f70f573460617a520d`
- evidence-local Level B paired reducer SHA-256:
  `7504c49e33b779aa84ca3aa49c5ec6068cb4a09c7dbca04ba7eda2e7a37e2c1e`
- formal manifest SHA-256:
  `b31f13112c482103a5c8e399a56c2e6c8ad7507b90a957b0d6458a860b8fecc8`

Candidate source, RXAS, pre-link RXBIN and linked images are retained under
`freeze/` and identified in `candidate-images.sha256`. Workload source hashes
are unchanged from the
checksum-verified selection-panel freeze. `logs/selection-panel-checksum-audit.log`
records a successful full `SHA256SUMS` verification of that package.

The first manifest omitted the candidate library. It completed five workloads
but failed the candidate RexxCPS warmup with
`rxfnsb.verify (SIGNAL FUNCTION_NOT_FOUND)`. That was a manifest construction
error, not a candidate result. Its raw failure is retained under
`timing/invalidated-missing-library/`; no sample from it enters the formal
summary.

## Claim boundary

This is one first ordinary Release verdict on one Apple ARM64 host. It is not a
production acceptance, lifecycle/RSS result, broad QA result, cross-platform
claim, final VM recommendation, PERF2-06-D01 decision, or authorization for
PERF2-08/09.
