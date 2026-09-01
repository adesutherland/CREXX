# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 22 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 17373419 |
| direct native | 0 |
| dynamic bytecode | 0 |
| dynamic native | 0 |
| external/root | 22 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 9341328 |
| 0 | 57148 |
| 2 | 2319711 |
| 3 | 2476768 |
| 4 | 1698469 |
| 7 | 9 |
| 5 | 10004 |
| 13 | 1310000 |
| 14 | 160000 |
| 9 | 4 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 4819350 |
| method | 12472656 |
| factory | 81435 |
| native | 0 |
| unknown | 0 |
| fresh frames | 314 |
| reused frames | 17373127 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 21980353 | 1.265171 |
| normal restoration swaps | 21980353 | 1.265171 |
| defensive argument copies | 109200 | 0.006285 |
| unclassified swaps | 247800 | n/a |
| unclassified copies | 14155030 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 2146093 |
| RET_REG local move | 5404143 |
| RET_REG non-local copy | 8518877 |
| ignored value | 0 |
| immediate | 1304306 |
| terminal/external | 22 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 0 |
| method selection failures | 0 |
| factory selections | 0 |
| factory selection failures | 0 |
| signal unwind events | 0 |
| bytecode frames discarded | 0 |
| bytecode windows restored | 0 |
| bytecode slots restored | 0 |
| native windows restored | 0 |
| native slots restored | 0 |
| restoration failures | 0 |

## NR-06 and NR-12 handoff

- NR-06 call-window work should use the path and exact-arity distributions above, together with 21980353 setup and 21980353 restoration swaps (2.530343 combined per instruction call across this portfolio run).
- Frame recycling supplied 17373127 of 17373441 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 109200 attributable argument-copy instructions from the unclassified copy population, and should treat 8518877 measured non-local RET_REG copies separately from 5404143 local moves.
- Dynamic selection occurred 0 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 22-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
