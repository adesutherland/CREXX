# NR-14 first profiling-off Release verdict

Verdict: **ACCEPT recommended, pending Adrian's explicit artifact-size
trade-off decision.** The selected implementation remains provisional and
frozen. No broad CTest, sanitizer, packaging, cross-platform closeout,
documentation polish, commit or push followed this gate.

## Scope and provenance

- Branch: `develop`
- Starting/source HEAD: `8424587f258ac37f133adab4194a3e80a5ee0875`
- `origin/develop`: `5626d6b871d740387765de40bfbebd246471102f`
- Starting tree: clean; the retained candidate is the task's uncommitted NR-14
  scope listed by `git status`.
- Host: Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs.
- Power: AC, battery 100%, low-power mode off; no thermal or performance
  warnings before or after measurement.
- Products: ordinary `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`, Ninja 1.13.2, Apple clang 21.0.0.
- Workloads: optimized, source/TRACE-metadata-stripped linked RXBIN 007 images.
- Execution: serial. Image order was interleaved and balanced by round.
- Interpretation boundary: same-session, same-host first-verdict observation;
  not a release-wide or cross-platform claim.

The baseline uses the pre-NR-14 compiler/RXAS/RXBIN/VM implementation from the
starting revision. The candidate changes only the NR-14 production scope. Both
products use the same benchmark source and library input. Full hashes and sizes
are in `artifact-inventory.csv`.

## Selected design

The A-E comparison selected design D: three exact, direct-result opcodes over
compiler-certified frozen shapes. They avoid runtime plan-text decoding,
generic descriptor traversal and the intermediate result array:

- opcode 405, `parsewords3 R,R,R,R`;
- opcode 408, `parsepos2 R,R,R,I`; and
- opcode 409, `parsewords3d R,R,R,R`.

RXBIN 007 feature bit `0x00000002` declares the family. There is no new section,
relocation, byte-order-sensitive payload or load-time private representation.
The existing RXBIN register/integer varints carry the operands. Unsupported or
uncertain source, modifier, target and template shapes retain generic
`parseExec`.

## Minimum focused correctness

- Debug focused CTest: 11/11 pass.
- Candidate Release focused CTest: 11/11 pass.
- Optimized and no-opt compiler lowering, all three forms, numeric source
  conversion, Unicode position boundaries and a raw source/result alias pass
  in both `rxvm` and `rxbvm`.
- Assembly, disassembly, reassembly and link propagation pass.
- A missing required feature bit and an unknown feature bit fail closed with
  precise diagnostics; the adjacent NR-21 fixed-call feature contract also
  passes with the combined supported mask.
- A `TRACE REXX` source probe has byte-identical stdout and stderr across
  baseline/candidate and both VMs. Candidate RXAS contains the direct opcode.
- Generic optimized/no-opt PARSE tests and adjacent fallback forms pass.

Final source review after timing found that relative `+0` must retain generic
semantics rather than enter the positive-position primitive. Eligibility was
narrowed from nonnegative to strictly positive and a focused fallback assertion
was added. The same Debug and Release 11-test sets were rerun and pass. Rebuilt
RexxCPS, focused PARSE and Richards RXAS, RXBIN and stripped linked candidate
images are byte-identical to the measured artifacts, so no timing cell changed;
the final compiler-exit bundle hash is recorded in `artifact-inventory.csv`.

The production compiler always writes distinct internal source/result
temporaries, so the selected hot path allocates no plan or result array. The VM
handlers retain an allocation-backed snapshot only for raw bytecode that
aliases a source operand with an output; the focused contract exercises that
compatibility path.

## First Release results

Candidate-versus-baseline percentages below use `(candidate / baseline - 1) *
100`. Positive native CPS is better; negative elapsed, RSS and footprint values
are better. The headline is the paired median; intervals are two-sided 95%
Student-t intervals around the mean paired percentage.

