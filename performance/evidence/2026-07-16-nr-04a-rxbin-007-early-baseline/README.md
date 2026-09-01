# NR-04A RXBIN 007 early baseline

Status: negative early gate; stopped for design direction

Later directed follow-up: Adrian subsequently requested an isolated full
implementation/size review. That work is retained under `isolated-rxgraph/`
and documented in
`performance/NR-04A-RXBIN-007-IMPLEMENTATION-REVIEW.md`. This README continues
to describe the earlier end-to-end stop gate as it occurred.

This is the deliberately early end-to-end measurement of the selected
T6/RXBIN 007 production implementation. It was taken after correctness was
green and after repairing the split-read `MTIME` defect and separating the
canonical RexxCPS contract from the CTest smoke override. No graph-layout
harness, cache/layout PoC, late/native overlay work, or broader portfolio rerun
was started.

## Provenance and boundary

- Source base: `db94bc9caebf3676131643b88514aaa22d6fc1db` on `develop`.
- Dirty measured scope: the time fix, RexxCPS contract/provenance fix, benchmark
  runner check, and programme/evidence documentation in the current worktree.
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs, AC power.
- Build: ordinary Release, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Current VM hashes:
  - `rxvm`: `35e4053970a5cb1d750ddc474ccadf0d43f33760ead9ac76da807b55ffb4dde0`
  - `rxbvm`: `86c94928f71b6ebbbdc92e7e48c7b9dadb7da61732222f4b7e9ed7e3e925ccbb`
- Current workload and linked-image hashes are retained in the cell manifests
  and in `images/`.
- Every cell ran serially and retained each stdout/stderr stream plus its
  process elapsed time. The interface cells used 2 warmups and 7 recorded
  samples; the canonical RexxCPS cells used 1 warmup and 3 recorded samples.
- `rxdas -g` accepted all four retained 007 linked images without a
  container/graph validation error.
- The host load averages immediately before capture were 3.79, 6.67, and 7.21
  after the rebuild. This makes the result an early screening gate, not a quiet
  paired attribution experiment. The stable stripped-`rxvm` and canonical
  RexxCPS samples nevertheless make the regression signal material.

The retained 006 timing streams were audited for the defect signature before
reuse. No retained process sample had an approximately 86,400-second elapsed
value and no retained RexxCPS result reported the characteristic 6/12 CPS.
At Adrian's direction, no further 006 timing was performed; three accidentally
completed control cells were removed from this bundle.

## Results

Lower is better for elapsed/method/factory time. Higher is better for RexxCPS.

| Cell | Retained pre-007 median | RXBIN 007 median | Change |
| --- | ---: | ---: | ---: |
| `rxvm` interface retained, process ms | 204.432 | 371.401 | +81.7% |
| `rxvm` interface retained, method us | 45,591 | 89,251 | +95.8% |
| `rxvm` interface retained, factory us | 152,985 | 286,454 | +87.2% |
| `rxvm` interface stripped, process ms | 170.090 | 300.816 | +76.9% |
| `rxvm` interface stripped, method us | 44,488 | 80,300 | +80.5% |
| `rxvm` interface stripped, factory us | 121,760 | 208,161 | +71.0% |
| `rxvm` canonical RexxCPS | 857,561 CPS | 737,623 CPS | -14.0% |

The interface comparator is the exact pre-T6 `7a599906b` retained evidence in
`../2026-07-16-nr-04a-kind-index/focused-runtime-lookup/`. The canonical
RexxCPS comparator is the retained clean O3 `100 x 100` median in
`../2026-07-15-crexx-rexxcps-o3-rerun/canonical-opt/`. That canonical VM was
built at `44dd4dbf`, five commits before the immediate pre-T6 `7a599906b`
state. The RexxCPS source is unchanged across that interval, but VM source did
change. The immediate pre-T6 NR-04A evidence used bounded `1 x 1` diagnostic
arguments rather than a canonical no-argument run. Therefore the -14.0% row is
a valid early warning against the retained canonical baseline, but it cannot
prove that the entire delta came from T6 without making a new 006 measurement,
which is neither required nor part of this gate.

The 007-only `rxbvm` medians were:

| Cell | Process median | Native median |
| --- | ---: | ---: |
| interface retained | 341.897 ms | method 84,751 us; factory 246,742 us |
| interface stripped | 368.630 ms | method 120,892 us; factory 238,809 us |
| canonical RexxCPS retained | 14,008.710 ms | 714,551 CPS |

The stripped `rxbvm` series trended down materially across its seven recorded
samples and has no retained 006 comparator, so no cross-version claim is made
from that row.

## Linked image size

| Image | Pre-007 bytes | RXBIN 007 bytes | Ratio |
| --- | ---: | ---: | ---: |
| interface retained | 28,360 | 127,024 | 4.48x |
| interface stripped | 22,762 | 105,712 | 4.64x |
| RexxCPS retained | 259,518 | 869,956 | 3.35x |
| RexxCPS stripped | 201,173 | 652,308 | 3.24x |

## Interpretation

This gate does not justify proceeding directly to graph-layout harness
refinement or loose-end closeout. The selected 007 implementation is
correctness-green but is currently slower on the graph-specific hot workload,
slower on canonical RexxCPS, and materially larger on disk. The programme
therefore stops here for design direction. A later approved investigation
should first attribute the cost between graph representation/validation,
runtime lookup/dispatch, image growth/load work, and unrelated code-layout
effects before selecting PoCs or production changes.
