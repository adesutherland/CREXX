# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 22 exact-image entries, 1294 ranked instruction rows, 1428 procedure-metric rows, 198 allocation rows, and 14009 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| sieve | 26419000 | 23313000 | -3106000 | -11.757 |
| permute | 72046000 | 96328000 | 24282000 | 33.703 |
| mandelbrot | 188427000 | 182869000 | -5558000 | -2.950 |
| towers | 344684000 | 766516000 | 421832000 | 122.382 |
| bounce | 324789000 | 412654000 | 87865000 | 27.053 |
| storage | 824148000 | 2103340000 | 1279192000 | 155.214 |
| list | 232918000 | 276402000 | 43484000 | 18.669 |
| richards | 60193000 | 693360000 | 633167000 | 1051.895 |
| json | 296820000 | 300448000 | 3628000 | 1.222 |
| base64 | 399705000 | 402617000 | 2912000 | 0.729 |
| rexxcps | 418871000 | 417610000 | -1261000 | -0.301 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
