# NR-26 transformation-panel first Release verdict

Verdict: **ACCEPT recommended on Adrian's correctness-plus-instruction-
avoidance gate.** The formal sweep records no correctness failure and no 3%
per-workload regression-guard hit. It proves one small elapsed improvement and
leaves the other lanes noisy/inconclusive; it does not prove a portfolio-wide
speedup. The implementation remains frozen, provisional and uncommitted
pending Adrian's mandatory verdict.

## Scope and provenance

- Branch and starting/current HEAD: `develop` at
  `4ab5f3d8da673c10b81af4249757763d052dda34`; the candidate is exactly the
  uncommitted NR-26 scope.
- Product: ordinary `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`.
- Baseline: retained accepted NR-14 compiler/product; its exact linked RexxCPS
  image reproduces the accepted SHA-256
  `e63425d71ab296ffdb4e8925f1c546e2d210fde311d1d8769c8b5687bb9ac440`.
- Runtimes: both baseline and candidate images ran under the same current
  `rxvm` and `rxbvm`, removing VM drift from the compiler comparison.
- Images: optimized and linked with `rxlink -s`, so source-step and TRACE-event
  metadata were stripped for the timing cells. The focused compiler contract
  separately proves retained authored TRACE identity and register retargeting.
- Workloads: the five benchmarks with exact instruction savings—Mandelbrot
  `500`, Permute `50`, Richards `1`, Towers `10`, Base64 `500`—plus canonical
  default RexxCPS (`100 x 100` iterations of 1,000 clauses) for the original
  F1 footprint/control.
- Sampling: serial, balanced/interleaved; one warmup per cell, then three
  governed 12-pair blocks, reaching the 36-pair cap under both VMs.
- Interpretation: narrow same-host first-Release verdict. These six cells are
  not the normative 12-item Tier A portfolio or five-workload common-language
  aggregate, so no release-wide or common-geomean claim is made.

## Frozen transformation panel

The no-architecture-change panel contains:

1. F1: remove a scalar local default initialization after exact
   must-write-before-first-read proof;
2. F2: remove a small scalar by-value entry copy after the same proof; and
3. P1/F3: exact small-scalar must-copy propagation, per-definition
   invalidation and must-join, destructive-promotion liveness guards,
   dead-copy fixed point, and guarded incoming-argument-slot sharing.

Unknown facts reject only the affected value, definition or use. The panel
does not change language syntax, ISA, RXBIN 007, public ABI or VM behavior.
The bounded construction census is 50,965 to 50,924 optimized source-RXAS
instructions: **41 instructions avoided**, exactly `copy -11` and
`icopy -30`. Five benchmark images account for 35; three performance-tool
selftests account for six. The focused optimized/no-opt structural matrix,
both VMs, adjacent compiler contracts and 11 changed/retargeted Release images
all pass. Full details are in the sibling `nr-26-panel-construction` bundle.

## Formal result

All 888 executions—24 warmups and 864 recorded samples—returned zero and
matched their expected correctness marker. Percentages are per-pair
`(candidate / baseline - 1) * 100`; negative elapsed and positive native rate
are favorable. The median is the headline; intervals are two-sided 95%
Student-t intervals around the mean paired change.

| Workload | `rxvm` elapsed median | Mean 95% interval | `rxbvm` elapsed median | Mean 95% interval |
| --- | ---: | ---: | ---: | ---: |
| Mandelbrot | -0.219% | -1.118% to +0.928% | -0.501% | -1.480% to +0.167% |
| Permute | -0.847% | -1.237% to +0.027% | **-0.662%** | **-2.438% to -0.375%** |
| Richards | +0.003% | -0.534% to +0.601% | -0.329% | -0.633% to +0.557% |
| Towers | +0.793% | -0.121% to +1.004% | +0.431% | -0.364% to +0.961% |
| Base64 | -2.838% | -4.629% to +1.596% | -2.517% | -3.810% to +5.327% |
| RexxCPS process | -0.077% | -0.291% to +0.365% | -0.336% | -0.587% to +0.164% |

Permute/rxbvm is the only interval wholly on the favorable side of zero:
28/36 favorable pairs. No lane has an interval wholly on the unfavorable side.
Canonical RexxCPS native-rate medians are +0.076% (`rxvm`) and +0.349%
(`rxbvm`); both intervals cross zero at the cap. Ten of 24 absolute cells still
cross the span/MAD noise rule after the governed appends, so the ambiguous
lanes remain `noisy/inconclusive`; no favorable subset or outlier was selected.

Every candidate linked image is smaller by 8 to 128 bytes. No correctness,
individual 3% elapsed, or artifact-size regression guard is hit. Process
elapsed includes startup/load/teardown but this narrow first verdict does not
claim separate phase timing or peak RSS.

## Decision boundary

The user's panel-construction test was correctness plus instructions avoided;
the panel passes it exactly, and the later formal sweep finds no regression
requiring a trade-off decision. **ACCEPT is therefore recommended**, followed
only after approval by the shortest agreed closeout path. Rework and revert
remain available. No broad CTest, sanitizer, install/package proof, further
optimization, cleanup, commit or push has followed this mandatory stop.

## Post-verdict closeout

Adrian accepted the panel on 2026-07-22. The subsequent broad correctness gate
found and corrected two exact flow-analysis boundaries involving counted-loop
block ownership and generated dispatch-selector reads. Final focused 8/8 and
full Debug 1877/1877 pass, while the 19-image source-RXAS census remains exactly
41 instructions lower. Five affected benchmark linked images reproduce the
hashes retained here. The corrected standard library changed only the RexxCPS
linked artifact; a same-session old/corrected drift control under both VMs
passed all 52 executions and hit no 3% guard. The final accepted implementation
and evidence are recorded in
`../2026-07-22-nr-26-closeout/README.md`.

## Evidence map

- `input-manifest.txt`: exact 24-cell commands and expected output markers.
- `timing/`: one warmup per cell and the initial 12-pair block.
- `timing-append-01/`, `timing-append-02/`: the two governed 12-pair appends.
- `timing-consolidated-36/summary.csv`: capped absolute-cell statistics.
- `paired-summary.csv`: capped paired quartiles, means, intervals and verdicts.
- `artifact-inventory.csv`: source, tool, runtime, library and image hashes.
- `commands.md`: build/link/capture/reduction contract.
- `host-state.md`: platform, power, thermal, load and capture times.
