# Frozen beta 3 core: combined normal correctness receipt

28 September 2026. Candidate: local, unpushed
`ce4a9273fc5752fd045170661140e322cdf192c2`, tree
`77322c667ae9bf37c70b1add864ca7b875276e12`. The working tree was
clean before build, before CTest and after the run. No overlapping CMake,
Ninja or CTest process was present at startup. The existing
`cmake-build-debug` tree points to this checkout and uses Ninja, Debug,
`ENABLE_LLAMA=OFF`, and `CREXX_QA_CTEST_JOBS=30`; its `CMakeCache.txt` SHA-256
was `b361c6619bfd5d235ffa3e39a9730771bd3ab962c24035d97e3da6558aa12d95`.
Later documentation-only commits do not change these tested source, test or
build inputs.

Commands, in order:

```sh
cmake --build cmake-build-debug --parallel 10
cmake --build cmake-build-debug --target qa-prep-comprehensive --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure --label-regex '^(essential|smoke|comprehensive)$'
```

| Phase | Result and retained local log |
| --- | --- |
| Product/build tree | PASS, 1,812 actions; `/tmp/beta3-combined-all.XXXXXX.log`, SHA-256 `04f3d9dde3d63495aa7590e10602e873beacfb712efc59dccec3cf5f025eeeb5`. |
| Correctness fixtures | PASS, 43 actions; `/tmp/beta3-combined-prep.thwsBA`, SHA-256 `5293af2621dc6c8fa4187f258930a57a31f13e479146b485d248f8e18da3b158`. |
| Combined normal tier | **2283 passed, 3 failed of 2286**, 820.57 seconds; `/tmp/beta3-combined-ctest.7ZvrqA`, SHA-256 `1b10fa1ddc5ac8df91bd206916e8c3522d1c065571ea3dc5e50ae7a48582cca8`. No CTest timeout. Stress and measurement tiers were not run. |

The failures represent two distinct causes:

1. `source_import_srcmap_factory` (#646) failed in 0.02 seconds before its
   source-map assertions. `compiler/tests/run_source_import_srcmap.cmake`
   invokes `rxc --no-exe-import --diagnostics raw` in an isolated workspace
   without the build `bin` library search path. The repaired compiler now
   correctly rejects a missing mandatory `library` with exit 255:
   `EXIT_MODULE_LOAD_ERROR: Failed to load required 'library': no VM load detail`.
   This is a test fixture dependency, not evidence of a source-map semantic
   failure. The coordinator is reviewing the exact fixture repair.
2. `rxc_diagnostic_catalogs` (#976) and `rxpp_diagnostic_catalogs` (#1335)
   both reported the same four missing translations. The new keys
   `RXBIN_IMPORT_READ_ERROR` and `IMPORT_DIRECTORY_READ_ERROR` exist in
   `messages/diagnostics.en_GB.msg` but not in either
   `messages/diagnostics.de_DE.msg` or `messages/diagnostics.nl_NL.msg`.
   Catalogue completeness remains a product input defect pending coordinator
   review; the two failed test entries do not establish two independent causes.

The serialized `binary_global_import_types` (128.13 s),
`crexx_project_build_contract` (97.53 s) and `crexx_process_runtime`
(98.86 s) passed. A zero-byte untracked
`lib/rxfnsb/tests_functional/ts_linein_stdin.crexx-driver.lock` remained after
CTest; no process held it, and it was removed as an exact generated artifact.
No production code, test fixture or catalogue was edited in response to this
run. Preserve the 2,283 passing results on these frozen inputs; after reviewed
repairs of the two causes, run the affected focused tests and repeat broader
correctness only if changed inputs or a distinct failure justify it. AC-07 is
not satisfied by this receipt. Native raw backend and AC-04/11, product
integration AC-09, and final hosted sanitizer AC-08 remain open separately.

## Focused resolution of the three failed entries

The coordinator accepted the two diagnoses and authorized narrow repairs.
Commit `64ba2ef0d4e2317c6e85673469d0747dcf6b5057` supplies the build
`bin` library path to all three RXC invocations in the source-map fixture and
registers its actual `library` preparation target alongside `rxc`. The
generated Debug and Apple ASan CTest files both record
`CREXX_PREP_TARGETS "rxc;library"`; `--no-exe-import` and the source-map/error
assertions remain. Commit `d454750bd344b44f2dcb48886f7260c1a899478b`
adds German and Dutch translations for `RXBIN_IMPORT_READ_ERROR` and
`IMPORT_DIRECTORY_READ_ERROR`, retaining `{file}` and `{directory}`.
No product source or prior passing test was changed. The resulting code/test/
catalogue tree is `088c61f77fda5eef407cf0fd1a54f0d13b55ef8d` at
`d454750bd`.

Commands and results on that input (the three-test selector was
`-R '^(source_import_srcmap_factory|rxc_diagnostic_catalogs|rxpp_diagnostic_catalogs)$'`):

| Command | Result and retained log |
| --- | --- |
| `cmake --build cmake-build-debug --target qa-prep-comprehensive --parallel 10` | PASS after CMake regeneration, `/tmp/beta3-focused-prep.d7FtPV`, SHA-256 `485ab8057a10cd836704d594697e21ed8ad9f2ea9feed26bcfc6aa0c2b0579b0`. The regeneration rebuilt the aggregate's generated fixtures; it was not a second correctness run. |
| `ctest --test-dir cmake-build-debug -R <three-test selector above> --parallel 1 --output-on-failure` | PASS 3/3, `/tmp/beta3-three-repair-ctest.mVprmP`, SHA-256 `49e4ecd0d2a6a0215ec877eb0bba06615a187cf39a6ba548ab81001ed59115f5`. |
| `tools/asan-run.sh --phase build --build-target rxc --build-target library --build-jobs 8 --build-leaks off --leaks off --no-live-tail` | PASS, `cmake-build-debugasan/asan-logs/20260928-142900-build/build.log`, SHA-256 `dedeb7b0265e48a6b8c84e2f6d3a54deac4056d4e6704eb4bc056e98d5906923`. |
| `tools/asan-run.sh --phase ctest --regex '^source_import_srcmap_factory$' --test-jobs 1 --leaks off --no-live-tail` | PASS 1/1, `cmake-build-debugasan/asan-logs/20260928-143155-ctest/ctest.log`, SHA-256 `223fca98f88919eaed940915385d3099777a21e520b5eb5a3a52f4`. No sanitizer diagnostic. Apple LSan is unsupported. |

The retained 2,283 passing combined results plus these three focused passes
resolve the local correctness failures without repeating unchanged tests.
This is a composite local proof, not a literal new 2,286/2,286 broad run or a
final native/platform/sanitizer/publication claim. Coordinator review of the
repairs and evidence remains open. Native raw backend and AC-04/11, final
Linux ASan/LSan plus macOS ASan AC-08, and AC-09 publication remain open.
