# NR-05 call-path census dashboard

Dynamic schema-5 observations aggregated across 4 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 16 |
| direct native | 0 |
| dynamic bytecode | 4800004 |
| dynamic native | 0 |
| external/root | 4 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 4400020 |
| 0 | 400004 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 20 |
| method | 4400000 |
| factory | 400004 |
| native | 0 |
| unknown | 0 |
| fresh frames | 16 |
| reused frames | 4800008 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 4400000 | 0.916663 |
| normal restoration swaps | 4400000 | 0.916663 |
| defensive argument copies | 0 | 0.000000 |
| unclassified swaps | 16 | n/a |
| unclassified copies | 4 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 0 |
| RET_REG local move | 400020 |
| RET_REG non-local copy | 0 |
| ignored value | 0 |
| immediate | 4400000 |
| terminal/external | 4 |

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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 4400000 setup and 4400000 restoration swaps (1.833326 combined per instruction call across this portfolio run).
- Frame recycling supplied 4800008 of 4800024 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 0 attributable argument-copy instructions from the unclassified copy population, and should treat 0 measured non-local RET_REG copies separately from 400020 local moves.
- Dynamic selection occurred 4800004 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 4-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
