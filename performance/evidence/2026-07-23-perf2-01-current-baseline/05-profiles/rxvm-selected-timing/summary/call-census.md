# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 10 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 4073238 |
| direct native | 0 |
| dynamic bytecode | 0 |
| dynamic native | 0 |
| external/root | 10 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 1196665 |
| 0 | 2258 |
| 7 | 9 |
| 4 | 909859 |
| 2 | 1188451 |
| 3 | 775998 |
| 5 | 4 |
| 9 | 4 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 2924017 |
| method | 1129026 |
| factory | 20205 |
| native | 0 |
| unknown | 0 |
| fresh frames | 109 |
| reused frames | 4073139 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 2445193 | 0.600307 |
| normal restoration swaps | 2445193 | 0.600307 |
| defensive argument copies | 0 | 0.000000 |
| unclassified swaps | 184800 | n/a |
| unclassified copies | 1786498 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 117293 |
| RET_REG local move | 3783462 |
| RET_REG non-local copy | 169277 |
| ignored value | 0 |
| immediate | 3206 |
| terminal/external | 10 |

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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 2445193 setup and 2445193 restoration swaps (1.200614 combined per instruction call across this portfolio run).
- Frame recycling supplied 4073139 of 4073248 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 0 attributable argument-copy instructions from the unclassified copy population, and should treat 169277 measured non-local RET_REG copies separately from 3783462 local moves.
- Dynamic selection occurred 0 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 10-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
