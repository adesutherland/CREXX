# Terminal tracer — local qualification, 6–7 September 2026

Scope: the approved first full-screen terminal host, low-level Level B C console
provider, Level G rendering/dialogs/resource executor, shared Text Inspector and
RXPP session launcher. Working tree: local `develop`, baseline `e614f0142fe6`.
This is local implementation evidence, not a published commit or release.
The approved 7 September compiler repair clears the original comprehensive
blocker: **2,269/2,269** prepared normal Debug tests now pass, with **24/24**
rebuilt GTK/UI checks. Historical failure evidence remains below; platform and
sanitizer qualification boundaries are unchanged.

## Focused evidence

- GTK-enabled Debug tree: **24/24** focused console, UI, Text Inspector and
  source-import/source-map checks passed. Includes normal and optimized-linked
  library execution through `rxc`, `rxas`, `rxlink` and `rxvm`, plus existing GTK
  and line-TUI regressions. Log: `/tmp/crexx-terminal-gtk-final.2aV9lS`.
- Real foreground PTYs: incremental key/text/paste input, modifier-bearing mouse
  press/release, resize, small-screen recovery during a dialog, file selection
  and word counting, confirmation accept/cancel, keyboard/mouse activation,
  explicit close, abandoned-lease VM cleanup and terminal disconnect all passed.
  The harness keeps a shell session alive after VM exit to verify exact saved
  termios restoration, alternate-screen exit and cursor visibility. Startup and
  interaction use output rendezvous, not fixed sleeps.
- Backend-free tests cover standard dialog state, terminal-safe projection,
  drawn hit geometry, small-screen suppression, file reads, error-looking file
  contents, missing files, foreign/session-mismatched grants and grant limits.
- Generated dependency audit: **1,742 nodes**, no missing generated-file
  dependencies on the three focused artifact targets. Log:
  `/tmp/crexx-terminal-missingdeps-20260906.log`.
- Native core/decoder/tests pass `clang -std=c99 -Wall -Wextra -Werror
  -fsyntax-only` on macOS.
- Isolated UI-subtree installation: images, documentation and the session macro
  installed under `/tmp/crexx-terminal-stage-prefix.K6nCr5`. An isolated copied
  VM/library plus the normal `bin/providers` console/fs layout ran Open -> count
  -> Quit in a PTY and restored its terminal. This is a UI/provider-closure smoke
  check, not a full-product installation qualification. Install log:
  `/tmp/crexx-terminal-stage.MCfRND`. No user/global installation was changed.

## Library-discovered compiler repair

RXPP-generated launchers exposed a source-import crash. The source-import
reader parsed raw source-map markers instead of applying the preprocessing used
by direct compilation. An argument diagnostic was then dereferenced as a formal
argument. The importer now consumes source maps; formal-argument walkers skip
diagnostic nodes while retaining their errors. No language or syntax change,
optimizer disablement or broad inlining fallback was introduced.

`source_import_srcmap_factory` permanently covers valid mapped imports, invalid
ARG placement (diagnostic instead of crash) and malformed source-map rejection.
Existing optional-factory and source-map tests also pass. The full generated
launchers exercise the repaired import through ordinary application execution.

## Initial comprehensive gate, 6 September

Prepared normal Debug comprehensive QA: **2,267/2,268 passed**, GTK disabled,
CTest parallelism 32, 468.13 seconds. The one failure is
`binary_forward_dependencies`, investigated below. All new console/dialog/UI
tests passed. That initial run was **not green** and was not an accepted
comprehensive closeout. Its frozen input fingerprint was verified unchanged
afterward.
Command: `cmake --build cmake-build-debug --target qa-comprehensive --parallel 10`.
Log: `/tmp/crexx-terminal-debug-qa.3MmKnM`.

Frozen code/build/test input fingerprint:

`505ad80e43d9a7f17b4416b36b0d6e64ac3908f8495b668a374378720d666f44`

The initial fingerprint used this input list (before the subsequent compiler
repair below):

```sh
{
  rg --files lib/ui lib/plugins/console examples/ui/text-inspector |
    rg '\.(crexx|rxpp|rxpm|cmake|json|jsonl|cjs|c|h|py|sh|txt)$|CMakeLists\.txt$'
  printf '%s\n' cmake/CrexxBuildStages.cmake compiler/rxcpfunc.c \
    compiler/rxcp_val_type.c compiler/tests/CMakeLists.txt \
    compiler/tests/run_source_import_srcmap.cmake lib/plugins/CMakeLists.txt
} | LC_ALL=C sort | while IFS= read -r ui_input; do
  shasum -a 256 "$ui_input"
done | shasum -a 256
```

## Initial compiler blocker and attribution

At the end of the 6 September run, comprehensive closeout was blocked by the existing
`binary_forward_dependencies` regression, not waived as pre-existing. It fails
outside CTest parallelism in `/tmp/crexx-terminal-dependency-debug.NQ0PpN`
(log `/tmp/crexx-terminal-dependency-debug-log.sw8Vep`). The installed compiler
identifies itself as `crexx-1.0.0-beta.3+local.g7de12145a069`, before this UI work;
using that compiler with `--no-exe-import` and the same rebuilt Debug binary
root reproduces the exact `opt unused source body must not invalidate: 2`
failure. Retained baseline reproducer:
`/tmp/crexx-terminal-before-ui-repro.yVj0c8`; log:
`/tmp/crexx-terminal-before-ui-repro-log.eOhaix`.

