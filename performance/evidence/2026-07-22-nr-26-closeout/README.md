# NR-26 accepted-panel closeout

Adrian accepted the frozen F1/F2/P1 panel on 2026-07-22 and authorized the
shortest gate-to-commit closeout. The final implementation passes correctness,
preserves the exact construction census, and retains the accepted Release
verdict without a regression claim or a release-wide speedup claim.

## Correctness hardening found by the broad gate

The first closeout build exposed two missing exact CFG/use boundaries before
any commit:

1. A counted-loop `DO` header traversal also visited its separately modelled
   body and initializer. It incorrectly retargeted a loop-condition read across
   a backedge. Block-owned traversal now visits only the header's real
   condition/control nodes; the synthetic latch restores its implicit
   control-variable use explicitly.
2. Generated `SELECT` lowering retains semantic `VAR_REFERENCE` nodes whose
   older connector read flag may be clear. Those nodes still feed comparisons
   or jump-table dispatch instructions, so flow analysis now treats the node
   kind itself as the mathematically required read. Small comparison ladders
   may still reuse a proved-equal source register. Opaque jump-table dispatch
   keeps its private selector copy.

The focused fixture now includes a counted-loop backedge kill and an eight-case
integer dispatch. It checks the exact retained/removed copy counts and runs the
optimized and no-opt images under both `rxvm` and `rxbvm`. The affected
`Substr` compiler-exit artifact returned from the invalid greater-than-14-minute
compile to a 0.82-second two-image target build. The final focused regression
set passed 8/8, followed by the required full Debug result: **1877/1877** in
278.12 seconds with `ctest --test-dir cmake-build-debug --parallel 30
--output-on-failure`.

## Static and Release revalidation

The ordinary profiling-off Release benchmark surface rebuilt successfully.
All 19 optimized source-RXAS images reproduce the frozen panel's exact hashes
and counts (`static-verification.csv`): 50,965 baseline instructions to 50,924
candidate instructions, **41 avoided**, exactly `copy -11` and `icopy -30`.
The five instruction-changing benchmark images also reproduce their accepted
linked hashes byte-for-byte.

The corrected standard library changed the otherwise unchanged RexxCPS linked
image from 195,471 bytes at
`b2934ad1d185d1e2a334c7dba0d418fcfbeea5d6a657d612229f0d5f79fa7c81`
to 195,447 bytes at
`96908d57901c3773bea5af7d235a612554270babcc3c9ca4234a4ba94db797f1`.
Because that one timing artifact changed, a narrow same-session drift control
compared the previously accepted and corrected candidate images under both
current Release VMs.

All four warmups and 48 recorded runs returned zero, emitted the PASS marker,
and reported exact canonical `100 x 100` provenance. Percentages are paired
`(corrected / pre-QA - 1) * 100`; negative elapsed and positive native rate are
favorable.

| VM | Elapsed median | Mean 95% interval | Native-rate median | Mean 95% interval |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | +0.141% | -0.564% to +0.490% | -0.138% | -0.514% to +0.575% |
| `rxbvm` | -0.450% | -0.899% to +0.640% | +0.443% | -0.621% to +0.898% |

Both lanes are neutral/noisy; neither interval is wholly unfavorable. The
largest adverse individual elapsed pair is +0.703% for `rxvm` and +2.688% for
`rxbvm`, below the 3% per-workload guard. No append is required because this
bounded control asks only whether the correctness hardening invalidates the
already accepted no-regression decision; it makes no new improvement claim.

## Evidence map

- `static-verification.csv`: final 19/19 exact source-RXAS count/hash match.
- `artifact-inventory.csv`: final product and linked-image hashes.
- `input-manifest.txt`: four-cell old/corrected RexxCPS comparison.
- `timing/`: raw 12-pair balanced/interleaved capture and absolute summaries.
- `paired-summary.csv`: paired R-7 quartiles and two-sided Student-t intervals.
- `host-state.md`: immediate pre/post power, thermal and load state.
