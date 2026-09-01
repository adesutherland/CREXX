# Exact-image performance evidence summary

Product timings below come only from serial samples of the ordinary profiling-off Release VM. Profile elapsed times are diagnostic instrumentation evidence and are not throughput measurements. Startup/lifecycle evidence remains a separate programme surface.

Retained 44 exact-image entries, 2556 ranked instruction rows, 2112 procedure-metric rows, 396 allocation rows, and 0 RXSEQ candidate rows.

| Pair | Baseline median ns | Candidate median ns | Delta ns | Delta % |
|---|---:|---:|---:|---:|
| Sieve-retained-vm | 14258000 | 14620000 | 362000 | 2.539 |
| Sieve-stripped-vm | 14507000 | 15923000 | 1416000 | 9.761 |
| Permute-retained-vm | 102443000 | 91616000 | -10827000 | -10.569 |
| Permute-stripped-vm | 85304000 | 90574000 | 5270000 | 6.178 |
| Mandelbrot-retained-vm | 176862000 | 182986000 | 6124000 | 3.463 |
| Mandelbrot-stripped-vm | 180336000 | 183213000 | 2877000 | 1.595 |
| Towers-retained-vm | 761831000 | 781997000 | 20166000 | 2.647 |
| Towers-stripped-vm | 756200000 | 783160000 | 26960000 | 3.565 |
| Bounce-retained-vm | 402549000 | 414178000 | 11629000 | 2.889 |
| Bounce-stripped-vm | 437321000 | 429290000 | -8031000 | -1.836 |
| Storage-retained-vm | 2035853000 | 2015338000 | -20515000 | -1.008 |
| Storage-stripped-vm | 2043557000 | 2047186000 | 3629000 | 0.178 |
| List-retained-vm | 276381000 | 297891000 | 21510000 | 7.783 |
| List-stripped-vm | 254955000 | 285962000 | 31007000 | 12.162 |
| Richards-retained-vm | 721938000 | 712709000 | -9229000 | -1.278 |
| Richards-stripped-vm | 708242000 | 709034000 | 792000 | 0.112 |
| JSON-retained-vm | 357548000 | 381610000 | 24062000 | 6.730 |
| JSON-stripped-vm | 346736000 | 373619000 | 26883000 | 7.753 |
| Base64-retained-vm | 415631000 | 437777000 | 22146000 | 5.328 |
| Base64-stripped-vm | 444039000 | 427459000 | -16580000 | -3.734 |
| RexxCPS-opaque-heavy-retained-vm | 138289000 | 135850000 | -2439000 | -1.764 |
| RexxCPS-opaque-heavy-stripped-vm | 129512000 | 127593000 | -1919000 | -1.482 |

Negative deltas mean the candidate completed faster. Review raw samples, correctness output, profile status, allocation definitions, and N=2/3/4 sequence candidates before drawing a causality conclusion.
