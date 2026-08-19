# rxc flow-copy fixed-point repair first Release verdict

Date: 2026-08-17

Status: **accepted by Adrian; favorable correctness verdict with neutral product
shape and compiler throughput**.

## Scope and provenance

The control is a clean detached ordinary profiling-off Release build of
`develop` commit `c38a5d184dee`. The candidate is the same source plus the
uncommitted `compiler/rxcp_flow.c` repair and its focused regression assets.
Both CMake caches record `CREXX_VM_PROFILING=OFF`, `Release`, AppleClang 21.0.0
and the default `profile-20` VM handler panel.

The defect was in the NR-26 copy/dead-store fixed point. A later rung could
replace an already valid deep read substitution with an intermediate copy
destination whose store had been omitted. The emitted optimized image then
read a register that had never received the logical result.

The repair uses reaching-definition order at the affected use. It distinguishes
an old physical value deliberately preserved by a skipped write from a new
value incorrectly attributed to the skipped destination. It rejects only that
unmaterialised replacement; nested inlining, ordinary copy propagation and the
previously accepted old-value forwarding shape remain enabled.

Control `rxc`:

- bytes: 3,451,392
- SHA-256: `6ee79cf8331a1a3de86474a7679e81b60945370b5566a5d1d1e85992117c1303`

Candidate `rxc`:

- bytes: 3,451,440
- SHA-256: `8e38ed98841d7f91cd58add88ff0ecad788523ac13bf8c189c674b5d1debf246`

## Decisive result

The control compiles and assembles both affected inputs but fails at runtime:

- AWFY Queens: `FAIL: eight-queens search did not find a placement`, RC 1;
- focused nested-inline reproducer: `FAIL: stale inline result 0`, RC 1.

The candidate passes both with RC 0. Its structural regression proves that
`run()` and `answer()` remain inlined and that the genuinely recursive method
call remains. This is a repair of the supported optimized shape, not an
inlining or copy-propagation fallback.

Queens and the focused reproducer retain exactly the same RXAS instruction
count and file size as the control. Only incorrect register operands change.
Eight established workload/tool images are byte-identical, including the
complete NR-26 fixture, five retained performance workloads, the maintained
evidence runner and `rxpp`.

Six alternating-order compiler-throughput pairs compiled the 67,692-byte
`performance/tools/run_evidence_bundle.crexx` source. All outputs were
byte-identical. Mean wall time was 1.075000 seconds for the control and
1.068333 seconds for the candidate, a paired mean of -0.620168%. The timer
resolution is 0.01 seconds, so this is retained as a neutral/no-regression
observation, not a compiler speed-up claim.

## Focused validation at the gate

- new optimized and unoptimized runtime regression: pass;
- new optimized structural inlining contract: pass;
- AWFY Queens optimized and unoptimized: 2/2 pass;
- complete NR-26 flow contract: pass;
- focused flow/inlining Release selection: 126/126 pass.

Broad Debug, sanitizer, install/package and cross-platform work were correctly
deferred until Adrian accepted this first Release verdict. Closeout validation
is recorded separately in `VALIDATION.md`.

## Host and interpretation boundary

- Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs;
- AC power, low-power mode off;
- no recorded thermal, performance or CPU-power warning;
- serial compiler timing after one warm-up per binary;
- source and TRACE metadata retained.

This proves the defect and repair on the macOS product-development host. It is
not a release-wide, cross-platform or runtime-speed claim. Existing retained
runtime baselines remain applicable because their representative generated
RXAS is byte-identical.

## Bundle map

- `artifact-comparison.csv`: exact control/candidate RXAS hashes, counts and
  byte sizes;
- `run-results.csv`: affected control/candidate assembly, link and runtime
  results;
- `compiler-timing.csv`: all six raw alternating-order compiler samples;
- `COMMANDS.md`: replay commands and measurement method;
- `VALIDATION.md`: post-acceptance closeout validation.