LLDB evidence in `/tmp/crexx-terminal-dependency-bt.zwcZux` shows imported inline
metadata from `packednumeric.crexx` causing
`inline_class_has_reference_attribute(..., "float")` to request the nonexistent
`rxfnsg.float` class. The on-demand resolver then loads `extension.crexx`, making
its body a dependency. The GTK-focused tree does not contain `rxfnsg.rxbin`, so
its passing source-import checks did not cover this binary set. This is not a
parallel-test race. No inliner/packed-number change or test weakening was made
in that turn; the further compiler repair required separate approval.

## Approved compiler repair, 7 September

Adrian approved the compiler fix. `inline_node_is_storage_selector()` in
`compiler/rxcp_inline_rewrite.c` now distinguishes a storage-format leaf from an
object-class value by its exact intrinsic syntax position and the existing
storage validators. It avoids object reference-attribute lookup only for those
selectors. Actual object/reference checks, eager inline-payload validation,
dependency snapshot rules and ordinary optimized inlining are preserved. No
language or metadata format change was made.

The new `binary_storage_selector_dependencies` test creates isolated binary
providers for `sizeof`, `at`, `packed` and `compare`. A second binary's public
class signature triggers imported-body inspection even when the application
does not use that class. An unused same-namespace source edit must not invalidate
the snapshot. A separate consumer executes each imported body through `rxc`,
`rxas`, `rxlink` and `rxvme`, checking the result and retained method inlining.
Both compiler optimization modes are covered. The existing
`binary_forward_dependencies` still verifies real-source invalidation and
invalid-call rejection; its assertions were not weakened.

- Before: the installed pre-UI compiler fails the standalone new fixture at
  `sizeof/opt unused source must not invalidate: 2`. Reproducer workspace:
  `/tmp/crexx-storage-selector-before-work.ze4vEv`; log:
  `/tmp/crexx-storage-selector-before-fixed-fixture.8HV4xg`.
- After: **42/42** focused compiler checks passed, including the original and
  new dependency regressions, packed/binary positive and negative syntax cases,
  reference-safety tests and the source-map repair. Log:
  `/tmp/crexx-primitive-focus-qualified.H4RYLj`.
- The preceding wider regex accidentally selected three unprepared
  performance-measurement cases; they failed to find their generated programs
  (`/tmp/crexx-primitive-focus-final.AHh59Z`). These are not compiler-result
  failures or performance qualification. The final focused selection excludes
  that separate harness; the prepared comprehensive product gate follows.
- Rebuilt GTK-enabled Debug tree: **24/24** console, UI, Text Inspector and
  source-map checks passed, including terminal PTYs, dialogs, file selection and
  GTK callbacks. Log: `/tmp/crexx-terminal-compiler-gtk-qa.ar7ulo`.
- Repeated generated-file dependency audit: **1,742 nodes**, no missing edges
  on the three rebuilt UI/console/example artifact targets. Log:
  `/tmp/crexx-terminal-compiler-missingdeps.nLQqes`.
- Prepared normal Debug comprehensive QA: **2,269/2,269 passed**, GTK disabled,
  CTest parallelism 32, 375.60 seconds. Command:
  `cmake --build cmake-build-debug --target qa-comprehensive --parallel 10`.
  Log: `/tmp/crexx-terminal-compiler-debug-qa.N5Nm56`. This includes both
  dependency regressions and the terminal UI checks. The code/build/test input
  fingerprint below was verified unchanged after completion; the documentation
  closeout does not invalidate that evidence or require another full run.

Frozen code/build/test input fingerprint for this repair:

`520878d8d8a31bcfe422fdb02082094f3b2b7940dbcae84b98bb8a4a528255d1`

```sh
{
  rg --files lib/ui lib/plugins/console examples/ui/text-inspector |
    rg '\.(crexx|rxpp|rxpm|cmake|json|jsonl|cjs|c|h|py|sh|txt)$|CMakeLists\.txt$'
  printf '%s\n' cmake/CrexxBuildStages.cmake compiler/rxcpfunc.c \
    compiler/rxcp_val_type.c compiler/rxcp_inline_rewrite.c \
    compiler/tests/CMakeLists.txt compiler/tests/run_source_import_srcmap.cmake \
    compiler/tests/binary_storage_selector_dependencies.cmake \
    lib/plugins/CMakeLists.txt
} | LC_ALL=C sort | while IFS= read -r ui_input; do
  shasum -a 256 "$ui_input"
done | shasum -a 256
```

## Remaining qualification boundaries

Evidence is macOS ARM64 only. Linux and native Windows execution have not been
qualified for this unpublished change. Windows has a native console backend but
does not advertise paste boundaries. Sanitizers remain the user's explicitly
deferred tracer-scope gate; no sanitizer-clean or release-ready claim is made.
Unicode cell width, asynchronous physical file I/O, save/multi-select dialogs,
rich containers and a browser frontend remain follow-on work.

Unrelated pre-existing roadmap/release/performance edits were left untouched.
No commit, push or system installation is part of this implementation turn.
