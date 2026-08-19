# DECIMAL-01 Stage 3 D2/D3 calibration verdict

Date: 2026-08-18

Status: **valid low-cost calibration; D2 and D3 rejected before formal L1;
retain current `mc_decimal`; no production or ABI change**

## Decision

Neither remaining Stage 3 candidate shows enough credible headroom to justify
a longer formal L1 screen or any L2/L3 work.

- D2 tested all 48 composed `decNumber` configurations. The best balanced
  result, `DECDPUN=8`, `DECNUMDIGITS=18`, `DECBUFFER=64`, was 0.23% slower at
  Common-18 and 1.67% slower at Classic-9 than the current provider. The best
  isolated Common-18 result was only 2.70% faster and was 1.74% slower at
  Classic-9. No grid point produced the 10% credible-headroom signal, much
  less the two-mode 15% L1 progression signal.
- D3 `decQuad` was 44.40% slower at Common-18 arithmetic and 64.76% slower at
  Classic-9 arithmetic through the adapter. The matched direct-core comparison
  was 49.28% and 66.73% slower respectively. Conversion and comparison were
  also adverse. Its 16-byte representation helped isolated copy/clear and
  context-sync cells, but those small gains do not compensate for the
  arithmetic regression. Per Adrian's direction, this arithmetic result is a
  sufficient rejection.

The complete first candidate batch is therefore closed: D4 libmpdec was
previously rejected at formal L1, while D2 and D3 are rejected by this
correctness-qualified calibration. The current 8/64/64 `mc_decimal` provider
remains selected by evidence, not by historical checksum identity.
The enduring no-repeat lessons and reopening triggers are recorded in
[`performance/DECISIONS.md`](../../DECISIONS.md).

## Valid captures

All valid blocks used the ordinary profiling-off Release build, serial
pairwise-balanced sampling, one warmup and eight recorded rounds. Every
execution passed its expected output guard; no observation was removed.

| Block | UTC interval | Rows | Recorded samples | Result |
| --- | --- | ---: | ---: | --- |
| D2 48-build grid plus current control | 18:04:02-18:09:42 | 98 | 882 | pass |
| D3 adapter, five modes and two contexts | 18:10:29-18:10:51 | 20 | 180 | pass |
| D3 matched direct core | 18:10:52-18:11:10 | 4 | 36 | pass |

The host remained on AC power with no recorded thermal or performance warning.
The pre/post audits show no competing compiler, build, test, VM or security
remediation process during these valid intervals.

## D2 decision detail

The current-provider medians were 288.596 ms for Common-18 and 338.455 ms for
Classic-9. The maintained ranking orders each complete build by its worse of
the two current/candidate throughput ratios, then its combined ratio product.

| Finding | Candidate | Common-18 | Classic-9 |
| --- | --- | ---: | ---: |
| Best balanced | `D2-decNumber-p8-n18-b64` | -0.23% | -1.67% |
| Best Common-18 | `D2-decNumber-p8-n64-b20` | +2.70% | -1.74% |

The Common-18 current-control summary recommends a rerun because its full
sample span is 11.02%, although its relative MAD is 0.95%. This does not open
a formal rerun: every observed D2 signal is far below the predeclared 10%/15%
progression boundaries, and the paired Classic-9 direction is adverse.

## D3 decision detail

Positive percentages below mean the candidate is faster; negative percentages
mean it is slower.

| Layer/mode | Common-18 | Classic-9 |
| --- | ---: | ---: |
| Adapter arithmetic | -44.40% | -64.76% |
| Adapter conversion | -33.48% | -41.34% |
| Adapter comparison | -17.34% | -16.48% |
| Adapter copy/clear | +8.88% | +15.68% |
| Adapter context sync | +11.44% | -0.56% |
| Direct-core arithmetic | -49.28% | -66.73% |

`decQuad` remains correctly labelled as a fixed-34 comparator. Contexts above
34 digits are unsupported, and the adapter's bounded integral power is not a
native `decQuad` capability and was excluded from these timing kernels.

## Invalid first D2 attempt

The first D2 attempt completed at 18:00:42 UTC, but macOS
`XProtectRemediatorAdload` began at 18:00:31 and overlapped its final 11
seconds. The entire attempt is classified invalid; no unaffected-looking tail
or subset was reused. The reason record and excluded raw capture remain under
`d2-grid-invalid-xprotect-overlap/`. Timing resumed only after the security
process chain ended and three subsequent host snapshots were clear.

## Reproduction and analysis

The source manifests are:

- `performance/manifests/decimal-01-d2-grid-calibration-mac-v1.txt`
- `performance/manifests/decimal-01-d3-decquad-calibration-mac-v1.txt`
- `performance/manifests/decimal-01-d3-decquad-core-calibration-mac-v1.txt`

Each valid capture used:

```text
cmake-build-release/bin/crexx performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args --manifest MANIFEST --output-dir OUTPUT \
  --measurement timing --warmups 1 --runs 8
```

The maintained Level B analysis is:

```text
cmake-build-release/bin/crexx \
  performance/decimal/summarize_stage3_calibration.crexx --nocolour --nokeep -args \
  --d2-summary performance/evidence/2026-08-18-decimal-01-stage3-calibration/d2-grid/summary.csv \
  --d3-adapter-summary performance/evidence/2026-08-18-decimal-01-stage3-calibration/d3-adapter/summary.csv \
  --d3-core-summary performance/evidence/2026-08-18-decimal-01-stage3-calibration/d3-core/summary.csv \
  --output-dir performance/evidence/2026-08-18-decimal-01-stage3-calibration/analysis
```

Decision outputs are `analysis/d2-ranking.csv`,
`analysis/d3-comparison.csv`, and `analysis/stage3-verdict.txt`. Raw samples,
guarded outputs, summaries, capture manifests, driver logs and host audits are
retained alongside them.
