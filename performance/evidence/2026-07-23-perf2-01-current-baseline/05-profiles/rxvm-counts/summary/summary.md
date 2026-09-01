# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1314 ranked instruction rows, 1098 procedure-metric rows, 198 allocation rows, and 11006 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 23298000 | 23038000 | -260000 | -1.116 |
| permute | 53010000 | 85976000 | 32966000 | 62.188 |
| mandelbrot | 154007000 | 152287000 | -1720000 | -1.117 |
| towers | 323770000 | 687815000 | 364045000 | 112.439 |
| bounce | 305952000 | 339655000 | 33703000 | 11.016 |
| storage | 810494000 | 1782604000 | 972110000 | 119.940 |
| list | 184325000 | 186077000 | 1752000 | 0.950 |
| richards | 44894000 | 589190000 | 544296000 | 1212.403 |
| json | 257703000 | 256986000 | -717000 | -0.278 |
| base64 | 388998000 | 367546000 | -21452000 | -5.515 |
| rexxcps | 1042987000 | 973993000 | -68994000 | -6.615 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
