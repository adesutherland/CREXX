# RCC-5D/RCC-5E provider split first Release verdict

## Accepted verdict

RCC-5D and RCC-5E pass their ordinary profiling-off Release gates. Adrian
accepted the shifted-origin RCC-5D rework on 2026-08-21.

The rework repeats only the four decisive statistics cells because RCC-5E was
already accepted and its production code was frozen. All 112 rework executions
passed: 16 warmups and 96 recorded observations. The candidate exactly matches
the historical provider for mean, sample standard deviation, covariance,
correlation, and checksum on both the ordinary and `1e12` datasets.

| Comparison | Pairs | Paired median | Mean 95% interval | Reading |
|---|---:|---:|---:|---|
| `rxstats`, `rxbvm` | 12 | +3.140% calls/s | +1.993% to +3.432% | clear favorable; no guard |
| `rxstats`, `rxtvm` | 12 | +2.077% calls/s | +0.655% to +2.557% | clear favorable; no guard |
| offset `rxstats`, `rxbvm` | 12 | +2.211% calls/s | +0.180% to +3.901% | clear favorable; no guard |
| offset `rxstats`, `rxtvm` | 12 | +1.585% calls/s | -0.069% to +2.546% | noisy/inconclusive; no guard |

The accepted implementation uses compensated shifted-origin accumulation and
a compensated second pass for ill-conditioned central moments. It restores the
historical cancellation result without giving up the first candidate's
performance advantage.

## Initial verdict and rework trigger

The initial RCC-5E provider split passed. There was no SHA-256,
compiler-driver lifecycle, or correctness guard hit. The first RCC-5D
Welford/Pébay accumulator passed the performance side but lost precision on
the retained `1e12` cancellation probe, so Adrian approved the narrow
accumulator rework rather than accepting that numerical form.

All 640 initial clear-host executions passed: 36 warmups and 604 recorded
observations. No sample was removed. Absolute-noise extensions and paired
extensions reached the standing 34/36-pair caps where required.

## Initial authoritative clear-host performance

Paired percentages are candidate versus historical provider/product.  Positive
statistics rates and negative SHA/driver elapsed changes are favorable.

| Comparison | Pairs | Paired median | Mean 95% interval | Reading |
|---|---:|---:|---:|---|
| `rxstats`, `rxbvm` | 34 | +1.064% calls/s | -0.327% to +2.281% | noisy/inconclusive at cap; no guard |
| `rxstats`, `rxtvm` | 34 | +0.959% calls/s | +0.460% to +2.497% | clear favorable |
| SHA-256 production, `rxbvm` | 36 | -0.340% elapsed | -1.331% to +1.581% | noisy/inconclusive at cap; direct-control median -0.561% |
| SHA-256 production, `rxtvm` | 24 | -1.295% elapsed | -2.395% to -0.292% | clear favorable; direct-control median -0.518% |
| `crexx -help` lifecycle | 34 | -0.060% elapsed | -1.294% to +3.613% | neutral; median improves by 16.5 microseconds; no lifecycle guard |

The ordinary boxed statistics workload contains 256 observations and 4,000
iterations of `mean`, sample `stddev`, sample `covariance`, and `correlation`.
The final median rates are 42,272 to 42,700 calls/s on `rxbvm` and 41,942 to
42,296 calls/s on `rxtvm` (historical to candidate).

## Initial numerical gate failure

For `x = 1e12 + ((index - 1) mod 17)` and `y = 3x + 7`, the two providers agree
on the ordinary zero-offset dataset.  At the `1e12` offset the historical
two-pass provider returns:

- sample standard deviation `4.92442890089805`;
- sample covariance `72.75`; and
- correlation `1`.

The candidate returns:

- sample standard deviation `4.92445644053579` (+5.592 ppm);
- sample covariance `72.7503772938953` (+5.186 ppm); and
- correlation `0.999999477647786` (-0.522 ppm).

The small error is deterministic, not timing noise.  Because RCC-5 explicitly
seeks algorithmic improvement and this provider will underpin later packed
statistics work, the regression should be corrected rather than blessed as the
new reference behavior.

## Correctness scope before the verdict

Focused Debug full-toolchain coverage passed 30/30 across both concrete VMs,
optimized and unoptimized provider images, the new statistics/hash/ID/filesystem/
platform providers, class-library adapters, concurrency packaging, and compiler
driver packaging.  The work also fixed native numeric-signal message propagation
in `rxvmintp.c` and a `crexx` build dependency on `classlib_native.rxbin` exposed
by this Release build.

## Post-acceptance proportional qualification

RCC-5D/E closeout now passes:

- full Debug build and CTest, 2,324/2,324, retained under
  `cmake-build-debug/asan-logs/20260821-150052-build` and
  `cmake-build-debug/asan-logs/20260821-151719-ctest`;
- focused profiling-off Release build and provider/package CTest, 37/37,
  retained under `cmake-build-release/asan-logs/20260821-145826-build` and
  `cmake-build-release/asan-logs/20260821-145940-ctest`; and
- focused Apple ASan provider, concurrency, installed consumer/native-package,
  RexxDoc, and native-signal coverage, 42/42, retained under
  `cmake-build-debugasan/asan-logs/20260821-155753-ctest`.

This is the proportional subphase gate. Per the approved efficiency boundary,
the consolidated RCC-5 sanitizer and cross-platform closeout runs once after
RCC-5F rather than being repeated after RCC-5D/E.

## Evidence

- `rework-paired-summary.csv`: accepted shifted-origin paired verdict.
- `rework-timing/`: all raw samples and outputs from the accepted rework.
- `rework-input-manifest.txt`: exact balanced rework command cells.
- `rework-host-before.txt` and `rework-host-after.txt`: clear-host state.
- `clear-paired-summary.csv`: authoritative paired distributions, intervals,
  favorable counts, and guards for the initial candidate.
- `clear-timing*/samples.csv`: every authoritative raw observation.
- `clear-timing*/outputs.csv`: retained semantic output for every observation.
- `input-manifest.txt`, `clear-noise-append-manifest.txt`, and
  `paired-append-2-manifest.txt`: exact balanced command cells.
- `summarize_paired.crexx`: Level B paired reducer.
- `PROVENANCE.md`: host, build, artifact hashes, and commands.
