# PERF3-11 K04e accepted-product Mac scorecard

## Scope

This is the formal same-session Apple ARM64 absolute scorecard for clean
detached accepted product `c4470635048e497417c4db92c03ecbcd79eaa750`.
Timing used the ordinary profiling-off Release product with source/TRACE
metadata retained, two warmups and ten recorded serial-rotated observations
per absolute cell. One governed ten-sample append was applied to
`permute-netrexx`, `base64-rxvm` and `base64-rxbvm`; all three remain noisy.

| Product | Bytes | SHA-256 |
| --- | ---: | --- |
| `rxvm` | 982,424 | `a973c34663682c102e4df1eb8e2abe646ae36cb66f8c852f7a81b9ef562eae67` |
| `rxbvm` | 999,144 | `73940500db81514c90c49601c57e4329f796e747dbffcb5576aa1d3d3b492d39` |
| `library.rxbin` | 936,949 | `7322d7ef98f30ecfc6ce52ac3e976a26fdeda5e24d0dfc2be4d3e83a2e68971f` |

## Common-five throughput

Medians are normalized equal work per second; higher is better. cREXX cells
show `rxvm / rxbvm`. `*` denotes a cell that remains noisy after the append.

| Workload | Equal work | cREXX | ooRexx | decimal NetRexx |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 5,500 | 4,961.805 / 4,433.274 | 711.654 | 2,747.814 |
| Permute | 5,000 | 2,487.332 / 2,305.488 | 306.970 | 4,528.528* |
| Bounce | 4,200 | 3,727.802 / 3,385.967 | 965.495 | 2,025.545 |
| Richards | 20 | 6.402 / 6.310 | 11.394 | 17.640 |
| Base64 | 2,500 | 1,581.665* / 1,562.697* | 2,126.927 | 1,828.590 |

| Comparison | Geometric mean | Outcome |
| --- | ---: | --- |
| `rxvm / ooRexx` | 2.465740x | Meets 2.00x; Richards and Base64 remain below parity. |
| `rxbvm / ooRexx` | 2.316900x | Meets 2.00x; Richards and Base64 remain below parity. |
| `rxvm / NetRexx` | 0.894608x* | Below parity; Permute, Richards and Base64 remain deficits. |
| `rxbvm / NetRexx` | 0.840606x* | Below parity; Permute, Richards and Base64 remain deficits. |

Exact per-workload ratios are retained in `timing/ratios.csv`. Sieve, Permute
and Bounce remain decisive ooRexx guards. Richards remains the largest stable
qualified common deficit. Base64 is below parity but too noisy for a firm
session-to-session movement claim.

## Separate qualified lanes

| Lane | cREXX median | Comparator median | Ratio / remaining need |
| --- | ---: | ---: | --- |
| Towers, 100 repetitions | 2,839.109 / 2,871.281 ms | ooRexx 1,118.052 ms | `0.393804x / 0.389391x`; needs 2.539x/2.568x to parity. |
| RexxCPS native rate | 46.349 / 45.253 MCPS | ooRexx 40.023 MCPS | `1.158075x / 1.130694x`; needs 1.295x/1.327x to 1.50x. |
| RexxCPS controls | — | Regina 32.996; NetRexx 48.057 MCPS | Labelled controls only. |

## Static movement

Compared with the exact PERF3-06 scorecard product, the accepted current
product has no static increase:

| Workload | PERF3-06 | Current | Delta |
| --- | ---: | ---: | ---: |
| Sieve | 72 | 72 | 0 |
| Permute | 209 | 209 | 0 |
| Bounce | 356 | 352 | -4 |
| Richards | 1,805 | 1,802 | -3 |
| Base64 | 606 | 604 | -2 |
| Towers | 552 | 551 | -1 |
| RexxCPS | 1,222 | 1,221 | -1 |

The shared library moves from 62,606 to 62,326 static instructions and from
937,413 to 936,949 bytes. These are cumulative accepted-product deltas since
PERF3-06. Only the final RexxCPS instruction is attributed to K04e by its
separate exact before/after evidence.

## Historical comparison and boundary

Relative to the unmatched PERF3-06 absolute session, the current geometric
means are descriptively +0.517%/+1.363% versus ooRexx and
-1.937%/-1.111% versus NetRexx for `rxvm`/`rxbvm`. Comparator movement and the
remaining noisy cells prevent a causal regression interpretation; the 1%
aggregate guard applies to matched candidate evidence, not these independent
sessions. The scorecard therefore confirms the accepted product remains
functionally equivalent and within the same high-level ranking: ooRexx common
aggregate target met, Richards/Base64 and Towers still open, and RexxCPS above
parity but below 1.50x.
