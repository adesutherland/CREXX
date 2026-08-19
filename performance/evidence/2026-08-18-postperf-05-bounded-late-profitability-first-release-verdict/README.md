# POSTPERF-05 bounded late-profitability first Release verdict

Date: 2026-08-18

Status: **accepted by Adrian; post-acceptance closeout complete**.

## Scope

H1-T20 keeps existing inline semantic eligibility and cleanup, but adds a
bounded final profitability decision before register allocation:

- exact scalar accessors retain the dedicated POSTPERF-04 proved path;
- validated callable bodies of at most 20 structural nodes retain the ordinary
  early-inline path;
- larger call sites compare the cleaned detached expansion with the executable
  original call plus the validated callable-body summary across structural
  nodes, assignments, branches, nested calls and inline temporaries;
- a losing site retains its ordinary call without banning other sites or
  changing supported semantics.

The 20-node floor was selected over a zero floor, which lost the accepted List
path and moved RexxCPS adversely, and a 40-node floor, which recovered
materially less of the qualified large-workload expansion. H1-T20 adds no
syntax, opcode, RXBIN format, ABI or VM semantic change. RXC owns the decision;
RXAS keeps its existing mechanical/proved-flow responsibilities.

## Accepted paired verdict

Values are paired median elapsed changes for H1-T20 versus exact H0. Negative
is faster. Each pair uses the same current profiling-off Release concrete VM;
only the separately compiled image and its matching compiler-produced library
differ.

| Workload | `rxtvm` | `rxbvm` | Pairs | Result |
| --- | ---: | ---: | ---: | --- |
| DeltaBlue | -82.3060% | -82.4190% | 12 | clear favourable on both VMs |
| CD | -47.2985% | -47.1455% | 12 | clear favourable on both VMs |
| Richards | -83.9321% | -84.5985% | 12 | clear favourable on both VMs |
| List | -2.5006% | -5.0658% | 12 | favourable; mean intervals remain below zero |
| Sieve | +0.1756% | -0.0459% | 36 | noisy/inconclusive at the pair ceiling |
| RexxCPS | +0.2316% | -0.0882% | 36 | noisy/inconclusive at the pair ceiling |

Sieve's `rxtvm` mean interval still crosses the 3% individual-workload guard,
but its paired median is only +0.1756%; RexxCPS intervals cross zero but remain
below the guard. No paired median reaches the 3% adverse guard. Adrian accepted
the two capped uncertainties with the large, consistent recoveries and the
preserved List/RexxCPS controls.

Every valid observation is retained. The initial multi-workload capture
completed all cells before RexxCPS, then reported failure because the optional
RexxCPS metric prefix was wrong. RexxCPS was completed separately with the
correct elapsed contract. A required ten-record absolute-noise append covered
the whole panel. Sieve and RexxCPS then received two unchanged twelve-pair
decision appends to the 36-pair ceiling. `paired-summary.csv` deliberately
excludes the absolute-noise append; `absolute-summary.csv` includes all 22 or
46 recorded observations per cell.

## Static product result

The late decision removes the qualified large-call expansion rather than
trading it for VM work:

| Workload | RXAS instructions H0 -> H1 | Copies H0 -> H1 | Calls H0 -> H1 | Max locals H0 -> H1 | RXBIN bytes H0 -> H1 |
| --- | ---: | ---: | ---: | ---: | ---: |
| DeltaBlue | 7,009 -> 2,111 | 644 -> 88 | 141 -> 75 | 206 -> 38 | 298,934 -> 102,342 |
| CD | 5,593 -> 2,304 | 463 -> 199 | 34 -> 63 | 93 -> 54 | 212,890 -> 109,113 |
| Richards | 1,872 -> 883 | 156 -> 17 | 25 -> 33 | 56 -> 30 | 81,566 -> 44,342 |
| Havlak | 2,730 -> 1,428 | 154 -> 65 | 124 -> 65 | 114 -> 60 | 134,477 -> 77,334 |
| List | 262 -> 262 | 16 -> 16 | 15 -> 15 | 34 -> 34 | 16,920 -> 16,920 |
| Sieve | 75 -> 77 | 4 -> 3 | 0 -> 1 | 16 -> 11 | 4,892 -> 5,213 |
| RexxCPS | 1,403 -> 865 | 79 -> 58 | 22 -> 22 | 117 -> 117 | 73,689 -> 51,202 |

