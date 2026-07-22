# Host and capture state

## Product host

- Host: `Mac.lan`
- OS: macOS 26.5.2, build 25F84; Darwin 25.5.0, ARM64 T8142
- Logical CPUs: 10
- C compiler: Apple clang 21.0.0 (`clang-2100.1.1.101`)
- CMake: 4.3.2
- Ninja: 1.13.2
- Builds: ordinary `Release`, `CREXX_VM_PROFILING=OFF`

## Environment controls

Every block's immediate pre-start check reported AC power, low-power mode 0,
no recorded thermal/performance/CPU-power warning and no known competing build,
test or benchmark campaign. The final block's recorded pre-start state at
2026-07-21T18:58:07Z had the largest listed process at 7.2% CPU; CLion was
1.4% and its backend 1.3%.

Capture windows from the retained manifests:

| Block | Warmups/cell | Pairs | Started UTC | Completed UTC | Result |
| --- | ---: | ---: | --- | --- | --- |
| `timing` | 1 | 12 | 2026-07-21T18:42:54Z | 2026-07-21T18:49:22Z | pass |
| `timing-append-01` | 0 | 12 | 2026-07-21T18:50:42Z | 2026-07-21T18:56:41Z | pass |
| `timing-append-02` | 0 | 12 | 2026-07-21T18:58:08Z | 2026-07-21T19:04:07Z | pass |

AC remained attached throughout (90% immediately before the last block, 88%
immediately after). Low-power mode remained 0 and `pmset -g therm` still
reported no warning at 2026-07-21T19:08:16Z.

The immediate post-capture process snapshot at 2026-07-21T19:04:23Z showed
transient `syspolicyd` 37.5% and `trustd` 19.1% CPU after the runner had
completed. A later post-capture snapshot showed a Codex renderer spike. Neither
is an independently attributable per-sample fault, so governance requires all
samples to remain. The resulting wide absolute spans and crossing paired
intervals are retained and labelled noisy/inconclusive rather than repaired by
sample selection.
