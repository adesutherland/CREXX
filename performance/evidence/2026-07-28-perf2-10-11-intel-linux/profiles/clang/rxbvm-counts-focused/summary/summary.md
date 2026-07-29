# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 12 exact-image entries, 599 ranked instruction rows, 504 procedure-metric rows, 108 allocation rows, and 4559 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 114042000 | 72578000 | -41464000 | -36.359 |
| permute | 123571000 | 99341000 | -24230000 | -19.608 |
| towers | 1120902000 | 1677216000 | 556314000 | 49.631 |
| bounce | 164552000 | 122747000 | -41805000 | -25.405 |
| richards | 103284000 | 1845871000 | 1742587000 | 1687.180 |
| base64 | 1511010000 | 1125861000 | -385149000 | -25.490 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
