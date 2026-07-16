# NR-05 call-path census dashboard

Dynamic schema-5 observations aggregated across 10 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 1490016 |
| direct native | 0 |
| dynamic bytecode | 4800004 |
| dynamic native | 0 |
| external/root | 10 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 4400026 |
| 0 | 400004 |
| 2 | 10000 |
| 5 | 10000 |
| 13 | 1310000 |
| 14 | 160000 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 1490026 |
| method | 4400000 |
| factory | 400004 |
| native | 0 |
| unknown | 0 |
| fresh frames | 44 |
| reused frames | 6289986 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 23550000 | 3.744026 |
| normal restoration swaps | 23550000 | 3.744026 |
| defensive argument copies | 0 | 0.000000 |
| unclassified swaps | 16 | n/a |
| unclassified copies | 7220006 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 0 |
| RET_REG local move | 870020 |
| RET_REG non-local copy | 0 |
| ignored value | 0 |
| immediate | 5420000 |
| terminal/external | 10 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 4400000 |
| method selection failures | 0 |
| factory selections | 400004 |
| factory selection failures | 0 |
| signal unwind events | 0 |
| bytecode frames discarded | 0 |
| bytecode windows restored | 0 |
| bytecode slots restored | 0 |
| native windows restored | 0 |
| native slots restored | 0 |
| restoration failures | 0 |

## NR-06 and NR-12 handoff

- NR-06 call-window work should use the path and exact-arity distributions above, together with 23550000 setup and 23550000 restoration swaps (7.488053 combined per instruction call across this portfolio run).
- Frame recycling supplied 6289986 of 6290030 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 0 attributable argument-copy instructions from the unclassified copy population, and should treat 0 measured non-local RET_REG copies separately from 870020 local moves.
- Dynamic selection occurred 4800004 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 10-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
