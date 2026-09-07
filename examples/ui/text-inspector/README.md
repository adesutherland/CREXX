# Text Inspector UI contract v0 tracer

Text Inspector is a **Level G** feature shared by GTK, the portable line TUI, the ANSI full-screen host and
tests. It counts physical text records, whitespace-delimited words and Unicode
scalar values excluding line terminators.

First read [the framework introduction](../../../lib/ui/README.md) and
[UI contract v0](../../../lib/ui/contracts/README.md). Then open
[text_inspector.rxpp](text_inspector.rxpp): its small application shell registers
commands and application events; its `text_inspector_component` owns only the
model, update policy and logical view. Follow into `ui_contract.crexx`, then
`ui_compat.crexx`, and finally the desired driver.

Open invokes an application command, which queues
`document.open.requested`. The feature requests `ui.resource.choose`.
A successful choice supplies `ui.resource.selected` with an opaque resource
descriptor. The feature requests `io.text.read`; its
`io.text.read.completed` result carries text records. Failure and cancellation
use `ui.effect.failed` and `ui.effect.cancelled`. The application owns the
counting policy, not the driver or framework. OS paths remain in the host's
resource executor; the feature reads only display text and passes a resource grant back
when requesting I/O.

Framework and widget names are standard; `document.*` intent is registered by
this feature. A larger application can register several cohesive components
and target commands/events at their owners. The feature's update returns an
effect collection, including an empty one: it never opens dialogs or files itself.

The existing scalar drivers remain behind the temporary `uibridge` shell,
not a second copy of the feature. The bridge is synchronous and supports one
root feature. Deferred completion and multiple owners are separately exercised
in the session conformance tests. The legacy close path cannot report a true
post-destruction `ui.closed`; that limitation is explicit in the contract.

RXPP's recent `##LOADMACRO` directive selects the `ui` macro directory.
`UI_COMMAND` generates checked command registration, `UI_NODE` generates the
logical view, and `UI_LAUNCHER`/`UI_SESSION_LAUNCHER` generate the legacy/session
backend launchers. Package filenames
are lowercase. CMake stages the package under the selected macro-library root.

The logical view still deliberately has only `label`, `line` and `button`,
with stable IDs and `root`/`below`/`right` placement. GTK renders a grid; the
line TUI groups nodes by row. The declarations illustrate a builder-compatible
authoring surface, not a completed rich widget system.

The authored launchers are `text_inspector_tui.rxpp` and
`text_inspector_gtk.rxpp`. RXPP emits one file per invocation, so CMake generates
their .crexx launchers separately, including `text_inspector_ansi.rxpp` for the
new session host. `##BUILDDIR` is useful in the command-line
driver, but not needed for CMake's already-explicit output directories.

From a configured Debug tree, build the launchers with:

```sh
cmake --build cmake-build-debug --target example_text_inspector_artifacts
```

## Full-screen terminal (new)

Run in a real terminal from the repository root; no Homebrew library or GTK is needed:

```sh
sh examples/ui/text-inspector/run-ansi.sh cmake-build-debug
```

Use **o** to open the file selector. Type or paste a path (for example
`examples/ui/text-inspector/fixtures/sample.txt`) and press Enter; alternatively
browse with arrows, PageUp/PageDown, Tab and the mouse. Escape cancels. **c**
requests Clear and opens the standard OK/Cancel confirmation. **q** closes;
Ctrl-C cancels an active dialog or requests application close. Resize preserves
state; below 32x10 a resize prompt replaces ordinary controls.

For the existing GTK-enabled build use the same command with
`cmake-build-ui-v0-gtk` instead of `cmake-build-debug`.

The new launcher uses `make_text_inspector_session` and `ui_ansi` directly,
without `uibridge`. Its loop retains a dialog as a pending effect while still
handling input and resize. Dialog completion returns to the feature as an event.
Read [terminal responsibilities and lifecycle](../../../lib/ui/TERMINAL.md),
then trace `ui_ansi` -> `ui_dialogs` / `ui_terminal_view` / `ui_local_resources`
and finally the C-backed, Level B `rxconsole` primitives.

Focused QA (the PTY checks require Python 3 on Unix, not an application dependency):

```sh
cmake --build cmake-build-debug --target rxconsole_test_artifacts ui_functional_tests example_text_inspector_artifacts --parallel 10
ctest --test-dir cmake-build-debug -R '^rxconsole_|^ui_|^text_inspector_' --output-on-failure
```

## Existing line TUI and GTK

Run the TUI from the repository root with:

```sh
cmake-build-debug/bin/rxvm \
  cmake-build-debug/examples/ui/text-inspector/text_inspector_tui \
  cmake-build-debug/examples/ui/text-inspector/text_inspector \
  cmake-build-debug/bin/ui_tui \
  cmake-build-debug/bin/ui \
  cmake-build-debug/bin/ui_compat \
  cmake-build-debug/bin/ui_contract \
  cmake-build-debug/bin/ui_catalog \
  cmake-build-debug/bin/library
```

With `ENABLE_GTK=ON`, run GTK with:

```sh
cmake-build-debug/bin/rxvm \
  cmake-build-debug/examples/ui/text-inspector/text_inspector_gtk \
  cmake-build-debug/bin/rx_ui_gtk_native \
  cmake-build-debug/examples/ui/text-inspector/text_inspector \
  cmake-build-debug/bin/ui_gtk \
  cmake-build-debug/bin/ui \
  cmake-build-debug/bin/ui_compat \
  cmake-build-debug/bin/ui_contract \
  cmake-build-debug/bin/ui_catalog \
  cmake-build-debug/bin/library
```

The original TUI remains line-oriented and dependency-free. The new ANSI host
uses the same feature; it does not replace this useful scripted-input driver.

The GTK loop enters cREXX through RXPA `CALLMETHOD`. Because cREXX class values
are copied, the synchronous driver carries an explicit weak reference to the
live `ui.uiruntime`; this is the identity-preserving pattern for callbacks that
cannot outlive their native procedure call.
