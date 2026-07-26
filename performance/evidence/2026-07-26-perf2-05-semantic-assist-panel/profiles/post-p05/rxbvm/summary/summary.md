# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1308 ranked instruction rows, 1074 procedure-metric rows, 198 allocation rows, and 10826 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-current | 26958000 | 25793000 | -1165000 | -4.322 |
| permute-current | 61352000 | 94811000 | 33459000 | 54.536 |
| mandelbrot-current | 181570000 | 175788000 | -5782000 | -3.184 |
| towers-current | 336394000 | 679831000 | 343437000 | 102.094 |
| bounce-current | 71398000 | 47759000 | -23639000 | -33.109 |
| storage-current | 826807000 | 899621000 | 72814000 | 8.807 |
| list-current | 205697000 | 96455000 | -109242000 | -53.108 |
| richards-current | 46421000 | 462078000 | 415657000 | 895.407 |
| json-current | 298423000 | 299315000 | 892000 | 0.299 |
| base64-current | 395107000 | 339052000 | -56055000 | -14.187 |
| rexxcps-current | 981996000 | 949331000 | -32665000 | -3.326 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
