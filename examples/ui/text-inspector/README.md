# Text Inspector UI tracer bullet

Text Inspector is one cREXX application run unchanged by GTK, a portable TUI,
and deterministic tests. It counts physical lines, whitespace-delimited words,
and Unicode characters excluding line terminators.

The example is architectural rather than a widget catalogue. `Open...`
produces a `choose_file` effect. Selecting a path produces `file.selected`,
which leads to a backend-neutral `read_text` effect. The runtime turns that
effect into `file.loaded` carrying the physical text records, or
`file.load.failed`. The application updates its state and returns a new logical
view after each event.

The authored application is `text_inspector.rxpp`. RXPP includes
`ui_macros.rxpm` and generates repetitive label and button construction into
the build tree; the generated source is source-mapped and then follows the normal
`rxc -> rxas -> rxlink/rxvm` toolchain.

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
full-screen terminal implementation can replace it behind `ui.uidriver`.

The GTK loop enters cREXX through RXPA `CALLMETHOD`. Because cREXX class values
are copied, the synchronous driver carries an explicit weak reference to the
live `ui.uiruntime`; this is the identity-preserving pattern for callbacks that
cannot outlive their native procedure call.
