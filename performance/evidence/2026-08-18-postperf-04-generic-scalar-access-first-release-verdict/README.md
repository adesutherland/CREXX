# POSTPERF-04 generic scalar-access first Release verdict

Date: 2026-08-18

Status: **accepted by Adrian and closed**.

## Scope

The candidate combines two bounded, type-generic changes:

1. G1 recognizes exact monomorphic boolean/integer/float receiver-owned scalar
   getters and setters and sends them through a terminating accessor rewrite
   before the ordinary bounded inliner. It reuses existing receiver capture,
   initialization, evaluate-once, copyback, signal, TRACE and import machinery.
2. RXAS records exact component/signal contracts and successful-edge facts for
   `ASSERTINITIALIZED`, `ASSERTTYPE`, the five `ICHKRNG` forms and
   `BCHECKRANGE`. Redundant guards are removed only when every reachable fact
   proves success; signal re-entry, mutations, mismatched types/ranges and
   predicates without branch-truth facts remain conservative.

Packed `i64`/`f32` wrappers remain unchanged controls. The candidate adds no
public syntax, opcode, RXBIN format, ABI or VM semantic change.

## Integrated verdict

Values are paired median elapsed changes for the complete candidate versus the
G0 status quo. Negative is faster.

| Operation | `rxtvm` | `rxbvm` | Result |
| --- | ---: | ---: | --- |
| integer accessor read | -1.693362% | -0.949281% | clear favorable on both VMs |
| integer accessor write | +1.728776% | +0.468924% | noisy/inconclusive at 36 pairs; below guard |
| float accessor read | +0.341561% | -0.274841% | noisy/inconclusive at 36 pairs; below guard |
| float accessor write | -35.413906% | -32.747951% | clear favorable on both VMs |

All 266 recorded processes pass both checksums and the benchmark oracle. Every
valid observation is retained: 72 initial executions, 50 required absolute-
noise append executions and two 72-execution balanced pair append blocks. The
four ambiguous integrated cells reached the 36-pair ceiling. No integrated
accessor median reaches the 3% adverse guard.

The `rxtvm` direct-float-read control is a clear +2.316502% median adverse
observation, below the 3% point guard. Other direct and packed-control medians
remain below the guard. This is a focused scalar microbenchmark verdict, not a
Tier A aggregate or a claim about the transferred `BINARY-01` design.

## Guard-salvage diagnostic

Against the rejected current-compiler/pre-guard intermediate, the selected
guard proof reduces elapsed medians for integer read/write by
11.10%/12.96% on `rxtvm` and 8.80%/13.84% on `rxbvm`. Float read improves by
1.54%/2.64%. Float write is 3.74% slower on `rxtvm` and noisy at +1.11% median
on `rxbvm`, consistent with a code-layout sensitivity rather than an integrated
G0 regression: the complete candidate still retains the clear 33-35% float-
write improvement above.

## Correctness and product shape at the gate

- focused RXAS metadata/flow/guard checks: 8/8 pass;
- optimized and no-opt scalar fixture under both concrete VMs: pass;
- source/binary-import accessor regression and generated-code contract: pass;
- ordinary Release build: pass with `CREXX_VM_PROFILING=OFF` and profile-20;
- current RXBIN: 42,518 bytes versus 41,654 bytes for G0, an increase of
  864 bytes (2.07%), below both artifact guard thresholds;
- current compiler RXAS assembled without guard deletion: 602 disassembled
  instructions and 17 `ASSERTINITIALIZED`; selected RXAS: 594 instructions
  and 11 checks, with only the required computed-receiver check in `main()`.

Post-acceptance broad Debug qualification is recorded in `VALIDATION.md`.

## Accepted assembler-lifecycle trade-off

Broad qualification exposed exponential reverse-dependency recursion in the
new signal-policy resolver on the real `httpcodec.rxas` graph. The production
fix replaces that recursion with a bounded worklist and adds a 36-diamond
regression; the real assembly now completes in about 0.25 seconds instead of
running beyond the diagnostic limits.

The corrected selected assembler nevertheless has a formal median lifecycle
cost on that large input of 0.249058 seconds versus 0.231589 seconds for the
retained pre-guard assembler: +8.104172%, or about 17.5 ms, with 12/12 paired
observations adverse. This crosses the lifecycle guard. Adrian explicitly
accepted that bounded compile-time trade-off to retain the runtime accessor and
guard result and close POSTPERF-04. A complete M08 guard pass itself measured
about 1.315 ms; the residual is dominated by code-placement sensitivity. A
bounded condition reorder was neutral at +0.109720% median and was reverted.
No diagnostic mask or experimental reorder remains in production.

## Host and interpretation boundary

The formal run used the Apple M5 macOS development host on AC power with low-
power mode off and no recorded thermal/performance warning. The same profiling-
off Release `rxtvm`, `rxbvm` and library served every image. Timing cells were
serial and balanced/interleaved; no build or other benchmark ran concurrently.

This proves the selected POSTPERF-04 product on the primary macOS host. It does
not qualify Windows/Linux, redesign packed binary access or promote any reserve
benchmark into an aggregate.

## Bundle map

- `timing-samples.csv`: all 266 recorded executions, checksums and timings;
- `paired-summary.csv`: R-7 quartiles, medians, favorable counts, means and
  two-sided 95% Student-t intervals for integrated and pre-guard comparisons;
- `absolute-summary.csv`: final absolute distributions and noise disposition;
- `artifact-summary.csv`: RXAS/RXBIN hashes, sizes and code-shape counts;
- `host-pre.txt`, `host-post.txt`, `hashes-and-sizes.txt`: environment and
  exact product identity;
- `assembler-lifecycle-samples.csv`: the accepted lifecycle comparison and
  rejected neutral reorder diagnostic;
- `release-build.log`: ordinary profiling-off Release build;
- `closeout-debug-build.log`, `closeout-debug-ctest.log` and
  `closeout-release-build.log`: final complete product qualification;
- `closeout-hashes-and-sizes.txt`: final Release identities and exact
  reproduction of the accepted selected RXBIN;
- `COMMANDS.md`: replay method;
- `VALIDATION.md`: post-acceptance closeout QA.
