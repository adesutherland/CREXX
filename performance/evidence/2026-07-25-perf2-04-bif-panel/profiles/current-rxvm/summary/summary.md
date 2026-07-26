# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1314 ranked instruction rows, 1086 procedure-metric rows, 198 allocation rows, and 10975 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-current | 24040000 | 23395000 | -645000 | -2.683 |
| permute-current | 53225000 | 86072000 | 32847000 | 61.713 |
| mandelbrot-current | 156266000 | 146520000 | -9746000 | -6.237 |
| towers-current | 327391000 | 645822000 | 318431000 | 97.263 |
| bounce-current | 62071000 | 41452000 | -20619000 | -33.218 |
| storage-current | 864546000 | 815184000 | -49362000 | -5.710 |
| list-current | 188629000 | 88348000 | -100281000 | -53.163 |
| richards-current | 41286000 | 459432000 | 418146000 | 1012.803 |
| json-current | 289636000 | 259567000 | -30069000 | -10.382 |
| base64-current | 371102000 | 351997000 | -19105000 | -5.148 |
| rexxcps-current | 996326000 | 956224000 | -40102000 | -4.025 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
