# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 12 exact-image entries, 599 ranked instruction rows, 504 procedure-metric rows, 108 allocation rows, and 4559 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 94067000 | 72440000 | -21627000 | -22.991 |
| permute | 138319000 | 109752000 | -28567000 | -20.653 |
| towers | 1129601000 | 1100883000 | -28718000 | -2.542 |
| bounce | 178893000 | 132827000 | -46066000 | -25.751 |
| richards | 109096000 | 1220014000 | 1110918000 | 1018.294 |
| base64 | 1340565000 | 1124007000 | -216558000 | -16.154 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
