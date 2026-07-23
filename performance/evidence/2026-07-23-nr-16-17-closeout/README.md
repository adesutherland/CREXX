# NR-16/NR-17 accepted-verdict closeout timing sweep

Status: complete and accepted, including Adrian's explicit acceptance of the
noisy `rxvm` Base64 trade-off and a final complete Tier A absolute checkpoint
on stable AC power.

## Comparison contract

- Source state: `develop` at
  `bdf64e2a3e9e24953642cf4aa21cb503c72de4f6`, with the uncommitted NR-16/NR-17
  activity in the worktree.
- Baseline VMs: exact post-NR-16/pre-NR-17 binaries retained at
  `/private/tmp/nr17-direct-call-baseline.xR29kj/bin/`.
- Candidate VMs: ordinary `cmake-build-release/bin/rxvm` and `rxbvm`, built
  with `CREXX_VM_PROFILING=OFF`.
- Both sides used the same freshly rebuilt optimized split workload image and
  `library.rxbin`; source/TRACE metadata was retained. This isolates the NR-17
  process-local direct-call operand binding from NR-16 compiler/library work.
- Host: Darwin arm64, 10 logical CPUs, AC power, low-power mode 0. Post-run
  `pmset -g therm` reported no thermal or performance warning. All workloads
  and cells ran serially under `caffeinate -i`.
- Exact executable, library and workload hashes are in
  `artifact-hashes.sha256`.

The Release benchmark-smoke gate passed 39/39 before timing. The initial
matrix ran from 11:13:00 to 11:42:09 UTC, append 01 from 11:46:10 to 12:10:20,
and append 02 from 12:11:16 to 12:16:36 on 2026-07-23.

## Sampling and disposition

`input-manifest.txt` covers all 11 Tier A steady-state workloads, both VMs and
both variants. One warmup preceded 12 balanced/interleaved recorded pairs per
cell. All 528 recorded samples passed correctness.

The absolute-noise and paired-interval rules selected the bounded cells in
`append-01-manifest.txt`; all 384 recorded samples passed. Six intervals still
crossed zero at 24 pairs, so `append-02-manifest.txt` added the final permitted
12 pairs; all 144 recorded samples passed. The total is 1,056/1,056 passing
recorded samples plus 88 passing warmups. No sample was removed.

The summary helper is Level B and retained as `summarize_paired.crexx`. A
single-process all-row summary reached an existing repeated-allocation/call
failure at its seventeenth output group. The exact compiled helper failed at
the identical point under both baseline and candidate VMs, excluding NR-17 as
the cause. Final individual results are consequently split without selection
between `paired-summary-part-1.csv` and `paired-summary-part-2.csv`; the common
aggregate is independent in `common-geomean-summary.csv`. Per-campaign block
summaries remain beside each raw sample file.

## Capped result

The five-workload common geomean remains within its -1% regression budget:

| VM | Pairs | Paired median | Mean 95% interval | Result |
| --- | ---: | ---: | ---: | --- |
| `rxvm` | 24 | -0.839% | -1.718% to -0.195% | clear adverse, no guard |
| `rxbvm` | 12 | +4.948% | +3.218% to +5.812% | clear favorable |

Canonical RexxCPS native rate is clear favorable at +1.846% (`rxvm`) and
+1.829% (`rxbvm`). List improves +8.206%/+6.565% and JSON
+2.267%/+4.030%. Clear adverse individual cells that remain within the -3%
budget are `rxbvm` Sieve -2.743%, `rxvm` Bounce -2.641%, and `rxbvm`
Mandelbrot -0.720%. Permute `rxvm` and both Towers cells remain
noisy/inconclusive at the 36-pair cap.

`rxvm` Base64 is the only guard hit: paired median -3.464% at 36 pairs,
11/36 favorable, with a still-noisy mean 95% interval of -5.609% to +1.138%.
The three chronological 12-pair medians are +0.512%, -2.777%, and -4.037%.
The corresponding `rxbvm` result is clear favorable at +29.432% over 24
pairs.

Retained NR-05 profiling gives the optimized Base64 control only 500
`CALL_REG_FUNC_REG` executions and 0.029078% attributed call self time. The
multi-percent and time-varying `rxvm` movement therefore lacks a demonstrated
NR-17 mechanism footprint, but it still trips the formal individual guard and
requires an explicit decision.

No RSS sweep, broad Debug CTest, sanitizer, install/package proof, commit or
push was performed after this guard appeared.

Adrian explicitly accepted the noisy Base64 trade-off on 2026-07-23. The host
subsequently moved to battery, so no further formal performance/RSS sampling
is part of this closeout. The accepted first-verdict RSS/artifact result remains
the memory/size decision evidence.

After broad correctness passed 1905/1905, Adrian returned the host to stable AC
and explicitly requested a final baseline across the complete benchmark
portfolio. That absolute current-product timing/lifecycle/RSS campaign is
retained under `final-baseline/`; it is not another before/after selection
verdict.

