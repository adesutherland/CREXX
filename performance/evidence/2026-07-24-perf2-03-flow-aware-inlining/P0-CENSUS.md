# P0 exact-current inline and cleanup census

Baseline: exact HEAD `086138f1e93da8e84d45f4cd3ba9b6620f792a14`, ordinary
profiling-off Release products. Static counts are from deterministic compiled
artifacts; dynamic counts are procedure/instruction profiles from the same
products. The maintained artifact summarizer was written and run in cREXX
Level B.

## Static product panel

| Workload | No-opt instructions / peak locals / RXBIN bytes | Current inline instructions / peak locals / RXBIN bytes | Current copies / branches / calls |
| --- | ---: | ---: | ---: |
| List | 246 / 30 / 16,421 | 233 / 34 / 16,226 | 9 / 14 / 21 |
| Permute | 105 / 19 / 7,584 | 227 / 30 / 11,965 | 22 / 15 / 6 |
| Richards | 792 / 28 / 39,772 | 1,897 / 66 / 79,646 | 223 / 182 / 25 |
| JSON | 59 / 11 / 4,652 | 46 / 9 / 4,033 | 3 / 8 / 1 |
| RexxCPS | 934 / 104 / 48,971 | 1,402 / 105 / 77,470 | 89 / 276 / 27 |

The linked library image is 54,722 instructions and 858,081 bytes.

## Dynamic call and instruction deltas

| Workload | Calls no-opt -> current | Removed calls | `rxvm` instructions no-opt -> current | `rxbvm` instructions no-opt -> current |
| --- | ---: | ---: | ---: | ---: |
| List | 4,396,101 -> 4,395,901 | 200 (0.005%) | 52,442,726 -> 50,752,126 (-3.224%) | same measured delta |
| Permute | 937,001 -> 432,951 | 504,050 (53.794%) | 15,463,326 -> 12,906,926 (-16.532%) | same measured delta |
| Richards | 469,639 -> 119,158 | 350,481 (74.628%) | 9,302,120 -> 9,119,155 (-1.967%) | same measured delta |
| JSON | 745,001 -> 745,001 | 0 | 118,625,035 -> 118,610,025 (-0.013%) | same measured delta |
| RexxCPS | 593,044 -> 348,044 | 245,000 (41.312%) | 27,629,221 -> 25,785,119 (-6.674%) | 27,629,249 -> 25,264,347 (-8.559%) |

The instruction saving per removed call is about 5.07 for Permute, 0.52 for
Richards and 7.53/9.65 for RexxCPS on `rxvm`/`rxbvm`. Richards is the clearest
cleanup target: inlining removes many calls but most of the theoretical gain is
consumed by expanded scaffolding.

## Ranked site dispositions

| Workload/site family | Current evidence | Disposition |
| --- | --- | --- |
| Richards small helpers | 350,481 calls removed, but optimized output adds 1,105 instructions, 38 peak locals, 221 copies and 119 branches for only 182,965 fewer dynamic instructions | cleanup required; retain only candidates that win after cleanup |
| RexxCPS `substr`, `word`, `length` | large dynamic call removal and 1.84M/2.36M both-VM instruction savings, but output adds 468 static instructions, 40 copies, 180 branches and 28,499 bytes | cleanup required; retain numeric-context rejection |
| Permute `swap` | 503,900 calls removed and 2.56M instructions saved; static image grows but measured dynamic value is clear | inline; protect with portfolio/image guard |
| Permute recursive `permute` | 432,950 residual calls; rejected for recursive cycle at lines 47/50 | do not inline |
| List reference/object helpers | hot residuals include `next` 3,820,600 and three 280,900-call helpers; structural diagnostics prove reference-bearing/unsupported-reference or class-write obligations are missing | proof missing at sites; do not blanket-exclude the procedures |
| JSON imported parse helpers | 655,000 `_json_parse_value`, 80,000 `_json_parse_object`, 5,000 `_json_parse_path`; imported template reports no instruction list | imported semantic-summary gap; retain calls |
| RexxCPS `upper` | 274,400 residual calls with actual/formal binding failures | site-binding proof required |
| RexxCPS `cps_subroutine`/one `word` site | 68,600 residual calls / selected residual `word`; numeric contexts differ | do not inline unless exact context equivalence is proved |
| Tiny/failing factories | small accepted current sites with low dynamic value | inline only when final cleaned cost wins |

This census supports per-site proof and profitability. It does not support a
whole-procedure ban for references, calls, TRACE, imported code or handwritten
RXAS.
