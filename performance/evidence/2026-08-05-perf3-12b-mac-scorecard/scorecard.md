# PERF3-12B accepted-product Mac scorecard

## Scope

This is the formal same-session Apple ARM64 absolute scorecard for clean merged
product `44d8b6a7ecd7800979b5db992c14bc7182aa89dd`. Timing used the
ordinary profiling-off Release product with source/TRACE metadata retained,
two warmups and ten recorded serial-rotated observations per absolute cell.
No cell requires an append.

| Product | Bytes | SHA-256 |
| --- | ---: | --- |
| `rxvm` | 982,424 | `9145a1ea01dbee4ea3dd90c0da88685b2adb5a3cf44254acda9967731eadf186` |
| `rxbvm` | 999,144 | `6efb8479b42eaa18f641cd9894400d110b0f8eb8ee9284b938f15f13224dec8e` |
| `library.rxbin` | 933,969 | `9af6f34243b1b49e8f6d1cd3f0747e09b594f6629c7f8c08301efa1cb1aa18fa` |

## Common-five throughput

Medians are normalized equal work per second; higher is better. cREXX cells
show `rxvm / rxbvm`.

| Workload | Equal work | cREXX | ooRexx | decimal NetRexx |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 5,500 | 4,589.703 / 4,313.626 | 704.361 | 2,615.734 |
| Permute | 5,000 | 2,193.508 / 2,177.899 | 296.204 | 4,391.667 |
| Bounce | 4,200 | 3,064.544 / 3,169.232 | 909.044 | 2,027.000 |
| Richards | 20 | 6.049 / 6.076 | 10.647 | 16.930 |
| Base64 | 2,500 | 1,698.267 / 1,752.846 | 2,072.818 | 1,781.409 |

| Comparison | Geometric mean | Outcome |
| --- | ---: | --- |
| `rxvm / ooRexx` | 2.375939x | Meets 2.00x; Richards and Base64 remain below parity. |
| `rxbvm / ooRexx` | 2.376230x | Meets 2.00x; Richards and Base64 remain below parity. |
| `rxvm / NetRexx` | 0.852882x | Below parity; Permute, Richards and Base64 remain deficits. |
| `rxbvm / NetRexx` | 0.852987x | Below parity; Permute, Richards and Base64 remain deficits. |

Exact per-workload ratios are retained in `timing/ratios.csv`. Sieve, Permute
and Bounce remain decisive ooRexx guards. Richards remains the largest stable
qualified common deficit. Base64 is stable in this session but remains below
ooRexx parity and just below NetRexx parity.

## Separate qualified lanes

| Lane | cREXX median | Comparator median | Ratio / remaining need |
| --- | ---: | ---: | --- |
| Towers, 100 repetitions | 2,836.197 / 2,839.714 ms | ooRexx 1,138.504 ms | `0.399148x / 0.400611x`; needs 2.505x/2.496x to parity. |
| RexxCPS native rate | 47.203 / 47.093 MCPS | ooRexx 40.126 MCPS | `1.172472x / 1.165701x`; needs 1.279x/1.287x to 1.50x. |
| RexxCPS controls | — | Regina 33.212; NetRexx 48.299 MCPS | Labelled controls only. |

## Static product

| Workload | Static instructions | Locals | RXBIN bytes | Change from K04e |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 73 | 16 | 4,892 | +1 instruction / -8 bytes |
| Permute | 209 | 24 | 11,810 | unchanged |
| Bounce | 352 | 20 | 17,167 | unchanged |
| Richards | 1,802 | 56 | 79,014 | unchanged |
| Base64 | 595 | 40 | 37,829 | -9 / -320 bytes |
| Towers | 548 | 34 | 28,550 | -3 / -64 bytes |
| RexxCPS | 1,210 | 104 | 68,361 | -11 / -216 bytes |
| Shared library | 61,223 | — | 933,969 | -1,103 / -2,980 bytes |

The selected H1 route itself is established by the exact B4 comparison:
RexxCPS S0 has 1,214 static instructions and H1 has 1,210, while `main` moves
from 369 to 365 and locals from 103 to 104. Other K04e-to-current static deltas
are cumulative accepted-product movement and are not attributed to H1. The
Sieve +1 predates production H1 and is present in its byte-identical B4
zero-candidate control.

## Historical comparison and boundary

Relative to the unmatched K04e absolute session, current geometric means are
descriptively -3.642%/+2.561% versus ooRexx and -4.664%/+1.473% versus NetRexx
for `rxvm`/`rxbvm`. Comparator movement and independent host sessions prevent a
causal regression interpretation; the 1% aggregate guard applies to matched
candidate evidence, not independent absolute scorecards.

The scorecard confirms the accepted product remains functionally equivalent
and within the same high-level ranking: both VM aggregates meet the ooRexx
target; Richards, Base64 and Towers remain open; and RexxCPS is above ooRexx
parity but below 1.50x. B4/B5 remain the causal evidence for H1.
