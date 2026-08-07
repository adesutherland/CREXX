# Allocator telemetry reuse

`summary.csv` and `deltas.csv` are the exact S0/S1/S1b subset of
[`2026-08-06-perf3-13-gate-c-c2-geometry-first-verdict`](../../2026-08-06-perf3-13-gate-c-c2-geometry-first-verdict/).
The formal panel uses the same binaries, optimized images and command inputs,
so deterministic allocation counts were reused rather than rerun. The original
per-cell stdout/stderr remains in that referenced bundle and is not duplicated
here.