| Lane | VM | Pairs | Paired median | Mean 95% interval | Favorable |
| --- | --- | ---: | ---: | ---: | ---: |
| RexxCPS native CPS | `rxvm` | 12 | **+45.965%** | +45.125% to +46.433% | 12/12 |
| RexxCPS native CPS | `rxbvm` | 12 | **+45.826%** | +45.441% to +46.354% | 12/12 |
| RexxCPS process elapsed | `rxvm` | 12 | **-31.466%** | -31.679% to -31.065% | 12/12 |
| RexxCPS process elapsed | `rxbvm` | 12 | **-31.399%** | -31.644% to -31.213% | 12/12 |
| focused frozen PARSE elapsed | `rxvm` | 12 | **-97.248%** | -97.313% to -97.167% | 12/12 |
| focused frozen PARSE elapsed | `rxbvm` | 12 | **-97.445%** | -97.478% to -97.410% | 12/12 |
| Richards drift control elapsed | `rxvm` | 36 | -0.201% | -0.284% to +0.137% | 23/36 |
| Richards drift control elapsed | `rxbvm` | 12 | -0.644% | -1.110% to -0.265% | 10/12 |

The focused PARSE `rxvm` candidate absolute cell crossed the 10% span rule, so
ten unchanged-condition samples were appended. Its 22-sample absolute median is
0.046126 s with 2.974% relative MAD; the retained 18.159% min/max span remains
labelled noisy. The paired effect is nevertheless decisive, large and favorable
in all 12 pairs.

The byte-identical Richards control's initial `rxvm` interval crossed zero. Two
12-pair balanced append blocks reached the 36-pair cap and remain
noisy/inconclusive, centered near zero. This is not evidence of an unrelated
regression. The `rxbvm` control is slightly favorable.

## Lifecycle, memory and artifacts

One-repetition process-inclusive lifecycle is below 5 ms and remained noisy.
The three cells that crossed the absolute noise rule received ten serial append
samples; both paired VM series then reached the 36-pair cap. Paired elapsed
medians are -0.983% (`rxvm`) and -0.991% (`rxbvm`), but both mean intervals
cross zero. They are noisy/inconclusive, not regressions, and are far below the
lifecycle guard's simultaneous 5% and 1 ms boundary.

Paired peak RSS medians are -0.463% (`rxvm`) and -0.236% (`rxbvm`); peak memory
footprint medians are -1.526% and -1.626%. No memory guard is hit. The selected
compiler-emitted path has no per-PARSE plan/result-array allocation.

| Artifact | Baseline | Candidate | Delta | Guard |
| --- | ---: | ---: | ---: | --- |
| `rxcexits.rxbin` | 1,210,849 B | 1,342,332 B | **+131,483 B (+10.859%)** | **hit** |
| `rxbvm` | 882,840 B | 899,352 B | +16,512 B (+1.870%) | no |
| RexxCPS unlinked RXBIN | 79,357 B | 80,406 B | +1,049 B (+1.322%) | no |
| RexxCPS stripped linked RXBIN | 209,273 B | 195,599 B | **-13,674 B (-6.534%)** | no |
| focused PARSE RXAS | 80,985 B | 85,722 B | +4,737 B (+5.849%) | **hit** |
| focused PARSE stripped linked RXBIN | 23,004 B | 23,852 B | +848 B (+3.686%) | no |
| Richards stripped linked RXBIN | 16,358 B | 16,358 B | 0 B | no |

The `rxcexits.rxbin` increase exceeds the simultaneous 5%/4 KiB artifact
guard. The focused benchmark RXAS also crosses that threshold, although its
linked executable image does not. The performance gain, linked-image reduction
on the real hot workload, neutral lifecycle and lower RSS support ACCEPT, but
governance requires Adrian to explicitly accept, revise or reject the product
artifact trade-off before closeout.

## Evidence map

- `raw/poc-a-e.csv` and `raw/poc-array-vs-direct-copy.csv`: guarded design
  comparison.
- `raw/*paired*.csv`: formal paired observations, warmups and bounded append
  blocks.
- `absolute-summary.csv`: required absolute-cell distribution statistics.
- `paired-summary.csv`: paired distribution, favorable counts and t intervals.
- `artifact-inventory.csv` and `artifact-deltas.csv`: exact hashes, sizes and
  guard classification.
- `focused-checks/`: Debug and candidate Release focused CTest output.
- `trace-probe/`: baseline/candidate, both-VM TRACE/source behavior output.
- `commands.md`: build, link, run, sampling and analysis commands/contracts.
- `host-state.md`: pre/post environment and build configuration.
- `compatibility.md`: semantic, RXBIN and observability contract.
