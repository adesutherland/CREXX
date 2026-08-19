# POSTPERF-03 AWFY Havlak qualification

Date: 2026-08-17

Status: **qualified cREXX reserve lane; no aggregate or cross-language claim**.

## Result

`awfy_havlak.crexx` follows `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. It retains the simple control-flow
graph, repeated Havlak/Tarjan loop recognition, 5,213-node constructed graph,
union-find collapse, loop-structure forest and final nesting calculation.
Stable integer handles lower Java block, loop and union-find identity into
Level B typed arrays; ordered integer vectors and insertion-ordered sets retain
the algorithm's traversal and work-list order.

The current source passes the published results at 1, 15, 150 and 1,500 dummy
recognitions: 1,605, 1,647, 2,052 and 6,102 loops respectively, always with
5,213 nodes. The 15,000 case is retained as 46,602/5,213 but is not claimed as
an executed pass. Two bounded attempts were intentionally interrupted at
916.56 and 1,567.48 seconds. The exact construction remains qualified by its
auditable relation `loops = 1,602 + 3 * dummy_loops`: each recognition of the
fixed simple graph contributes three persistent loops, while the final fixed
large-graph recognition contributes 1,602. The first four published scales,
including 1,500, execute that relation exactly.

The upstream-shaped direct program still defaults to all 50 discarded
large-graph recognitions. CTest uses `1 0`, and the maintained process-smoke
runner uses `1 1`; the second argument controls only the discarded recognition
count and does not alter the persistent result.

## Representation and generated-code boundary

Normal `.int[]` is deliberately retained for the equivalence lane. It is a
typed array of ordinary VM values, not packed 8-byte integer storage. A narrow
`appendKnownUnique` path is used only where the algorithm proves one parent
insertion per loop and one body insertion per node-pool member; predecessor
sets retain their duplicate check.

Optimized source RXAS has 2,600 executable instructions and 399,422 bytes;
no-opt has 1,159 instructions and 165,494 bytes, a 2.243313x instruction
expansion. Aggregate procedure locals rise from 442 to 678 and the maximum
procedure rises from 65 locals in no-opt `findLoops` to 114 in optimized
`run`. Stripped linked images retain 2,536 versus 1,160 executable
instructions. This is evidence for generic scalar access, register
finalisation and bounded late inlining; it is not hidden by rewriting the
benchmark or selecting no-opt as the product lane.

Packed storage remains a separate control. The follow-on proof will compare
the portable checked little-endian `i64` accessor with an explicitly selectable
aligned `rxinteger` path. Because Release 1 fixes `rxinteger` at signed 64-bit,
the latter can check alignment, extent and host representation once and then
avoid per-element conversion. Its public syntax and whether storage must remain
`.binary` are intentionally undecided.

## Bounded Release orientation

The final optimized/unoptimized images pass under `rxvm`, `rxtvm` and `rxbvm`,
6/6, at 1,605 loops and 5,213 nodes. A single maintained-runner process sample
per mode also passes one discarded full-graph recognition: 42.058 seconds
no-opt and 37.862 seconds optimized under product `rxvm`.

These single serial samples are lifecycle and integration evidence only. They
are not a statistical performance baseline, an engine comparison or an
aggregate result.

## Evidence map

- `artifacts.csv`: pinned upstream, source, runner, RXAS, RXBIN and linked-image hashes;
- `correctness-results.csv`: reference, build/mode/VM and bounded runner outcomes;
- `generated-code.txt`: source and stripped linked-image observations;
- `pilot/summary.csv` and `pilot/samples.csv`: raw bounded process evidence;
- `provenance.txt`: host, power, build and dirty-scope facts;
- `COMMANDS.md`: replay commands and interpretation boundaries;
- `VALIDATION.md`: qualification and closeout QA;
- `checksums.sha256`: recursive evidence hashes.
