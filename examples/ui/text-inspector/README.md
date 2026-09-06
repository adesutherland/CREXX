# Text Inspector UI contract v0 tracer

Text Inspector is a **Level G** feature shared by GTK, the portable line TUI and
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
counting policy, not the driver or framework. OS paths remain in the compatibility
executor; the feature reads only display text and passes a resource grant back
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
logical view, and `UI_LAUNCHER` generates each backend launcher. Package filenames
are lowercase. CMake stages the package under the selected macro-library root.

The logical view still deliberately has only `label`, `line` and `button`,
with stable IDs and `root`/`below`/`right` placement. GTK renders a grid; the
line TUI groups nodes by row. The declarations illustrate a builder-compatible
authoring surface, not a completed rich widget system.

The authored launchers are `text_inspector_tui.rxpp` and
`text_inspector_gtk.rxpp`. RXPP emits one file per invocation, so CMake generates
both .crexx launchers separately. `##BUILDDIR` is useful in the command-line
driver, but not needed for CMake's already-explicit output directories.

From a configured Debug tree, build both launchers with:

```sh
cmake --build cmake-build-debug --target example_text_inspector_artifacts
```

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

The TUI is intentionally line-oriented and dependency-free. It proves the
driver/event-loop boundary without introducing ncurses/PDCurses policy. A
cursor-positioned ANSI driver can replace it at the `ui_contract` session boundary and can be
mostly cREXX. Truly curses-like portable input still needs a narrow native
terminal capability for raw mode, key reads, sizing, and reliable restoration;
the feasibility assessment is in `lib/ui/README.md` and is not implemented in
this tracer increment.

The GTK loop enters cREXX through RXPA `CALLMETHOD`. Because cREXX class values
are copied, the synchronous driver carries an explicit weak reference to the
live `ui.uiruntime`; this is the identity-preserving pattern for callbacks that
cannot outlive their native procedure call.
