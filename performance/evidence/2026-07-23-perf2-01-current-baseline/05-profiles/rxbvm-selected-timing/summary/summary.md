# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 10 exact-image entries, 693 ranked instruction rows, 654 procedure-metric rows, 90 allocation rows, and 6981 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 27179000 | 27014000 | -165000 | -0.607 |
| bounce | 326394000 | 350048000 | 23654000 | 7.247 |
| richards | 49742000 | 605704000 | 555962000 | 1117.691 |
| base64 | 503966000 | 331970000 | -171996000 | -34.128 |
| rexxcps | 969797000 | 1032992000 | 63195000 | 6.516 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
