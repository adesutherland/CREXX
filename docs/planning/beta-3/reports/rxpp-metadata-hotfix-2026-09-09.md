# RXPP build metadata fix forward, 9 September 2026

Worktree: `/Users/adrian/CLionProjects/CREXX-hotfix`, branch `hotfix`.
Base revision: `f75614c306941070309011eb9c0b670399960e1f`.

## Peter's existing CI repair

Peter added build metadata in `7ecda0922efbe5251798fc15f15855bf5273ef33`.
Its [Build CREXX run](https://github.com/adesutherland/CREXX/actions/runs/34321080612)
failed the headerless, argument and RexxScript driver smoke tests because the
driver printed extra blank lines. His follow-up `f75614c30` removed that output.
The [subsequent Build CREXX run](https://github.com/adesutherland/CREXX/actions/runs/34325781764)
passed Linux, Windows, both macOS architectures and Linux Debug optimizer parity.
This local repair starts from that follow-up; it does not duplicate its output fix.

## Remaining reproduced defects and repairs

The new `crexx_rxpp_metadata_*` tests and strengthened `rxpp_loadmacro` fixture
all failed against the base revision, for eight independently checked cases:

1. `##LINK` ran before RXC/RXAS and linked only external modules. A valid source
   produced no compiled root and a bundle that did not run the authored program.
   Linking now follows successful compilation and assembly, with the generated
   RXBIN first. Later command-line inputs are still processed; `--nokeep` preserves
   the linked deliverable even when it replaces the generated member.
2. The same early link returned success for syntactically invalid source.
   Compiler failure now stops the recipe before it can publish a linked image.
3. A bare source name, such as `bare.rxpp`, became the input directory, producing
   `bare.rxpp/dep.rxbin`. The path helper now returns `./` for bare filenames.
4. The driver split a complete external path on whitespace. A dependency beside
   a source under `source with spaces/` was passed as separate module names.
   Each typed manifest record now stays one path through argv construction.
5. Prefix matching discarded `dep` after `dependency_long.rxbin`; relative
   normalization also cancelled consecutive leading `..` components. Matching
   now uses complete names, preserving case and necessary parent components.
6. `##EXTERNAL` without an argument printed a warning-like message but allowed
   compilation and execution. It now fails preprocessing with the catalogued
   `RXPP_EXTERNAL_REQUIRES_MODULE` diagnostic and exit status 8.
7. `##NORUN` changed the invocation-wide execution option, suppressing a later
   ordinary source member. Its state now resets per member. The regression also
   checks manifest replacement when `##NORUN` is removed from the source.
8. External-path normalization overwrote the exposed `syspath` macro-library
   root. A following `##LOADMACRO` selected the conflicting source-relative
   macro package. A local base-path variable keeps the selected library root.

These changes repair the documented pipeline and existing directive behaviour.
They do not settle whether source files should own these build directives.
The existing requirement for a link recipe to declare external input remains.

## Additional existing driver regression

The broader run found `crexx_levelc_classic_runtime_smoke` failing on verbose
output: it requires `crexx executes:`, but `7ecda0922` changed the literal to
`crexx executes  :`. Inspection of `f75614c30` confirms that its output repair
left this change present. Program results were correct. The hotfix restores
the established literal and moves this existing test into smoke QA so the
push-triggered gate checks it too. No test expectation was weakened.

## Local validation

- Before repair: 0/8 passed in `/tmp/crexx-rxpp-red.JAtnI7`.
- After repair: 8/8 passed in `/tmp/crexx-rxpp-focused.IlQnGB`, using the
  ordinary optimized driver and RXPP built in `cmake-build-debug`.
- Broader Debug correctness: 2,292/2,293 passed in 431.59 seconds, retained in
  `/tmp/crexx-rxpp-debug-qa.Kzohf0`. The sole failure was the verbose-output
  regression described above.
  Preparation uses `cmake --build cmake-build-debug --target qa-prep --parallel 10`,
  followed by `ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
  --label-exclude '^performance-measurement$'`. The 186 performance-measurement
  tests belong to their separate quiescent-host lane.
- Final affected panel after restoring the verbose literal: 57/57 passed in
  137.97 seconds, retained in `/tmp/crexx-rxpp-final-panel.Sp5LT7`.
  It covers all `crexx_*`, `rxpp_*`,
  `text_inspector_*` and `rxc_srcmap` tests. The unaffected broader results are
  retained: hashes prove the only code/test changes since that run are the
  verbose literal and the existing test's smoke-tier membership.
  The entire 2,293-test run was not repeated after this final formatting repair.

Original CI logs are retained in `/tmp/crexx-rxpp-ci.GlIF5c`; small standalone
reproducers and output are in `/tmp/crexx-rxpp-repro.ll4Hl3/`. Code and test input
hashes are retained in `/tmp/crexx-rxpp-qualified-inputs.json` so completed QA
can be reused when only this report changes. Final qualified hashes are in
`/tmp/crexx-rxpp-final-qualified-inputs.json`.

No remote branch update, installation, hosted qualification or sanitizer run
has been performed for this local change.
