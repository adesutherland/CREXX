# NR-05 call-path census dashboard

Dynamic schema-5 observations aggregated across 44 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 24631992 |
| direct native | 0 |
| dynamic bytecode | 11284 |
| dynamic native | 0 |
| external/root | 44 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 15414660 |
| 2 | 2232816 |
| 3 | 2433348 |
| 4 | 1496012 |
| 7 | 12 |
| 5 | 25608 |
| 13 | 2620000 |
| 14 | 320000 |
| 0 | 100864 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 3678244 |
| method | 20912640 |
| factory | 52436 |
| native | 0 |
| unknown | 0 |
| fresh frames | 744 |
| reused frames | 24642576 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 69783800 | 2.831758 |
| normal restoration swaps | 69783800 | 2.831758 |
| defensive argument copies | 2210772 | 0.089711 |
| unclassified swaps | 88 | n/a |
| unclassified copies | 34772768 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 2691204 |
| RET_REG local move | 2476248 |
| RET_REG non-local copy | 16283752 |
| ignored value | 0 |
| immediate | 3192072 |
| terminal/external | 44 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 11256 |
| method selection failures | 0 |
| factory selections | 28 |
| factory selection failures | 0 |
| signal unwind events | 0 |
| bytecode frames discarded | 0 |
| bytecode windows restored | 0 |
| bytecode slots restored | 0 |
| native windows restored | 0 |
| native slots restored | 0 |
| restoration failures | 0 |

## NR-06 and NR-12 handoff

- NR-06 call-window work should use the path and exact-arity distributions above, together with 69783800 setup and 69783800 restoration swaps (5.663516 combined per instruction call across this portfolio run).
- Frame recycling supplied 24642576 of 24643320 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 2210772 attributable argument-copy instructions from the unclassified copy population, and should treat 16283752 measured non-local RET_REG copies separately from 2476248 local moves.
- Dynamic selection occurred 11284 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 44-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
