# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 22 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 13523419 |
| direct native | 0 |
| dynamic bytecode | 0 |
| dynamic native | 0 |
| external/root | 22 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 5509528 |
| 0 | 57148 |
| 2 | 2311311 |
| 3 | 2476768 |
| 4 | 1688669 |
| 7 | 9 |
| 5 | 10004 |
| 13 | 1310000 |
| 14 | 160000 |
| 9 | 4 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 4792750 |
| method | 8649256 |
| factory | 81435 |
| native | 0 |
| unknown | 0 |
| fresh frames | 312 |
| reused frames | 13523129 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 21974753 | 1.624941 |
| normal restoration swaps | 21974753 | 1.624941 |
| defensive argument copies | 109200 | 0.008075 |
| unclassified swaps | 233800 | n/a |
| unclassified copies | 16511786 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 2141893 |
| RET_REG local move | 5378943 |
| RET_REG non-local copy | 4698277 |
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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 21974753 setup and 21974753 restoration swaps (3.249881 combined per instruction call across this portfolio run).
- Frame recycling supplied 13523129 of 13523441 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 109200 attributable argument-copy instructions from the unclassified copy population, and should treat 4698277 measured non-local RET_REG copies separately from 5378943 local moves.
- Dynamic selection occurred 0 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 22-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
