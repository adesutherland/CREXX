# Production slice 2 - candidate profitability gate

Date: 2026-07-24

Starting commit: `d51bdf30d71e2a6a7c7c5e42a28a512a29f0d50e`

## Scope

- Added candidate cost vectors for structural nodes, assignments/copy proxies,
  branches, calls and inline temporary definitions/local-pressure proxies.
- Captured original-call, reference-candidate and final-candidate costs inside
  `InlineExpansionPlan`.
- Added an opt-in strict gate: every metric must be no worse and at least one
  must improve. Missing cost proof or a losing metric leaves the original call
  attached.
- Existing slice 1 expansions record the reference/final costs but do not yet
  require a cleanup win. Slice 3 is the first consumer of the strict gate.
- No eligibility, emitted RXAS/RXBIN, ABI or VM behavior change.

## Correctness and QA

- Focused Debug `rxc` build: pass.
- Full inline matrix: 233/233 pass.
- Full Debug CTest: 1,907/1,907 pass at `--parallel 30`.
- Ordinary profiling-off Release `rxc`, linked library and Richards build:
  pass.

## Release verdict

The smallest decisive comparison is exact artifact identity with slice 1:

| Artifact | Slice 1 SHA-256 | Slice 2 SHA-256 | Verdict |
| --- | --- | --- | --- |
| Richards RXBIN | `57d803889249c2c22bc7aba4812bdf916aa16661e739fc052cb21cd024e552a3` | same | byte-identical |
| Linked library RXBIN | `b40270671758443299465b67c6cfffe6edeb8b118b607eaa6a8e8a02f7426a8c` | same | byte-identical |

With the same VMs, byte-identical program/library images make runtime timing
identical by construction; a noisy timing rerun would add no evidence.
Verdict: **accepted parity infrastructure**.

## Identities before commit

- `compiler/rxcp_inline_internal.h`:
  `54467aac48c90ec5a9c1a5ec44b7de95f90f2b0bbd7ef23d626c54d554a75aa0`
- `compiler/rxcp_inline_rewrite.c`:
  `6910e3c615d0f78036f8141fe955af86e1a54c86c9a4163c3dc8ba7c776ee703`
- Release `rxc`:
  `1416b5db7821714c90eaf2cfe8fe0196647558c3c74b1f61fa003bb5aac0c50b`
