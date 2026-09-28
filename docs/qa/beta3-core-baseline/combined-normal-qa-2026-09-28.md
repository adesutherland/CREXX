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
