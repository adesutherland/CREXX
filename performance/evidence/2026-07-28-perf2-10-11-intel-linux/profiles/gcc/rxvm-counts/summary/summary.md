# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1303 ranked instruction rows, 1074 procedure-metric rows, 198 allocation rows, and 10716 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 78253000 | 91039000 | 12786000 | 16.339 |
| permute | 149247000 | 113401000 | -35846000 | -24.018 |
| mandelbrot | 612312000 | 558814000 | -53498000 | -8.737 |
| towers | 1149843000 | 1103118000 | -46725000 | -4.064 |
| bounce | 198051000 | 166944000 | -31107000 | -15.707 |
| storage | 3127825000 | 4220103000 | 1092278000 | 34.921 |
| list | 517496000 | 303943000 | -213553000 | -41.267 |
| richards | 223518000 | 1189110000 | 965592000 | 431.997 |
| json | 964436000 | 1004669000 | 40233000 | 4.172 |
| base64 | 1596854000 | 1331214000 | -265640000 | -16.635 |
| rexxcps | 1273600000 | 2069118000 | 795518000 | 62.462 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
