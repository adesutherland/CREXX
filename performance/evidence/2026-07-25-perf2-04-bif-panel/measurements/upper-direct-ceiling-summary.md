# PERF2-04 scratch-only RexxCPS UPPER direct ceiling

This is a non-production control. It was built from detached commit
`6567f0ba23f20623e01322f5a62323b2347ab09d` in
`/private/tmp/crexx-perf2-04.3k3Dfw/upper-direct-src` with the exact B0-R
Release, profiling-off compiler and assembler. No production source was
changed.

## Control shape

The four timed `UPPER` calls are represented by four source registers, four
separate result registers, and four direct `STRUPPER result,source`
instructions. Result registers are initialized before the timed loops. After
the timer has stopped, assertions prove the four expected uppercase results
and prove the four sources were not mutated. This is a machine-ceiling control;
it does not preserve the observable `UPPER` function TRACE event and is not a
production implementation. It is the direct dynamic-input ceiling, not the
absolute ceiling for these four exact call sites: B0-R has already folded all
four arguments to constants, so a semantics- and TRACE-correct constant fold
could remove the four runtime case scans as a stronger companion candidate.

## B0-R identity

- `rxc`: `900c2ba2229632c74da2a00cc313efa517beeb13ed8ab58f7e0e1afb41bad857`
- `rxas`: `80d3ff3e5b28e7132158c1b186513755457baf1472c2edd653874005a2648fc4`
- `rxvm`: `aab099d2f1e52f09976002935b21b189c104200f7a4b4155c65eef6eb21ac1d4`
- `rxbvm`: `a4a61df9cceac8a0178ef5583835953f55f882f7b4bab29c754d3c41aed87b5f`
- `library.rxbin`: `d4b35ddefa1b7d6711788b38dc33d0b66ff8a1af43690f2367c98a8ee5f7fcf1`
- B0-R configuration: `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`

## Artifact identity

- source: `e8990654e08cefbf77628aed4a6ac32bc61946c496f42470e4161783f80df94e`
- RXAS: `47934e8d09cacb4d36aa6abbca9cae641897bcaeba3784bb4b0b298338e967c0`
- RXBIN: `3674a1574aa20fadb53e089005cb3ee8109f6ebdf5ca5875defd18601cfd8898`

## Static comparison

| cell | total executable RXAS | main executable RXAS | main locals | calls | UPPER calls | STRUPPER | RXAS bytes | RXBIN bytes | timed UPPER block |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| B0-R current literal | 1,498 | 573 | 105 | 27 | 4 | 0 | 220,731 | 77,438 | 17 |
| current symbol/inliner control | 1,506 | 581 | 113 | 23 | 0 | 4 | 224,205 | 78,610 | 25 |
| direct result-placement ceiling | 1,536 | 611 | 113 | 23 | 0 | 4 | 228,697 | 80,506 | 13 |

The whole-module direct count and byte size include eight scratch
predeclarations plus post-timer semantic assertions. They are not an estimate
of a compiler lowering's artifact overhead. The isolated timed UPPER block is
4 executable instructions smaller than current and 12 smaller than the
symbol/inliner control.

## Dual-VM smoke result

Both profiling-off Release VMs exited 0, emitted no stderr, passed the
post-timer result/source assertions, and printed `PASS: RexxCPS 2.2d cREXX
port`.

- `rxvm`: effective count 300, diagnostic single-run rate 31,277,922 clauses/s
- `rxbvm`: effective count 290, diagnostic single-run rate 29,151,735 clauses/s

These adaptive smoke runs are correctness evidence, not a formal timing
verdict.

## Current-profile normalized counts

The normalization denominator is `(initial_count + effective_count) * 100`
top-level timed iterations, accounting for both measured trials. The RXAS is
the exact B0-R-produced artifact; profiling runtimes are diagnostic only.

| VM/cell | denominator | instructions/iteration | CALL1 | RETREG | LOADSTR | SCOPY | BR | STRUPPER | frame activations | standalone values | string buffers |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| rxvm current | 5,000 | 5,260.985800 | 56.200800 | 56.525000 | 438.249600 | 519.417000 | 169.170000 | 56.283200 | 71.008800 | 55.007800 | 14.004600 |
| rxvm symbol | 5,300 | 5,145.986604 | 0.189434 | 0.495283 | 438.065660 | 575.280189 | 225.047170 | 56.267170 | 14.951698 | 55.007358 | 14.004340 |
| rxvm direct | 5,300 | 4,977.985849 | 0.189434 | 0.495283 | 382.066981 | 519.279623 | 169.046792 | 56.267170 | 14.951698 | 55.007358 | 14.004340 |
| rxbvm current | 4,800 | 5,263.193542 | 56.209167 | 56.546875 | 438.385000 | 519.517708 | 169.260417 | 56.295000 | 71.050833 | 55.008125 | 14.004792 |
| rxbvm symbol | 5,100 | 5,147.941373 | 0.196863 | 0.514706 | 438.185686 | 575.369020 | 225.127059 | 56.277647 | 14.989020 | 55.007647 | 14.004510 |
| rxbvm direct | 5,100 | 4,979.953137 | 0.196863 | 0.514706 | 382.187451 | 519.369608 | 169.127647 | 56.277647 | 14.989020 | 55.007647 | 14.004510 |

Against B0-R current, direct result placement reduces normalized retired
instructions by 5.379219% on rxvm and 5.381531% on rxbvm. Against the current
symbol/inliner control it removes almost exactly 56 LOADSTR, 56 SCOPY and 56
BR instructions per timed iteration, reducing the total by 3.264695% on rxvm
and 3.263212% on rxbvm. STRUPPER scans, standalone values, string buffers and
frame activations are identical to the symbol control. Current-to-direct also
removes the per-call argc LOADINT, CALL, NUMSCI and RET scaffolding plus result
initialization, approximately 56 of each per timed iteration.

## Evidence paths

- source worktree: `/private/tmp/crexx-perf2-04.3k3Dfw/upper-direct-src`
- candidate RXAS/RXBIN: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_upper_direct_ceiling.{rxas,rxbin}`
- compiler/assembler logs: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps_upper_direct_ceiling.{rxc,rxas}.{stdout,stderr}.txt`
- Release smoke: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-upper-direct-smoke/{rxvm,rxbvm}/`
- profile counts: `/private/tmp/crexx-perf2-04.3k3Dfw/evidence/rexxcps-upper-counts/{rxvm,rxbvm}/upper-direct-ceiling/`
