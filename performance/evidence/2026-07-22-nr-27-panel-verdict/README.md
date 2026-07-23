# NR-27 whole-procedure RXAS first Release verdict

The frozen NR-27 panel passes Adrian's mathematical-correctness plus instruction-
reduction gate. The formal wall-clock result is **no regression, but mostly
neutral/noisy**: it does not support a release-wide speedup claim.

## Construction and correctness result

- The final first-pass 19-image census is **46,469 -> 45,476 executable
  instructions**, a reduction of **993 (2.137%)** across nine images, with no
  image growth. Exact debug accounting is 979 unreachable instructions plus 14
  proved `ICOPY_REG_REG` eliminations over 154 procedures; four redirected uses
  prepare comparisons.
- The hot Richards site executes **9,179,035 -> 9,119,155 instructions** in
  each VM: **-59,880 (-0.652356%)**, with the same 23,246 queued packets, 9,297
  holds and PASS result.
- The isolated hand-RXAS panel is 32 -> 25 static instructions and executes
  28 -> 22 instructions in each VM.
- Focused structural coverage passes 10/10. Optimized and unoptimized runtime
  fixtures pass under both `rxvm` and `rxbvm` (4/4). All 708 formal executions
  returned zero and passed their benchmark-specific observable-result check.
- P3 dead numeric-result removal is rejected because a numeric write may
  release hidden reference/native-payload state. Unproved allocation,
  ownership, cursor, numeric-context and conversion cases remain deferred.

## Formal result

Both variants ran on the same ordinary profiling-off Release VM binary per
mode. Baseline and candidate linked images used their matching exact libraries;
source and TRACE metadata were retained. Negative paired elapsed percentages
are favorable. Intervals are two-sided 95% Student-t intervals around the mean.

| Workload | VM | Pairs | Paired median elapsed | Mean 95% interval | Reading |
| --- | --- | ---: | ---: | ---: | --- |
| Sieve | `rxvm` | 34 | -0.147% | -2.314% to +5.366% | noisy/inconclusive |
| Sieve | `rxbvm` | 34 | +0.172% | -5.392% to +11.666% | noisy/inconclusive |
| Permute | `rxvm` | 34 | +0.350% | -0.469% to +3.649% | noisy/inconclusive |
| Permute | `rxbvm` | 34 | -0.541% | -1.803% to +0.149% | noisy/inconclusive |
| Bounce | `rxvm` | 34 | +0.086% | -0.217% to +1.298% | noisy/inconclusive |
| Bounce | `rxbvm` | 34 | -0.906% | -1.468% to +1.052% | noisy/inconclusive |
| Richards | `rxvm` | 36 | +0.066% | -0.427% to +0.395% | noisy/inconclusive |
| Richards | `rxbvm` | 36 | **-0.238%** | **-0.705% to -0.097%** | clear small favorable result |
| Base64 | `rxvm` | 34 | +2.435% | -0.276% to +6.846% | noisy/inconclusive |
| Base64 | `rxbvm` | 34 | -0.147% | -2.586% to +6.368% | noisy/inconclusive |

The equal-weight five-workload ratio-of-medians throughput geometric mean is
**-0.552% on `rxvm`** and **+0.323% on `rxbvm`**. Neither hits the 1% common-
portfolio regression guard. No paired workload median hits the separate 3%
guard; the most adverse is Base64/`rxvm` at +2.435% elapsed. The panel therefore
survives the no-regression gate, but the overall timing verdict remains neutral.

## Sampling and interpretation

The primary capture used one warmup and 12 balanced/interleaved recorded pairs
per workload/VM. Because the absolute noise rule fired, ten unchanged pairs were
appended for Sieve, Permute, Bounce and Base64. Their intervals still crossed
zero, so one further 12-pair block took them to 34 pairs. Richards used two
12-pair decision blocks and reached the 36-pair cap. Every sample is retained.

Instruction reduction is the acceptance evidence for the panel; process time is
kept as a separate product observation. Adrian accepted the panel after the
mandatory first-Release review stop. The unchanged closeout then passed a full
Debug build and **1,885/1,885** CTests in 145.65 seconds. No sanitizer,
install/package, cross-platform or additional timing campaign was required.

## Evidence map

- `static-census.csv`: exact 19-image first-pass instruction counts.
- `artifact-inventory.csv`: exact module, library, linked-image, executable and
  driver hashes/sizes.
- `timing/`: clean one-warmup/12-pair primary capture.
- `noise-append/`: required ten-pair noise append for four workload groups.
- `decision-append-01/`: 12-pair decision append for all five workloads.
- `richards-final-append/`: final 12 Richards pairs.
- `combined/summary.csv`: consolidated absolute statistics.
- `paired-summary.csv`: paired R-7 quartiles, favorable counts and mean
  Student-t intervals.
- `common-geomean.csv`: equal-weight five-workload median-throughput result.
- `host-state.md` and `commands.md`: environment and reproducibility record.
