# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 22 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 13321818 |
| direct native | 0 |
| dynamic bytecode | 0 |
| dynamic native | 0 |
| external/root | 22 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 5237928 |
| 0 | 57148 |
| 2 | 2314111 |
| 3 | 2476767 |
| 4 | 1755869 |
| 7 | 9 |
| 5 | 10004 |
| 13 | 1310000 |
| 14 | 160000 |
| 9 | 4 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 4591149 |
| method | 8649256 |
| factory | 81435 |
| native | 0 |
| unknown | 0 |
| fresh frames | 310 |
| reused frames | 13321530 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 21977553 | 1.649741 |
| normal restoration swaps | 21977553 | 1.649741 |
| defensive argument copies | 109200 | 0.008197 |
| unclassified swaps | 142800 | n/a |
| unclassified copies | 16511786 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 2204893 |
| RET_REG local move | 5043042 |
| RET_REG non-local copy | 4769577 |
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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 21977553 setup and 21977553 restoration swaps (3.299483 combined per instruction call across this portfolio run).
- Frame recycling supplied 13321530 of 13321840 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 109200 attributable argument-copy instructions from the unclassified copy population, and should treat 4769577 measured non-local RET_REG copies separately from 5043042 local moves.
- Dynamic selection occurred 0 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 22-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
