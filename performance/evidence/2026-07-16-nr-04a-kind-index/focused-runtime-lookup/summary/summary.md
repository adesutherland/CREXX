# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 4 exact-image entries, 200 ranked instruction rows, 96 procedure-metric rows, 36 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| runtime-interface-retained-vm | 204432000 | 106384000 | -98048000 | -47.961 |
| runtime-interface-stripped-vm | 170090000 | 107155000 | -62935000 | -37.001 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
