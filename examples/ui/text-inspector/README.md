# Text Inspector UI tracer bullet

Text Inspector is one Level G cREXX application run unchanged by GTK, a
portable TUI, and deterministic tests. It counts physical lines,
whitespace-delimited words, and Unicode characters excluding line terminators.

Read `lib/ui/README.md` first for the short framework vocabulary and
lifecycle. Then read the files in this order:

1. `text_inspector.rxpp` — application model, `update`, effects, and logical
   view;
2. `lib/ui/ui.crexx` — contracts, relative layout, and runtime;
3. `lib/ui/ui_tui.crexx` — the simplest complete driver event loop; and
4. `lib/ui/ui_gtk.crexx`, then `lib/ui/drivers/gtk/ui_gtk.c` — the cREXX and
   native halves of GTK.

The example is architectural rather than a widget catalogue. `Open...`
produces a `choose_file` effect. Selecting a path produces `file.selected`,
which leads to a backend-neutral `read_text` effect. The runtime turns that
effect into `file.loaded` carrying the physical text records, or
`file.load.failed`. The application updates its state and returns a new logical
view after each event.

These names illustrate the layered vocabulary. `app.ready`, file-selection
and file-loading outcomes, and their effects are framework contracts;
`document.open.requested` and `document.clear.requested` are feature events
added by Text Inspector. The dotted form runs from subject to completed fact
and stays stable across GTK, TUI, future web drivers, and tests.

The authored application is `text_inspector.rxpp` and explicitly selects
`OPTIONS LEVELG`. RXPP includes
`ui_macros.rxpm` and generates repetitive logical-node construction into the
build tree. Its declarations show `label`, `line`, and `button` nodes placed
`root`, `below`, or `right` of stable node IDs. The view resolves that
relative layout to a shared logical grid: GTK renders a `GtkGrid`, while the
TUI groups nodes with the same row.

The backend launchers are authored as the two declarative files
`text_inspector_tui.rxpp` and `text_inspector_gtk.rxpp`. Both call the shared
`UI_LAUNCHER` macro; CMake runs RXPP separately because RXPP has one output per
invocation. The resulting `_tui.crexx` and `_gtk.crexx` files are generated in
the build tree, source-mapped, and follow the normal
`rxpp -> rxc -> rxas -> rxlink/rxvm` toolchain.

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
  cmake-build-debug/bin/library
```

The TUI is intentionally line-oriented and dependency-free. It proves the
driver/event-loop boundary without introducing ncurses/PDCurses policy. A
cursor-positioned ANSI driver can replace it behind `ui.uidriver` and can be
mostly cREXX. Truly curses-like portable input still needs a narrow native
terminal capability for raw mode, key reads, sizing, and reliable restoration;
the feasibility assessment is in `lib/ui/README.md` and is not implemented in
this tracer increment.

The GTK loop enters cREXX through RXPA `CALLMETHOD`. Because cREXX class values
are copied, the synchronous driver carries an explicit weak reference to the
live `ui.uiruntime`; this is the identity-preserving pattern for callbacks that
cannot outlive their native procedure call.
