# CRI-09 A2 ordinary Release verdict

Status: **pass; accepted by Adrian on 2026-07-30**

Date: 2026-07-30

## Frozen candidate

Adrian approved A2, the lean hybrid streaming core, and required its
performance verdict before B1 closure. A2 retains one recursive JSON grammar
and the 256-byte table-driven token-boundary classifier. Tight handwritten
loops consume string and number interiors; query paths retain precomputed
binary segments; unescaped object keys compare directly against source bytes;
validation avoids path parsing; and each recursive frame caches its hot
mode/count/capture state as scalars.

- production `rxjson.crexx` SHA-256:
  `c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318`;
- unchanged benchmark SHA-256:
  `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b`;
- focused correctness: 11/11 passed, zero failed or skipped, across legacy,
  document and noisy contracts, optimized/non-optimized compilation, and both
  VMs;
- focused log `/tmp/cri09-a2-focused.log` SHA-256:
  `2c2cd916eb8b03655abe84f5c605d9d49706f4c4710078ccececb990174a5b00`;
- `git diff --check` passed at freeze and verdict.

No production or benchmark source changed after the freeze. B1 documentation,
broad Debug, sanitizer, install/package and other closeout work did not run.
B1 is already defined and approved as accepting one allocation-free
boundary-validation pass plus one indexing pass for the successful slice. It
requires documentation and closure evidence after A2 acceptance, not another
tokenizer implementation.

## Ordinary Release product and method

The clean product is `/tmp/crexx-cri09-release-a2.N4ELYs/build`, configured
with Ninja, `Release`, `CREXX_VM_PROFILING=OFF`, `ENABLE_PARSER_MODE=OFF` and
`BUILD_TESTING=ON`. AppleClang is `21.0.0.21000101`; source HEAD is
`d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf`.

| Artifact | SHA-256 |
| --- | --- |
| `library.rxbin` | `e8a053609d5abf58b39f9621a7d86a3a1b7ea109f8db7e416970b6459cc61826` |
| `rxvm` | `b1bd31897ac26a4378f52f9498999a7b0b1d32c91443ce908c36ecadf888cb9e` |
| `rxbvm` | `92bcf6d11bb9c393b7116ac04565f641ea708e7ed9578b4dde31936c77d5538a` |
| optimized linked benchmark | `fec439ef32058fb8b0315793a75db0e5c45bf6066c06440a2727462dcb2c1ebe` |
| non-optimized linked benchmark | `90093b9bec2bc914c08350a14f756e3a5ae1d8bc57acb03fb818e3eb3344087a` |
| optimized benchmark RXAS | `4b2a0563999680e0e09efaa4d41489c01ca3a2aa203302ccf45bf3fe0e0b1b25` |
| non-optimized benchmark RXAS | `a1edfed556e2e36def9c92b32755afe6ecfa9e63fa54417e8bf2647ee00b44fc` |

The VM executables are byte-identical across pre-edit, V1, V2 and A2.

The maintained workload is unchanged: 60 rows, 30 operations, 4,394 payload
bytes and exact result checks. After three warmups per product and VM, 12
balanced optimized quadruplets ran pre-edit, V1, V2 and A2. Each product
occupied every slot three times. A separate A2 optimized/non-optimized run used
three warmups and 12 alternating pairs per VM. No recorded sample failed and no
outlier was removed.

The preliminary correctness smoke ran intentionally before warmups and showed
large cold-start optimized times. It is retained in `smoke.log` but was never a
declared measurement sample. The balanced warmed samples do not reproduce an
optimizer inversion.

Retained evidence under `/tmp/crexx-cri09-release-a2.N4ELYs/`:

| Evidence | SHA-256 |
| --- | --- |
| `quad-raw.log` | `0951242d5823bd5b6efee50833680021e7d4a3fce1490fe8b57aeb3db8367129` |
| `quad-medians.txt` | `34c219bc8b72c3f03751d175a9befe939467508d50ed3c726a50bbe19b429332` |
| `a2-opt-noopt-raw.log` | `797202250f93f332bfd6171dfe86c8346e6c4d436c4e3eddb02875d9885f4237` |
| `a2-opt-noopt-medians.txt` | `6cb9589b43d95d0f07d69425237aceadbef2fc898f62ee165a40ce0dba7bcb08` |
| `a2-verdict-calculations.txt` | `3883f43ac167aa12b5ddd8f77ae0073303acf7a8e978daf5a2e218281f31a020` |
| `a2-optimizer-calculations.txt` | `2db7e9e9ce3daefdd9be2b5788ae1919304f436307544eea8ef0e86c8b980a04` |
| `static-instruction-summary.txt` | `516d3931ba5a9df8b46229a46a8d83e1f1659dc7e70396bff1b8a76682f602a3` |
| `smoke.log` | `ae07c38f49bef68152a76fd8d059936b384f78630042810fb1a0eed243e3034c` |

