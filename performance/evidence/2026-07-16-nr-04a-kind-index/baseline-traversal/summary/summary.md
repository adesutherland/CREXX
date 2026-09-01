# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 26 exact-image entries, 1620 ranked instruction rows, 1752 procedure-metric rows, 234 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-opt-metadata | 15352000 | 14477000 | -875000 | -5.700 |
| permute-opt-metadata | 101242000 | 88534000 | -12708000 | -12.552 |
| mandelbrot-opt-metadata | 180588000 | 177070000 | -3518000 | -1.948 |
| towers-opt-metadata | 759697000 | 760996000 | 1299000 | 0.171 |
| bounce-opt-metadata | 397953000 | 421850000 | 23897000 | 6.005 |
| storage-opt-metadata | 2231349000 | 2239629000 | 8280000 | 0.371 |
| list-opt-metadata | 290682000 | 256499000 | -34183000 | -11.760 |
| richards-opt-metadata | 706477000 | 701065000 | -5412000 | -0.766 |
| json-opt-metadata | 359075000 | 350494000 | -8581000 | -2.390 |
| base64-opt-metadata | 406606000 | 438223000 | 31617000 | 7.776 |
| rexxcps-opt-metadata | 592540000 | 685512000 | 92972000 | 15.690 |
| sieve-noopt-metadata | 18010000 | 16165000 | -1845000 | -10.244 |
| rexxcps-noopt-metadata | 703542000 | 668861000 | -34681000 | -4.929 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
