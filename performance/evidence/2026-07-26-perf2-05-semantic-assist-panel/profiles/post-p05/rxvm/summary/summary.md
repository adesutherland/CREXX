# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1309 ranked instruction rows, 1074 procedure-metric rows, 198 allocation rows, and 10810 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve-current | 24594000 | 23929000 | -665000 | -2.704 |
| permute-current | 53958000 | 87829000 | 33871000 | 62.773 |
| mandelbrot-current | 160012000 | 149549000 | -10463000 | -6.539 |
| towers-current | 319351000 | 638083000 | 318732000 | 99.806 |
| bounce-current | 62753000 | 41512000 | -21241000 | -33.849 |
| storage-current | 817459000 | 862953000 | 45494000 | 5.565 |
| list-current | 195592000 | 88604000 | -106988000 | -54.700 |
| richards-current | 42233000 | 456039000 | 413806000 | 979.817 |
| json-current | 266393000 | 263847000 | -2546000 | -0.956 |
| base64-current | 377040000 | 406245000 | 29205000 | 7.746 |
| rexxcps-current | 986381000 | 811995000 | -174386000 | -17.679 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
