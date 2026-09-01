# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 26 exact-image entries, 1620 ranked instruction rows, 1752 procedure-metric rows, 234 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-opt-metadata | 14054000 | 14177000 | 123000 | 0.875 |
| permute-opt-metadata | 98935000 | 89288000 | -9647000 | -9.751 |
| mandelbrot-opt-metadata | 180922000 | 184423000 | 3501000 | 1.935 |
| towers-opt-metadata | 738188000 | 780940000 | 42752000 | 5.791 |
| bounce-opt-metadata | 391272000 | 402915000 | 11643000 | 2.976 |
| storage-opt-metadata | 1987972000 | 1999651000 | 11679000 | 0.587 |
| list-opt-metadata | 284321000 | 269445000 | -14876000 | -5.232 |
| richards-opt-metadata | 710984000 | 695485000 | -15499000 | -2.180 |
| json-opt-metadata | 358699000 | 359757000 | 1058000 | 0.295 |
| base64-opt-metadata | 409078000 | 416399000 | 7321000 | 1.790 |
| rexxcps-opt-metadata | 860116000 | 945010000 | 84894000 | 9.870 |
| sieve-noopt-metadata | 18763000 | 15570000 | -3193000 | -17.018 |
| rexxcps-noopt-metadata | 802611000 | 850296000 | 47685000 | 5.941 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
