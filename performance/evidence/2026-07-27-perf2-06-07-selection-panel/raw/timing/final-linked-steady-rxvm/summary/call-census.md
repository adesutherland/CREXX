# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 12 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 5469787 |
| direct native | 0 |
| dynamic bytecode | 0 |
| dynamic native | 0 |
| external/root | 12 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 1056717 |
| 0 | 2308 |
| 2 | 2082401 |
| 3 | 1279897 |
| 7 | 9 |
| 4 | 1048459 |
| 5 | 4 |
| 9 | 4 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 2950618 |
| method | 2498926 |
| factory | 20255 |
| native | 0 |
| unknown | 0 |
| fresh frames | 125 |
| reused frames | 5469674 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 2473193 | 0.452155 |
| normal restoration swaps | 2473193 | 0.452155 |
| defensive argument copies | 0 | 0.000000 |
| unclassified swaps | 151200 | n/a |
| unclassified copies | 1560166 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 1583743 |
| RET_REG local move | 3642211 |
| RET_REG non-local copy | 240627 |
| ignored value | 0 |
| immediate | 3206 |
| terminal/external | 12 |

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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 2473193 setup and 2473193 restoration swaps (0.904311 combined per instruction call across this portfolio run).
- Frame recycling supplied 5469674 of 5469799 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 0 attributable argument-copy instructions from the unclassified copy population, and should treat 240627 measured non-local RET_REG copies separately from 3642211 local moves.
- Dynamic selection occurred 0 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 12-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
