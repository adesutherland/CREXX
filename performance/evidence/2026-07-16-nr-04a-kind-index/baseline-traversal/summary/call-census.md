# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 26 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 12144502 |
| direct native | 0 |
| dynamic bytecode | 756 |
| dynamic native | 0 |
| external/root | 26 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 7676478 |
| 2 | 1089876 |
| 3 | 1162766 |
| 4 | 729022 |
| 7 | 6 |
| 5 | 10344 |
| 13 | 1310000 |
| 14 | 160000 |
| 0 | 6792 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 1704210 |
| method | 10414838 |
| factory | 26236 |
| native | 0 |
| unknown | 0 |
| fresh frames | 568 |
| reused frames | 12144716 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 34739214 | 2.860311 |
| normal restoration swaps | 34739214 | 2.860311 |
| defensive argument copies | 1105388 | 0.091014 |
| unclassified swaps | 1022 | n/a |
| unclassified copies | 17194570 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 1289394 |
| RET_REG local move | 1164520 |
| RET_REG non-local copy | 8107570 |
| ignored value | 0 |
| immediate | 1583774 |
| terminal/external | 26 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 728 |
| method selection failures | 0 |
| factory selections | 28 |
| factory selection failures | 0 |
| signal unwind events | 0 |
| bytecode frames discarded | 0 |
| bytecode windows restored | 0 |
| bytecode slots restored | 0 |
| native windows restored | 0 |
| native slots restored | 0 |
| restoration failures | 0 |

## NR-06 and NR-12 handoff

- NR-06 call-window work should use the path and exact-arity distributions above, together with 34739214 setup and 34739214 restoration swaps (5.720622 combined per instruction call across this portfolio run).
- Frame recycling supplied 12144716 of 12145284 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 1105388 attributable argument-copy instructions from the unclassified copy population, and should treat 8107570 measured non-local RET_REG copies separately from 1164520 local moves.
- Dynamic selection occurred 756 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 26-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
