# Post-acceptance validation

Adrian accepted the first ordinary profiling-off Release verdict on
2026-08-17.

The proportional closeout then passed:

- focused Debug compiler regression, structural proof, DeltaBlue and maintained
  runner: 8/8;
- focused profiling-off Release equivalent: 8/8;
- direct optimized/unoptimized regression and DeltaBlue gate matrix under
  `rxvm`, `rxtvm` and `rxbvm`: 12/12;
- finalized direct DeltaBlue optimized/unoptimized three-VM matrix: 6/6;
- full Debug CTest at `--parallel 30`: 2,236/2,236 in 311.69 seconds;
- `git diff --check`: pass.

The first focused Release invocation found only that its no-opt DeltaBlue
artifact had not yet been built in that partial tree. Building the declared
artifact target made the cell pass, and the final complete focused selection
passed 8/8.

The finalized benchmark qualification and bounded pilot are retained in
[`2026-08-17-postperf-02-awfy-deltablue-qualification`](../2026-08-17-postperf-02-awfy-deltablue-qualification/README.md).
No sanitizer, install/package or cross-platform claim is made for this bounded
compiler/benchmark slice, following the accepted shortest closeout path.
