# P1-P4 isolated prototype results

Each prototype used an independent detached worktree at exact HEAD
`086138f1e93da8e84d45f4cd3ba9b6620f792a14` and a distinct Release build.
P4 is explicitly the only combination row.

## Static portfolio

Each cell is `instructions / peak locals / RXBIN bytes`. Copy/branch/call
details highlight selection-relevant changes.

| Workload | P0 current | P1 Q1 | P2 read-only formal | P3 result placement | P4 combined + 100-node fallback |
| --- | ---: | ---: | ---: | ---: | ---: |
| List | 233 / 34 / 16,226 | unchanged | 233 / 34 / 16,170 | unchanged | 233 / 34 / 16,170 |
| Permute | 227 / 30 / 11,965 | unchanged | 227 / 32 / 11,485 | 227 / 30 / 11,965 | 227 / 32 / 11,485 |
| Richards | 1,897 / 66 / 79,646 | 1,886 / 66 / 79,390 | 1,876 / 66 / 77,662 | 1,886 / 66 / 79,574 | 1,385 / 59 / 58,942 |
| JSON | 46 / 9 / 4,033 | unchanged | 46 / 9 / 3,977 | unchanged | 46 / 9 / 3,977 |
| RexxCPS | 1,402 / 105 / 77,470 | unchanged | 1,402 / 105 / 77,414 | 1,403 / 105 / 77,462 | 804 / 107 / 48,427 |

Richards copy counts are P0 223, P1 212, P2 202, P3 220 and P4 110. P4
also reduces Richards branches 182 -> 112 and peak locals 66 -> 59. The same
static threshold is not a valid general gate: RexxCPS residual call instructions
rise 27 -> 84 and peak locals 105 -> 107. P4's linked library shrinks from
54,722 instructions / 858,081 bytes to 33,091 / 714,925, demonstrating working
fallback ownership but over-broad rejection rather than a selected policy.

## Correctness ledger

| Prototype | Focused result | Interpretation |
| --- | --- | --- |
| P1 | 33/33 exact-head Q1 focused tests; canonical Richards passes both VMs | no semantic or golden regression |
| P2 | 118/118 inline runtime tests; canonical Richards passes both VMs; 22/115 optimized compiler goldens differ | runtime semantics pass; expected assembly-shape deltas require refreshed goldens only if selected |
| P3 | 118/118 inline runtime tests; canonical Richards passes both VMs; 10/115 optimized compiler goldens differ | runtime semantics pass; bounded placement changes assembly shape |
| P4 | 118/118 inline runtime tests and all five canonical workloads on both VMs; 26/115 optimized compiler goldens differ | combined semantics pass; broad assembly-shape change confirms this is an architecture probe |

The canonical P4 portfolio is List, Permute, Richards, JSON and no-argument
RexxCPS on `rxvm` and `rxbvm` (10/10). An initial `RexxCPS 1 1` invocation was
rejected by the benchmark's argument contract and was corrected to the
canonical no-argument run; it is setup correction, not a product failure.

## Exact-head P2 timing on stable AC power

Ordinary profiling-off Release binaries were sampled serially in rotated order:
one warmup plus three recorded Richards work=10 runs per cell. The capture ran
2026-07-24 13:16:52Z to 13:18:27Z. Power was AC before and after, low-power mode
was 0, and the battery was charging after capture. All benchmark outputs passed
with queue_packets=23,246 and holds=9,297.

| VM | P0 median | P2 median | Speed change | Relative MAD P0 / P2 |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 5.794188 s | 5.791194 s | +0.052% | 0.377% / 0.091% |
| `rxbvm` | 5.868553 s | 5.936940 s | -1.152% | 0.697% / 0.211% |

Neither result supports P2 as a standalone production optimization. The fact
belongs in the selected architecture, subject to register and final cost gates.

## Prototype verdicts

- **P1:** the strongest smallest production seed; precise, measured, no peak
  local regression. It is not the complete architecture.
- **P2:** useful early fact, unsafe as an unconditional rewrite and not an
  exact-head timing win.
- **P3:** useful post-clone mechanism, but needs profitability and ownership
  proof per site.
- **P4:** positive proof of detached fallback; negative proof for static
  node-count-only profitability. Do not ship its threshold.
