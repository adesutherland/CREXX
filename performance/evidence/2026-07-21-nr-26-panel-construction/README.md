# NR-26 transformation-panel construction census

This is static construction evidence, not a performance verdict. It isolates
the first stronger NR-26 panel slice from the frozen F1/F2 worktree and applies
Adrian's correctness-plus-instructions gate. No timing comparison was run.

Both sides use the ordinary profiling-off `cmake-build-release` product at
starting commit `4ab5f3d8da673c10b81af4249757763d052dda34` with the documented dirty
NR-26 scope. The baseline RXAS snapshot was taken immediately before the
small-scalar must-copy/dead-store slice; the candidate was regenerated after
its fixed point and incoming-argument-slot proof. The CSV retains both RXAS
hashes rather than duplicating generated benchmark artifacts in the repository.

Instructions are counted with the existing programme convention:

```text
^   [a-z][a-z0-9]*([ \t]|$)
```

The bounded 19-image optimized census is 50,965 to 50,924 instructions: **41
instructions avoided**. The complete mnemonic delta is `copy -11`, `icopy
-30`; no other mnemonic count changes. Five benchmark workloads account for
35 avoided copies: Mandelbrot 1, Permute 8, Richards 12, Towers 10 and Base64
4. Three performance-tool selftests account for the other six. Sieve and both
RexxCPS images have register/TRACE retargeting but zero instruction reduction,
so they receive no savings credit.

Correctness gates passed:

- focused optimized/no-opt structural cases and exact counts for straight
  paths, equal/different joins, loop backedges, source overwrites, fixed-point
  dead copies, integer/float/boolean copies, destructive-promotion guards,
  effectful RHS retention and incoming-slot sharing;
- exact `ok`/RC 0 under both `rxvm` and `rxbvm` in `nr26_flow_contract`;
- `nr09_codegen_contract` and `rxc_inline_byvalue_arg_reuse`;
- all 11 changed/retargeted Release images under the registered `rxvm` tests;
  and
- the same 11 final linked images under `rxbvm`, each with its required PASS
  marker.

`static-instruction-census.csv` is the exact 19-image ledger. Zero-delta rows
are retained so the bounded denominator and unchanged images remain explicit.
