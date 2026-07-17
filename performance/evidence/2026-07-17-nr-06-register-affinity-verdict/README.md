# NR-06 register-affinity first Release verdict

Status: **accepted and retained; implementation verified; executed reductions
found outside RexxCPS; SWAP cost is too small for material product benefit**.

## Scope and provenance

- Source branch/HEAD: `develop` at `5e5e3b397`, with the NR-06
  affinity-guided register-numbering/window-placement PoC uncommitted. The
  rejected specialist-loop extension was removed before either compiler was
  built.
- Product: ordinary profiling-off Release build
  (`CREXX_VM_PROFILING=OFF`, `cmake --build cmake-build-release --parallel
  32`), completed successfully.
- Baseline and candidate: exact retained Release `rxc` binaries built from the
  same source state with only the affinity preassignment and exact-window
  selection disabled/enabled. Both generated modules were assembled and
  linked with the same current Release `rxas`, `rxlink`, and preserved
  `library-common.rxbin`.
- Workload: canonical optimized `tests/benchmarks/rexxcps_levelb.crexx`,
  default 100 x 100 contract with no argv.
- Sampling: one warmup per image/VM, then six round-interleaved recorded
  samples per image/VM. Order alternated inside each VM, so each variant
  preceded the other exactly three times per VM. All 28 executions passed the
  canonical provenance and `PASS: RexxCPS 2.2c cREXX port` marker.

## Candidate and correctness boundary

The PoC preassigns distinct procedure-local symbols into lexical call-affinity
groups. Call allocation reuses such a window only when occupied slots are the
exact argument symbols and every remaining required slot is free; otherwise it
rolls back and calls the existing wholly-free contiguous allocator. It does not
merge live ranges, change the call ABI, alter RXBIN, or require a VM change.

The strengthened structural fixture changes 20 optimized `SWAP`s to two while
retaining one repeated-source snapshot and its incompatible-window
swap/restore fallback. Exact optional, string, and reference arguments are
covered. No-opt remains at 20 swaps. Debug and Release checks covered duplicate
arguments, object/reference behavior, optional arguments, interface and method
calls, both signal-unwind VMs, and dynamic/static native unwind. The final
Release focused set passed 25/25.

## Static result

Across the exact retained 11-source portfolio, the candidate removes 58
`SWAP`s (423 to 365) while summed `.locals` rises by five (1,317 to 1,322).
RexxCPS supplies 44 of the removed swaps (121 to 77), its summed `.locals`
falls from 181 to 180, and its stripped linked image is 128 bytes smaller
(210,538 to 210,410 bytes). No runtime instruction is added for an exact
argument.

## Release timing result

Higher benchmark-native CPS is better; lower process elapsed time is better.
Host throughput varied heavily during the experiment, so the same-round paired
median is the primary view.

| VM | Baseline median CPS | Candidate median CPS | Cell-median CPS change | Paired-median CPS change | Paired-median elapsed change |
|---|---:|---:|---:|---:|---:|
| `rxvm` | 1,056,523.5 | 1,011,738.0 | -4.239% | -1.928% | +1.962% |
| `rxbvm` | 955,212.5 | 970,319.0 | +1.581% | +1.676% | -1.647% |

The VM modes disagree, individual paired CPS changes range from -7.299% to
+7.996%, and baseline throughput ranges from 818,007 to 1,158,376 CPS over the
run. This does not provide reliable evidence that removing 44 static RexxCPS
swaps improves the ordinary product. It also does not establish a stable
regression; order and host drift are larger than the candidate signal.

The mandatory first Release gate therefore did not establish the bounded
greedy implementation as a material performance win. The implementation stayed
provisional while the causal audit below established whether the expected work
reduction was real. Raw per-cell manifests and samples, exact images, the
resumable interleaved runner, build logs, and summaries are retained here.

## Follow-up causal audit

The follow-up audit verified that the correct distinct linked images ran and
that RXAS/linking preserve exactly the compiler's 44-instruction RexxCPS
reduction. All 44 removed instructions are in the unexecuted trace handler, so
both exact RexxCPS images execute 484,376 swaps: the original timing comparison
had no executed SWAP difference to measure.

Across the rest of the portfolio, four workloads do remove 248,362 executed
swaps at their retained bounded argv. A direct 100,000,000-iteration RXAS
control experiment measures an ordinary profiling-off Release swap at only
0.434 ns in `rxvm` and 0.706 ns in `rxbvm`; the profiling build reports 13--14
ns because it times every instruction. The implied end-to-end opportunity is
only 0.000108%--0.040474%. A compact 64-execution order-balanced Release check
is, as expected, dominated by much larger host noise and splits direction.

The causal result is therefore stronger than the first timing verdict: the
implementation works, but avoiding these cheap pointer-swap dispatches is not
a material product-speed opportunity. Adrian nevertheless selected the bounded
implementation for retention because it verifiably removes static and executed
work without adding runtime instructions. Full details and raw profiles are in
[`audit/`](audit/).

## Accepted closeout

The retained implementation rebuilt in Debug and ordinary profiling-off
Release, with 26/26 focused selections, including the linked-runtime fixture,
passing in each tree. Seven pre-existing
optimized compiler goldens intentionally changed register layout and together
remove another 86 static swaps while every other opcode count remains
unchanged; every corresponding runnable fixture passed.
After accepting those outputs through the documented `--update-gold` path, the
focused compiler/runtime set passed 16/16. The complete Debug build passed and
the final broad Debug suite passed 1,849/1,849 at parallelism 30 in 331.21
seconds. The initial seven-golden failure log is retained alongside the clean
rerun.

NR-07 direct-condition and specialist-loop lowering remain rejected and
removed. No sanitizer, install/package, cross-platform or additional timing
campaign was added beyond the approved closeout scope.
