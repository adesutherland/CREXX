# Core console native packaging repair

## Vision and intended outcome

Make a fresh core product and its installed/ZIP/SDK payload contain the static
rxconsole provider required by `crexx --native`. Keep the existing dynamic
provider and RXPA package conventions. The Windows Lotto owner can then build
the console application against matching development tools and libraries.
GTK implementation, its static provider and native Windows app qualification
remain owned by the Windows task. No new language/API/architecture decision is
part of this correction. Do not cut/tag an interim release or sign/publish a
distribution before that native app proof is available.

The clean existing hotfix checkout was fast-forwarded to current develop
`c51c65bea1c418b71b36b0bf67014338fd1370b3`, preserving the unrelated primary
hotfix2 checkout. This is an urgent packaging repair promoted to develop.

## Acceptance criteria

1. **CP-AC-01 — PASS:** Reproduce the missing producer in a fresh product build
   graph. Inspect install, core ZIP, SDK and installer paths to identify where
   archives can be omitted. Retain the source revision and dependency evidence.
2. **CP-AC-02 — PASS:** A normal stage-product selection builds console_static;
   install/ZIP/SDK retain canonical `bin/providers/rxconsole.a` or `.lib`, the
   compatibility `_static` archive, and the dynamic `.rxplugin`. No optional
   GUI dependency is introduced into the core.
3. **CP-AC-03 — PASS:** Permanent core ZIP smoke builds a native console contract
   consumer and runs it after relocation without the producer directory or
   dynamic provider. A missing-archive control fails, and the matching restored
   archive passes. SDK inventory rejects a missing console static archive.
4. **CP-AC-04 — PASS:** The normal core product build and focused console/package
   regressions pass, with exact input identities and retained logs. Provider C
   sources are unchanged; no broad compiler/sanitizer rerun is added solely for
   this packaging correction. New test work is workflow package smoke rather
   than a nested-build aggregate CTest.
5. **CP-AC-05 — PASS:** Document the existing signed Windows snapshot route,
   same-commit core/plugin/manager requirements, required tools, identity checks
   and moving-snapshot race behavior. Identify what GTK-dependent app delivery
   must add without changing the signing architecture or issuing a release.
6. **CP-AC-06 — PUBLICATION READY:** Commit code, regression, docs and receipts together on
   hotfix and promote to develop; verify remote refs. Report concrete artifacts
   in this chat so the Windows owner can fetch them. Native Windows and signed
   interim distribution proof remain visibly outside this Mac correction.
7. **CP-AC-07 — PASS:** The existing RXPA helper accepts concrete LINK_FILES for
   GTK import libraries and scans .rxplugin roots for runtime dependencies.
   Preserve the version-1 manifest and existing LINK_TARGETS behavior. Retain
   generated-package link/runtime hash checks and a missing module dependency
   negative control in the standalone provider-package fixture. Measure its
   controls in normal Debug and with maintained Apple ASan flags.
8. **CP-AC-08 — PASS:** crexx-provider-package builds/installs/packages with
   ENABLE_LLAMA=OFF and appears in the ordinary product and SDK inventories.
   Preserve the existing validator/copier/fingerprint implementation and llama
   consumers. Use retained pinned nlohmann header/license bytes without enabling
   inference or introducing an online core-build dependency. Verify real native
   use of a selected provider carrying .native.json from the extracted core.
9. **CP-AC-09 — PASS:** Dependency collection accepts colliding source paths only
   when their bytes match, stages the transitive closure, and rejects missing
   dependencies or conflicting bytes under the same published name. Reproduce
   the partial-flat GTK-style DLL layout with the standalone fixture before
   editing the collector; preserve manifest hash and collision validation.
10. **CP-AC-10 — MAC PASS / WINDOWS HANDOFF:** Correct the generic helper's Windows command-line path
    handling for Unicode package/consumer directories. Preserve all validation,
    copying, hashing and fingerprint rules and the existing acceptance harness.
    Retain fcb MSVC/MinGW job diagnostics, pass normal Mac Unicode/package proof,
    then hand off the exact published fix for native Windows revalidation.
11. **CP-AC-11 — CLASSIFIED / WINDOWS HANDOFF:** Classify the MinGW first-fingerprint failure's actual
    return code/runtime closure rather than assuming its empty stderr is the
    MSVC Unicode failure. Apply a focused runtime packaging correction only if
    independently established; leave native proof visibly open if unavailable.

12. **CP-AC-12 — PASS:** Plugin staging consumes the manifest-verified core
    helper, never substitutes an independently rebuilt helper, and rejects a
    core missing that helper. A permanent staging/finalization regression proves
    different rebuild bytes and an absent rebuild do not replace the core.
13. **CP-AC-13 — MAC PASS / LINUX CI HANDOFF:** The partial fixture's loader search exposes the SDK
    dependency to its differing-byte control on ELF as well as Mach-O. Preserve
    positive, missing-dependency and hash controls; pass the three focused normal
    and Apple-ASan-flag fixture tests. Native Linux confirmation remains assigned
    to the normal automatic publication workflow.
