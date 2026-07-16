# NR-05 call-path census dashboard

Dynamic schema-5 observations aggregated across 2 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 2206 |
| direct native | 0 |
| dynamic bytecode | 98 |
| dynamic native | 0 |
| external/root | 2 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 450 |
| 0 | 536 |
| 3 | 720 |
| 2 | 344 |
| 4 | 224 |
| 5 | 32 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 1750 |
| method | 538 |
| factory | 18 |
| native | 0 |
| unknown | 0 |
| fresh frames | 162 |
| reused frames | 2144 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 1944 | 0.843750 |
| normal restoration swaps | 1944 | 0.843750 |
| defensive argument copies | 2 | 0.000868 |
| unclassified swaps | 44 | n/a |
| unclassified copies | 2520 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 716 |
| RET_REG local move | 1002 |
| RET_REG non-local copy | 412 |
| ignored value | 0 |
| immediate | 174 |
| terminal/external | 2 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 84 |
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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 1944 setup and 1944 restoration swaps (1.687500 combined per instruction call across this portfolio run).
- Frame recycling supplied 2144 of 2306 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 2 attributable argument-copy instructions from the unclassified copy population, and should treat 412 measured non-local RET_REG copies separately from 1002 local moves.
- Dynamic selection occurred 98 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 2-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
