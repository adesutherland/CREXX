# PERF2-04 scratch-only RexxCPS UPPER constant-fold ceiling

This is CAS-L2, a non-production exact-site ceiling. It was built from detached
commit `6567f0ba23f20623e01322f5a62323b2347ab09d` in
`/private/tmp/crexx-perf2-04.3k3Dfw/upper-const-src` with the exact B0-R
Release, profiling-off compiler and assembler. No production source was
changed.

## Control shape and semantic limit

B0-R already folds the four hot UPPER arguments to `"with"`, `"2"`, `"args"`
and `"(This is the second)11"`. CAS-L2 substitutes their canonical results
`"WITH"`, `"2"`, `"ARGS"` and `"(THIS IS THE SECOND)11"` in the original
left-to-right order. Four separate assignment statements retain a distinct
RXAS source step and assignment observation for each original site; comments
on those source steps retain the original UPPER expression text. A post-timer
assertion checks all four results.

A source-only PoC cannot reproduce the four original `UPPER` function TRACE
events or their exact original line/column identities. The control therefore
proves the exact TRACE-off RexxCPS sites and machine ceiling, not a production
constant folder. Production would have to use the canonical simple Unicode
case mapping and synthesize equivalent source/function TRACE observations, or
fail closed when that contract cannot be preserved.

## B0-R and artifact identity

- B0-R: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`
- `rxc`: `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857`
- `rxas`: `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4`
- source: `a5cc34abf8eebc8fb7cec874a827d915f7fddbf8d239ec378c6ab3a8e551f77b`
- RXAS: `d7b3b8fffb180ed864192f3ff875443b19bc3b196310b198fd58c76bc1ebffa0`
- RXBIN: `dd9cfe3259718f792fbaf0fe0643e36910975c63f85ad9eb7d60d4a4578c87f2`

The image base for the maintained wall matrix is
`/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_upper_const_ceiling`; use
`/private/tmp/crexx-perf2-04.3k3Dfw/build-release/bin/library` as its module.

## Static comparison

| cell | total executable RXAS | main executable RXAS | main locals | calls | UPPER calls | STRUPPER | RXAS bytes | RXBIN bytes | timed UPPER block |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| B0-R current literal | 1,498 | 573 | 105 | 27 | 4 | 0 | 220,731 | 77,438 | 17 |
| current symbol/inliner | 1,506 | 581 | 113 | 23 | 0 | 4 | 224,205 | 78,610 | 25 |
| direct result placement | 1,536 | 611 | 113 | 23 | 0 | 4 | 228,697 | 80,506 | 13 |
| CAS-L2 constant fold | 1,511 | 586 | 109 | 23 | 0 | 0 | 224,587 | 78,914 | 9 |

The whole-module CAS-L2 count and bytes include four scratch predeclarations
and a post-timer validation branch, so they are not an estimate of production
constant-fold overhead. The isolated timed block is 8 executable instructions
smaller than current, 16 smaller than the symbol/inliner control, and 4 smaller
than direct result placement.

## Dual-VM smoke result

Both profiling-off Release VMs exited 0, emitted no stderr, passed the
post-timer value assertion, and printed `PASS: RexxCPS 2.2d cREXX port`.

- `rxvm`: effective count 320, diagnostic single-run rate 32,491,862 clauses/s
- `rxbvm`: effective count 310, diagnostic single-run rate 30,353,323 clauses/s

These adaptive smoke runs are correctness evidence, not a formal wall-clock
verdict.

## Current-profile normalized counts

The normalization denominator is `(initial_count + effective_count) * 100`
top-level timed iterations, accounting for both measured trials.

| VM/cell | denominator | instructions/iteration | STRUPPER/iteration | frame activations/iteration | standalone values/iteration | string buffers/iteration |
|---|---:|---:|---:|---:|---:|---:|
| rxvm current | 5,000 | 5,260.985800 | 56.283200 | 71.008800 | 55.007800 | 14.004600 |
| rxvm symbol | 5,300 | 5,145.986604 | 56.267170 | 14.951698 | 55.007358 | 14.004340 |
| rxvm direct | 5,300 | 4,977.985849 | 56.267170 | 14.951698 | 55.007358 | 14.004340 |
| rxvm CAS-L2 | 5,400 | 4,921.057963 | 0.262222 | 14.934074 | 55.007222 | 14.004259 |
| rxbvm current | 4,800 | 5,263.193542 | 56.295000 | 71.050833 | 55.008125 | 14.004792 |
| rxbvm symbol | 5,100 | 5,147.941373 | 56.277647 | 14.989020 | 55.007647 | 14.004510 |
| rxbvm direct | 5,100 | 4,979.953137 | 56.277647 | 14.989020 | 55.007647 | 14.004510 |
| rxbvm CAS-L2 | 5,300 | 4,921.983585 | 0.267170 | 14.951698 | 55.007358 | 14.004340 |

CAS-L2 reduces normalized retired instructions by 6.461295% on rxvm and
6.482945% on rxbvm versus current. It improves on direct result placement by
1.143593% and 1.164058%, respectively. The only repeated opcode delta from the
direct control is the removal of approximately 56 STRUPPER scans per top-level
timed iteration. Normalized frame activations and allocation counts remain the
same apart from fixed-run dilution.

## Evidence paths

- source worktree: `/private/tmp/crexx-perf2-04.3k3Dfw/upper-const-src`
- candidate RXAS/RXBIN: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_upper_const_ceiling.{rxas,rxbin}`
- compiler/assembler logs: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_upper_const_ceiling.{rxc,rxas}.{stdout,stderr}.txt`
- Release smoke: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-upper-const-smoke/{rxvm,rxbvm}/`
- profile counts: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-upper-counts/{rxvm,rxbvm}/upper-const-ceiling/`
