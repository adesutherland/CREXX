# NR-05 call-path census dashboard

Dynamic schema-5 observations aggregated across 2 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 198622 |
| direct native | 0 |
| dynamic bytecode | 5642 |
| dynamic native | 0 |
| external/root | 2 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 36486 |
| 0 | 50432 |
| 3 | 61506 |
| 2 | 30638 |
| 4 | 22400 |
| 5 | 2804 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 156982 |
| method | 47266 |
| factory | 18 |
| native | 0 |
| unknown | 0 |
| fresh frames | 162 |
| reused frames | 204104 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 173412 | 0.848960 |
| normal restoration swaps | 173412 | 0.848960 |
| defensive argument copies | 2 | 0.000010 |
| unclassified swaps | 44 | n/a |
| unclassified copies | 218340 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 64274 |
| RET_REG local move | 86934 |
| RET_REG non-local copy | 39022 |
| ignored value | 0 |
| immediate | 14034 |
| terminal/external | 2 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 5628 |
| method selection failures | 0 |
| factory selections | 14 |
| factory selection failures | 0 |
| signal unwind events | 0 |
| bytecode frames discarded | 0 |
| bytecode windows restored | 0 |
| bytecode slots restored | 0 |
| native windows restored | 0 |
| native slots restored | 0 |
| restoration failures | 0 |

## NR-06 and NR-12 handoff

- NR-06 call-window work should use the path and exact-arity distributions above, together with 173412 setup and 173412 restoration swaps (1.697920 combined per instruction call across this portfolio run).
- Frame recycling supplied 204104 of 204266 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 2 attributable argument-copy instructions from the unclassified copy population, and should treat 39022 measured non-local RET_REG copies separately from 86934 local moves.
- Dynamic selection occurred 5642 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 2-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
