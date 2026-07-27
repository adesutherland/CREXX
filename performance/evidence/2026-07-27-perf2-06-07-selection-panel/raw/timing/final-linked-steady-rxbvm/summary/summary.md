# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 12 exact-image entries, 773 ranked instruction rows, 684 procedure-metric rows, 108 allocation rows, and 7221 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 16579000 | 16542000 | -37000 | -0.223 |
| permute | 37781000 | 76286000 | 38505000 | 101.916 |
| bounce | 55064000 | 41064000 | -14000000 | -25.425 |
| richards | 29453000 | 448990000 | 419537000 | 1424.429 |
| base64 | 464125000 | 325763000 | -138362000 | -29.811 |
| rexxcps | 971191000 | 955879000 | -15312000 | -1.577 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
