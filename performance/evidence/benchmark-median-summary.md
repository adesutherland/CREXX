# Benchmark median summary

Status: live comparison index; qualification pilots and initial seed evidence,
not an NR-10 formal baseline

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

Do not combine rows from different dates/bundles into one median. Add a new row
for each future benchmark/run so environment drift remains visible.
