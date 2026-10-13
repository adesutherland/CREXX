# Signing-readiness packaging corrections

The status check on 13 October found two failures in [Build
38402040395](https://github.com/adesutherland/CREXX/actions/runs/38402040395)
at cf98b71b10794d5f214aa07bba18c23cd6370d73. All core lanes, Linux optimizer
parity and both Windows correctness gates passed. The source PR #717 remained
draft. The read-only signer still selected 5c57df76 assets; no signing/tagging
or release promotion was performed.

## Reproduced causes and repairs

[Windows Vulkan staging](https://github.com/adesutherland/CREXX/actions/runs/38402040395/job/115327254703)
tried to replace the qualified core's crexx-provider-package.exe with its
independent plugin-lane rebuild. Moving that helper to the core had left this
legacy packaging ownership behind. The new regression reproduced the exact
error before repair. Staging now requires the hash-verified core helper and
does not import another copy; finalization retains the core bytes. Different
rebuild bytes, an absent rebuild and a missing core helper are permanent
controls. All other shared-file hash and overlap checks remain unchanged.

[Linux Vulkan's small dependency fixture](https://github.com/adesutherland/CREXX/actions/runs/38402040395/job/115327254761)
passed its positive control but accepted the changed staged library. The fixture
searched its staged directory first. [CMake's ELF scanner](https://github.com/Kitware/CMake/blob/master/Source/cmBinUtilsLinuxELFLinker.cxx)
resolves each needed name once within a root's dependency traversal, so that
search order can hide the differing SDK copy. The fixture now searches its SDK
directory first, exposing its original bytes to the existing published-name hash
comparison. The collector and Python controls are byte-identical; their rejection
policy is not relaxed. Native Linux replay belongs to the automatic Build lane.

## Focused qualification

The existing hotfix checkout and remote hotfix/develop were ba62fbe7f when work
started. Only packaging and fixture inputs change; retained core compiler,
runtime, C1/product and normal correctness proof remains applicable. There is
no new aggregate or material workload expansion requiring another scheduling
measurement; the existing three fixture tests retain their scheduling/timeouts.

| Check | Result | Evidence |
| --- | --- | --- |
| New helper ownership controls before repair | Exact replacement error reproduced; missing-core control failed | [baseline](helper-ownership-baseline.log) |
| Full split-package suite after repair | 16 passed; stage/finalize/core hash and overlap checks included | [tests](split-package-tests.log) |
| Existing release pipeline/signing controls | 15 passed | [tests](pipeline-tests.log) |
| Normal dependency fixture build and CTest | Warning-free build; 3/3 passed, 0.36 s | [build](fixture-debug-build.log), [tests](fixture-debug-tests.log) |
| Apple ASan-flag fixture via tools/asan-run.sh | Warning-free build; same 3/3 passed, 0.25 s | [build](fixture-asan-build.log), [tests](fixture-asan-tests.log) |
| Current snapshot signing inputs | Read-only selection remains old 5c57df76 cohort | [dry-run](signer-dry-run.log) |

These fixture binaries are scanned as dependency data. This is not full native
callback sanitizer qualification, and Apple LeakSanitizer is unavailable.
The failed step excerpts and [exact qualified hashes](qualified-inputs.json)
are retained beside this receipt. Native Linux and repaired Windows plugin
staging remain explicit hosted/Windows-owner follow-up gates.

Expected publication is two reviewable substantive fixes together to hotfix and
develop, including this receipt and the authoritative
[plan](../../../planning/release-1/core-console-native-packaging-2026-10-13.md).
Normal automatic Build/CodeQL checks are expected to pass; live refs and observed
CI are reported in the conversation without a status-only follow-up commit.
The Windows owner must integrate these fixes into PR #717, complete promotion,
then provide successful matching core/plugin/manager artifacts before signing.
