# Provenance

## Source and build

- branch: `temp/level-l-inline-storage`
- common source base: `269c8f1642f2bc49b3fe7abd719b6290c049c6ab`
  (`origin/develop` at branch creation)
- baseline: detached, untouched develop worktree at the common source base
- candidate: the provisional I7 receiver-storage edit on the same base
- build: ordinary `Release`, Ninja, `CREXX_VM_PROFILING=OFF`
- execution isolation: both baseline and candidate images were assembled and
  executed by the same candidate Release `rxas`, `library.rxbin`, `rxtvm` and
  `rxbvm`; only the compiler-produced dependency/main images differ
- matrix runner: unchanged maintained Level B
  `performance/tools/run_cross_runtime_matrix.crexx`

## Host and capture

- capture: 2026-08-26 21:16:18 to 21:17:28 UTC
- host: Apple M5, 10 logical CPUs, arm64
- OS kernel: Darwin 25.5.0 arm64
- power: AC attached, battery 80%, low-power mode 0
- load average: 2.51/3.41/6.88 before; 2.09/2.77/6.01 after
- competing cREXX build/test/VM processes: none observed
- thermal query: `pmset -g therm` was unavailable on this host and returned an
  IOKit capability error; no thermal-clean claim is made
- sampling: serial, two warmups and twelve balanced/interleaved recorded rounds
  for each of four cells

## Artifact SHA-256

| Artifact | SHA-256 |
|---|---|
| baseline `rxc` | `4465456eaf5bcb2b35f98b0dbfc67466320b8c119bbd0af3ec5ae5f230fcc68b` |
| candidate `rxc` | `c2c39ce5b5bfbfbc58bd4e955221ca3143ae3caa524be212f0b568aabef7ee87` |
| candidate `rxtvm` | `e3cbf26b2ab6a750a1b9d82a522c5b05e339bae3f5f29a82fe7627948952f27f` |
| candidate `rxbvm` | `30864267f562a40e48c9d9c41f0a876df91479c5838364da90b5ff4659e5a449` |
| candidate `library.rxbin` | `0429367438d8efefa7cf8b5e347ee793cbad0bbf8d9043b88a80fcf4e213d67d` |
| baseline main RXBIN | `3cd8e153bfd3d73b51b7744e85585a90f35470dc837f1585d97846c475790e5c` |
| candidate main RXBIN | `6a5f342d9c61b3ad30736014e3a21bffd08db619634b9ff8815291203484d265` |
| manifest | `715b735052c24d894f2f48befaa1d63014b02188df79f4a402529ed840f497c8` |
| matrix runner | `51a4c7982c3a4d31a00b36e991d069a411a61db1d49d51bf0d4c6e2811fa1633` |

## Timing command

The complete existing Release `crexx` driver launched the maintained Level B
runner; it is outside the timed child process. The candidate VMs and images in
`manifest.txt` are the measured commands.

```sh
caffeinate -i /Users/adrian/CLionProjects/CREXX/cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-26-level-l-inline-storage-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-26-level-l-inline-storage-first-release-verdict/timing \
  --measurement timing --warmups 2 --runs 12
```

An earlier launch through an incomplete freshly built driver stopped before
measurement because its optional `rx_treemap` module was absent. It produced no
sample and is not part of this evidence.
