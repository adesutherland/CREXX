# Beta 3 development defect batch — 18 September 2026

Local qualification on macOS ARM64, based on
`15c8a3ba42009ab8b5a9b447aa8c06ce86b9b392`. Adrian authorized publication of
the qualified RXAS alias repair, fixes for #700/#702, and evidence-based
reconciliation of #699. This is development work, not a beta release.

## Changes and results

- **OPT-BOUNDARY-01:** publish the incoming-storage alias correction and its
  eight-case optimized/unoptimized runtime fixture. The three qualified
  source/test hashes in the [original manifest](../optimization-boundary-audit-2026-09-18/repair/manifest.json)
  still match. Reuse the retained **102/102** affected RXAS/flow/runtime results;
  the two permanent runtime variants also pass in this batch's focused panel.
  The [repair record](../optimization-boundary-audit-2026-09-18/repair/README.md)
  describes exact-identity handling and preserved optimization controls.
- **#700:** add `import rxfnsb` to the complete fixed-fields documentation
  example and state the prerequisite for the chapter's other `binresize` use.
  The verbatim original fails with `FUNCTION_NOT_FOUND` (exit 2); the corrected
  block compiles and runs in optimized and unoptimized modes (exit 0, no
  diagnostics). Its marker records this manual check, not an automated suite.
- **#702:** keep the shell basename lookup and return value const-qualified,
  with the cast confined to the POSIX mutable-typed `argv` boundary. The core
  build and a syntax-only compile of the real translation unit with qualifier
  warnings treated as errors pass. This is Apple Clang verification; it does
  not claim reproduction of the reporter's exact compiler diagnostic.
  Existing spawn inheritance, termination and launch-diagnostic tests pass.
  `ADDRESS SYSTEM` and configured `ADDRESS SHELL` produce the expected output
  with an absolute executable and a relative executable without a slash.
- **#699:** repair `f786b15d86a66b52c8ab1ce631054afc7a5937e8` and closeout
  `94f2f228c31339f87bb66b714724bdb4ed3a8420` are already in develop ancestry.
  Reuse the [published qualification](../issue-699-20260916/repair.md): full
  normal correctness 2,294/2,294, focused Apple ASan 7/7, and successful normal
  hosted publication including optimizer parity 742/742. Those remain results
  for their recorded revision. Both permanent source-import regressions also
  pass at the present working inputs. No new importer change is needed.

## Batch verification

`core-build.log` records a successful incremental build of `rxc`, `rxas`,
`rxlink`, `rxvm` and the three focused spawn test executables:

```sh
cmake --build cmake-build-debug --target rxc rxas rxlink rxvm test_rxspawn_inheritance test_rxspawn_diagnostics test_rxspawn_posix_termination --parallel 8
ctest --test-dir cmake-build-debug --parallel 4 --output-on-failure -R '^(rxspawn_|entry_alias_runtime_rxvm_|source_import_private_dependency_)'
```

The focused panel passed **7/7**, 26.81 seconds (`focused.log`).
`commands.json` records the exact documentation, strict compile and shell
commands, with their outputs alongside. `fixed-fields.crexx` is the verbatim
corrected documentation block; the `before` copy omits only the import.
`qualified-inputs.json` identifies the source/test inputs and current local
tools. The alias record retains its own earlier tool hashes.

For the relative-shell check, `shell-relative` is a temporary symlink to
`/bin/sh` in the command's working directory. Configured POSIX SHELL uses
`execv`/`execve`, so an earlier bare `sh` attempt without a local executable
was an invalid test setup: it does not request a PATH lookup. An unused
import was also removed from the smoke source before retaining final results.
Neither test setup correction changes product code.

Normal automatic Build CREXX (Release product/smoke and optimizer parity) and
CodeQL remain the publication gates for the combined pushed head. Their
terminal URLs belong in the issue closure receipts. No extra overnight run
is requested; this record makes no new full sanitizer or release-ready claim.
Unrelated roadmap, architecture-review and native-inference edits are excluded
from the publication batch. KeyAccess acceptance remains closed.
