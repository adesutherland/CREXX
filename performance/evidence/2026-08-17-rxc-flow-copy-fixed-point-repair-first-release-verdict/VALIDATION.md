# Post-acceptance validation

Adrian accepted the first ordinary profiling-off Release verdict on
2026-08-17. The proportional closeout then completed without changing the
accepted implementation.

## Build and test results

- complete Debug build: pass, 1,513 Ninja actions;
- required full Debug CTest: **2,224/2,224 pass**, 408.16 seconds;
- complete ordinary profiling-off Release build: pass, 1,209 Ninja actions;
- focused ordinary Release closeout: **7/7 pass**, 1.97 seconds.

The focused Release set includes the optimized and unoptimized nested-inline
runtime regression, the structural inlining contract, optimized and
unoptimized AWFY Queens, the complete NR-26 flow contract, and the linked
optimized runtime-artifact fixture.

The earlier partial automatic fixture attempt encountered stale/incomplete
generated Parse and RexxCPS artifacts. A complete Release rebuild regenerated
those products successfully, and the subsequent fixture-backed focused run
passed. There is no surviving product regression or bypassed fixture.

## Accepted boundary

The closeout adds no sanitizer, install/package, runtime-speed or cross-platform
claim. The defect is compiler-output correctness, its regression is exercised
in both optimization modes, and the optimized structural contract proves that
the supported nested-inline shape remains enabled.
