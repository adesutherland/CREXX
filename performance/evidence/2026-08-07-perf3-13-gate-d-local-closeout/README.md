# PERF3-13 Gate D local industrialisation closeout

Date: 2026-08-07  
Status: **Mac local closeout passes; cross-platform Gate D validation remains**

## Decision

Retain the industrialised 176-byte L32SDH value and S0/R0 worker allocator.
The final ordinary Apple Clang Release product is correctness-clean outside the
explicitly deferred spawn migration, remains guard-clean in two fresh
post-correction performance blocks, passes the affected AddressSanitizer lane,
and installs and runs as the selected `rxvm -> rxbvm` product.

This closes the Mac implementation/evidence portion of Gate D. It does not open
automatic reclamation, `rxtvm` selection, multi-worker execution, transport or
channel/RXAS semantics. Intel Linux, Linux ARM64 and same-machine Windows
rebuild-together ABI validation remain Gate D work before Gate E.

## Source and selected representation

- Worktree: `/private/tmp/crexx-rxvm-gated.6HZvFK/source`.
- Branch: `codex/rxvm-gate-d-industrialise`.
- Base: `0d1fe884782ff369960b1c67c38127407ce54588`.
- Value ABI: version 2, layout L32SDH, 176 bytes on 64-bit UTF-8.
- Allocator: 64 KiB S0 slabs, power-of-two byte classes through 16 KiB,
  typed value/reference silos and exact oversized extents.
- Reset policy: R0 sticky reuse; no automatic reclamation or hot-path pressure
  check.
- Decimal shape: direct payload pointer with one co-allocated 16-byte raw
  `size_t` length/capacity header immediately before the payload.

## Correctness findings resolved during closeout

The broad Debug sweep exposed one genuine non-spawn correctness problem.
`POSCHAR_REG_REG_REG` used the UTF-8 byte length as a character-index loop
bound. For the Arabic digit table this searched beyond the ten real
codepoints, could reuse a stale codepoint and normalized `12.0` to `1200`.
The UTF build now iterates over `string_chars`; `NUTF8` retains the byte-count
bound. The previously failing classic `datatype` no-opt test passes under both
`rxbvm` and `rxtvm`, and both no-opt and opt forms are in the focused gates.

AddressSanitizer then exposed a test-injected allocator-family defect in
decimal reserve. When `RXVM_VALUE_MALLOC` selects an external libc-style test
allocator, no slab metadata exists and the exact requested allocation is now
used as the capacity contract. The production worker-allocation branch is
unchanged and the final `__text` and flattened `run()` geometry is identical
before and after this correction.

## Correctness and lifecycle evidence

- Focused Debug: 15/15 pass.
- Focused ordinary Release: 15/15 pass.
- Full Debug: 1,887/1,909 pass in the 30-way sweep. Serial rerun clears two
  parallel transients and leaves exactly 20 expected spawn-path failures.
- Every retained failure reports `RXVM memory teardown detected N live
  allocation(s)` and crosses either the `crexx` driver or ADDRESS CREXX spawn
  path. There is no remaining non-spawn correctness failure.
- Diagnostic proof showed the existing `crexxcmd_run_argv()` stdout and stderr
  reader threads both entering the same `memory_worker`. Concurrent 32-byte
  sidecar allocations can receive the same slot, with the observed failure
  reaching `clear_value_contents -> clear_value -> crexxcmd_run_argv`. The
  child `rxvme` tears down cleanly. This is the already accepted spawn migration
  gap, not a general L32SDH lifetime defect.
- Final Apple AddressSanitizer: 12/12 affected value/slab/stem/decimal tests
  plus 1/1 Unicode `datatype` no-opt pass with `detect_leaks=0`. LeakSanitizer
  is unsupported on this macOS runtime. Allocator counters and Debug teardown
  checks remain the ownership/leak evidence.
- The ASan linked-runtime setup independently found a pre-existing assembler
  use-after-free in `rxas_flow_proof.c`; it is outside RXVM and this diff and is
  not counted as a Gate D failure.

## Final performance evidence

Both post-`POSCHAR` blocks use the same production machine-code geometry, the
frozen 192-byte L32S control and the same compiled benchmark images. Each block
contains one warmup and 12 serial pairwise-balanced recorded rounds over Sieve,
Richards, Towers and canonical RexxCPS. All 208 processes pass their correctness
markers. Positive percentages favor L32SDH.

| Workload | Pairs | Paired mean | 95% interval | Favorable |
|---|---:|---:|---:|---:|
| Sieve | 24 | +1.573% | +1.290% to +1.856% | 24/24 |
| Richards | 24 | -0.699% | -2.080% to +0.682% | 12/24 |
| Towers | 24 | +2.717% | +2.035% to +3.399% | 23/24 |
| RexxCPS | 24 | +1.229% | +0.807% to +1.652% | 21/24 |
| Pooled core four | 96 | **+1.205%** | **+0.748% to +1.663%** | **80/96** |

The final exact-product block alone is +0.836% pooled with a 95% interval of
+0.047% to +1.626%, 39/48 favorable. Its Richards point estimate is -2.160%,
still inside the 3% regression guard; all other workload means are positive.
No sample was removed or appended.

## Artifact and installation proof

| Metric | L32S | Final L32SDH | Change |
|---|---:|---:|---:|
| `value` bytes | 192 | 176 | -16 (-8.333%) |
| `rxbvm` file bytes | 1,001,000 | 1,001,048 | +48 |
| `__text` bytes | 813,332 | 815,420 | +2,088 |
| flattened `run()` bytes | 531,180 | 533,440 | +2,260 |

The final timing-block candidate SHA-256 is
`23262695e031528857b058240faa86e4722f36dc56c772d8eb1a6923c0c0910a`.
The final source-level rebuild after comment/whitespace closeout is
`0a1060e850633656b6e5b1fabda6cbf0cb6bf0cdf74c326f44e98e887d8eed7c`
with the same file, `__text` and flattened `run()` sizes. The frozen control is
`2d3dc6c8df5e260cf6100413ee11d0bf4d497ba6ca775e12ebb9494abf138174`.
Mach-O link UUIDs make whole-file hashes rebuild-specific; executable geometry
is the stable comparison.

A full Release build and isolated-prefix install pass. The installed product
contains `rxvm -> rxbvm` and runs the installed-library Sieve check with the
expected marker. The broad Debug suite also passes both installed SDK consumer
tests.

The host was reserved exclusively, on AC at 80%, with low-power mode off for
both timing blocks.
