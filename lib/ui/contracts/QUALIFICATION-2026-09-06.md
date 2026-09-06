# UI contract v0 — local qualification, 6 September 2026

Scope: the 47 standard events and 18 effects, Level G contract/session library,
temporary scalar-driver adapter, migrated Text Inspector, RXPP directory macros,
generated transport artifacts, conformance tests and build/install wiring.
Changes were developed on local `develop`, based on `7de12145a069`.
This record describes working-tree code, not a published commit or release.

## Accepted evidence

- **Comprehensive normal Debug QA:** 2,260/2,260 passed, GTK disabled, using
  `cmake --build cmake-build-debug --target qa-comprehensive --parallel 10`.
  The configured CTest parallelism was 32; runtime was 541.63 seconds.
  Retained local log: `/tmp/crexx-ui-v0-final-qa.nCQKWk`.
- **Separate clean GTK-enabled Debug tree:** built `ui_functional_tests` and
  `example_text_inspector_artifacts`; 14/14 focused UI/Text Inspector tests
  passed, including real GTK/RXPA callbacks and ordinary/optimized-linked modes.
  Log: `/tmp/crexx-ui-v0-gtk-final.FcQA8I`.
- **Generated dependency audit:** `ninja -t missingdeps` on those focused targets
  examined 1,685 nodes and reported no missing generated-file dependencies.
  Log: `/tmp/crexx-ui-v0-missingdeps.iu7Chl`.
- **Client artifacts:** JSON Schema validated against the 20 shared wire fixtures
  with Ajv 8.17.1. TypeScript 5.9.3 checked generated types/builders and positive
  and expected-error client calls, using `CheckClient.cjs`. Development packages
  were installed into a temporary directory, not made product dependencies.
- **Isolated UI installation:** installed the UI subtree into a temporary prefix
  and ran the Text Inspector TUI end-to-end test against those installed UI
  images. Passed. No user/global cREXX installation was changed.
  Log: `/tmp/crexx-ui-v0-install.Seo59q`.

The clean build exposed a focused-test preparation gap: the default runner path
did not add the `rxvm` producer target. The UI matrices now explicitly declare
`RUNNER rxvm`, so their owning preparation targets also build the runner.
The earlier missing-runner attempts and an interrupted pre-final broad run are
not counted as accepted qualification evidence.

## Input fingerprint and boundaries

The final UI source/build/test input fingerprint, verified unchanged through
the accepted broad run, is:

`b4890a3ddc07749ef6311866add36c7c4340a881dad8a76792dadb618a25abc7`

Recompute from the repository root:

```sh
{
  rg --files lib/ui examples/ui/text-inspector |
    rg '\.(crexx|rxpp|rxpm|cmake|json|jsonl|cjs|c|h)$|CMakeLists\.txt$'
  printf '%s\n' cmake/CrexxBuildStages.cmake
} | LC_ALL=C sort | while IFS= read -r ui_input; do
  shasum -a 256 "$ui_input"
done | shasum -a 256
```

This is local macOS ARM64 evidence. Hosted Linux/Windows qualification has not
been run for this unpublished change. ASan/LSan remain the explicitly deferred
tracer-scope gate; no sanitizer-clean or release-ready claim is made. The staged
UI check is not a claim of full-product packaging qualification.

The documented v0 limits remain: flat tracer views, synchronous physical legacy
drivers, owner-only cancellation control, no remote transport/security layer,
and no implemented full-screen console or browser driver.