14. **CP-AC-14 — OPEN:** Following Adrian's 13 October continuation instruction,
    integrate the published packaging fixes into PR #717 and verify its exact
    final head passes the normal full Build and CodeQL checks. Retain the native
    Windows owner’s app/package proof and keep any unmet platform claims explicit.
15. **CP-AC-15 — OPEN:** After CP-AC-14 passes, mark PR #717 ready and merge it
    into develop without bypassing checks. Verify the promoted commit and tree
    against the qualified candidate and preserve occupied dirty checkouts.
16. **CP-AC-16 — OPEN:** Complete the promoted commit's automatic Build/snapshot
    publication, then pin/download/hash-verify matching Windows core, Vulkan
    plugin and retained manager artifacts. Verify manifests, toolchain, BUILDINFO,
    bootstrap identity and local signing-tool/configuration availability before
    asking Adrian to log in to SimplySign. Preparing these inputs is authorized;
    this continuation does not itself claim actual signing or release completion.

## Implementation steps

1. **CP-STEP-01 — DONE (AC-01/05):** Inspect fresh CMake closure and
   packaging/signing sources; capture the baseline omission and signing inputs.
2. **CP-STEP-02 — DONE (AC-02/03/07):** Add the existing static target to the owning
   G product stage, add an extracted-ZIP native console consumer, and require
   the archive in the shipped SDK inventory. Extend the existing helper's file
   input and module scan, with no change to package schema or native CLI policy.
3. **CP-STEP-03 — DONE (AC-03/04/07):** Build core through its ordinary target,
   install/package/extract it, run the relocated consumer and archive-removal
   control, then run focused provider/package/signing regression checks.
4. **CP-STEP-04 — PUBLICATION READY (AC-05/06):** Record evidence and signing handoff, synchronize
   with concurrent develop changes, commit/publish the substantive correction,
   and verify refs. Normal automatic publication checks are expected to pass;
   no status-only CI commit or manual overnight dispatch is planned.
5. **CP-STEP-05 — DONE (AC-08):** Correct the generic helper's optional-llama
   target ownership and header dependency, then verify an actual selected native
   dependency consumer and unchanged validation/fingerprint/copy semantics.
6. **CP-STEP-06 — PUBLICATION READY (AC-09):** Reproduce partial-flat module/runtime dependency
   conflicts, accept only identical resolved copies, stage/validate the resulting
   closure, and measure positive/negative controls in Debug and maintained ASan
   flags. Publish the compatible corrections with retained evidence.
7. **CP-STEP-07 — PUBLICATION READY (AC-10/11):** Inspect completed fcb Windows job logs and
   artifacts, normalize Windows wide command-line arguments to the existing
   UTF-8 boundary, run unchanged acceptance and actual package/SDK checks, and
   publish with a Windows proof handoff. Investigate any distinct MinGW loader
   failure separately. No signing/tagging or publication architecture redesign.

8. **CP-STEP-08 — PUBLICATION READY (AC-09/12/13/06):** Retain both failed jobs, reproduce
   the helper ownership regression, consume the qualified core helper, correct
   the fixture search order, run focused package/fixture tests, then publish the
   two reviewable causes together on hotfix/develop with their receipt. Hand off
   exact source to the Windows owner; do not sign or tag older artifacts.
9. **CP-STEP-09 — ACTIVE (AC-14/15/16):** Coordinate the existing Windows owner’s
   candidate integration, follow the normal CI and repair actual failures,
   promote the passing PR, and verify the successful same-commit snapshot/manager
   cohort. Reuse valid unchanged local QA; do not dispatch overnight matrices or
   add status-only pushes. Retain preparation evidence outside source publication
   and request SimplySign login only when the matching inputs are ready.

## Signing-readiness CI repair continuation

Build 38402040395 at cf98b71b10794d5f214aa07bba18c23cd6370d73 passed all
core lanes, optimizer parity and the native Windows quality gates, but exposed
two remaining packaging issues. Windows Vulkan staging attempted to replace the
now core-owned generic helper with an independently compiled plugin-lane copy.
Linux's partial-package differing-byte control did not expose its SDK dependency
because the ELF fixture searches its already staged copy first. These repairs
preserve the unchanged-core distribution contract and the collector's strict
hash/collision policy. Native Lotto delivery and actual signing remain separate
Windows-owner gates; the current signer still selects older 5c57df76 assets.

## Windows Unicode and GNU runtime diagnostics

The fcb Build (38389192522) completed both Windows build/correctness jobs, then
MSVC failed the unchanged Unicode acceptance harness with a multi-byte mapping
error. The helper treated narrow CRT argv as UTF-8. The correction reads the
native wide command line, normalizes its arguments at the existing UTF-8 boundary,
and leaves the manifest validator/copier checks intact.

The Windows owner's same-source GCC16 probe independently established MinGW
return -1073741515 (STATUS_DLL_NOT_FOUND) with empty output under system-only PATH.
Adding the exact compiler's libstdc++-6.dll/libgcc_s_seh-1.dll/libwinpthread-1.dll
restored the ASCII fingerprint. The unchanged Unicode harness then failed with
an illegal-byte-sequence conversion error, proving both defects. Core staging
therefore supplies the selected GNU runtime files rather than relaxing PATH.
Native Windows acceptance remains owned by that task after the focused Mac
qualification and source publication; no signed/release-ready claim follows.

