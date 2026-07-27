# Benchmark median summary

Status: live comparison index; qualification pilots, historical evidence and
the current 2026-07-27 PERF2-09 same-session Mac closure

This master table gives one row per dated evidence bundle and benchmark/run.
Platform cells are `median (recorded sample count)`. Process-time rows are in
milliseconds and lower is better. RexxCPS self-calibrates its executed count,
so its cross-platform comparison rows use the benchmark-native rate in millions
of nominal Rexx clauses per second (MCPS), where higher is better; comparing its
process duration would compare different amounts of work.

Displayed medians are rounded to three significant digits. This preserves
useful sub-unit RexxCPS rates while avoiding the false precision of the raw
timers; the retained sample files remain the source for exact values.

`NC` means the runtime cell ran or was attempted but is not comparable under the
common correctness/equivalence contract. `—` means the platform was outside the
bundle's scope or no valid observation exists.

| Date / evidence run | Benchmark / run | Metric | CREXX | ooRexx | Regina | NetRexx |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| 2026-07-27 / PERF2-09 Mac closure | Sieve | work/s | 5,080 / 3,760 (10) | 704 (10) | — | 2,680 (10) |
| 2026-07-27 / PERF2-09 Mac closure | Permute | work/s | 2,460 / 2,150 (10) | 307 (10) | — | 4,580 (10) |
| 2026-07-27 / PERF2-09 Mac closure | Bounce | work/s | 3,590 / 2,730 (10) | 921 (10) | — | 2,100 (10) |
| 2026-07-27 / PERF2-09 Mac closure | Richards | work/s | 2.87 / 2.84 (10) | 10.7 (10) | — | 18.2 (10) |
| 2026-07-27 / PERF2-09 Mac closure | Base64 | work/s | 1,500 / 1,510 (20) | 2,080 (10) | — | 1,810 (10) |
| 2026-07-27 / PERF2-09 Mac closure | Towers / qualified object port | process ms | 3,630 / 3,710 (10) | 1,190 (10) | — | 35.2 (10) control |
| 2026-07-27 / PERF2-09 Mac closure | RexxCPS 2.2d / disclosed versus canonical | native MCPS | 37.9 / 35.5 (10) | 38.1 (10) | 32.2 (10) | 46.0 (10)††† |
| 2026-07-23 / PERF2-01 same-session | Sieve | work/s | 5,100 / 3,860 (10) | 713 (10) | — | 2,730 (10) |
| 2026-07-23 / PERF2-01 same-session | Permute | work/s | 675 / 633 (10) | 315 (10) | — | 4,420 (10) |
| 2026-07-23 / PERF2-01 same-session | Bounce | work/s | 330 / 316 (10) | 994 (10) | — | 1,990 (10) |
| 2026-07-23 / PERF2-01 same-session | Richards | work/s | 1.74 / 1.71 (10) | 11.4 (10) | — | 18.0 (20) |
| 2026-07-23 / PERF2-01 same-session | Base64 | work/s | 1,540 / 1,640 (20) | 2,120 (10) | — | 1,830 (10) |
| 2026-07-23 / PERF2-01 same-session | RexxCPS 2.2d / disclosed versus canonical | native MCPS | 29.4 / 27.1 (10) | 40.1 (10) | 33.3 (10) | 49.3 (10)††† |
| 2026-07-15 / seed portfolio | Sieve / 50 repetitions | process ms | 24.3 (10) | — | — | — |
| 2026-07-15 / seed portfolio | Permute / 50 repetitions | process ms | 99.8 (10) | — | — | — |
| 2026-07-15 / seed portfolio | Mandelbrot / size 500 | process ms | 186 (10) | — | — | — |
| 2026-07-15 / seed portfolio | Towers / 10 repetitions | process ms | 699 (10) | — | — | — |
| 2026-07-15 / seed portfolio | RexxCPS 2.2c / canonical | process ms | 11,700 (10) | — | — | — |
| 2026-07-15 / seed portfolio | RexxCPS 2.2c / canonical | native MCPS | 0.843 (3) | — | — | — |
| 2026-07-15 / NR-02 pilot | Sieve / 50 repetitions | process ms | 31.8 (3) | 80.2 (3) | — | 38.5 (3) |
| 2026-07-15 / NR-02 pilot | Permute / 50 repetitions | process ms | 125 (3) | 203 (3) | — | 35.1 (3) |
| 2026-07-15 / NR-02 pilot | Mandelbrot / size 500 | process ms | 325 (3) | NC | — | 70.4 (3)† |
| 2026-07-15 / NR-02 pilot | Towers / 10 repetitions | process ms | 1,450 (3) | 242 (3)‡ | — | 43.2 (3) |
| 2026-07-15 / NR-02 pilot | RexxCPS / canonical | native MCPS | 0.528 (1) | 17.8 (3) | 22.8 (3) | 39.1 (3) |
| 2026-07-15 / NR-02 pilot | RexxCPS / opaque A | native MCPS | 0.535 (1) | 19.3 (2) | 18.7 (2) | 38.0 (2) |
| 2026-07-15 / NR-02 pilot | RexxCPS / opaque B | native MCPS | 0.553 (1) | 18.6 (2) | 20.7 (2) | 28.1 (2) |
| 2026-07-15 / CREXX O3 rebuild runner§ | Sieve / 50 repetitions | process ms | 23.2 (10) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild runner§ | Permute / 50 repetitions | process ms | 98.1 (10) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild runner§ | Mandelbrot / size 500 | process ms | 185 (10) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild runner§ | Towers / 10 repetitions | process ms | 688 (10) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild runner§ | RexxCPS 2.2c / canonical | process ms | 12,700 (10) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild native§ | RexxCPS / canonical | native MCPS | 0.858 (3) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild native§ | RexxCPS / opaque A | native MCPS | 0.840 (3) | — | — | — |
| 2026-07-15 / CREXX O3 rebuild native§ | RexxCPS / opaque B | native MCPS | 0.837 (3) | — | — | — |
| 2026-07-15 / CREXX O3 AC isolated | Sieve / 50 repetitions | process ms | 23.5 (10) | — | — | — |
| 2026-07-15 / CREXX O3 AC isolated | Permute / 50 repetitions | process ms | 97.1 (10) | — | — | — |
| 2026-07-15 / CREXX O3 AC isolated | Mandelbrot / size 500 | process ms | 188 (10) | — | — | — |
| 2026-07-15 / CREXX O3 AC isolated | Towers / 10 repetitions | process ms | 766 (10) | — | — | — |
| 2026-07-15 / CREXX O3 AC repeat | Towers / 10 repetitions | process ms | 771 (10) | — | — | — |
| 2026-07-15 / portfolio expansion pilot | Bounce / 100 repetitions | process ms | 356 (3) | 115 (3) | — | 32.4 (3) |
| 2026-07-15 / portfolio expansion pilot | Storage / 10 repetitions | process ms | 2,070 (3)¶ | 31.9 (3) | — | 30.6 (3) |
| 2026-07-15 / portfolio expansion pilot | List / 100 repetitions | process ms | 243 (3)‡‡ | 245 (3) | — | 31.7 (3) |
| 2026-07-15 / portfolio expansion pilot | Richards / 1 repetition | process ms | 648 (3) | 104 (3) | — | 40.9 (3) |
| 2026-07-15 / portfolio expansion pilot | JSON / 5,000 repetitions | process ms | 321 (3)†† | 650 (3)†† | — | 71.2 (3)†† |
| 2026-07-15 / portfolio expansion pilot | Base64 / 500 repetitions | process ms | 433 (3) | 278 (3) | — | 65.3 (3) |
| 2026-07-15 / lifecycle pilot | compile / translate | process ms | 74.7 (3) | 4.72 (3) | — | 428 (3) |
| 2026-07-15 / lifecycle pilot | assemble | process ms | 6.90 (3) | — | — | — |
| 2026-07-15 / lifecycle pilot | load to first result | process ms | 3.22 (3) | 8.79 (3) | — | 28.0 (3) |
| 2026-07-17 / NUMERIC-01 accepted | RexxCPS 2.2d / canonical `rxvm` | native MCPS | 1.15 (3) | — | — | — |
| 2026-07-17 / NUMERIC-01 accepted | RexxCPS 2.2d / canonical `rxbvm` | native MCPS | 1.11 (3) | — | — | — |
| 2026-07-19 / NR-09 final QA refresh | RexxCPS 2.2d / final canonical `rxvm` | native MCPS | 1.21 (12) | — | — | — |
| 2026-07-19 / NR-09 final QA refresh | RexxCPS 2.2d / final canonical `rxbvm` | native MCPS | 1.21 (12) | — | — | — |
| 2026-07-20 / NR-10 corrected equal-work | Sieve | work/s | 5,040 / 4,980 (10) | 714 (10) | — | 2,710 (10) |
| 2026-07-20 / NR-10 corrected equal-work | Permute | work/s | 647 / 642 (10) | 315 (10) | — | 4,350 (20) |
| 2026-07-20 / NR-10 corrected equal-work | Bounce | work/s | 328 / 323 (10) | 993 (10) | — | 2,010 (10) |
| 2026-07-20 / NR-10 corrected equal-work | Richards | work/s | 1.73 / 1.72 (10) | 11.4 (10) | — | 17.7 (10) |
| 2026-07-20 / NR-10 corrected equal-work | Base64 | work/s | 1,580 / 1,550 (20) | 2,130 (10) | — | 1,840 (10) |
| 2026-07-20 / NR-10 formal | RexxCPS | native MCPS | 1.23 / 1.22 (10) | 39.9 (10) | 33.2 (10) | 48.1 (10) |

