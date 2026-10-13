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
