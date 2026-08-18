# Performance portfolio manifest v3

Manifest version: 3

Status: **Stage 4 source, capability and work boundary frozen for the Stage 5
pre-release scorecard**

Frozen: 2026-08-18

## Purpose

Version 3 is the current-product closeout portfolio. It retains every cREXX
AWFY/SOM lane, Base64 v2, RexxCPS, the JSON/PARSE/lifecycle capability lanes
and the DECIMAL CDB-1 controls. It also adds pinned canonical Java and CPython
controls and replaces historical runtime-name assumptions with an explicit
capability classification for every ooRexx and NetRexx cell.

The pinned AWFY control source is `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. Java and CPython are reported in
their own columns and are never Rexx aggregate members.

## Runtime and aggregation rules

| Runtime row | v3 treatment |
| --- | --- |
| cREXX `rxtvm` | direct-threaded product engine, reported separately |
| cREXX `rxbvm` | portable product engine, reported separately |
| ooRexx | only correctness-qualified language ports are timed |
| genuine NetRexx | only cells whose material timed state and algorithm use NetRexx language facilities are timed as NetRexx capability |
| Java/JVM control | primitive-Java implementations remain useful controls but are not NetRexx capability |
| canonical Java / CPython | pinned AWFY controls, separate from Rexx results |
| Regina | canonical RexxCPS control only |

The historical v2 common-five aggregate was Sieve, Permute, Bounce, Richards
and Base64 v2. Stage 4 found that the Base64 implementation performs its
material byte work in primitive Java storage, so it is **not a NetRexx
capability**. Version 3 does not silently substitute or pool it. The scorecard
may show a cREXX/ooRexx common-five continuity view and a separately named
genuine-NetRexx common-four view, but those are different aggregates and must
not be compared as if membership matched.

## Workload and source-review contract

| Workload | cREXX review disposition | v3 aggregate |
| --- | --- | --- |
| Sieve | canonical integer-array algorithm, work and result retained | cREXX/oo five; genuine-NetRexx four |
| Permute | canonical recursion, mutation, work and result retained | cREXX/oo five; genuine-NetRexx four |
| Bounce | canonical object/reference algorithm, work and result retained | cREXX/oo five; genuine-NetRexx four |
| Richards | canonical state machine, packet counts and result retained | cREXX/oo five; genuine-NetRexx four |
| Towers | canonical object/allocation algorithm retained | separate |
| Storage | algorithm retained; wrapper required by current nested-reference-container surface remains disclosed | separate adapted lane |
| List | algorithm retained; weak-reference arena ownership remains disclosed | separate adapted lane |
| Mandelbrot | canonical size/result source retained; hoisted diagnostic remains a different identity | separate |
| Full Json | exact fixture and result retained through the indexed standard-library document adaptation | separate adapted lane |
| DeltaBlue | chain/projection algorithms retained through stable indexed constraint-graph state | separate adapted lane |
| CD | 200-frame detector and red/black-map algorithm retained through indexed value state and native `rxmath` | separate adapted lane |
| Havlak | CFG, union-find, loop forest and 50 recognition passes retained; insertion-ordered flat edges and lazy predecessor containers are fair current cREXX spelling | separate adapted lane |
| Queens | recursive search and ten searches per iteration retained with typed arrays | separate |
| NBody | object algorithm and result retained; native `rxmath` square root disclosed | separate control |
| Base64 v2 | byte length, round trip, equality and checksum retained through the fastest current binary surfaces | cREXX/oo five only |
| RexxCPS 2.2d | canonical default 1,000-clause mix, provenance, decimal/string work and observable result retained | separate native-rate row |
| JSON/PARSE/lifecycle | existing identities and timing boundaries remain distinct | never |
| DECIMAL CDB-1 | current-provider core/application identities and optimizer controls retained | never |

The final review found one product correctness family rather than a benchmark
shortcut: early `RETURN`, labelled `LEAVE`/`ITERATE`, block-expression results
and runtime `SIGNAL` could cross generated cleanup incorrectly, and nested
handlers could remain installed. Those compiler/VM defects were repaired and
regression-tested before this manifest was frozen. Havlak's optimized build
remaining slower than no-opt is retained as an honest result and a future
optimization question; it is not grounds to alter required work.

## Stage 5 steady-state work

Each absolute cell uses two warmups and ten recorded serial processes. The
counts below are the Stage 4 calibrated cREXX work arguments and the matching
runtime-port work where that port is qualified. The canonical Java/CPython
harness receives one outer measurement and the listed supported inner/problem
argument, except Full Json where ten complete fixture parses are explicit.

| Workload | Work argument | Notes |
| --- | ---: | --- |
| Sieve | 5,500 | common |
| Permute | 5,000 | common |
| Bounce | 4,200 | common |
| Richards | 20 | common |
| Towers | 100 | separate |
| Storage | 20 | separate adapted |
| List | 2,000 | separate adapted |
| Mandelbrot | size 500 | one canonical invocation; ooRexx is not comparable |
| Full Json | 10 fixture parses | full 25,820-byte fixture |
| DeltaBlue | size 500 | separate adapted cREXX and canonical controls |
| CD | 100 aircraft, 200 frames | separate adapted cREXX and canonical controls |
| Havlak | one construction plus 50 recognition passes | separate adapted cREXX and canonical controls |
| Queens | 4,000 | ten searches per iteration |
| NBody | 250,000 steps | native-math disclosure applies |
| Base64 v2 | 18,000 round trips | cREXX/ooRexx capability only |
| RexxCPS | canonical auto-calibrated 1,000-clause mix | report native clauses/second, never pool |

The diagnostic calibration used zero warmups and two observations. All 32
cREXX dual-engine cells passed. Stable medians ranged from 0.043 seconds for
Full Json at ten parses to 4.648 seconds for CD; Stage 5's ten observations
provide the formal sampling. Calibration numbers are not scorecard results.

## Capability matrix

| Workload | ooRexx | genuine NetRexx |
| --- | --- | --- |
| Sieve | qualified | qualified |
| Permute | qualified | qualified |
| Bounce | qualified | qualified |
| Richards | qualified | qualified |
| Towers | qualified | not NetRexx capability - Java/JVM control |
| Storage | qualified adaptation | not NetRexx capability - Java/JVM control |
| List | qualified | not NetRexx capability - Java/JVM control |
| Mandelbrot | attempted, not comparable at canonical 500/750 | not NetRexx capability - Java/JVM control |
| Full Json | qualified standard-library DOM adaptation | failed/unresolved genuine port |
| DeltaBlue | failed/unresolved | failed/unresolved genuine port |
| CD | failed/unresolved | failed/unresolved genuine port |
| Havlak | failed/unresolved | failed/unresolved genuine port |
| Queens | qualified | qualified |
| NBody | qualified decimal/native-math adaptation | failed/unresolved genuine port |
| Base64 v2 | qualified byte-string port | not NetRexx capability - Java/JVM control |
| RexxCPS | canonical 2.2 | qualified disclosed 2.2n capability |

No failed port is described as unsupported: Stage 4 found no proved language
prohibition. Missing cells remain visible in the scorecard as
`failed/unresolved`; no Java result fills them.

## Qualification and evidence boundary

- optimized and unoptimized cREXX correctness: 14 AWFY workloads on both
  concrete VMs, 56/56 cells passed;
- full Debug: 2,262/2,262 tests passed;
- focused Release portfolio/runner/optimizer qualification: 56/56 passed;
- Release decimal/RexxCPS integrity: 82/82 passed;
- external qualification: 47 positive cells passed plus the retained expected
  ooRexx Mandelbrot not-comparable failure;
- Stage 4 diagnostic calibration: 32/32 cREXX cells passed; and
- ordinary Release build: profiling disabled.

The compact Stage 4 record is
[`2026-08-18-performance-closeout-stage4`](../evidence/2026-08-18-performance-closeout-stage4/).
Stage 5 must build from the commit containing this manifest in a fresh source
tree and record the exact commit and generated runtime identities.
