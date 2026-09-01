# PERF2-04 scratch-only RexxCPS LENGTH direct ceiling

This is a non-production control. It was built from detached commit
`6567f0ba23f20623e01322f5a62323b2347ab09d` in
`/private/tmp/crexx-perf2-04.3k3Dfw/length-direct-src` with the exact B0-R
Release, profiling-off compiler and assembler. No production source was
changed and no formal wall-clock portfolio was run.

## Control shape and boundary

Only the timed-kernel `LENGTH(j as .string)` site is replaced. The decimal-to-
string conversion remains first, in the original left-to-right position; a
direct `STRLEN` writes a distinct integer result register; the original
subtract, compare and failure branch follow. Typed-null source/result
predeclarations keep the two registers live for post-timer checks. Those checks
prove the final converted source is `"2.2"`, its codepoint length is `3`, and
the primitive did not mutate the source.

The selected taken path executes 28 times per top-level timed iteration. The
current PERF2-03 inline path is:

`DCOPY, DTOS, LOAD result=0, STRLEN, ICOPY return, BR block-exit, ISUB, ILT, BRF`

The hand-equivalent control is:

`DCOPY, DTOS, STRLEN, ISUB, ILT, BRF`

This is therefore a 9-to-6 executable-instruction reduction, exactly three
instructions per selected call and 84 per top-level timed iteration. It does
not remove or reduce the conversion or Unicode `STRLEN` scan.

## B0-R and artifact identity

- `rxc`: `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857`
- `rxas`: `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4`
- `rxvm`: `aab099d2f1e52f09976002935b21b189c104200f7a4b4155c65eef6eb21ac1d4`
- `rxbvm`: `a4a61df9cceac8a0178ef5583835953f55f882f7b4bab29c754d3c41aed87b5f`
- `library.rxbin`: `d4b35ddefa1b7d6711788b38dc33d0b66ff8a1af43690f2367c98a8ee5f7fcf1`
- baseline RXAS: `e9c8b16163b5ff647ee9bb9345080e045823fd1ea0ab5892e7206ee4450150a8`
- baseline RXBIN: `9b535403baddc5b7076d5eb23027a296d982660546bcecc74d3784bb2fa23741`
- candidate source: `397a1c76312af5cbe1831b4011de240eb28c5c3acd9518f89a563c40c304c54a`
- candidate patch: `7355da512c31e633d7b2841eb66f6af0bca12293a6799aa9cab6d7e50016d9a9`
- candidate RXAS: `26b5b4f1f78ebc4ed694c5d27e275adcd393cbe13e02f3f0238e77f40c17b6b2`
- candidate RXBIN: `77d1a5bcacec6c61409ef99ae083888db599377fb204d883983cd82fd5eb26cb`

## Static comparison

Executable RXAS counts exclude directives, labels and comments.

| cell | total executable RXAS | main executable RXAS | main locals | call opcodes | residual LENGTH calls | STRLEN | RXAS bytes | RXBIN bytes | selected taken block |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| B0-R current | 1,498 | 573 | 105 | 27 | 0 | 18 | 220,731 | 77,438 | 9 |
| direct result placement | 1,510 | 585 | 107 | 27 | 0 | 18 | 222,737 | 78,270 | 6 |

The candidate's whole-module increase of 12 executable instructions, two
locals, 2,006 RXAS bytes and 832 RXBIN bytes includes scratch predeclarations
and post-timer semantic guards. It is not a production-lowering size estimate.

## Dual-VM Release smoke

Both profiling-off B0-R VMs exited 0, emitted no stderr, passed both post-timer
guards and printed `PASS: RexxCPS 2.2d cREXX port`.

- `rxvm`: effective count 290; diagnostic rate 29,134,807 clauses/s.
- `rxbvm`: effective count 260; diagnostic rate 26,630,504 clauses/s.

These adaptive smokes are correctness evidence only, not a timing verdict.

## B0-P normalized counts

Each selected comparison has denominator `(initial_count + effective_count) *
100 = 5,000` top-level timed iterations. Profiling is attribution-only; both
images were produced by B0-R.

| VM/cell | instructions/iteration | LOADINT | ICOPY | BR | DCOPY | DTOS | STRLEN | frame activations | standalone values | string buffers |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| rxvm current | 5,260.980200 | 412.211400 | 29.595000 | 169.169600 | 111.000000 | 111.000000 | 143.221800 | 71.008800 | 55.007800 | 14.004600 |
| rxvm direct | 5,176.980800 | 384.211400 | 1.595000 | 141.169600 | 111.000000 | 111.000000 | 143.221800 | 71.008800 | 55.007800 | 14.004600 |
| rxbvm current | 5,260.985800 | 412.211800 | 29.595000 | 169.170000 | 111.000000 | 111.000000 | 143.222200 | 71.008800 | 55.007800 | 14.004600 |
| rxbvm direct | 5,176.980800 | 384.211400 | 1.595000 | 141.169600 | 111.000000 | 111.000000 | 143.221800 | 71.008800 | 55.007800 | 14.004600 |

The observed reduction is 83.999400 instructions/iteration (1.596649%) on
`rxvm` and 84.005000 (1.596754%) on `rxbvm`. The exact recurring hot-path delta
is 28 each of LOADINT, ICOPY and BR, or 84 total. Fractional 0.0004 differences
outside those three opcodes are fixed setup/reporting-path dilution. STRLEN,
conversion, frame and allocation counts do not move.

