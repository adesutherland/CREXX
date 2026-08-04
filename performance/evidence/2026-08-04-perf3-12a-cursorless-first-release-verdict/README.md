# PERF3-12A cursorless RXAS first Release verdict

Status: **ready for Adrian's acceptance**

## Scope and provenance

- Product baseline: `0db998beadd263a894487ede16d2cd9b473ae08b`
  (`perf: refresh K04e Mac and RexxCPS evidence`).
- Implementation starting point: `0cc6203783388e647426764c5893cdf511dcec42`
  (`perf: baseline cursorless PERF3-12A`).
- Candidate: the uncommitted cursorless RXAS/RXBIN redesign controlled by
  [`PERF3-12A-WORKLIST.md`](../../PERF3-12A-WORKLIST.md). Its product/source
  diff excluding performance control and evidence files has SHA-256
  `ad43e00ab4e214fbd97bc0f2975bf802c7856f7ccb23980c9cfb113bc923f913`.
- Retained control counts come from
  [`2026-08-04-perf3-12-k04e-clause-reassessment`](../2026-08-04-perf3-12-k04e-clause-reassessment/).
  Candidate counts use the same fixed `200 x 100` non-calibrating RexxCPS
  workload under both profiling VMs.
- Candidate no-opt image SHA-256:
  `6a3d8844d60f28c4c60315031b7e8116fb344af10f6ac4422359d508c138d223`.
  This is byte-identical to the retained control no-opt image. Candidate
  optimized image SHA-256:
  `1cd57cf2c73407b064b111b2aa4bd6dfe20fb5ae189b6a54731991563e27ada2`.
  Candidate linked library SHA-256:
  `84808f0c15fa3e0cdcdd70750abd38559cd3943ca61e71ba8aafc06e6aa86163`.
- Host: Apple M5, Darwin 25.5.0 arm64, 10 logical CPUs, Apple clang 21.0.0,
  CMake 4.3.2 and Ninja 1.13.2.

## Correctness and structural audit

The breaking ISA change removes public string/binary cursor state and uses
explicit four-register `substring destination,source,start,length` and
`bslice destination,source,start,length` operations. Unrelated numeric opcode
slots are not renumbered. String character-to-byte position state is now a
VM-private cache; binary values have no replacement cursor.

The logical `string_pos`, `string_char_pos` and `binary_pos` members were
deleted before completing the migration. A clean Debug build of RXAS, RXC,
both VMs, opcode/flow tests and the bundled libraries succeeds, making field
removal a compile-time inventory check. The final residual-name audit found
and removed one stale current register diagram and the obsolete generated
cursor-operation assets. Retired mnemonics remain only in the negative
assembler fence, which proves that all four names and the old three-operand
slice forms are rejected. All 116 authored `.crexx` slice sites and 13 tracked
`.rxas` slice sites have exactly four operands.

A fresh-tree focused Debug selector passes **24/24**. It covers retired
mnemonic/arity rejection, opcode metadata, flow-graph contracts, register
value/cache behavior, explicit string/binary slices and UTF flags under both
VMs, instruction-gap preservation, ASUTF, compiler partial calls/inlining and
the linked library/runtime paths. The ordinary profiling-off Release product
builds successfully; its RexxCPS no-opt/opt smoke cells pass under both VMs
with zero stderr. The four fixed-work count cells also return result 0, report
no invalid events or counter overflow, and contain the RexxCPS PASS marker.

## Exact fixed-work result

| VM | Mode | Retained control | Cursorless | Change | Change | Removed setter dispatches |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | no-opt | 148,700,911 | 143,099,442 | -5,601,469 | -3.766937% | 1,400,643 |
| `rxbvm` | no-opt | 148,700,911 | 143,099,442 | -5,601,469 | -3.766937% | 1,400,643 |
| `rxvm` | optimized | 53,660,581 | 53,659,088 | -1,493 | -0.002782% | 643 |
| `rxbvm` | optimized | 53,660,552 | 53,659,041 | -1,511 | -0.002816% | 642 |

`SETSTRPOS` is absent from the candidate ISA and therefore executes zero
times. The explicit four-operand `SUBSTRING` executes 1,400,645 times in each
no-opt cell and 644/639 times in optimized `rxvm`/`rxbvm`. The direct dispatch
reduction is consequently exact and both VMs agree on the fixed-work result.

The larger no-opt reduction is intentional rather than an unexplained change.
The old setter made a source argument formally mutable, so generated library
procedures defensively isolated it. For example, generated `word.rxas` falls
from 172 to 163 lines and now reads `a1` directly. Its prologue loses one each
of `BRTPANDT`, `SCOPY`, `BR` and `SWAP`; its setter plus three-operand slice
becomes one explicit four-operand slice. Old/new generated file SHA-256 values
are `2393b76cd3b91fa7b1b7a6f977a4fe54cf5031362db8f27ecf4603884980ca83`
and `ca039aea8c4c3960a06015aaf504a1e3436c9a6e6d14780251bf93a70d307b15`.
The byte-identical no-opt benchmark image further confines the dynamic change
to the rebuilt linked library rather than benchmark lowering drift.

## Runtime boundary and recommendation

These are exact instruction counts, not a wall-clock performance claim. The
current remote terminal materially disturbs Mac timings, so no candidate CPS
number is promoted into the scorecard. Adrian explicitly accepted
instruction-count reduction plus functional equivalence as tonight's first
verdict evidence; the wider profiling-off Mac panel remains queued for a
clean-host rerun.

The first verdict is therefore **accept cursorless RXAS on correctness,
architectural simplification and exact dispatch reduction**, subject to
Adrian's decision. Acceptance authorizes the separately governed copied-XTOY
component-placement implementation; rejection stops for redesign. No
copied-XTOY production edit is included here.

## Evidence map

- `analysis/count-summary.csv`: exact retained/candidate totals and setter/
  explicit-slice counts.
- `analysis/word-simplification.csv`: one concrete direct-argument lowering
  proof.
- `profiles/{rxvm,rxbvm}`: raw schema-5 count profiles, benchmark stdout and
  empty stderr.
- `release-smoke/{rxvm,rxbvm}`: profiling-off ordinary Release correctness
  stdout and empty stderr for no-opt and optimized images.
- `validation/focused-debug-ctest.txt`: final clean-tree 24/24 selector.
- `checksums.sha256`: recursive evidence closure excluding itself.
