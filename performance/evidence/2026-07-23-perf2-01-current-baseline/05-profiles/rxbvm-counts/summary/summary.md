# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1314 ranked instruction rows, 1098 procedure-metric rows, 198 allocation rows, and 10999 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 30173000 | 26773000 | -3400000 | -11.268 |
| permute | 60691000 | 91692000 | 31001000 | 51.080 |
| mandelbrot | 178629000 | 170036000 | -8593000 | -4.811 |
| towers | 342386000 | 737055000 | 394669000 | 115.270 |
| bounce | 319941000 | 360026000 | 40085000 | 12.529 |
| storage | 789003000 | 1799013000 | 1010010000 | 128.011 |
| list | 213703000 | 223507000 | 9804000 | 4.588 |
| richards | 49692000 | 594088000 | 544396000 | 1095.541 |
| json | 323118000 | 309916000 | -13202000 | -4.086 |
| base64 | 492686000 | 313471000 | -179215000 | -36.375 |
| rexxcps | 742362000 | 1011787000 | 269425000 | 36.293 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