## Bounded profiling-off Release wall result

The maintained Level B cross-runtime driver ran the exact B0-R product and
library serially from a four-cell manifest: current `LEN-C0-current` and the
scratch `LEN-H1-direct-result`, separately on `rxvm` and `rxbvm`. The canonical
RexxCPS default workload used two warmups and seven recorded rounds per cell.
All 36 invocations exited 0, passed the RexxCPS correctness contract and
retained an empty stderr stream. Every absolute cell remained below the
driver's variability thresholds (`relative MAD < 3%`, `span < 10%`), so no
rerun was requested.

| VM | current median clauses/s | LEN-H1 median clauses/s | ratio, higher is better | median change | paired median change | paired mean 95% t interval |
|---|---:|---:|---:|---:|---:|---:|
| `rxvm` | 29,363,287 | 29,362,309 | 0.999966693102 | -0.003330690% | -0.003330690% | [-0.559483352%, +0.861937036%] |
| `rxbvm` | 27,533,292 | 27,739,431 | 1.007486899859 | +0.748689986% | +0.614624455% | [-1.000215330%, +1.863454144%] |

The recorded paired rounds favored LEN-H1 in three of seven `rxvm` rounds and
five of seven `rxbvm` rounds. The `rxvm` result is effectively exactly neutral;
the sub-1% `rxbvm` median uplift is not reproduced across both VMs, and both
paired mean intervals cross zero. Thus the exact 1.5967% instruction reduction
does not establish a reliable end-to-end gain in the smallest deciding workload
cell. This bounded 2+7 run is not the formal 12-pair production-decision
protocol and does not justify more timing for this control.

Disposition: **neutral; not production-selecting alone**. LEN-H1 remains useful
bounded PERF2-03-F03 reopen evidence for formal/result/block-exit cleanup, and
compiler-owned inline cleanup remains the most efficient plausible owner if a
broader selected slice supplies independent materiality evidence. This result
does not support a new opcode, VM assist, native owner or standalone LENGTH
production slice.

## Semantic boundary and placement

- `STRLEN` is the same existing public primitive used by the Level B fallback;
  in the UTF build it counts codepoints, leaves the source/cursor unchanged and
  raises `UNICODE_ERROR` for invalid UTF-8.
- The runtime proof here is deliberately narrow: the selected decimal values,
  ordinary TRACE-off execution and both VMs. It is not the full LENGTH semantic
  matrix for arbitrary strings, invalid UTF-8, aliases, TRACE or no-opt.
- The scratch source does not preserve the original single call expression's
  source topology. It removes the imported `length.crexx` body source steps,
  adds named assignment/assembler source steps and cannot claim exact TRACE or
  signal-source identity. Any production optimization must preserve/synthesize
  the existing observation contract or fail closed.
- A source register initialized to `""` before `DCOPY; DTOS` produced stale
  codepoint-count metadata and made direct STRLEN return 0 on both VMs. Typed-
  null declaration avoids carrying that prior UTF-8-count-valid state and the
  final candidate returns 3. The empty-initialized Level B/typed-instruction
  sequence is valid, so its failure is retained and routed explicitly to
  PERF2-07 V3 representation-validity/mutation-invalidation work. It is not
  counted as a valid LEN-H1 performance result, and PERF2-04 installs no fix.
- This cell is bounded reopen evidence for PERF2-03-F03 result initialization,
  return-copy and block-exit cleanup. The efficient owner is compiler inline
  cleanup/composition over existing `STRLEN`; it provides no evidence for a
  new RXAS opcode, VM assist or native LENGTH owner.

## Evidence paths

- source worktree: `/private/tmp/crexx-perf2-04.3k3Dfw/length-direct-src`
- patch: `/private/tmp/crexx-perf2-04.3k3Dfw/patches/rexxcps-length-direct-ceiling.patch`
- image base: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_length_direct_ceiling`
- library module base: `/private/tmp/crexx-perf2-04.3k3Dfw/build-release/bin/library`
- compiler/assembler logs: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_length_direct_ceiling.{rxc,rxas}.{stdout,stderr}.txt`
- Release smoke: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-length-direct-smoke/{rxvm,rxbvm}/`
- selected profile counts: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-length-counts/{rxvm,rxbvm}/length-direct-ceiling/`
- selected current profiles: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-length-counts/rxvm/baseline/` and `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-length-counts/rxbvm/baseline-attempt3/`
- excluded initial empty-string construction: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-length-direct-smoke/initial-empty-init-failed/`
- diagnostic value proof for that excluded construction: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_length_direct_debug.rxvm.stdout.txt`
- retained representation regression and classification: `performance/evidence/2026-07-25-perf2-04-bif-panel/diagnostics/length-empty-init-representation-regression/`
- retained shared patch: `performance/evidence/2026-07-25-perf2-04-bif-panel/pocs/length-direct-result-ceiling.patch`
- exact generated-artifact comparison: `performance/evidence/2026-07-25-perf2-04-bif-panel/diagnostics/length-generated-artifact-comparison.txt`
- Level B timing manifest: `performance/evidence/2026-07-25-perf2-04-bif-panel/controls/length-rexxcps-poc-matrix-v1.txt`
- retained wall evidence and paired analysis: `performance/evidence/2026-07-25-perf2-04-bif-panel/measurements/length-rexxcps-timing/`
