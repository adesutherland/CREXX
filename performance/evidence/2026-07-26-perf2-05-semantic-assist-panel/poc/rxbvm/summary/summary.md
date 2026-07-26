# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 8 exact-image entries, 425 ranked instruction rows, 312 procedure-metric rows, 72 allocation rows, and 2886 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| list-r1 | 86077000 | 84680000 | -1397000 | -1.623 |
| list-r2 | 86101000 | 80802000 | -5299000 | -6.154 |
| list-r12 | 86053000 | 79510000 | -6543000 | -7.603 |
| base64-b1 | 317608000 | 304680000 | -12928000 | -4.070 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
