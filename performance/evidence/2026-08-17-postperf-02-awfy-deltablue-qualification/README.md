# POSTPERF-02 AWFY DeltaBlue qualification

Date: 2026-08-17

Status: **qualified cREXX reserve lane; no aggregate or cross-language claim**.

## Result

`awfy_deltablue.crexx` follows `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. It runs the complete upstream
`innerBenchmarkLoop(n)`: `chainTest(n)` propagates 100 edit values through the
equality chain, and `projectionTest(n)` verifies forward/backward propagation
plus changed scale and offset values. Optimized and unoptimized images pass
under `rxvm`, `rxtvm` and `rxbvm`, 6/6.

Level B object assignment copies values, so the benchmark represents graph
identity with stable integer handles into planner-owned typed arrays. One
tagged constraint record retains the equality, scale, stay and edit behaviours
of the pinned hierarchy. This is a disclosed
`stable-indexed-constraint-graph` adaptation and remains outside every
aggregate.

## Generated-code and optimizer boundary

The optimized source RXAS has 6,656 instructions and 1,011,990 bytes; no-opt
has 1,566 instructions and 227,542 bytes. The optimized form raises aggregate
procedure locals from 592 to 1,227, the largest `projectionTest` frame from 31
to 206, linked-attribute text occurrences from 369 to 1,902 and copy
occurrences from 15 to 639. The stripped linked images retain 6,597 versus
1,566 instructions.

This expansion is observable in the bounded process pilot: optimized median
time is 18.323543x no-opt for product `rxvm` and 16.810123x for `rxtvm`. It is
not hidden by rewriting the workload, selecting no-opt as the product result,
or promoting DeltaBlue into an aggregate. It is retained as concrete input to
the approved generic scalar-access and bounded late-inlining/register-
finalisation work.

## Bounded Release pilot

The maintained Level B process-smoke runner exposes the explicit non-default
name `deltablue`. Each sample runs both full contracts at bounded size 500.
One warm-up and five serial recorded samples were taken per cell:

| Runtime | Mode | Role | Median | Range |
| --- | --- | --- | ---: | ---: |
| `rxvm` | optimized | product, selected `rxbvm` | 2,428.071 ms | 2,425.292–2,438.375 ms |
| `rxtvm` | optimized | concrete-engine control | 2,440.023 ms | 2,428.420–2,442.642 ms |
| `rxvm` | no-opt | product, selected `rxbvm` | 132.511 ms | 131.411–146.577 ms |
| `rxtvm` | no-opt | concrete-engine control | 145.152 ms | 135.818–147.007 ms |

`rxbvm` was not timed again because it is byte-identical to selected product
`rxvm`. These sequential process cells are bounded orientation only, not a
formal baseline or engine comparison. Program and library modules are loaded
separately with source/TRACE metadata retained, so the measurements include VM
startup and metadata handling.

## Compiler repair discovered during qualification

The first optimized build exposed an indexed-target register-lifetime defect.
The accepted repair and mandatory ordinary profiling-off Release verdict are
retained separately in
[`2026-08-17-postperf-02-deltablue-register-lifetime-first-release-verdict`](../2026-08-17-postperf-02-deltablue-register-lifetime-first-release-verdict/README.md).
The repair preserves the supported inline/indexed shape, passes the focused
structural proof and leaves ten established representative RXAS images
byte-exact.

## Evidence map

- `artifacts.csv`: source, runner, RXAS, RXBIN and linked-image hashes;
- `correctness-results.csv`: opt/no-opt product/concrete-VM and runner results;
- `generated-code.txt`: source and stripped linked-image observations;
- `pilot/summary.csv` and `pilot/samples.csv`: raw bounded process evidence;
- `provenance.txt`: host, power, build and dirty-scope facts;
- `COMMANDS.md`: replay commands and interpretation boundaries;
- `VALIDATION.md`: post-acceptance closeout QA;
- `checksums.sha256`: recursive evidence hashes.
