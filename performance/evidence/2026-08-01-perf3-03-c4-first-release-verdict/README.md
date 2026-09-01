# PERF3-03 C4 first Release verdict

Date: 2026-08-01

Status: favourable provisional production verdict, stopped for Adrian's
accept/rework/revert direction. No broad closeout, commit or push is included.

## Selected implementation

The production candidate adds one private, portable no-inline
`rxvm_loose_string2float()` helper. It rejects only empty spans and leading
bytes that cannot begin a `strtod()` subject in the active locale. Numeric and
uncertain spans use the exact existing `string2float()` path. `STOI`, `STOF`,
public APIs, RXAS/RXBIN, signals and the language contract are unchanged.

The ordinary profiling-off Release products are 998,936 bytes for `rxvm` and
999,144 bytes for `rxbvm`, increases of 96 and 128 bytes over the retained
current product. Their private helper symbol addresses and file sizes match the
selected disposable PoC.

## Minimum correctness gate

- both Debug VM variants build successfully; and
- `06_logic_noopt`, `06_logic_opt`, both RXAS logic-flow tests and both RXAS
  conversion tests pass: 6/6.

This is the minimum correctness gate required before the first Release verdict,
not broad closeout validation.

## First Release timing verdict

Percentage change is oriented so positive is faster. The headline is the
per-round paired median; intervals are two-sided 95% Student-t intervals around
the arithmetic mean.

| Workload | VM | pairs | favourable | paired median | mean interval | Verdict |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Base64 2500 | `rxvm` | 34 | 21/34 | +4.859% | +0.469% to +8.445% | decisive favourable at cap |
| Base64 2500 | `rxbvm` | 22 | 16/22 | +5.780% | +1.034% to +12.010% | decisive favourable |
| canonical RexxCPS | `rxvm` | 12 | 12/12 | +2.517% | +1.401% to +3.261% | decisive favourable |
| canonical RexxCPS | `rxbvm` | 34 | 11/34 | -0.609% | -1.557% to +1.009% | noisy/neutral at cap |

No comparison reaches the -3% per-workload regression guard. The verdict is
therefore favourable: the production edit reproduces the material gains under
both VMs for Base64 and under `rxvm` for RexxCPS, while the opposite RexxCPS
dispatch mode remains neutral and guard-clean.

The initial block used one warmup and 12 balanced pairs. Both Base64 absolute
cells triggered the predefined ten-pair noise append. Base64 `rxvm` and
RexxCPS `rxbvm` then received the governed 12-pair interval append; the latter
received its final ten-pair absolute-noise append to the 34-pair cap. All 212
executions passed. No sample was removed or replaced.

## Stop boundary

The implementation remains provisional and revertable until Adrian accepts
this first Release verdict. Acceptance permits broad Debug/sanitizer and
cross-platform closeout; rejection permits bounded rework or revert. Neither
path is included here.

## Evidence map

- `paired-summary.csv`: final paired reductions and intervals;
- `timing/`: all raw initial and append samples plus intermediate/final
  summaries;
- `provenance/production-source.patch`: exact provisional production edit;
- `provenance/product-identity.txt`: build mode, hashes and sizes;
- `provenance/focused-debug-*.log`: minimum correctness gate;
- `provenance/*manifest*.txt` and `provenance/*state.txt`: exact sampling and
  host state; and
- `checksums.sha256`: bundle integrity.

The disposable Release root was
`/tmp/crexx-perf3-03-first-verdict.xCMBN2`; binaries are identified by hash and
are intentionally not copied into this compact evidence bundle.
