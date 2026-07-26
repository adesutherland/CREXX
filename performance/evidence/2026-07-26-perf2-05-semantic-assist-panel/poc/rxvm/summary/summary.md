# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 8 exact-image entries, 425 ranked instruction rows, 312 procedure-metric rows, 72 allocation rows, and 2886 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| list-r1 | 76918000 | 75185000 | -1733000 | -2.253 |
| list-r2 | 77357000 | 72582000 | -4775000 | -6.173 |
| list-r12 | 77828000 | 71041000 | -6787000 | -8.721 |
| base64-b1 | 312968000 | 313963000 | 995000 | 0.318 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
