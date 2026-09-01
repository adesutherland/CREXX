# Post-acceptance validation

Adrian accepted the first ordinary profiling-off Release verdict on
2026-08-17.

The proportional closeout then passed:

- focused Debug compiler regression, structural proof, Full AWFY Json and
  maintained runner: 8/8;
- focused profiling-off Release equivalent: 8/8;
- direct Full AWFY Json optimized/unoptimized execution under `rxvm`, `rxtvm`
  and `rxbvm`: 6/6;
- full Debug CTest at `--parallel 30`: 2,231/2,231 in 359.53 seconds;
- `git diff --check`: pass.

The finalized benchmark qualification and bounded pilot are retained in
[`2026-08-17-postperf-01-full-awfy-json-qualification`](../2026-08-17-postperf-01-full-awfy-json-qualification/README.md).
No sanitizer, install/package or cross-platform claim is made for this bounded
compiler/benchmark slice.
