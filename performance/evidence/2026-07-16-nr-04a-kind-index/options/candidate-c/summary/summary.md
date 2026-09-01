# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 10 exact-image entries, 416 ranked instruction rows, 180 procedure-metric rows, 90 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| runtime-interface-retained-C | 221466000 | 98789000 | -122677000 | -55.393 |
| runtime-interface-stripped-C | 202647000 | 108491000 | -94156000 | -46.463 |
| lifecycle-retained-C | 3589000 | 3929000 | 340000 | 9.473 |
| lifecycle-stripped-C | 3634000 | 3710000 | 76000 | 2.091 |
| JSON-stripped-C | 403927000 | 387765000 | -16162000 | -4.001 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
