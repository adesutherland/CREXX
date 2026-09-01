# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 4 exact-image entries, 76 ranked instruction rows, 24 procedure-metric rows, 36 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| lifecycle-retained-vm | 3589000 | 3674000 | 85000 | 2.368 |
| lifecycle-stripped-vm | 3765000 | 5136000 | 1371000 | 36.414 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
