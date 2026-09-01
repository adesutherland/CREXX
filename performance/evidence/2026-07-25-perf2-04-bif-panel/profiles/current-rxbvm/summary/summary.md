# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1314 ranked instruction rows, 1086 procedure-metric rows, 198 allocation rows, and 10975 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-current | 26190000 | 25227000 | -963000 | -3.677 |
| permute-current | 59456000 | 93787000 | 34331000 | 57.742 |
| mandelbrot-current | 179353000 | 172550000 | -6803000 | -3.793 |
| towers-current | 329869000 | 653706000 | 323837000 | 98.171 |
| bounce-current | 71353000 | 47419000 | -23934000 | -33.543 |
| storage-current | 774567000 | 812396000 | 37829000 | 4.884 |
| list-current | 206662000 | 107331000 | -99331000 | -48.064 |
| richards-current | 45949000 | 474157000 | 428208000 | 931.920 |
| json-current | 294816000 | 294878000 | 62000 | 0.021 |
| base64-current | 416496000 | 331943000 | -84553000 | -20.301 |
| rexxcps-current | 1000729000 | 973210000 | -27519000 | -2.750 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