## Verdict against the unchanged rule

Positive compatibility changes are slower. Every cell is within the maximum
25% regression guard.

| VM | Operation | Pre-edit | V2 | A2 | A2 vs pre-edit | Guard |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| `rxvm` | valid | 3,803.5 us | 5,097.0 us | 4,472.5 us | +17.59% | pass |
| `rxvm` | deep get | 4,091.0 us | 4,983.5 us | 4,355.0 us | +6.45% | pass |
| `rxvm` | tail get | 4,021.0 us | 5,039.5 us | 4,515.0 us | +12.29% | pass |
| `rxvm` | count | 7,172.5 us | 4,846.0 us | 4,290.5 us | -40.18% | pass/improves |
| `rxvm` | members | 3,806.0 us | 4,907.5 us | 4,294.0 us | +12.82% | pass |
| `rxbvm` | valid | 4,710.0 us | 6,526.0 us | 5,549.5 us | +17.82% | pass |
| `rxbvm` | deep get | 5,096.0 us | 6,469.0 us | 5,597.5 us | +9.84% | pass |
| `rxbvm` | tail get | 5,004.0 us | 6,499.0 us | 5,605.0 us | +12.01% | pass |
| `rxbvm` | count | 9,092.5 us | 6,366.5 us | 5,567.5 us | -38.77% | pass/improves |
| `rxbvm` | members | 4,955.5 us | 6,358.5 us | 5,573.5 us | +12.47% | pass |

A2 improves every legacy cell by 10.41--14.96% versus V2.

The retained parse-once criterion passes with margin. Construction plus 30
indexed path gets is 955.0 us (`rxvm`) and 1,048.5 us (`rxbvm`), a
76.25%/79.05% reduction from matched legacy repeated parsing. Construction
plus 30 resolved-node gets reduces the same totals by 93.87%/94.11%.

The scanner remains decisive: 394.5 us (`rxvm`) and 480.5 us (`rxbvm`),
99.92%/99.91% below V1 and 3.55%/6.15% faster than V2.

There is no material optimizer inversion. Across all measured cells the
optimized/non-optimized difference ranges from -2.17% to +2.86%, with no
consistent adverse shape. All exact checksums pass.

Static Release RXAS shrinks from V2's 2,627,055 bytes/51,969 lines to A2's
2,432,257 bytes/48,089 lines. Object/array parser bodies shrink from 444/284 to
375/270 diagnostic static instructions. These counts reflect compiler inlining
and are corroborative, not substitutes for wall clock.

The A2 ordinary Release verdict is **pass**. The implementation remains
frozen at the measured source hash. Adrian accepted the verdict on 2026-07-30;
the approved B1 documentation and broader CRI-09 validation subsequently
passed. Final evidence is in [`CRI09-CLOSEOUT.md`](CRI09-CLOSEOUT.md).

## Smallest exact decision

> Accept or reject frozen A2 at SHA-256
> `c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318`
> as the CRI-09 production candidate. Acceptance authorizes the already
> approved B1 documentation/closure and proportional broad CRI-09 validation;
> it does not authorize a new public surface, ABI, format or later CRI item.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-09 A2 ordinary Release
stop. Read performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI09-A2-RELEASE-VERDICT.md.

I accept frozen A2 at rxjson.crexx SHA-256
c2ff6f246ecf8f13837cd82a1f6c5e35cc6f855fa18889eef6a48ee0128a9318.
Proceed with the already approved B1 documentation/closure, then the required
focused sanitizer, full Debug CTest and proportional CRI-09 closeout. Preserve
the public Option B API, ABI and formats; do not start CRI-10 or touch
crexx-rag until CRI-09 is accepted and its worklist evidence is complete.
```
