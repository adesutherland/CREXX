# Production slice 3 - gated scalar and result cleanup

Date: 2026-07-24

Starting commit: `6687d64d5`

## Scope

- Added candidate-local bounded cleanup for compiler-added scalar self-copies
  and instructions proven unreachable after an inline boundary exit.
- Let a direct caller-local read-only numeric scalar formal share the caller's
  physical register while retaining the cloned formal symbol, binding event,
  source identity and TRACE name. Sharing requires exact type identity and
  private, non-reference-backed caller storage.
- Retained the normal call when one caller variable is bound to both a
  by-value and an `arg expose` formal. A code-review-only audit found that the
  established call-entry ordering is observable, so this case is now covered
  by optimized/unoptimized compile-golden and linked runtime CTests.
- Enabled compatible scalar block-result destination placement only for a
  candidate accepted by the slice 2 multi-metric profitability gate.
- Kept writable formals, optional/default arguments, computed actuals,
  strings, binary, references, arrays, objects and class attributes on their
  previous isolated paths.
- Added no public RXAS/RXBIN/ABI or VM contract.

## Correctness and QA

- Focused Debug `rxc` build: pass.
- Full inline matrix after the expected optimized-shape refresh: 233/233 pass.
- Mixed by-value/`arg expose` alias regression: 4/4 focused CTests pass across
  optimized/unoptimized compile-golden and linked runtime modes.
- The first broad run identified five additional optimized goldens containing
  the same removed read-only scalar binding copy; all runtime counterparts
  passed. Their exact TRACE-preserving shapes were reviewed and refreshed.
- Final full Debug CTest: 1,907/1,907 pass at `--parallel 30`.
- Ordinary profiling-off Release build, five-workload artifact inventory and
  rotated Richards comparison on both VMs: pass.

## Artifact verdict

The P0 column is the retained exact-head product. Slice 2 is byte-identical to
slice 1; the slice 2 Richards shape is shown where its exact production
comparison is available.

| Workload | P0 instructions / peak locals / bytes | Slice 3 | Verdict |
| --- | ---: | ---: | --- |
| List | 233 / 34 / 16,226 | 233 / 34 / 16,186 | no instruction/local regression |
| Permute | 227 / 30 / 11,965 | 227 / 28 / 11,925 | prototype local regression avoided; two locals removed |
| Richards | 1,897 / 66 / 79,646 | 1,867 / 62 / 79,094 | -30 instructions and -4 peak locals vs P0; -19/-4 vs slice 2 |
| JSON | 46 / 9 / 4,033 | 46 / 9 / 4,001 | no instruction/local regression |
| RexxCPS | 1,402 / 105 / 77,470 | 1,402 / 105 / 77,438 | prototype +1 instruction avoided |

The shared library changes from the retained P0 54,722 instructions / 858,081
bytes to 54,387 / 856,873. The final gate therefore accepts real cleanup while
rejecting the P2/P3 prototype regressions it was designed to guard.

## Release timing verdict

Stable AC power, low-power mode 0, ordinary profiling-off Release products,
Richards work=10, one warmup plus three serially rotated recorded runs per
cell. Exact P0 VMs were held constant; each cell used its matching program and
linked library. Power remained attached throughout the 16:20:57Z-16:22:20Z
capture.

| VM | P0 median | Slice 3 median | Speedup | Relative MAD P0 / Slice 3 |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 5.839667 s | 4.380598 s | 33.308% | 0.102% / 0.241% |
| `rxbvm` | 5.912798 s | 4.509535 s | 31.118% | 0.037% / 0.111% |

Against the retained slice 1 medians, slice 3 is 0.307% faster on `rxvm` and
1.130% slower on `rxbvm`; the primary exact-P0 verdict remains strongly
favorable on both VMs while improving the static product and peak locals.
Verdict: **favorable gated cleanup**. Slice 3 is eligible for its independent
commit.

## Identities before commit

- `compiler/rxcp_emit_reg.c`:
  `61e7ae2387516d0497770579cf8899106477414d53c66823011ca125c6a0ed3e`
- `compiler/rxcp_inline_bind.c`:
  `96db66d263b194bd999672c405c0b090d6b19de308a1f7c64dd61c37e2540c52`
- `compiler/rxcp_inline_internal.h`:
  `e223d4ba09822677e01bd83d4c070a132a1f824ebc020ebad603d20fc2dfa4d2`
- `compiler/rxcp_inline_rewrite.c`:
  `f3c571e53735995e6b44b7f2a016931eadb16f30dd8cc95cd7efa49bc9a5d5f4`
- `compiler/rxcp_sym.h`:
  `4d1f12488ca89a1543a3e36edc800a2931e9ae869bc3e0ce7dc3617ef5f37b6b`
- `compiler/rxcpsymb.c`:
  `ba46e13da0c72d5158290ef01f57238675e3f31e93822d0f7fed852e759a9e0d`
- Release `rxc`:
  `49f2ecb4a38e019322c1fcbe8e89c7769d5128d60c5193854b29045484016ac4`
- Richards RXBIN:
  `6aad1ca91ddb53089fe0b5040f47e1267cabf6bf13c1cc8527b8940d77b50f9a`
- Linked library RXBIN:
  `b519736953051da4be15165674bdeba780cb25f973b1cbda9903a6d6f15b41b2`
- Mixed-alias regression source / noopt golden / opt golden:
  `3c925292ea14b6f843db76e789615cca0ab92397e46695cd159fd269689b5ab5` /
  `c308cda75056cecaa3bdf83f076bcd36086aaf1b9eae86253b91ca213ea2a91e` /
  `42542fccc3f5bf69892d757d389bd4e07a7ef04888bed9dfcd11e6d8bf0ef368`
