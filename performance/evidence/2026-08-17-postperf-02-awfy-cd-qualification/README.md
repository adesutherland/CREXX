# POSTPERF-02 AWFY CD qualification

Date: 2026-08-17

Status: **qualified cREXX reserve lane; no aggregate or cross-language claim**.

## Result

`awfy_cd.crexx` follows `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. It retains the exact 200-frame
paired-aircraft simulator, moving-point quadratic collision test, recursive
voxel reduction and all seven published collision totals. The complete no-opt
reference matrix passes for 2, 10, 100, 200, 250, 500 and 1,000 aircraft.
Optimized execution passes at 2, 10 and 100 aircraft; the bounded size-10
optimized/unoptimized images also pass under `rxvm`, `rxtvm` and `rxbvm` in
Debug and ordinary profiling-off Release, 12/12.

Vector, aircraft, motion and collision records preserve value semantics.
Benchmark-private typed arrays hold stable red/black node and voxel-occurrence
handles, while insertion, rotations, traversal and removal remain in the
measured Level B source. The port uses the maintained `floattrunc` routine for
the upstream voxel hash and the existing `rxmath` `sin`, `cos` and `sqrt`
functions. This disclosed `indexed-red-black-tree-native-math` adaptation
remains outside Tier A and every aggregate.

## Generated-code and optimizer boundary

The optimized source RXAS has 5,613 instructions and 697,831 bytes; no-opt has
1,957 instructions and 253,688 bytes. Optimized instruction count is
2.868166x no-opt, aggregate procedure locals rise from 541 to 751 and linked-
attribute/copy text rises materially. The stripped linked images retain 5,765
versus 2,172 executable instructions.

This expands into a workload-sensitive runtime gap. In the maintained size-10
process pilot, optimized median time is 5.155663x no-opt for product `rxvm`
and 5.414906x for `rxtvm`. A separate stripped-linked size-100 orientation
passed the exact 4,305-collision result in both modes but took 33.96 seconds
optimized versus 0.43 seconds no-opt on the same Release product lifecycle.
The latter is an orientation, not a formal baseline, but it proves that the
current expansion cost grows materially with the workload.

The adverse result is retained for generic scalar-access proof and bounded
late-inlining/register-finalisation. It is not hidden by rewriting CD,
selecting no-opt as the product result, or promoting the lane into an
aggregate. Because compiled semantics do not vary by aircraft count, the
remaining larger optimized references were not repeated after the decisive
size-100 pass; complete reference coverage is retained in no-opt and optimized
coverage spans 2, 10 and 100.

## Bounded Release pilot

The maintained Level B process-smoke runner exposes the explicit non-default
name `cd`. Each sample runs all 200 frames at the published bounded size of 10
aircraft. One warm-up and five serial recorded samples were taken per cell:

| Runtime | Mode | Role | Median | Range |
| --- | --- | --- | ---: | ---: |
| `rxvm` | optimized | product | 312.990 ms | 309.329-315.802 ms |
| `rxtvm` | optimized | concrete-engine control | 312.922 ms | 307.415-315.978 ms |
| `rxvm` | no-opt | product | 60.708 ms | 56.847-60.717 ms |
| `rxtvm` | no-opt | concrete-engine control | 57.789 ms | 54.944-60.707 ms |

These sequential process cells are bounded orientation only, not a formal
baseline or engine comparison. Program, plugin and library modules are loaded
separately, so the measurements include VM startup and metadata handling.

## Evidence map

- `artifacts.csv`: pinned upstream, source, runner, RXAS, RXBIN and linked-image hashes;
- `correctness-results.csv`: exact reference, build/mode/VM and runner results;
- `generated-code.txt`: source and stripped linked-image observations;
- `pilot/summary.csv` and `pilot/samples.csv`: raw bounded process evidence;
- `provenance.txt`: host, power, build and dirty-scope facts;
- `COMMANDS.md`: replay commands and interpretation boundaries;
- `VALIDATION.md`: qualification and closeout QA;
- `checksums.sha256`: recursive evidence hashes.
