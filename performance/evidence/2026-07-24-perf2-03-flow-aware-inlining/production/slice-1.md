# Production slice 1 - receiver transaction

Date: 2026-07-24

Starting commit: `086138f1e93da8e84d45f4cd3ba9b6620f792a14`

Accepted production commit: `d51bdf30d71e2a6a7c7c5e42a28a512a29f0d50e`

## Scope

- Added `InlineExpansionPlan` as the per-site speculative ownership boundary.
- Routed final statement, assignment-expression, call, value-expression and
  eager-operator replacements through its single commit operation.
- Retained the original call/target until the detached candidate is complete.
- Enabled only P1's proved direct method-receiver placement. Computed or
  locator-copyback receivers retain the existing capture path.
- No P2/P3/P4 cleanup, public RXAS/RXBIN/ABI or VM change.

## Correctness and QA

- Focused Debug build of `rxc`, `crexx_test_driver`, `rxvm` and `rxbvm`: pass.
- Full inline matrix: 233/233 pass.
- Full Debug CTest: 1,907/1,907 pass at `--parallel 30`.
- Ordinary profiling-off Release build and Richards output checks: pass.

## Artifact verdict

| Metric | P0 | Slice 1 | Delta |
| --- | ---: | ---: | ---: |
| Richards static instructions | 1,897 | 1,886 | -11 |
| Richards peak locals | 66 | 66 | 0 |
| Richards RXBIN bytes | 79,646 | 79,406 | -240; build-path metadata differs from the isolated P1 byte count |

## Release timing verdict

Stable AC power, low-power mode 0, ordinary profiling-off Release bytecode,
P0 VMs/library held constant, Richards work=10, one warmup plus three serially
rotated recorded runs per cell.

| VM | P0 median | Slice 1 median | Speedup | Relative MAD P0 / Slice 1 |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 5.739104 s | 4.394061 s | 30.610% | 0.314% / 0.026% |
| `rxbvm` | 5.865173 s | 4.458560 s | 31.549% | 0.037% / 0.063% |

Verdict: **favorable**. The exact-head production implementation exceeds the
retained P1 ceiling while preserving the static/local result and all tested
semantics. Adrian accepted the verdict and Slice 1 was committed independently
as `d51bdf30d`.

## Identities before commit

- `compiler/rxcp_inline_internal.h`:
  `44b5d6c0086bc9b8e1036b49b4f4c1cc92d08459270e881561eb3b32897022a3`
- `compiler/rxcp_inline_bind.c`:
  `2a017a11b9faf7c8038095407a6539a85abd06ef4a1e3eeef85e4083f3b49328`
- `compiler/rxcp_inline_rewrite.c`:
  `f11bbfd65aea52febb8030e7a97ac1f8ab10a2f0f2e7895d5a8d294bdd844911`
- Release `rxc`:
  `b435b29a75bc844230e00ff354584628278ea9f812a94a5cf3b60eb87b4a21cd`
- Richards RXBIN:
  `57d803889249c2c22bc7aba4812bdf916aa16661e739fc052cb21cd024e552a3`