The PERF2-09 cREXX cells show `rxvm / rxbvm`. Its exact common-five geometric
means are 2.125260/1.842840 versus ooRexx and 0.742985/0.644251 versus decimal
NetRexx. Richards is the largest qualified common deficit. The new Towers row
uses the PERF2-08-approved object-equivalent ooRexx port; its NetRexx cell is a
binary/JVM startup control and receives no cross-Rexx ratio. Both cREXX Base64
series remain noisy after their single governed append, with every
correctness-passing sample retained. Full identities, ratios and separate
lifecycle/RSS/artifact results are in
`2026-07-27-perf2-09-mac-closure/scorecard.md`.

The NR-09 rows are the final corrected-product Cell C medians: 1,211,556 CPS
for `rxvm` and 1,208,420.5 CPS for `rxbvm`. In the same-session paired
complete-product comparison against Cell A, the medians are +1.385% and
+2.868%, respectively; the `rxvm` 95% interval crosses zero, while the
`rxbvm` interval is wholly positive.

The NR-10 CREXX cells show `rxvm / rxbvm`. The five corrected common workloads
use one equal work argument per workload across all four cells. NetRexx uses
`options nobinary decimal`, timed `Rexx` numeric state and the default HotSpot
JIT; Base64 separately discloses its Java `byte[]` storage. NetRexx Permute and
both CREXX Base64 cells received the required ten-sample timing append and
remain labelled noisy in the scorecard. The original 0.006220/0.006149
CREXX/NetRexx aggregates used `options binary` and are withdrawn as Rexx
results; the raw version-1 files remain binary/JVM controls. The four corrected
N=5 geometric means and all separate lifecycle, RSS, artifact and control
results are in
`2026-07-20-nr-10-formal-baseline/scorecard.md`.

