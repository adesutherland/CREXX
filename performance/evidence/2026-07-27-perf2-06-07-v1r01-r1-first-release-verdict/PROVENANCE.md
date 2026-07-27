# Provenance

## Source and worktree

- live repository: `/Users/adrian/CLionProjects/CREXX`, branch `develop`;
- live/detached candidate HEAD: `b08611179db5ff4257c3be3103f3aeab55ea5b50`;
- upstream `origin/develop`: `53f7757c5b21c15d405b17920d4cd7f6c554c46b`;
- isolated candidate source:
  `/private/tmp/crexx-perf2-0607.HOrlKC/v1r01-r1-src`;
- combined replay patch SHA-256:
  `9fc376d1a24b06a11196bee9e633048546890181e58f8e31dcd697d4c91b8331`;
- V3 prerequisite patch SHA-256:
  `0c864c2b9d834001c83d4649b16db218a1fb0a71c0da20e8c433664102a8e04a`.

The main worktree retains the previously reported V3 correctness change and
programme documentation. V1R01-R1 production source exists only in the
detached candidate and retained patch. Nothing was staged, committed or pushed.

## Independently named builds

| Identity | Path | Configuration |
| --- | --- | --- |
| retained accepted control | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v3-fixed` | Release, profiling off |
| focused candidate | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v1r01-r1-focused-debug` | Debug, testing on, profiling off |
| deterministic counts | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v1r01-r1-counts` | Release, testing on, profiling on |
| ordinary verdict authority | `/private/tmp/crexx-perf2-0607.HOrlKC/build-v1r01-r1-release` | Release, testing on, profiling off |

All use Ninja, AppleClang 21.0.0.21000101, native ARM64, TLS `NETWORK`, default
Release `-O3 -DNDEBUG`, and no explicit `CMAKE_OSX_ARCHITECTURES`. The ordinary
authority cache records `CREXX_VM_PROFILING:BOOL=OFF`. Its six generated RXAS
files are byte-identical to the deterministic-count product. Formal current and
candidate images execute under the same retained accepted `rxvm`/`rxbvm`
binaries; each loads its matching linked image and matching `library.rxbin`.

## Host and session

- MacBook Air `Mac17,3`, Apple M5, 10 logical cores, 24 GB;
- macOS 26.5.2 build 25F84, Darwin ARM64;
- Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2;
- AC attached at 80%, low-power mode off, no thermal/performance warning;
- formal blocks ran from 2026-07-27T14:24:51Z through 14:32:18Z;
- load averages moved from 4.75/5.37/4.46 to 3.27/3.71/3.95. Persistent
  `avconferenced`/WindowServer load is disclosed in the raw host snapshots;
  the comparison uses balanced/interleaved same-session pairs.

Sampling comprises one warmup per 24 cells, 12 initial paired rounds and two
unchanged 12-pair governance appends. All 24 warmups and 864 recorded
executions passed. No sample was removed or replaced.

## Claim boundary

This package supports only the Apple ARM64 first-Release accept/rework/revert
decision for this exact candidate. Lifecycle, RSS, broad QA, packaging,
retained-RXBIN compatibility and cross-platform validation remain unstarted
until acceptance.
