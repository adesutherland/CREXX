# PERF3 closeout portfolio-v2 scorecard

## Common-five throughput

Medians are normalized work per second; higher is better. `rxvm` is the
compiler-selected product. `rxtvm` is a concrete-engine control and is never a
second product aggregate. `*` means at least that cell remains variability-
flagged after the one permitted append.

| Workload | Equal work | product `rxvm` | `rxtvm` control | ooRexx | decimal NetRexx |
| --- | ---: | ---: | ---: | ---: | ---: |
| Sieve | 5,500 | 4,857.607 | 4,873.470 | 693.056 | 2,539.001 |
| Permute | 5,000 | 2,456.713 | 2,326.232 | 287.179* | 4,434.600* |
| Bounce | 4,200 | 3,264.322 | 3,197.514 | 820.526 | 1,967.847* |
| Richards | 20 | 8.095 | 8.155 | 9.756* | 15.258 |
| Base64-v2 | 18,000 | 14,805.407 | 12,905.480 | 1,039.800 | 1,577.317 |

| Comparison | N | Geometric mean | Disposition |
| --- | ---: | ---: | --- |
| product `rxvm / ooRexx` | 5 | 4.897751 | aggregate ahead; Richards remains below parity |
| product `rxvm / NetRexx` | 5 | 1.543319 | aggregate ahead; Permute and Richards remain below parity |
| `rxtvm / ooRexx` control | 5 | 4.703857 | diagnostic control only |
| `rxtvm / NetRexx` control | 5 | 1.482221 | diagnostic control only |

The common-five identity changed when Base64-v2 replaced Base64-v1. These
geometric means therefore start a v2 series and must not be presented as an
improvement over the retained v1 aggregate.

## Product ratios

Each ratio is product/reference; higher is better.

| Workload | vs ooRexx | vs decimal NetRexx | Disposition |
| --- | ---: | ---: | --- |
| Sieve | 7.008970 | 1.913196 | ahead of both |
| Permute | 8.554628* | 0.553987* | NetRexx deficit; noisy reference rows retained |
| Bounce | 3.978329 | 1.658829* | ahead; NetRexx row remains noisy |
| Richards | 0.829769* | 0.530537 | material remaining deficit |
| Base64-v2 | 14.238704 | 9.386450 | strong new idiomatic-byte cell; not a v1 comparison |

## Separate lanes

| Lane | Product median | Comparator median | Ratio / disposition |
| --- | ---: | ---: | --- |
| Towers, elapsed for 100 repetitions | 2,377.611 ms* | ooRexx 1,375.771 ms | `0.578636`; product remains slower. NetRexx is a binary/JVM control, not a Rexx comparator. |
| RexxCPS 2.2d native rate | 38.006 MCPS | canonical ooRexx 34.102 MCPS* | `1.114476`; product ahead on this session |
| RexxCPS controls | 38.006 MCPS | Regina 29.004 MCPS*; NetRexx 38.226 MCPS* | `1.310360` vs Regina and `0.994240` vs the disclosed NetRexx 2.2n adaptation; controls remain outside the aggregate |

The product/control medians for the new cREXX-only lanes are: Storage
1,603.998/1,610.935 ms; List 1,421.423/1,461.632 ms; JSON-legacy
1,273.207/1,294.164 ms; JSON-parse 1,262.202/1,318.490 ms; JSON-query
1,436.830/1,465.235 ms; Queens 1,433.309/1,504.436 ms; and NBody
1,687.385/1,753.315 ms (`rxvm/rxtvm`). JSON and Queens flagged rows remain in
those medians. NBody is a native-math control and discloses `rxmath`.

## RSS and lifecycle

Product peak-RSS medians are 17.44-17.79 MiB outside Storage; Storage is
276.72 MiB by design of its allocation workload. RSS is descriptive and not a
throughput score.

Lifecycle medians are 113.684 ms cREXX compile, 15.817 ms assemble, 15.741 ms
product load-to-first-result, 15.205 ms ooRexx translate, 15.720 ms ooRexx
load-to-first-result, 484.971 ms NetRexx compile and 42.889 ms NetRexx
load-to-first-result. All seven rows remain variability-flagged. The command
surfaces do not expose a portable load-without-execution boundary, so none is
relabelled as pure load.
