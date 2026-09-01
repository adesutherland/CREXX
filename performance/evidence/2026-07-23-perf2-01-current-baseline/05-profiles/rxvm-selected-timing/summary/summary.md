# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 10 exact-image entries, 694 ranked instruction rows, 654 procedure-metric rows, 90 allocation rows, and 6990 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 24367000 | 23685000 | -682000 | -2.799 |
| bounce | 308033000 | 330681000 | 22648000 | 7.352 |
| richards | 43158000 | 594409000 | 551251000 | 1277.286 |
| base64 | 410649000 | 360214000 | -50435000 | -12.282 |
| rexxcps | 984241000 | 1004676000 | 20435000 | 2.076 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
