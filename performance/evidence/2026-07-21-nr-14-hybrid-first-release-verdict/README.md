# NR-14 hybrid first profiling-off Release verdict

Verdict: **ACCEPTED.** On 2026-07-21 Adrian accepted the hybrid and explicitly
approved the compiler-exit/RXAS artifact trade-off, full closeout and a local
commit without push. The first-verdict measurements below remain the immutable
decision evidence; `qa-closeout/` records the later broad validation.

## Scope

- Branch/HEAD: `develop` at
  `8424587f258ac37f133adab4194a3e80a5ee0875`; the NR-14 changes are
  uncommitted.
- Products: ordinary profiling-off Release, `-O3 -DNDEBUG`, on Apple M5
  Darwin arm64.
- Baseline: pre-NR-14 compiler, assembler, RXBIN and VM products retained from
  the same starting revision.
- Candidate: the original exact D family plus one compact prepared-plan opcode.
- Execution: serial, balanced by round; round 1 is warmup and rounds 2-13 are
  the 12 recorded pairs. Richards was extended unchanged to the 36-pair noise
  cap.

## Selected hybrid

The original exact opcodes remain the preferred lowering for their proven
shapes:

- 405 `parsewords3 R,R,R,R`;
- 408 `parsepos2 R,R,R,I`; and
- 409 `parsewords3d R,R,R,R`.

Longer odd implicit-word templates chain opcode 405 because the guarded
comparison found it materially faster than a result vector. Other mechanically
frozen templates use new opcode 410, `parseplan R,R,S`, with a compact immutable
little-endian descriptor and a reusable result vector. Logging/TRACE and
explicit `INTO` retain `parseExec`. Regex is unchanged.

The six-round, 300,000-repetition output-strategy PoC measured medians of
1.685/1.645 s for `parseExec`, 0.230/0.230 s for the prepared vector, and
0.060/0.060 s for the exact chain (`rxvm`/`rxbvm`). Thus the prepared form is
about 86% lower elapsed than `parseExec`, while the exact chain is about 74%
lower again for the narrower eligible shape.

## Correctness gate

- Focused Debug: 13/13 pass.
- Focused candidate Release: 13/13 pass.
- The 140-case PARSE semantic fixture passes optimized and unoptimized on both
  `rxvm` and `rxbvm`.
- Added longer odd/even templates, literals including Unicode, absolute and
  relative positions, compact drops, repeated targets, modifier handling,
  source aliasing and fallback assertions.
- Exact/chained/prepared selection is asserted in compiler-exit tests.
- Assembly, disassembly, reassembly, link propagation and both RXBIN feature
  checks pass. Opcode 410 without feature bit `0x00000002` and malformed
  descriptors fail closed.
- The measured generic images reproduced byte-for-byte when rebuilt at their
  canonical paths after final focused validation.

## First Release results

Candidate-versus-baseline percentages use `(candidate / baseline - 1) * 100`.
Negative elapsed is favorable; positive CPS is favorable.

| Lane | VM | Pairs | Paired median | Mean 95% interval | Favorable |
| --- | --- | ---: | ---: | ---: | ---: |
| generic frozen PARSE elapsed | `rxvm` | 12 | **-90.757%** | -90.834% to -90.618% | 12/12 |
| generic frozen PARSE elapsed | `rxbvm` | 12 | **-90.744%** | -90.821% to -90.655% | 12/12 |
| exact frozen PARSE elapsed | `rxvm` | 12 | **-97.262%** | -97.351% to -97.233% | 12/12 |
| exact frozen PARSE elapsed | `rxbvm` | 12 | **-97.423%** | -97.462% to -97.391% | 12/12 |
| RexxCPS native CPS | `rxvm` | 12 | **+45.249%** | +44.440% to +47.185% | 12/12 |
| RexxCPS native CPS | `rxbvm` | 12 | **+45.499%** | +44.954% to +47.102% | 12/12 |
| Richards control elapsed | `rxvm` | 36 | -0.193% | -0.242% to +0.388% | 22/36 |
| Richards control elapsed | `rxbvm` | 36 | -0.020% | -0.209% to +0.232% | 18/36 |

The Richards images are byte-identical. Both capped intervals span zero and are
centered within 0.1% at the mean, so this is neutral/inconclusive noise rather
than evidence of an unrelated regression.

One-repetition process lifecycle is also favorable: paired elapsed medians are
-5.439%/-3.966%, maximum RSS -6.512%/-5.201%, and peak memory footprint
-19.228%/-15.445% (`rxvm`/`rxbvm`). The large memory reduction reflects the
much smaller stripped generic linked image; the sub-6 ms elapsed cells remain
startup-noisy.

## Artifact decision

| Artifact | Baseline | Candidate | Delta | Guard |
| --- | ---: | ---: | ---: | --- |
| `rxcexits.rxbin` | 1,210,849 B | 1,460,028 B | **+249,179 B (+20.579%)** | **hit** |
| `rxvm` | 899,224 B | 916,168 B | +16,944 B (+1.884%) | no |
| `rxbvm` | 882,840 B | 899,768 B | +16,928 B (+1.917%) | no |
| generic RXAS | 106,013 B | 112,723 B | **+6,710 B (+6.329%)** | **hit** |
| generic unlinked RXBIN | 32,741 B | 35,248 B | +2,507 B (+7.657%) | no, below 4 KiB |
| generic stripped linked RXBIN | 21,065 B | 7,440 B | **-13,625 B (-64.681%)** | favorable |
| exact focused RXAS | 80,985 B | 86,773 B | **+5,788 B (+7.147%)** | **hit** |
| exact focused stripped RXBIN | 23,004 B | 23,940 B | +936 B (+4.069%) | no |
| RexxCPS stripped linked RXBIN | 209,273 B | 195,599 B | **-13,674 B (-6.534%)** | favorable |
| Richards stripped linked RXBIN | 16,358 B | 16,358 B | 0 B | no |

The compiler-exit bundle and textual RXAS increases cross the simultaneous
5%/4 KiB artifact guard. The shipped VMs remain below 2% growth, executable
generic and RexxCPS images shrink materially, lifecycle/RSS are favorable, and
all measured runtime effects are favorable or neutral. Adrian explicitly
accepted that trade-off on 2026-07-21.

## Evidence map

- `raw/generic-output-strategy-poc.csv`: prepared-vector versus exact-chain
  design selection.
- `raw/*paired.csv` and the Richards append: all warmup and recorded samples.
- `paired-summary.csv` and `absolute-summary.csv`: reductions and distribution
  statistics.
- `artifact-inventory.csv` and `artifact-deltas.csv`: exact hashes, sizes and
  guard classification.
- `focused-checks/`: final Debug and Release 13-test logs.
- `commands.md`, `host-state.md` and `compatibility.md`: reproducibility and
  semantic boundaries.
- `qa-closeout/README.md`: accepted decision, broad Debug/ASan, documentation,
  isolated install and native-package proof.
