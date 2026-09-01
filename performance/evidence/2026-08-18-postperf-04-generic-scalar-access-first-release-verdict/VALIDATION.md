# Post-acceptance validation

Status: **complete**.

The mandatory first Release verdict was accepted before broad closeout work.
The following post-acceptance QA closes the stage without rewriting that
first-verdict evidence.

## Product qualification

- Complete Debug build: pass (`closeout-debug-build.log`).
- Full Debug CTest: **2,249/2,249 pass** in 418.19 seconds
  (`closeout-debug-ctest.log`).
- Complete ordinary Release build: pass with `CMAKE_BUILD_TYPE=Release` and
  `CREXX_VM_PROFILING=OFF` (`closeout-release-build.log`).
- Focused Release closure: **14/14 pass**, covering the scalar fixture in
  optimized/no-opt form under `rxtvm` and `rxbvm`, source/binary import,
  generated code, RXAS metadata/flow and the four guard families.
- The final Release compiler emits the exact accepted RXAS SHA-256
  `e6079165152f468c5da4b8466a28cab848a09eac58e313c7343456d6faa05d96`.
  Reassembly using the original absolute input identity reproduces the
  accepted selected RXBIN byte-for-byte: SHA-256
  `b8807ceac584954400e6e629c1f163d37b3a089f219374ed64bb9dfbf2831a15`,
  42,518 bytes.

The first broad run exposed a stale generated-product condition: the library
image had been rebuilt after the embedded-library `crexx` executable, so 11
launcher/late-load tests reported `ERROR reading linked library buffer`.
Completing the interrupted build relinked `crexx`; the 11 tests then passed
serially and the final full run above passed without exception. No production
code change was required for that build-artifact mismatch.

## Compiler and assembler defects closed during QA

- Method inlining now preserves the normal method-entry `ASSERTINITIALIZED`
  signal boundary after receiver and argument evaluation. Fifteen affected
  compiler goldens were audited and refreshed; their non-metadata changes are
  the required guards and derived control-flow identities.
- The exact-scalar fixed point prepares inline eligibility once, refreshes
  changed callable summaries quietly and reuses the prepared first general
  pass. This removes duplicate whole-program diagnostics while keeping
  summaries current. The original 18-test failure set passes 18/18.
- The RXAS signal-policy resolver uses a bounded reverse-dependency worklist.
  A 36-diamond regression and the real `httpcodec` path prevent a return to
  exponential recursive resolution.

## Accepted non-runtime cost

The selected assembler's formal `httpcodec` lifecycle median is 0.249058
seconds versus 0.231589 seconds for the retained pre-guard assembler,
+8.104172% (12/12 adverse). Adrian accepted this approximately 17.5 ms
compile-time cost for closure after the scaling defect was fixed. Raw paired
observations are in `assembler-lifecycle-samples.csv`; the neutral bounded
reorder was reverted.
