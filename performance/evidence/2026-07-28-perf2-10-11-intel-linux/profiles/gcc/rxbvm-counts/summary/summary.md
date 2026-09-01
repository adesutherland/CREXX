# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1302 ranked instruction rows, 1074 procedure-metric rows, 198 allocation rows, and 10719 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 82126000 | 77689000 | -4437000 | -5.403 |
| permute | 145294000 | 119305000 | -25989000 | -17.887 |
| mandelbrot | 600156000 | 614943000 | 14787000 | 2.464 |
| towers | 1124876000 | 1772023000 | 647147000 | 57.531 |
| bounce | 198579000 | 163001000 | -35578000 | -17.916 |
| storage | 3081826000 | 3338680000 | 256854000 | 8.334 |
| list | 535412000 | 333810000 | -201602000 | -37.654 |
| richards | 123899000 | 1215176000 | 1091277000 | 880.780 |
| json | 893137000 | 869745000 | -23392000 | -2.619 |
| base64 | 1638693000 | 1370188000 | -268505000 | -16.385 |
| rexxcps | 1026691000 | 976834000 | -49857000 | -4.856 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
