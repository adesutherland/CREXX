# RCC-5F packed statistics first Release verdict

## Accepted verdict

Adrian accepted the RCC-5F verdict on 2026-08-22. The production `rxstats`
provider now consumes borrowed read-only `.packedfloat` payloads directly; the
former boxed-array provider remains test-only as the numerical and performance
oracle. A second test-only provider calls the exact production accumulation
kernel with RXPA object handling removed, establishing the native ceiling.

All recorded executions passed and produced exact matching statistics and
checksums. The ordinary checksum was `346572.715603584`; the `1e12`
cancellation-probe checksum was `4000000000347300`.

| Comparison | Pairs | Paired median | Mean 95% interval | Reading |
|---|---:|---:|---:|---|
| packed versus boxed, `rxbvm` | 12 | +2054.025% calls/s | +2015.645% to +2100.070% | clear favorable; 12/12 favorable |
| packed versus boxed, `rxtvm` | 24 | +2062.894% calls/s | +2030.107% to +2084.759% | clear favorable; 24/24 favorable |
| offset packed versus boxed, `rxbvm` | 12 | +2055.093% calls/s | +2026.891% to +2065.735% | clear favorable; 12/12 favorable |
| offset packed versus boxed, `rxtvm` | 24 | +2080.291% calls/s | +2059.537% to +2140.691% | clear favorable; 24/24 favorable |

No performance guard fired. One permitted unchanged `rxtvm` rerun was retained
because the original capture recommended it for noise; both captures remain in
the evidence and no sample was removed. Using the median rates from the
authoritative `rxbvm` capture and accepted final `rxtvm` rerun, the public
packed path reaches 90.757% and 91.562% of the direct native control on the
ordinary dataset. On the offset probe it reaches 91.221% and 90.922%.

## Product contract proved

- `mean`, sample `stddev`, sample `covariance`, Pearson `correlation`, and
  ordinary least-squares `regression` receive `.packedfloat`, never boxed
  arrays, `.packedint`, or raw `.binary` overloads.
- Input payloads are borrowed without a Rexx wrapper, reference/dereference,
  boxed element loop, mutation, retained pointer, or whole-buffer copy.
- The shifted-origin compensated algorithms and conditional compensated second
  pass match the RCC-5D oracle on ordinary and cancellation-sensitive data.
- NaN/infinity, count, unequal-pair, zero-variance, overflow, uninitialized
  owner, and wrong packed-kind behavior have explicit signal coverage.
- Regression returns an immutable `.rxstats..linearfit`; its Rexx class is only
  a small named result carrier, not a second statistics implementation.

## Post-acceptance qualification

- Focused current profiling-off Release build and contract/package panel:
  19/19 passed, including both VMs, optimized/no-opt provider images, packed
  rejection, direct/oracle controls, compiler native return-class import,
  concurrency, RexxDoc, fresh scratch install, autoload, and native packaging.
- Normal Debug broad closure: the first 2,356-test pass exposed 66 stale
  compiler/codegen expectations from the already accepted BINARY-01 lowering.
  After a reviewed mechanical refresh, the exact failed surface plus its build
  fixture passed 67/67. This is broad-plus-targeted closure, not a claim that a
  second monolithic 2,356-test Debug invocation was run.
- Complete Apple-ASan build and CTest: 2,356/2,356 passed with no sanitizer
  report in `cmake-build-debugasan/asan-logs/20260822-104056-full`.
- Adrian approved RCC-5 publication on 2026-08-22 and assigned the unavailable
  supported Linux ASan/LSan proof for live SAN-001/002 to RCC-8 release QA. The
  handoff remains release-blocking and is not a cross-platform clean claim.

## Evidence

- `paired-summary.csv`: combined paired distributions, confidence intervals,
  favorable counts, and guard results.
- `timing-final/`: original two-warmup, twelve-recorded-round balanced matrix.
- `rxtvm-final-rerun/`: unchanged targeted two-warmup, twelve-round rerun.
- `input-manifest.txt` and `rxtvm-final-rerun-manifest.txt`: exact command
  cells.
- `summarize_paired.crexx`: Level B reducer used for the combined verdict.
- `PROVENANCE.md`: host, build, artifact hashes, and commands.
