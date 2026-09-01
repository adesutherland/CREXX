# NR-03 automated performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 10 exact-image entries, 675 ranked instruction rows, 984 procedure-metric rows, 90 allocation rows, and 9108 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 24787000 | 23698000 | -1089000 | -4.393 |
| bounce | 377711000 | 389054000 | 11343000 | 3.003 |
| richards | 61788000 | 656396000 | 594608000 | 962.336 |
| base64 | 499289000 | 452153000 | -47136000 | -9.441 |
| rexxcps | 412047000 | 421376000 | 9329000 | 2.264 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
