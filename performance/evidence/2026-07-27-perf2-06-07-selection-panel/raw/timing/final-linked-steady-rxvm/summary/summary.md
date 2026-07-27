# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 12 exact-image entries, 773 ranked instruction rows, 684 procedure-metric rows, 108 allocation rows, and 7208 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 13751000 | 12739000 | -1012000 | -7.359 |
| permute | 31519000 | 69229000 | 37710000 | 119.642 |
| bounce | 43554000 | 29230000 | -14324000 | -32.888 |
| richards | 24326000 | 436204000 | 411878000 | 1693.160 |
| base64 | 370151000 | 321907000 | -48244000 | -13.034 |
| rexxcps | 987452000 | 940089000 | -47363000 | -4.796 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
