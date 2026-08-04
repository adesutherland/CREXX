# PERF3-11 D0.6 bounded peephole-first verdict

Status: **accepted and complete**

## Scope and provenance

- Source control: `23ac11330e63b977d01035a39c0fb2de46423e3c` on
  `codex/perf3-rxas-flow-infrastructure`.
- Candidate dirty scope: `OPTIMISER_TARGET_MAX_QUEUE_SIZE` changes from 20 to
  100 plus the D0.6 control-plane and architecture documentation.  No other
  production code differs.
- Control: a fresh detached worktree at the exact source commit, ordinary
  profiling-off Release, queue 20.
- Candidate: the same source and build configuration with queue 100.
- Input: exact generated `benchmark_rexxcps_levelb_opt.rxas`, SHA-256
  `b575305ab154f60f378f8cdbd3a44811e66368daa894d8b4add934066726707f`.
- Host: Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs, Apple clang
  21.0.0, CMake 4.3.2 and Ninja 1.13.2.
- Power: AC attached, low-power mode off, and `pmset -g therm` reported no
  thermal or performance warning.
- Temporary full logs and exact binaries remain under
  `/private/tmp/crexx-perf3-d06.mcFyo5`.  Build outputs are not duplicated in
  this retained bundle.

Both products were configured independently with:

```text
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build <build> --target rxas --parallel 10
```

## Correctness and graph boundary

The focused Debug RXAS optimizer/flow panel passes **77/77** with queue 100.
Both Release assemblers emit the same 68,609-byte ordinary RXBIN, SHA-256
`361fd499a82cc786a1e04ca917adf965d5464ecae2729fddd04558d901c9b9da`.
The separately run `-d` images are also exact across candidates, SHA-256
`a3b8ebd6adc50864d4812245c464afba8cbcb012c7d48342d2c29e9c1298e70e`.
The ordinary and diagnostic hashes differ only because their invocations used
absolute and relative input paths respectively; that module path is retained
in RXBIN.

Every extracted census/CFG/SSA/use diagnostic is byte-identical between queue
20 and queue 100.  For the hot `main` procedure both configurations report:

| Boundary | Exact value |
| --- | ---: |
| records entering whole-procedure analysis | 1,145 |
| instructions entering whole-procedure analysis | 380 |
| CFG blocks / registers | 254 / 107 |
| instructions after semantic rewrites | 373 |
| SSA states / joins | 652 / 256 |
| storage versions / phis | 1,348 / 1,238 |
| value versions / phis | 5,698 / 2,506 |
| indexed uses / phi dependency edges | 4,561 / 28,277 |

The one-shot diagnostics were 0.57 s and 286,392,320 bytes for queue 20 versus
0.55 s and 286,441,472 bytes for queue 100.  These diagnostic elapsed values
are observations only, not the profiling-off Release verdict.

## Profiling-off Release verdict

One warmup per candidate preceded 12 balanced/interleaved recorded rounds.
No sample was removed; the 199,114,752-byte queue-100 RSS sample is retained.
All 24 images have the same hash above.

| Metric | Queue 20 | Queue 100 | Candidate change |
| --- | ---: | ---: | ---: |
| median process elapsed | 0.270 s | 0.280 s | +0.010 s / +3.70% |
| median peak RSS | 233,701,376 B | 233,627,648 B | -73,728 B / -0.032% |
| favourable elapsed pairs | — | 0/12 | all rounded samples slower |

`/usr/bin/time` reports elapsed time at 10 ms resolution here.  The result is
therefore best interpreted as a repeatable approximately 10 ms assembler cost,
not a more precise percentage claim.  Queue 100 provides belt-and-braces local
lookahead but makes the current RexxCPS graph no sparser.

The retained paired runtime context across the earlier K04 migration remains:

- `rxvm`: **46,221,991.5 -> 46,231,723.5 RexxCPS** (`+0.021%`);
- `rxbvm`: **45,149,051.0 -> 45,158,611.5 RexxCPS** (`+0.021%`).

Those numbers demonstrate no runtime regression from the migration.  D0.6
measures assembler preprocessing only and does not replace them.

## Decision boundary

The evidence supports retaining the peephole permanently and the new standing
ownership instruction.  It does not show a RexxCPS sparsity benefit from
increasing 20 to 100.  The selected 100-record candidate is provisional until
Adrian accepts the explicit approximately 10 ms assembler-cost tradeoff.

Adrian accepted the approximately 10 ms tradeoff and the permanent 100-record
bound on 2026-08-04.  The complete Debug build and broad **2,021/2,021** test
sweep pass in 284.69 seconds.

K04e remains the next separately gated production change.  It will repair the
lost in-place `ILT`/`BRF` fusion using SSA-owned ValueId, liveness, alias,
hidden-cleanup and TRACE proof rather than recreating the old tactical solver.
