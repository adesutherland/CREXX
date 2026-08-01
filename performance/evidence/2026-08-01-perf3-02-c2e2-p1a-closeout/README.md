# PERF3-02-C2E2-P1A accepted closeout

Status: **complete — A1 retained; A3 rejected and replayable**

Date: 2026-08-01

Source baseline: local `develop` at
`4a3940395980dc40ea45917d71d99caa080e89bb`, three commits ahead of
`origin/develop`. Five protected untracked lifecycle RXBIN files remained
outside this scope and were not changed or staged.

## Accepted disposition

Adrian accepted the recommendation to remove A3 feature-gated typed CFG
construction and retain A1 demand-driven storage attachment. Typed normal,
signal-skip and signal-retry edges and the complete bounded graph-owned storage
service remain available. The current debug identity consumer attaches the
service; ordinary non-debug assembly no longer builds an unused point
environment.

A3's checksum-closed correctness and negative timing evidence remains at
`../2026-08-01-perf3-02-c2e2-p1a-a3-first-release-verdict/` and was not
rewritten.

## Restoration and validation

- The final `assembler/rxas_flow.c` diff is byte-for-byte identical to the
  retained A1 verdict diff.
- The rebuilt ordinary profiling-off Release `rxas` is byte-for-byte identical
  to the preserved A1 oracle, SHA-256
  `7cec2f1245da24cf24ffd37ec94faba0309dfc41f088d5b94c382522c0e9258d`.
- Focused RXAS/storage/optimized-no-opt dual-VM matrix: 24/24 pass.
- Full Debug CTest with `--parallel 30`: 1,972/1,972 pass.
- `git diff --check` passes.

No new timing was run: the accepted A1 and rejected A3 verdict bundles remain
the timing authorities. No commit or push is part of this closeout.

## Evidence map

- `logs/debug-build.log` and `logs/release-build.log`: affected builds;
- `logs/focused-ctest.log`: 24/24 focused proof;
- `logs/broad-debug-ctest.log`: 1,972/1,972 broad proof;
- `source.diff`: exact retained A1 production patch;
- `artifacts.csv`: binary and source identities; and
- `checksums.sha256`: recursive bundle integrity.
