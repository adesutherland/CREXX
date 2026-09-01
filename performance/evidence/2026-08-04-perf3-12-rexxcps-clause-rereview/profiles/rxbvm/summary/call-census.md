# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 2 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 3645883 |
| direct native | 0 |
| dynamic bytecode | 0 |
| dynamic native | 0 |
| external/root | 2 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 1684002 |
| 4 | 1400002 |
| 0 | 1006 |
| 2 | 560034 |
| 3 | 833 |
| 5 | 4 |
| 9 | 4 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 3645849 |
| method | 32 |
| factory | 4 |
| native | 0 |
| unknown | 0 |
| fresh frames | 44 |
| reused frames | 3645841 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 562574 | 0.154304 |
| normal restoration swaps | 562574 | 0.154304 |
| defensive argument copies | 0 | 0.000000 |
| unclassified swaps | 840000 | n/a |
| unclassified copies | 1700 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 560832 |
| RET_REG local move | 3083047 |
| RET_REG non-local copy | 0 |
| ignored value | 0 |
| immediate | 2004 |
| terminal/external | 2 |

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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 562574 setup and 562574 restoration swaps (0.308608 combined per instruction call across this portfolio run).
- Frame recycling supplied 3645841 of 3645885 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 0 attributable argument-copy instructions from the unclassified copy population, and should treat 0 measured non-local RET_REG copies separately from 3083047 local moves.
- Dynamic selection occurred 0 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 2-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