## Final absolute checkpoint

The frozen profiling-off Release product ran all 11 Tier A workloads in both
VMs with optimized split images, retained source/TRACE metadata, equal work,
two warmups and ten recorded serial samples. All 44 warmups and 220 recorded
samples passed. The strict `MAD > 3%` or min/max span `> 10%` gate selected
nine cells for an unchanged ten-sample append; all 90 added samples passed and
none was removed. The authoritative timing summary is
`final-baseline/timing-consolidated/summary.csv`.

Peak RSS used zero warmups and three recorded serial samples for the same 22
cells. All 66 samples passed and no cell crossed the noise gate. Lifecycle used
ten cREXX-only sequences followed by the policy-required unchanged ten-sequence
append; all 80 phase rows passed. The final campaign therefore retains 500/500
passing warmup, timing, RSS and lifecycle rows.

The host drew AC power throughout, low-power mode was zero, and post-run
`pmset -g therm` reported no thermal or performance warning. The exact final
artifacts remained `84532c7b...` (`rxvm`), `8df3dcfc...` (`rxbvm`) and
`b50d7d85...` (`library.rxbin`), matching `artifact-hashes.sha256`.

### Same-laptop historical comparison

Except for RexxCPS, values are normalized work units/second; larger is better.
RexxCPS values are millions of clauses/second. Current cREXX is the 2026-07-23
checkpoint. External columns are the last complete formal same-laptop,
same-governance 2026-07-20 session: its corrected equal-work decimal-NetRexx
data are used for the five common workloads, and its disclosed diagnostic
lanes are used elsewhere. `*` marks a value that remained over the strict
noise rule after its governed append.

| Workload | Current `rxvm` | Current `rxbvm` | Historical ooRexx | Historical Regina | Historical NetRexx |
| --- | ---: | ---: | ---: | ---: | ---: |
| Sieve | 5,081.47 | 3,849.26* | 713.560 | — | 2,709.44 |
| Permute | 672.685 | 627.842 | 315.287 | — | 4,354.91* |
| Bounce | 316.994 | 305.156 | 993.490 | — | 2,009.95 |
| Richards | 1.635 | 1.618 | 11.437 | — | 17.702 |
| Base64 | 1,517.20* | 1,689.51* | 2,130.12 | — | 1,840.57 |
| Mandelbrot | 3,203.22* | 2,723.99* | invalid checksum | — | 10,062.89 |
| Towers | 14.283 | 13.843 | 148.137* | — | 351.154* |
| Storage | 5.134 | 4.985 | 412.244* | — | 354.001 |
| List | 518.219* | 445.650* | 427.216 | — | 3,447.80 |
| JSON | 18,337.49* | 15,265.12* | 8,116.72 | — | 88,393.10* |
| RexxCPS | 28.120 M | 26.119 M | 39.921 M | 33.214 M | 48.068 M |

Regina is deliberately qualified only for canonical RexxCPS. ooRexx
Mandelbrot is omitted because the retained port failed its checksum. The
NetRexx common-five cells use decimal `Rexx` state; its other rows are the
previously disclosed binary/JVM or diagnostic forms, not aggregate-equivalent
Rexx implementations. RexxCPS is also a visible community/diagnostic rate,
not part of the common aggregate, because cREXX and NetRexx use disclosed
adaptations.

For the exact five-workload common set, the cross-session geometric means are
0.868336 (`rxvm/ooRexx`), 0.819722 (`rxbvm/ooRexx`), 0.322321
(`rxvm/NetRexx`) and 0.304276 (`rxbvm/NetRexx`). This balanced aggregate still
shows the remaining object/state-machine work; it does not obscure the large
RexxCPS movement. Current RexxCPS is 22.912x (`rxvm`) and 21.410x (`rxbvm`)
the July 20 cREXX checkpoint, reaching 84.7%/78.6% of historical Regina,
70.4%/65.4% of historical ooRexx and 58.5%/54.3% of historical NetRexx.

### Lifecycle and peak RSS

| Current cREXX phase | Median | n | Versus 2026-07-20 | Noise disposition |
| --- | ---: | ---: | ---: | --- |
| shared compile | 76.893 ms | 20 | -1.543% | stable |
| shared assemble | 6.733 ms | 20 | -3.849% | noisy retained extreme |
| `rxvm` load-first-result | 2.756 ms | 20 | -5.406% | noisy span |
| `rxbvm` load-first-result | 2.673 ms | 20 | -3.397% | noisy span |

All 22 current RSS cells are stable. Canonical RexxCPS medians are 18.188 MiB
(`rxvm`) and 18.172 MiB (`rxbvm`). Storage remains the disclosed allocation
outlier at 485.484/485.469 MiB; the other workload medians range from 16.54 to
20.20 MiB.
