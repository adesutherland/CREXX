# NR-05 call-path census dashboard

Dynamic schema-4 observations aggregated across 22 exact images. Counts reflect each manifest cell's disclosed bounded argv; they are census evidence, not product timing or a performance-win claim.

## Calls by runtime path

| Path | Calls |
|---|---:|
| direct bytecode | 16439188 |
| direct native | 0 |
| dynamic bytecode | 210 |
| dynamic native | 0 |
| external/root | 22 |
| signal bytecode | 0 |
| signal native | 0 |

## Actual arity

| Arity | Calls |
|---:|---:|
| 1 | 8767984 |
| 0 | 56829 |
| 2 | 2222712 |
| 3 | 2477091 |
| 4 | 1434707 |
| 7 | 9 |
| 5 | 10088 |
| 13 | 1310000 |
| 14 | 160000 |

## Callable and frame disposition

| Category | Count |
|---|---:|
| procedure | 3883871 |
| method | 12474100 |
| factory | 81449 |
| native | 0 |
| unknown | 0 |
| fresh frames | 443 |
| reused frames | 16438977 |
| native no-child | 0 |
| failed/no frame | 0 |

## Call-window mechanics

| Mechanic | Count | Per instruction call |
|---|---:|---:|
| setup swaps | 41981144 | 2.553691 |
| normal restoration swaps | 41981144 | 2.553691 |
| defensive argument copies | 675554 | 0.041094 |
| unclassified swaps | 266 | n/a |
| unclassified copies | 13603218 | n/a |
| degraded attributions | 0 | n/a |

## Return placement

| Placement | Count |
|---|---:|
| void | 2028919 |
| RET_REG local move | 4517546 |
| RET_REG non-local copy | 8591365 |
| ignored value | 0 |
| immediate | 1301568 |
| terminal/external | 22 |

## Dynamic selection and signal unwind

| Counter | Count |
|---|---:|
| method selections | 196 |
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

- NR-06 call-window work should use the path and exact-arity distributions above, together with 41981144 setup and 41981144 restoration swaps (5.107382 combined per instruction call across this portfolio run).
- Frame recycling supplied 16438977 of 16439420 observed bytecode activations; preserve the per-workload rows before generalising from the aggregate.
- NR-12 should distinguish 675554 attributable argument-copy instructions from the unclassified copy population, and should treat 8591365 measured non-local RET_REG copies separately from 4517546 local moves.
- Dynamic selection occurred 210 times and signal unwind occurred 0 times. Zero values mean unobserved in these bounded portfolio cells, not unreachable VM paths; focused semantic fixtures cover the cold paths.
- These aggregate conclusions are portfolio-wide for the exact 22-image set. Workload-specific conclusions must be drawn from call-census.csv and the retained raw profile for that entry.
