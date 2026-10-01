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

## Linux x64 ASan/LSan: PASS

Job `109018428967` completed successfully at **18:21:08 UTC**. Checkout and
configured build identity both identify `9f2f44cfd`. Build and comprehensive
QA preparation passed; CTest passed **2,376/2,376** in 4,928.55 seconds.
The retained `run.env` records `build_leaks=on`, `test_leaks=on` and
`stop_on_failure=1`, with four build jobs, eight test jobs and only the
performance-measurement label excluded. Coordinator inspection found no
AddressSanitizer/LeakSanitizer diagnostic or failed/not-run test in the
build, preparation or CTest logs.

The Linux job log and artifact are retained alongside the macOS evidence in
the durable directory above. Artifact `10988706770`, `sanitizer-logs-linux`,
was 257,172 bytes and unexpired when downloaded. Relevant new codec and raw
CMS/TSO host fixtures all pass; they remain host mocks, not native proof.

| Retained file, relative to the durable directory | SHA-256 |
| --- | --- |
| `sanitizer-logs-linux-10988706770.zip` | `0ab466dfef0f1bd96c9ab1a2b9b18d872b10633c0577feafd7736a47f5a37789` |
| `linux-job-109018428967.log` | `af752f632d2673b728f780936c2dbcafdcb0584a114c3ba1a64c5437995841d2` |
| `linux/20260928-161017-full/build.log` | `0fafb32074e50608da2b82ac4c6110b21c9494d547f0edb409213f6015be5e82` |
| `linux/20260928-161017-full/qa-prep.log` | `cee98027c72216ea8754dc53f34db2b622734bc9a799a7f305e0b4544108f91b` |
| `linux/20260928-161017-full/ctest.log` | `62c0942c60c1c5f797d8c3b4747ee22d5c6510ba9da83410e2154878b0198134` |
| `linux/20260928-161017-full/run.env` | `136b5fc2b5c5ca4d0b6a8ddc15b5854acb529f8069a92627fd4902acaa16cc30` |

## Acceptance boundary

The complete matrix is SUCCESS and qualifies the named revision on both
maintained platforms. The cache-marker job is correctly skipped for a manual
dispatch; it is written only by scheduled runs. Later RXBIN format repair
`b63ee4b6a` in PR #710 is outside this matrix's source and has its own
focused Debug/Apple ASan receipt. Native CMS/TSO raw backends, standard-stream
integration and packages are also outside this hosted platform proof.
AC-08's matrix requirement is fulfilled for integrated baseline `9f2f44cfd`.
Do not relabel that exact-source result as qualification of later code or
as complete native/final-baseline acceptance. No further broad run has been
dispatched or implied by this receipt.
