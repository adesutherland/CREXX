# Generic core helper and partial DLL closure continuation

Windows native GTK reached a selected .native.json provider after successful
bytecode linkage, then failed because crexx-provider-package.exe existed only
when optional llama CMake was enabled. The first console repair did not exercise
this helper: archive-only providers legitimately bypass its manifest protocol.
The new permanent extracted-core consumer uses a real DLL dependency and the
generic protocol, without an inference engine or model.

## Corrections and preserved behavior

Core RXPA now owns the existing crexx-provider-package target, and C1, product
and driver builds require it. The optional llama declaration is removed; llama
users still consume the same target. The existing provider_package.cpp and
shared package.hpp are byte-identical, preserving validation, SHA-256,
fingerprinting, path traversal/symlink checks, idempotent copying and unowned
destination collision rejection. Their private legacy namespace/macro names
remain unchanged. A local JSON header removes the optional engine/download
dependency from the core target.

nlohmann/json 3.12.0 and its full MIT license were extracted from the verified
existing pinned source archive (commit 5266f24da75dc449bd56cbed7addb9c8e4a6a73e,
archive SHA-256 2de0d87eda4696e9f6bbd771d4c623267f4e95856cce6f99793f91522f993e43).
The unmodified header SHA-256 is
aaf127c04cb31c406e5b04a63f1ae89369fccde6d8fa7cdda1ed4f32dfc5de63,
matching the Windows engineering helper. [Provenance](../../../../rxpa/vendor/nlohmann/provenance.json)
and license ship with the core/SDK. Core builds remain offline and llama-free.
The helper's include directories omit unneeded generated headers so the text
generated/VERSION file cannot shadow standard C++ <version> on macOS.

A second reproduced cause was partial flat DLL staging. A copied module and
declared DLL can find the same library through package and SDK paths. The writer
now captures conflicting resolution paths, checks every incoming published name
and SHA-256, accepts only identical bytes, and stages all discovered dependencies.
Different bytes and unresolved libraries still fail. Prior build outputs can be
updated; current conflicting identities are rejected rather than overwritten.
Manifest version, fields and runtime hash checks are unchanged.

## Evidence

Base: published 1031ef530b6e21d0a2a2b9f764c50e40cced28c6 on the clean hotfix
checkout; [qualified input hashes](qualified-inputs.json) retain the exact
substantive changes. Unrelated occupied/dirty develop worktrees remain untouched.

| Gate | Result | Evidence |
| --- | --- | --- |
| Llama-free helper, C1 and product build | Passed without compiler warnings | [helper build](core-helper-build.log), [product summary](product-build-summary.txt) |
| Actual extracted core including declarative provider consumer | 32 commands passed, 37.103 s | [summary](core-summary.json) |
| Original helper omission control | Project exits 1 after native stage rc=2 and successful rxlink | [log](core-provider-missing-helper.log) |
| Restored helper, real DLL consumer, producer directories removed | Exit 0, native value=7 | [build](core-provider-native-build.log), [relocated](core-provider-relocated.log) |
| Existing validator/fingerprint/tamper/traversal/Unicode/idempotency/collision controls | Passed unchanged | [log](core-provider-validation.log) |
| Installed SDK inventory/CMake/native consumer | Passed with helper and MIT notice | [archive](sdk-archive.json) |
| Partial-flat baseline | Identical paths rejected by old collector | [log](partial-package-baseline.log) |
| Collector pre-registration scheduling measurements | Debug 0.178 s; maintained Apple ASan flags 0.207 s | [Debug](partial-package-debug-measure.log), [ASan](partial-package-asan-measure.log) |
| Final complete/missing/different/link-file controls | Debug 3/3, 0.27 s; ASan-flag fixture 3/3, 0.26 s | [Debug](partial-package-debug-ctest.log), [ASan](partial-package-asan-ctest.log) |
| Existing split-package/pipeline regressions | 13 + 15 passed | [split](core-helper-split-tests.log), [pipeline](core-helper-pipeline-tests.log) |
| Offline ENABLE_LLAMA=ON compatibility configuration | Passed with the same core target; engine not built | [log](core-helper-llama-config-final.log) |

The new provider fixture stages both flat compiler plugin/DLL roots and canonical
native provider files. This prevents a test setup from hiding Windows declaration
inspection dependencies. The absent-helper control checks the project driver's
observed exit 1 and child native rc=2, not a guessed wrapper exit. Fixture files
remain outside public archives. The final core and SDK hashes were reverified.

The ASan fixture is dependency-scanned binary data, not full callback sanitizer
qualification. The actual dependency consumer is normal native execution.
Native GTK Windows proof remains with the Windows owner; no signed interim
release/tag was produced here. Existing Build/CodeQL publication checks are
expected to pass, and will be reported from their run records without a
status-only follow-up commit.

The two additional causes receive separate reviewable commits, published together
with this receipt and the authoritative plan. The Windows owner should fetch the
combined source and replace the temporary engineering helper with matched core
output before recipient packaging. It can use the writer directly without
manually pre-staging all transitive DLLs. Core/static/GTK/App tests and same-commit
signer inputs still govern eventual signed distribution delivery.

## Concurrent develop integration

Before publication, develop advanced to 8d3572094 with the approved Level L
grammar-source integration. The helper and collector changes were committed
separately (afd66dc2e / bd76275e6), then merged with that upstream work. The
collector, fixture and generic helper inputs remain byte-identical; the changed
driver/grammar/package-smoke inputs were rebuilt and requalified.

The combined source (pre-receipt merge 890b67f4b) passed a warning-free C1/product
build, the actual extracted core's 30 commands in 42.957 s, and the SDK
inventory/CMake/native proof. It includes the updated `.crexx`/`.rxl` grammar
workflow, both ordinary optimization paths, the missing-helper failure after
rxlink, preserved package validation, and a relocated real DLL consumer.
[Integrated core](integrated-core-summary.json),
[SDK](integrated-sdk-result.txt), [build](integrated-build-summary.txt) and
[final input hashes](integrated-qualified-inputs.json) retain this final proof.
Only this receipt was added to the unpublished integration commit afterwards;
no qualified product/fixture input changed. The unchanged Debug/ASan-flag
collector controls and offline llama-enabled configuration are reused.
