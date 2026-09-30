# Text Inspector: one feature, three hosts

Text Inspector is a **Level G** worked example. It counts physical text records,
whitespace-delimited words and Unicode scalar values excluding line terminators.

Read [the framework introduction](../../../lib/ui/README.md), then
[text_inspector.rxpp](text_inspector.rxpp). Its session factory registers schemas,
commands and one feature; `text_inspector_component` owns the model, update and
view. Follow into [the contract](../../../lib/ui/ui_contract.crexx), your chosen
driver and [resource executor](../../../lib/ui/ui_local_resources.crexx).
There is one application contract and no compatibility shell.

## What to trace

Open invokes `document.open`, which queues `document.open.requested`.
The feature returns `ui.resource.choose`; the driver later completes
`ui.resource.selected` with an opaque grant. The feature requests `io.text.read`;
the shared executor completes `io.text.read.completed`. The feature counts the
records and returns a fresh logical view. Failure/cancellation are
`ui.effect.failed`/`ui.effect.cancelled`, not magic filename values.

Clear demonstrates `ui.dialog.confirm` and its OK/Cancel outcomes. Quit queues
`ui.close.requested`; this read-only feature accepts by returning `ui.close`.
After native destruction/restoration, the driver completes `ui.closed` and
tears down the session. Tests wrap the same feature to prove close can be vetoed.

Framework/widget events are standard; `document.*` belongs to this application.
Commands and effects are not native callbacks. No feature opens files, stores
GTK handles, emits terminal escape sequences or knows its driver.

`##LOADMACRO ui` loads `UI_COMMAND`, `UI_NODE` and the single `UI_LAUNCHER`
macro. The three launcher files differ only in driver namespace/class.
RXPP emits one source per invocation and retains source maps to the authored
`.rxpp`. Stable view IDs and `root`/`below`/`right` layout are the builder
seam; this is not yet a rich or responsive widget system.

## Build and run

All commands below run from the repository root.

```sh
cmake --build cmake-build-debug --target example_text_inspector_artifacts rxvm --parallel 10
```

### Line TUI

```sh
cmake-build-debug/bin/rxvm \
  cmake-build-debug/examples/ui/text-inspector/text_inspector_tui \
  cmake-build-debug/examples/ui/text-inspector/text_inspector \
  cmake-build-debug/bin/ui_tui \
  cmake-build-debug/bin/ui_local_resources \
  cmake-build-debug/bin/ui_contract \
  cmake-build-debug/bin/ui_catalog \
  cmake-build-debug/bin/ui \
  cmake-build-debug/bin/library
```

Enter `o`, then a file path; an empty path cancels. Enter `c`, then `ok` or
`cancel`; `q` closes. This host blocks on line input and is also useful for
deterministic scripted tests. It does not require GTK or raw-console support.

### ANSI full-screen terminal

```sh
sh examples/ui/text-inspector/run-ansi.sh cmake-build-debug
```

Run in a real terminal. `o` opens the selector: type/paste a path, or browse with
arrows, PageUp/PageDown, Tab and the mouse. Enter opens; Escape cancels.
`c` asks for Clear confirmation; `q` closes. Ctrl-C cancels an active dialog
or requests close. Resize preserves state; below 32x10 a resize prompt replaces
ordinary controls. No ncurses or GTK library is needed.

See [terminal lifecycle and limits](../../../lib/ui/TERMINAL.md), then trace
`ui_ansi` -> `ui_dialogs` / `ui_terminal_view` / `ui_local_resources` and
the Level B C console provider.

### GTK

Use a tree configured with `-DENABLE_GTK=ON` and GTK 3 discoverable through
`pkg-config`. On this checkout the existing GTK tree is `cmake-build-ui-v0-gtk`.

```sh
cmake --build cmake-build-ui-v0-gtk --target example_text_inspector_artifacts rxvm --parallel 10
cmake-build-ui-v0-gtk/bin/rxvm \
  cmake-build-ui-v0-gtk/examples/ui/text-inspector/text_inspector_gtk \
  cmake-build-ui-v0-gtk/bin/rx_ui_gtk_native \
  cmake-build-ui-v0-gtk/examples/ui/text-inspector/text_inspector \
  cmake-build-ui-v0-gtk/bin/ui_gtk \
  cmake-build-ui-v0-gtk/bin/ui_local_resources \
  cmake-build-ui-v0-gtk/bin/ui_contract \
  cmake-build-ui-v0-gtk/bin/ui_catalog \
  cmake-build-ui-v0-gtk/bin/ui \
  cmake-build-ui-v0-gtk/bin/library
```

Click Open, select a file, then Open/Cancel. Clear uses an explicit OK/Cancel
dialog. Quit and the window close control go through the same vetoable intent.
GTK uses its native loop and non-blocking dialog responses. Its idle mailbox
enters cREXX through synchronous RXPA; callbacks never outlive the native run.

## QA and extension

```sh
cmake --build cmake-build-debug --target rxconsole_test_artifacts ui_functional_tests example_text_inspector_artifacts --parallel 10
ctest --test-dir cmake-build-debug -R '^rxconsole_|^ui_|^text_inspector_' --output-on-failure
```

Repeat with the GTK tree to include real GTK signal/dialog tests (they briefly
open windows and require an available display). Optimized tests link bytecode
before running it. The line and GTK tests compare a shared event journal;
ANSI tests exercise a real PTY, resize, input, dialogs and terminal restoration.
Session tests separately cover routing, correlation, stale outcomes and limits.
See [how to extend](../../../lib/ui/EXTENDING.md) for the feature/widget/effect/
driver checklist. Sanitizers remain the agreed later tracer-scope gate.
