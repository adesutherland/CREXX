# PERF3-11 Stage 0 clean oracle

Status: **complete — Gate 0 passes**

This bundle freezes the clean behavioural and assembler-cost oracle before the
PERF3-11 RXAS flow refactor.  It is not a new runtime benchmark baseline and
contains no production edit.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`
- Planning commit: `a13e88ceedfc0e3fb50edd2e790aa38ea430ac4e`
- Accepted code base: `2cabe88cd94e42d95281dadb8f8269b7673ca606`
- Provisional pre-refactor recovery point:
  `70719fc2d27ffcdd6cdbb87413dd97fbde21520a`
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 24 GiB RAM
- Power: AC; low-power mode off
- Thermal state: no recorded thermal, performance or CPU-power warning
- Builds: CMake/Ninja Debug and Release, `CREXX_VM_PROFILING=OFF`
- Temporary frozen binaries and full logs:
  `/tmp/crexx-perf3-11-stage0.35IBzf`

## Gate result

- The ordinary Debug and Release `rxc`, `rxas`, `rxlink`, `rxvm` and `rxbvm`
  targets build successfully.
- The focused signal/storage/trace-safe conversion matrix passes **61/61** on
  the applicable optimized/no-opt and `rxvm`/`rxbvm` paths.
- Richards and Towers RXAS/RXBIN hashes match their retained accepted C1abc
  identities.  RexxCPS RXBIN matches the accepted PERF3-10 C1 identity.
- No pre-existing correctness or generated-artifact drift blocks PERF3-11.

## Idle assembler oracle

After the build/test load settled to `vm.loadavg: { 1.39 4.09 3.57 }`, each
workload received two unrecorded warmups and ten serial high-resolution
records.  RXAS process startup is included.

| Workload | Median elapsed | Mean elapsed | Range | Peak RSS |
| --- | ---: | ---: | ---: | ---: |
| Richards | 0.051400543 s | 0.053409195 s | 0.050735950–0.061601877 s | 8,896,512 bytes |
| Towers | 0.020090103 s | 0.021103477 s | 0.018738985–0.024242878 s | 4,784,128 bytes |
| RexxCPS | 0.051542520 s | 0.052761507 s | 0.047620058–0.058758020 s | 9,797,632 bytes |

The RSS observations came from a separate ten-record `/usr/bin/time -lp`
capture after the same build/test session.  They are scaling guards rather
than formal comparative timing claims.

## Current deterministic flow scale

| Workload | Procedures | Blocks | Instructions before/after | Largest procedure | Maximum registers | Complete identity cells | Largest identity matrix |
| --- | ---: | ---: | ---: | --- | ---: | ---: | ---: |
| Richards | 24 | 354 | 1,813 / 1,808 | `richardsscheduler.runtask` (376 instructions) | 58 | 117,773 | 59,392 |
| Towers | 13 | 208 | 560 / 553 | `towersbenchmark.movedisks` (132 instructions) | 38 | 38,572 | 15,200 |
| RexxCPS | 5 | 429 | 1,246 / 1,243 | `__rxtrace_handler` (839 instructions) | 107 | 118,556 | 118,556 (`main`) |

RexxCPS `main` currently has 101 blocks, 107 registers, 378 instructions
before/375 after, 1,108 storage-analysis nodes and a dense 118,556-cell
identity environment.  The replacement must preserve the accepted three
redundant-`ITOS` removals without making this matrix the permanent component
proof representation.

Raw elapsed observations are in [`assembler-samples.csv`](assembler-samples.csv).
Hashes and exact reproduction commands are retained alongside this file.

