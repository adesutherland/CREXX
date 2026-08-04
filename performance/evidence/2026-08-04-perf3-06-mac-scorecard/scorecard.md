# PERF3-06 accepted-product Mac scorecard

## Scope and identities

This is the formal same-session Apple ARM64 absolute scorecard for clean
detached accepted product `5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2`.
The common aggregate is exactly Sieve, Permute, Bounce, Richards and Base64
(`N=5`).  Towers and RexxCPS remain separate governed lanes; Mandelbrot,
Storage, List and JSON retain their approved no-ratio dispositions.

Timing used the ordinary profiling-off `Release` product with source/TRACE
metadata retained, two warmups and ten recorded serial rotated observations
per absolute cell.  The governed ten-sample append was applied to
`bounce-oorexx`, `base64-rxvm` and `base64-rxbvm`.  All three remain
noise-labelled; no sample was removed and no second append was taken.

| Product | Bytes | SHA-256 |
| --- | ---: | --- |
| `rxvm` | 982,424 | `5a4641a60a81c8679773b37b6397bd9080f97417090878dd4d99c997c503b409` |
| `rxbvm` | 999,144 | `64784a966562fa457750f7fab375e8671c06976e5877599b28c4b5e1eadf9727` |
| `library.rxbin` | 937,413 | `56d215e2d6222fba7fff806a33aaace6676c68ec4e425df50d9b6493f2053a5f` |

## Common-five throughput

Medians are normalized equal work per second; higher is better.  cREXX cells
show `rxvm / rxbvm`.  `*` remains noisy after the one permitted append.

| Workload | Equal work | cREXX | ooRexx | decimal NetRexx |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 5,500 | 4,944.870 / 4,432.337 | 714.825 | 2,732.749 |
| Permute | 5,000 | 2,518.980 / 2,310.786 | 316.668 | 4,333.997 |
| Bounce | 4,200 | 3,681.108 / 3,406.523 | 994.253* | 2,029.068 |
| Richards | 20 | 6.447 / 6.308 | 11.426 | 17.488 |
| Base64 | 2,500 | 1,636.951* / 1,544.558* | 2,118.473 | 1,822.221 |

| Comparison | Geometric mean | Outcome |
| --- | ---: | --- |
| `rxvm / ooRexx` | 2.453066x | Meets the 2.00x aggregate band; Richards and Base64 remain below parity. |
| `rxbvm / ooRexx` | 2.285744x | Newly clears the 2.00x aggregate band in this absolute scorecard; Richards and Base64 remain below parity. |
| `rxvm / NetRexx` | 0.912280x | Below parity; Permute, Richards and Base64 remain deficits. |
| `rxbvm / NetRexx` | 0.850054x | Below parity; Permute, Richards and Base64 remain deficits. |

## Exact per-workload ratios and remaining gain

Each ratio is cREXX/reference, higher is better.  The needs are additional
multiplicative gains from the current product.

| Workload | VM | vs ooRexx | Parity need | 1.50x need | vs NetRexx |
| --- | --- | ---: | ---: | ---: | ---: |
| Sieve | `rxvm` | 6.917593x | met | met | 1.809486x |
| Sieve | `rxbvm` | 6.200587x | met | met | 1.621933x |
| Permute | `rxvm` | 7.954639x | met | met | 0.581214x |
| Permute | `rxbvm` | 7.297186x | met | met | 0.533177x |
| Bounce | `rxvm` | 3.702385x | met | met | 1.814187x |
| Bounce | `rxbvm` | 3.426212x | met | met | 1.678861x |
| Richards | `rxvm` | 0.564256x | 1.772x | 2.658x | 0.368665x |
| Richards | `rxbvm` | 0.552019x | 1.812x | 2.717x | 0.360669x |
| Base64 | `rxvm` | 0.772703x* | 1.294x | 1.941x | 0.898328x* |
| Base64 | `rxbvm` | 0.729090x* | 1.372x | 2.057x | 0.847624x* |

Richards remains the largest qualified common deficit.  Sieve, Permute and
Bounce remain decisive guards.  Base64 remains both below parity and formally
noisy, so this scorecard does not claim that its qualified deficit is closed.

## Separate qualified lanes

| Lane | cREXX median | Comparator median | Ratio / remaining need |
| --- | ---: | ---: | --- |
| Towers, elapsed for 100 repetitions | 2,858.298 / 2,864.959 ms | ooRexx 1,117.142 ms | `0.390842x / 0.389933x`; needs 2.559x/2.565x to parity. NetRexx 33.889 ms remains an excluded binary/JVM control. |
| RexxCPS native rate | 46.155 / 45.433 MCPS | canonical ooRexx 40.089 MCPS | `1.151301x / 1.133307x`; both clear parity but still need 1.303x/1.324x to the 1.50x band. |
| RexxCPS controls | — | Regina 33.127; NetRexx 49.609 MCPS | Labelled controls only; the cREXX 2.2d and NetRexx 2.2n adaptations do not enter the common aggregate. |

## Static product movement

Compared with the exact retained PERF3-01 product, selected program static VM
instructions change as follows:

| Workload | PERF3-01 | PERF3-06 | Delta |
| --- | ---: | ---: | ---: |
| Sieve | 72 | 72 | 0 |
| Permute | 209 | 209 | 0 |
| Bounce | 352 | 356 | +4 |
| Richards | 1,815 | 1,805 | -10 |
| Base64 | 609 | 606 | -3 |
| Towers | 557 | 552 | -5 |
| RexxCPS | 1,240 | 1,222 | -18 |

The shared `library.rxbin` grows by 362 static instructions (0.582%) and 1,496
bytes.  This includes the accepted ordered TRACE and signal-library contract
work; it is a static loaded-module cost, not a claim that every added
instruction executes in each workload.  Exact image sizes, hashes, locals and
module-set totals are retained in [`static/`](static/).

## Historical context and claim boundary

PERF3-01 recorded common-five geometric means of `2.139811x/1.818954x` versus
ooRexx and `0.779920x/0.662974x` versus NetRexx.  The current absolute means
are descriptively 14.639%/25.663% and 16.971%/28.218% higher, respectively.
Richards' current ooRexx ratios are 2.166x/2.156x the PERF3-01 ratios, and the
current RexxCPS ratios are descriptively 19.402%/25.088% higher.  These are
unmatched-session accounting comparisons, not paired regression estimates.
The accepted individual candidate verdicts remain the causal evidence for
their own product decisions.

The PERF3 north star is therefore only partly met:

- both VMs clear the 2.00x ooRexx common-five aggregate band;
- both clear 1.50x on Sieve, Permute and Bounce;
- neither clears parity or 1.50x on Richards or Base64;
- neither RexxCPS lane clears its separate 1.50x band; and
- no default or alternate VM can yet be said to beat ooRexx on every qualified
  common cell.

This is an Apple ARM64 absolute outcome scorecard.  It does not select a
default VM, establish a cross-platform result, re-open a rejected option,
replace D0.5 assembler RSS evidence, or authorize a new production candidate.
