# NR-21 first production Release verdict

Verdict: **ACCEPTED** by Adrian on 2026-07-20.

This is the smallest decisive ordinary profiling-off Release comparison for
the approved production `CALL1...CALL4` implementation. Negative timing deltas
mean that the candidate is faster.

## Product provenance

- Baseline source: clean detached worktree at
  `5626d6b871d740387765de40bfbebd246471102f`.
- Candidate source: the same commit plus the provisional NR-21 production
  changes; reported version `crexx-1.0.0-beta.3+local.g5626d6b871d7.dirty`.
- Baseline build: `/tmp/crexx-nr21-baseline-build.kzJjZz`.
- Candidate build: `/tmp/crexx-nr21-candidate-build.bR2xbW`.
- Both builds: CMake 4.3.2, Ninja, AppleClang 21.0.0.21000101,
  `Release` (`-O3 -DNDEBUG`), `CREXX_VM_PROFILING=OFF`, and
  `CREXX_BUILD_SQLITE_ADDRESS_DEMO=OFF`.
- Both builds used DSL-Syntax-Highlighter commit
  `a64c99385aa48022087d7749dbd7f15d6548d3e0`.
- The pre-existing repository Release tree identified itself as the stale
  `g1596d7` product, so it was not used as the baseline.

The two products were configured equivalently with:

```text
cmake -S <source> -B <build> -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DCREXX_BUILD_SQLITE_ADDRESS_DEMO=OFF
```

The focused product targets were `rxc`, `rxas`, `rxlink`, `rxvm`, `rxbvm`,
the runtime library, and `compiler_exit`.

## Correctness gate

- Minimum focused Debug CTest: 17/17 passed, covering the new NR-21 contract,
  retained NR-06 behavior, optimized/no-opt runtime cells, signal behavior,
  RXBIN formatting, disassembly and opcode metadata.
- Pre-timing ordinary Release smoke matrix: 16/16 passed across four workloads,
  baseline/candidate, and `rxvm`/`rxbvm`.
- Candidate linked images carry RXBIN 007 fixed-call feature bit 0; baseline
  images have a zero feature word.
- A saved old VM/disassembler rejects feature-bearing candidate images. The
  new product reads old zero-feature images and precisely rejects both a
  fixed-call opcode with a missing feature bit and unknown feature bits.

## Measurement method

Each workload was compiled, assembled, and stripped-linked (`rxlink -s`) by
its own product. The checked-in Level B timing runner then executed one warmup
and 12 recorded serial rounds, with the baseline/candidate order rotated by
workload. All recorded cells passed correctness and every summary cell has
`rerun_recommended=no`.

The host was Darwin 25.5 arm64 on an Apple M5 with 10 logical CPUs, connected
to AC power at 100% battery with low-power mode disabled. The run was from
16:33 to 16:35 UTC. No thermal or performance warning was reported; load
averages moved from 3.07/2.97/2.55 before to 2.75/2.94/2.60 after.

## Paired result

| Workload | Work | `rxvm` paired median | `rxbvm` paired median | Verdict reading |
| --- | ---: | ---: | ---: | --- |
| List | 100 | -6.015255% | -5.784389% | clear positive |
| Permute | 50 | -3.848622% | -3.223565% | clear positive |
| Richards | 1 | +0.024715% | -0.332876% | neutral |
| JSON | 5,000 | -0.904035% | -1.983448% | high-arity control positive/no regression |

The primary paired result therefore shows a repeatable gain on both targeted
call-heavy workloads, no meaningful Richards change, and no unrelated JSON
regression.

## Linked-image size

| Workload | Baseline bytes | Candidate bytes | Delta |
| --- | ---: | ---: | ---: |
| List | 7,274 | 7,130 | -144 (-1.980%) |
| Permute | 3,853 | 3,813 | -40 (-1.038%) |
| Richards | 16,646 | 16,414 | -232 (-1.394%) |
| JSON | 39,871 | 39,783 | -88 (-0.221%) |

## Decision boundary

This evidence supported **ACCEPT** for the approved fixed direct-call
production design at the mandatory first Release stop. Adrian subsequently
accepted the verdict and approved the shortest QA/documentation closeout.
That closeout passed complete Debug build, focused 17/17 and final broad CTest
1,871/1,871; its concise audit is in [`qa-closeout/README.md`](qa-closeout/README.md).
Sanitizer, packaging/install, cross-platform and follow-on performance work
remain outside the accepted closeout scope.

Raw serial samples, paired deltas, runner logs, configure/build logs, smoke
logs, exact images, concise summaries, provenance, and checksums are retained
beside this file.