The PERF2-01 CREXX cells also show `rxvm / rxbvm`. Its current same-session
five-workload geometric means versus ooRexx are 0.892218 and 0.833885. Sieve
and Permute exceed 1.50x in both VMs; Base64 remains the closest deficit and
Bounce/Richards the largest. The separately disclosed cREXX RexxCPS 2.2d
ratios to canonical ooRexx Classic 2.2 are 0.732569 and 0.677217. Base64 remains
labelled noisy after its one governed append, and NetRexx Richards remains
labelled noisy after its append; all correctness-passing samples remain. Full
comparability labels and attribution are in
`2026-07-23-perf2-01-current-baseline/10-dossiers/workload-dossiers.md`.

† NetRexx Mandelbrot has a disclosed timed arithmetic-XOR/padding adaptation;
its aggregate equivalence review remains open.

‡ The ooRexx Towers time is shown as a diagnostic, but the cell is `NC` for a
common object/allocation score because stems and numeric node ids replace object
allocation and dispatch.

§ The rebuilt-O3 timing session began on battery and ended on AC. The later
`AC isolated` rows were wholly on AC. Both modes remained interactive desktop
diagnostics rather than a quiet-machine NR-10 baseline.

¶ The cREXX Storage time is shown as a diagnostic but is `NC` for the common
allocation score: each logical upstream array needs a `StorageNode` plus an
`.object[]`, so the timed allocation/object work is materially different.