The exact H1 compiler-produced library is 719,809 bytes versus 907,207 bytes
for H0, a 20.66% reduction. List is byte-identical. Sieve adds 321 bytes, below
the 4 KiB artifact guard. Executable VM sizes are unchanged because both sides
use the same current runtime products.

## Compiler defects closed at the gate and during closeout

The retained HTTP helper path exposed an independent optimized-RXC defect:
binary and decimal constants returned from a non-inlined function were emitted
as illegal RXAS immediate `ret` operands. RXAS supports direct SAY/RETURN
operands only for boolean, integer, float and string constants. RXC now retains
a register for all other constant types, emits `load`, then returns that
register. Recursive binary/decimal regressions prove optimized RXAS shape and
runtime behaviour. This is a compiler correction, not an ISA extension.

Broad closeout then exposed a separate NR-26 copy fixed-point defect through
the optimized HTTP-policy test. The first pass correctly redirected a client
read to the second `sockaccept` result and skipped the now-dead copy store. A
later pass treated the preserved old physical target as the current equality
and replaced that deeper substitution, so `read_request` consumed the first
accepted client. RXC now preserves a read substitution only when it exactly
matches the source of a reaching copy definition whose store was skipped. The
focused structural regression isolates scenario 2 and proves that
`read_request` consumes the register produced by the reaching `sockaccept`;
all four optimized/unoptimized `rxtvm`/`rxbvm` HTTP modes pass. This is a
narrow fixed-point correction, not a broad optimization backoff.

## Correctness at the mandatory gate

- focused compiler/benchmark CTest: 26/26 pass;
- exact generated linked-runtime artifact target: all 735 products build;
- binary/decimal optimized structural and runtime regression: pass;
- H1 small-site admission and expansion-heavy rejection regression: pass in
  optimized/no-opt form under `rxtvm` and `rxbvm`;
- ordinary profiling-off Release target build: pass.

Post-acceptance closeout is complete:

- combined post-defect focused Debug validation: 36/36 pass;
- full ordinary Debug build and CTest: 2,251/2,251 pass in 298.29 seconds;
- full ordinary Release build: pass;
- focused Release compiler, shape, benchmark and HTTP qualification: 56/56
  pass in 46.50 seconds;
- all 17 rebuilt H1 benchmark RXAS/RXBIN and compiler-produced library
  artefacts are byte-identical to the frozen accepted set after the fixed-point
  repair.

The last identity check includes AWFY JSON in addition to the fifteen formal
verdict files. It proves that the accepted timing/static products were not
changed by the closeout correction, so no rebenchmark was required. Exact
commands and results are recorded in `COMMANDS.md` and `VALIDATION.md`.

## Host and interpretation boundary

The formal run used the Apple M5 macOS development host on AC power with no
recorded thermal or performance warning. Cells were serial and balanced. H0
was the exact pre-edit Release compiler at base commit
`51c93c421bf444de28bc52eb9a0faedc99d1ae27`; H1 was the frozen candidate.

These are reserve/diagnostic workload decisions, not a Tier A aggregate and
not a Windows/Linux qualification. The result selects bounded RXC
late-profitability; it does not authorize general loop hoisting, wholesale
late RXAS inlining or a public-format change.

## Bundle map

- `paired-summary.csv`: paired quartiles, medians, favourable counts, means and
  corrected two-sided 95% Student-t intervals;
- `paired-summary.crexx`: the Level B analyzer used to produce that summary;
- `absolute-summary.csv`: final absolute distributions and noise disposition;
- `artifact-summary.csv`, `hashes-and-sizes.txt`: exact H0/H1 identities,
  sizes and static shape;
- `*-samples.csv`, `*-outputs.csv`, `*-capture-manifest.json`: all raw initial,
  completion, noise and decision-append observations;
- `*-manifest.txt`: exact maintained-runner cell definitions;
- `release-build.log`, `focused-debug-ctest.log` and
  `linked-runtime-artifacts-build.log`: mandatory-gate build/correctness logs;
- `postflow-focused-debug-ctest.log`: 36-test combined focused replay after
  the NR-26 fixed-point correction;
- `closeout-debug-build.log`, `closeout-debug-ctest.log`: final ordinary Debug
  build and 2,251-test broad qualification;
- `closeout-release-build.log`, `closeout-release-ctest.log`: final ordinary
  Release build and 56-test focused qualification;
- `release-artifact-identity.log`: 17/17 exact H1 post-repair identity proof;
- `host-post.txt`: retained host state captured immediately after the verdict;
- `COMMANDS.md`: replay method;
- `VALIDATION.md`: post-acceptance closeout QA.
