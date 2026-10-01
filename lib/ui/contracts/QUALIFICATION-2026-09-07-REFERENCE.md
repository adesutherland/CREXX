# Direct UI reference model — local qualification, 7 September 2026

Scope: the approved removal of scalar compatibility APIs and the direct line,
GTK and ANSI reference hosts, with the separately approved compiler repair.
Qualification baseline: `develop` at `d3f9f07c75c86b9eee443f3cf530a902c5d608b2`,
plus the then-uncommitted changes recorded here. This is local macOS ARM64
evidence, not publication or cross-platform release qualification. Earlier qualification records describe
their historical implementation; this record covers the replacement model.

## Compiler defect and regression

The valid expression `if _closed | _session.pending() = 0 then return` emitted
`ieq r-2,...` and `icopy ...,r-2`. A separate 19-line source reproduced it with
compiler optimization both enabled and disabled. The AST has the correct OR /
right-comparison shape; the register planner tested the left child's deferred
cleanup when deciding whether to assign the right child's result register.

The repair in `compiler/rxcp_emit_reg.c` checks `defer_reg_return(child2)` at
that right-child assignment. It preserves short-circuiting, linked storage
lifetimes and result-register sharing. No language change, no optimizer
disablement, no broad inlining fallback and no assembler validation relaxation.
The faulty line was already present in commit `268ab77537` (22 January 2026).

Permanent test: `compiler/tests/rexx_src/short_circuit_linked_left.crexx` and
its run golden. It exercises AND/OR truth tables with linked scalar/array
attributes, computed and method-call RHSs, skipped side effects and subsequent
attribute reads. Before the repair it failed assembly at six RHS sites;
afterward compile, assemble, link and VM execution pass in both modes.

- Minimal source: `/tmp/crexx-ui-shortcircuit.AGjOfQ/probe.crexx`.
- Before repair: `/tmp/crexx-ui-shortcircuit-evidence.XhGHH8` and
  `/tmp/crexx-ui-shortcircuit-noopt.Baz3Bu`.
- Permanent-test before/after: `/tmp/crexx-short-circuit-baseline.ToEOuw` and
  `/tmp/crexx-short-circuit-fixed.1ohtyv`.
- Focused compiler suite: **20/20**, `/tmp/crexx-short-circuit-tests.zciHZu`.

## Reference host evidence

Rebuilt GTK-enabled focused console/UI/example suite: **21/21**, including
opt/noopt linked execution. Log: `/tmp/crexx-ui-reference-final-focus.JRPA3o`.
It covers real GTK buttons/file chooser/confirmation responses; line and GTK
share the same feature event journal and a deliberately vetoed close. ANSI
uses real foreground PTYs for input, resize, dialogs, mouse and restoration.

All three hosts additionally run the same modal-close, unsupported-purpose,
overlapping-dialog and invalid-effect probes. They verify cancellation before
`ui.closed`, no fabricated successful close on a contract fault, and clean
subsequent runs. These probes found and repaired ANSI's per-run error-state
reset: the expected fault in one run must not prevent the next session from
starting. The initial failing probe log is
`/tmp/crexx-ui-reference-complete-focus.MAsVro`; the final suite above is green.

Shared resource tests retain missing files, sentinel-looking valid text,
foreign/session-mismatched grants, grant limits and oversized-record failure
with preserved correlation. The deleted scalar-runtime test was redundant with
the direct feature/resource tests; size-failure coverage now lives beside its
actual executor, not in a removed adapter.

Generated dependency audit: **1,750 nodes**, no missing generated-file
dependencies across `ui_functional_tests`, `example_text_inspector_artifacts`
and `rxconsole_test_artifacts` in the GTK tree.

GTK C syntax check passed with C99, `-Wall -Wextra -Werror`, excluding only
unused parameters/labels emitted by the existing RXPA macros. Log:
`/tmp/crexx-ui-reference-native-check-final.m251Lu`. This is not a sanitizer run.

## Installation/provider smoke

UI and example install scripts staged to `/tmp/crexx-ui-reference-stage.b5M1v3`.
All three launchers, direct host images, the three-macro RXPP package and updated
guides are present. Neither the retired compatibility image nor the GTK test
injector is installed. The initial install exposed a missing ANSI launcher;
the example install rule now includes it.

For isolated runtime checks, the matching VM/library and normal console/fs
provider files were copied into the stage. The GTK check used explicitly named
test-only observer/injector artifacts outside the stage; production UI modules
and native GTK mechanism came from the stage.

- Install: `/tmp/crexx-ui-reference-install-final.9xqbFq`.
- Interactive line Open/count/confirmation/quit: `/tmp/crexx-ui-reference-stage-line.n91gB1`.
- ANSI PTY Open/count/confirmation/resize/restoration: `/tmp/crexx-ui-reference-stage-ansi.NuCziE`.
- GTK real-signal lifecycle/error/cleanup checks: `/tmp/crexx-ui-reference-stage-gtk.zWtoS1`.

No global/user installation was changed. This proves the scoped UI/provider
closure, not a full-product package or native Windows/Linux installation.

## API and documentation changes

The removed scalar event/effect/runtime APIs and bridge intentionally lose their
RexxDoc blocks. The retained logical view API has exactly the same coverage:
20 blocks, 31 parameter tags and 11 return tags before and after. Driver `run`
tags now describe the session contract; the removed automated GTK constructor
argument is replaced by a separate test-only injector. Feature documentation is
retained; only the duplicate application's shell documentation is removed.

`lib/ui/README.md` explains lifecycle and terms; `EXTENDING.md` provides the
feature/widget/effect/driver checklist; the example README gives commands for
all three hosts. Independent `lib/plugins/gui` experiments are untouched.

## Comprehensive gate

The prepared Debug comprehensive gate passed **2,271/2,271 tests**, with zero
failures, in **401.95 seconds**. This tree has `ENABLE_GTK=OFF` and configured
CTest parallelism of 32; the separate GTK-enabled 21/21 suite above supplies
the GTK host coverage. The initial prep was interrupted before CTest to correct
the ANSI installation rule; no completed broad test run was discarded or repeated.

Command: `cmake --build cmake-build-debug --target qa-comprehensive --parallel 10`.
Log: `/tmp/crexx-ui-reference-comprehensive-final.n7BMfd`.

Frozen source/build/test input fingerprint (tracked and untracked non-Markdown
files outside `docs/`, path plus NUL plus file bytes, sorted and SHA-256):

`4f95fbc27aaaa5fb4761e7d6fccfff6291ccb60b9d23b354f24a84a7cc1dba1d`

The fingerprint was recomputed after the comprehensive gate and matched exactly.
Only this Markdown qualification record was then updated; no source, build or
test input changed after the passing run.

Sanitizer testing remains the explicitly agreed later tracer-scope gate.
No Linux/Windows, hosted-CI, release-ready or sanitizer-clean claim is made.
