# Hosted beta 3 baseline sanitizer assurance

The one approved manual matrix is [Sanitizer QA run 36448935081](https://github.com/adesutherland/CREXX/actions/runs/36448935081),
dispatched on `develop` at 16:09 UTC on 28 September 2026. Its workflow and
actual checked-out product revision are
`9f2f44cfd888d324858769809b0381e524850b4c`. The default-branch scheduling
repair was separately published at `2d24ae989fdb530942c73d81301d6affd243a671`;
this manual run does not need or create the scheduled-success cache marker.

## macOS ARM64: PASS

Job `109018428816` completed successfully at **18:00:11 UTC**. Checkout and
configured build identity in the retained job log both identify `9f2f44cfd`.
The instrumented Debug product build and comprehensive QA preparation passed;
CTest passed **2,376/2,376**, taking 4,000.68 seconds. Coordinator inspection
of the retained build, preparation and CTest logs found no AddressSanitizer
diagnostic, failed test or not-run test.

The workflow uses `ENABLE_LLAMA=OFF` for the maintained first-party boundary.
`tools/asan-run.sh --phase full` uses four build jobs and eight test jobs,
excluding only `^performance-measurement$`. Recorded build/test leak settings
are both `off`, reflecting the unsupported Apple LeakSanitizer capability;
this is address-safety evidence, not Linux leak qualification.

The job log and artifact were copied to durable local storage:
`/Users/adrian/.codex/qa/crexx-beta3/36448935081/`.
Uploaded artifact `10987583183`, `sanitizer-logs-macos`, was 278,852 bytes
and unexpired when downloaded. Its original archive and extracted runner
logs are retained locally beyond GitHub's 14-day retention.

| Retained file, relative to that directory | SHA-256 |
| --- | --- |
| `sanitizer-logs-macos-10987583183.zip` | `ff2db66daff4ea4819f340e31bbd105d70e94ea37a22694abb819b213246eac8` |
| `macos-job-109018428816.log` | `f58e741e85f58734d8171452ed1210d40c09b2e6bf541b34ab85de1c7c26e1b8` |
| `macos/20260928-160958-full/build.log` | `27224a891d03214b3959a14b9f78757e462c53a55a9bcaaeb67dc25683d6b3ba` |
| `macos/20260928-160958-full/qa-prep.log` | `fbf87eea6eba373d91e1d291c17cc1e1bf960910f283e15424037078b39157ba` |
| `macos/20260928-160958-full/ctest.log` | `fba984eecb18fb27a97012fd2fced59922ba5fdf0af113a319fc24cfde9d1aa7` |
| `macos/20260928-160958-full/run.env` | `6474f6b5685e5c9fb6799a4460748c884c2eb02741b363f16e9b0292b07bef6c` |

## Linux x64 ASan/LSan: RUNNING

Job `109018428967` is still in its full instrumented build/CTest step at
this checkpoint. The workflow enables leak detection for both build and
tests. Retain and inspect its terminal result and artifact before recording
a Linux or combined pass; macOS completion does not close this lane.

## Acceptance boundary

This qualifies the named macOS revision only. Later RXBIN format repair
`b63ee4b6a` in PR #710 is outside this matrix's source and has its own
focused Debug/Apple ASan receipt. Native CMS/TSO raw backends, standard-stream
integration and packages are also outside this hosted platform proof.
AC-08 remains open for Linux completion and the final combined baseline;
no further broad run has been dispatched or implied by this receipt.
