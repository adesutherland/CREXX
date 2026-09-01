# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 26 exact-image entries, 1621 ranked instruction rows, 1752 procedure-metric rows, 234 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-opt-metadata | 13948000 | 16443000 | 2495000 | 17.888 |
| permute-opt-metadata | 87609000 | 89632000 | 2023000 | 2.309 |
| mandelbrot-opt-metadata | 175275000 | 183057000 | 7782000 | 4.440 |
| towers-opt-metadata | 690320000 | 681495000 | -8825000 | -1.278 |
| bounce-opt-metadata | 355066000 | 381639000 | 26573000 | 7.484 |
| storage-opt-metadata | 2135576000 | 2138409000 | 2833000 | 0.133 |
| list-opt-metadata | 236939000 | 229176000 | -7763000 | -3.276 |
| richards-opt-metadata | 677811000 | 680317000 | 2506000 | 0.370 |
| json-opt-metadata | 339703000 | 342571000 | 2868000 | 0.844 |
| base64-opt-metadata | 408471000 | 444719000 | 36248000 | 8.874 |
| rexxcps-opt-metadata | 583522000 | 664883000 | 81361000 | 13.943 |
| sieve-noopt-metadata | 16842000 | 21120000 | 4278000 | 25.401 |
| rexxcps-noopt-metadata | 608116000 | 728136000 | 120020000 | 19.736 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