‡‡ The cREXX List time uses explicit weak references and a typed-array arena to
own their targets. It remains a disclosed adaptation pending aggregate review.

†† JSON timings are native-surface diagnostics, not a common score. cREXX uses
the string/path `rxjson` API, ooRexx builds its supplied DOM and NetRexx builds
Java collections.

††† The cREXX row is the disclosed 2.2d Level B adaptation, ooRexx and Regina
use canonical Classic 2.2, and NetRexx uses its disclosed 2.2n adaptation.
These rates are visible together but are not a common-portfolio aggregate.

## Inclusion and outlier policy

The median is calculated from recorded rows only, after these gates:

1. Exclude warmups.
2. Exclude non-zero exits and failed correctness checks.
3. Exclude a sample only when an independently documented defect makes that
   observation invalid; record the exact sample and reason before recomputing.
4. Do not reject a value merely because it is the minimum, maximum, or visually
   distant from the others. Retain raw samples and let the median limit its
   influence.
5. Show a valid diagnostic time when useful, but mark the cell `NC` when its
   algorithm, numeric model, or measured language feature is not equivalent.

Current explicit exclusions are:

- every warmup row in both evidence bundles;
- ooRexx Mandelbrot size-500 and size-750 runs, because they failed the common
  checksum contract (`255` versus `191`, and `128` versus `50`);
- the exploratory NetRexx RexxCPS 2.1n observation, because it is the older
  workload version and its internally averaged timed region was only about
  0.00415 seconds; and
- trace runs, translated-image proofs and the partial CREXX trace, because they
  are diagnostic modes rather than timing samples.

No correctness-passing recorded timing sample is currently rejected solely as
an outlier. The previously noted low Regina opaque-A and NetRexx opaque-B
observations, high CREXX Mandelbrot observation, seed Permute maximum and
ooRexx Permute spread have no demonstrated external fault, so they remain in
the medians. The rebuilt-O3 Towers series are also all retained: the same image
reported medians from 688 to 771 ms across three passing series. If a fault is
later established, add its sample id and reason here before changing the table.

## Evidence sources

- `2026-07-15-seed-portfolio/samples.csv` and `rexxcps-cps.csv`
- `2026-07-15-nr-02-cross-runtime/**/pilot/samples.csv`
- `2026-07-15-nr-02-cross-runtime/mandelbrot/oorexx/` for retained negative
  correctness captures
- `2026-07-15-crexx-rexxcps-o3-rerun/` for the clean-build O3 confirmation,
  all-five runner pass and AC-only repeats
- `2026-07-15-nr-02-portfolio-expansion/` for the six added workload pilots,
  generated runtime forms and separate lifecycle phase samples
- `2026-07-17-numeric-01-first-release-verdict/accepted-c/` for the accepted
  exact-hash RexxCPS 2.2d canonical `rxvm`/`rxbvm` samples
- `2026-07-18-nr-09-large-instruction-batch-first-release-verdict/finalrun01/`
  for the final same-session A/B/C campaign, Cell C medians and paired
  complete-product comparisons
- `2026-07-20-nr-10-formal-baseline/` for the formal normalized-throughput,
  RexxCPS, lifecycle, peak-RSS, artifact and labelled native-C control evidence
- `2026-07-23-perf2-01-current-baseline/` for the current same-session common
  matrix, separately disclosed RexxCPS results, schema-5 attribution, native
  samples, allocation profiles and workload dossiers
- `2026-07-27-perf2-09-mac-closure/` for the current accepted-product
  same-session Mac common-five baseline, separately qualified Towers/RexxCPS,
  lifecycle, RSS, artifact inventory and closure dossiers

Do not combine rows from different dates/bundles into one median. Add a new row
for each future benchmark/run so environment drift remains visible.
