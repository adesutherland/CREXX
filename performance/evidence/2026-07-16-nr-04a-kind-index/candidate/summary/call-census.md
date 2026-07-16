# NR-05 call-path census dashboard

Dynamic schema-5 observations aggregated across 26 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 12270118 |
| direct native | 0 |
| dynamic bytecode | 3976 |
| dynamic native | 0 |
| external/root | 26 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 7702140 |
| 2 | 1108983 |
| 3 | 1198071 |
| 4 | 744926 |
| 7 | 6 |
| 5 | 11954 |
| 13 | 1310000 |
| 14 | 160000 |
| 0 | 38040 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 1805906 |
| method | 10441978 |
| factory | 26236 |
| native | 0 |
| unknown | 0 |
| fresh frames | 568 |
| reused frames | 12273552 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 34836438 | 2.838209 |
| normal restoration swaps | 34836438 | 2.838209 |
| defensive argument copies | 1105388 | 0.090059 |
| unclassified swaps | 4802 | n/a |
| unclassified copies | 17318310 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 1327065 |
| RET_REG local move | 1225210 |
| RET_REG non-local copy | 8129995 |
| ignored value | 0 |
| immediate | 1591824 |
| terminal/external | 26 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 3948 |
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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 34836438 setup and 34836438 restoration swaps (5.676417 combined per instruction call across this portfolio run).
- Frame recycling supplied 12273552 of 12274120 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 1105388 attributable argument-copy instructions from the unclassified copy population, and should treat 8129995 measured non-local RET_REG copies separately from 1225210 local moves.
- Dynamic selection occurred 3976 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 26-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