## Baseline evidence

Fresh Debug Ninja configuration with BUILD_TESTING=OFF, GTK/llama/parser mode
disabled: CMake File API stage-product closure includes `console` and excludes
`console_static`. The static target already exists and its post-build action
publishes both archive names into bin/providers. Top-level directory install
copies all existing bin files and does not fail for an unbuilt static producer.
This repeats the build-selection mechanism repaired for cstream in
`ci-core-stream-packaging-2026-10-10.md`, with a different affected provider.

The Windows owner reserved RXPluginFunction.cmake and WriteProviderPackage.cmake
for this task to avoid concurrent edits. The GTK provider uses an INTERFACE
PkgConfig target, which has no TARGET_LINKER_FILE; its concrete
GTK3_LINK_LIBRARIES need the already-supported writer link_files input.
WriteProviderPackage currently discards .rxplugin roots before dependency scans.
The Windows owner must attach both its static target and generated runtime
package target to ui_gtk/product selections; an ALL target alone does not enter
a selective stage-product build.

## Final retained proof and publication contract

`docs/qa/console-native-packaging-2026-10-13/README.md` records the independently
inspected signed Windows Beta 3 ZIP, fresh graph omission/correction, warning-free
core/toolchain/optional builds, the actual extracted core's 26 passing commands,
SDK relocated proof, missing/restored archive controls, dynamic console opt/noopt,
Debug 2/2 and Apple-ASan-flag 2/2 helper controls, and package/signing regressions.
The SDK and core hashes were restored and reverified after the negative controls.
The final upstream fast-forward to 54b610b95 adds only optional source-only Forth
examples; it changes no qualified core/helper/test input or producer selection.
Those unrelated changes are preserved without repeating unchanged package QA.

The first push contains two reviewable substantive commits for the distinct core
producer and helper dependency causes, with this plan, receipt and Windows
handoff. Expected final refs: remote hotfix/develop and unoccupied local develop
point at the qualified combined changeset. Normal automatic publication checks
are expected to pass; actual ref verification is reported in the conversation.
CP-AC-06 is verified by those final refs, without a CI-status-only follow-up.
Native Windows app proof, GTK runtime delivery and signed interim distribution
remain with the Windows owner; none is claimed complete by this Mac repair.

## Windows continuation, additional concrete core gap

After the first two commits (e56c90ddd / 1031ef530), the Windows owner proved the
native console ConPTY path and the bundled NetRexx launcher. GTK native linking
then reached a missing crexx-provider-package.exe: its generic target is declared
only inside optional lib/plugins/llama despite the documented provider-neutral
driver protocol. This Mac task owns the correction; GTK source changes stay with
the Windows owner. The provider/writer source files were explicitly reserved to
avoid concurrent edits. Native GUI qualification and signing remain pending.

## Generic helper / partial closure qualified continuation

The core helper target now builds with llama disabled using the pinned, licensed
header. Shared copier/validator source bytes are unchanged. C1/product/driver
closure includes the helper, and core/SDK inventories carry it and its MIT
notice. The actual extracted core passed 32 commands, including unchanged
validation/fingerprint controls, the absent-helper rc=2 control (project exit 1),
a real DLL consumer and relocation after producer directories were removed.
SDK inventory/CMake/native proof passed. The offline llama-enabled configuration
still accepts the same target with no engine build.

The partial-flat GTK-style conflict is reproduced and repaired with strict
per-name hashes; identical paths pass and differing/missing bytes fail. Controls
were measured before CTest registration in Debug and maintained Apple ASan flags,
then all three fixture CTests passed.
`docs/qa/console-native-packaging-2026-10-13/generic-helper/README.md` retains
exact inputs, commands, source/header identities and expected publication state.
The compatible core-target and collector causes are published together as two
reviewable commits. Remote refs provide CP-AC-06 proof; occupied dirty develop
checkouts are preserved instead of force-updated. Windows GUI and signed interim
delivery remain owned by the Windows task and are not claimed complete here.
The final integration with concurrent develop 8d3572094 preserves its approved
grammar/driver changes. Qualified unchanged helper/collector fixtures are reused;
the changed product/package workflow was rebuilt and passed 30 actual extracted
core commands plus SDK proof. The unpublished integration commit includes this
updated receipt before the first push, with no post-CI status commit planned.

Mac source-phase proof for the Windows continuation passed: normal C1/product
build without warnings, actual extracted core 30/30 commands including the
unchanged acceptance harness, SDK/native consumer, and 13 split-package tests.
The command-line and GNU loader causes are retained separately. Native Windows
wide-command-line and runtime staging revalidation remains with the Windows
owner after this qualified source publication. The validator and acceptance
harness are unchanged; no architecture or signing operation was introduced.
`docs/qa/console-native-packaging-2026-10-13/windows-unicode/README.md` records
exact inputs, failed job artifacts, qualification bounds and publication state.
