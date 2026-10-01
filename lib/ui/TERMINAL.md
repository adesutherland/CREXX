# Full-screen terminal tracer

The full-screen driver uses the same Text Inspector feature and UI contract as
the GTK and line-TUI tracer. It adds a standard confirmation dialog and an
Open-file selector. This is a bounded first terminal UI, not a complete curses
replacement, debugger frontend, or finished cross-platform GUI toolkit.

## Responsibilities and lifecycle

| Layer | Owns | Does not own |
| --- | --- | --- |
| Level B `rxconsole`, implemented in C | Terminal lease, raw input, timed reads, UTF-8/VT decoding, mouse/resize, output batches, restoration | Widgets, JSON, application callbacks, dialog policy |
| Level G `ui_terminal_view` | Cell projection, layout, frame output, focus traversal and hit geometry | Native console state or application model |
| Level G `ui_dialogs` | Confirmation and file-picker state, editing, directory navigation, logical views | Native windows or an event loop |
| Level G `ui_local_resources` | Session-local file grants and text-read execution | Application word-count policy |
| Level G `ui_ansi` | One owner loop, input routing, modal lifetime, effect execution and correlation | Text Inspector-specific actions or counters |
| Application component | Model, update, view, requested work | Terminal escape sequences and filesystem handles |

The launcher creates a session with implemented capabilities, registers the
feature and commands, then gives it to the host. The host acquires the console,
posts `ui.ready` and the viewport, pumps admitted input, executes requested
effects, and renders when state changes. Idle waits do not redraw the screen.

A dialog is a **deferred effect**. The host keeps the originating request and
routes input to the dialog while it is active. The original loop continues to
service resize and other queued work. There is no nested blocking `ask()` loop
and no reentrant call into a feature's `update()`.

On acceptance/cancellation, the host posts exactly one correlated outcome and
restores the previous focus. The application receives the result through its
ordinary update path. On accepted close, the host restores the terminal before
completing `ui.closed`, then releases the session. An unexpected VM exit also
has provider-session cleanup; the usual OS limitations on uncatchable death and
foreground ownership still apply.

## Events and effects in this slice

- Resize becomes `ui.viewport.changed` with `unit="cells"`, including while a
  dialog is active. A terminal below 32x10 displays a resize prompt, preserves
  dialog state and suppresses accidental activation. Ctrl-C remains available.
- Native key/text/paste records become `ui.input.key`, `ui.input.text` and
  `ui.input.pasted`. Paste never invokes application shortcuts.
- Mouse input becomes `ui.pointer.changed`; wheel input becomes `ui.scrolled`.
  Activation requires left-button press/release on the same drawn control.
  Clipped nodes are absent from hit-testing and Tab traversal.
- `ui.dialog.confirm` produces the first standard OK/Cancel dialog. OK completes
  `ui.dialog.accepted` with response `ok`; Cancel/Escape produces
  `ui.effect.cancelled`.
- `ui.resource.choose` with purpose `open` produces the local file selector.
  Acceptance becomes `ui.resource.selected` with an opaque resource grant.
  The resulting `io.text.read` is resolved through the grant registry, never by
  trusting a display name supplied in a request.

The current catalogue names remain authoritative. No terminal-specific event
names leak into the feature. A second simultaneous dialog is reported as
`dialog_busy`; unsupported effects/purposes get explicit failure, not false
success. The current host handles admission errors by stopping cleanly; it is
not an unbounded, concurrent event service.

## Standard dialogs and RXPP

The confirmation dialog is used for **Clear results?** when the composition
root supplies `ui.dialog`. All three reference hosts supply it; a host without
that optional service may be composed with immediate Clear. The feature, not
the driver, chooses that application policy.

The first selector opens one existing regular file. It provides directory and
parent navigation, scrolling, editable filename/path, Open and Cancel. Use
arrows/PageUp/PageDown to select entries, type or paste a filename, and press
Enter to open it. Tab/Shift-Tab traverses visible controls; mouse clicks select
entries and operate buttons. Clicking a directory enters it; selecting a file
fills the filename field. Save, multi-select, file deletion and creation are
outside this increment. Browsing never changes the process working directory.

`UI_NODE` now also accepts the logical `input` kind used by the selector.
`UI_LAUNCHER` generates the same session/host composition for every driver.
It uses the RXPP directory macro package. A future
builder can generate these declarations without generating native event code or
overwriting handwritten dialog/feature logic.

## Deliberate limits and qualification

- The display projection currently supports known single-cell ASCII/Latin-1.
  Controls and uncertain-width scalars display as `?`; original Unicode input,
  filenames and model values are preserved. Full grapheme/cell-width policy is
  separate follow-on work, not a claimed Unicode-terminal conformance result.
- Directory navigation uses the existing filesystem provider, whose Windows
  `A` APIs still limit path encoding. More than 4096 entries are not listed;
  the dialog gives an explicit message and still accepts a typed filename.
  Enumeration allocates before that check. Long edit lines are clipped; horizontal
  cursor-follow scrolling and filename filters are later improvements.
- File reads are synchronous and bounded by the tracer's record/message
  contract; an individual long record can allocate before validation. Async
  directory enumeration, streaming and cancellation during physical I/O are
  later work.
- Local grants are session-checked, capped at 64 distinct paths of at most 4096
  scalars, and expire with the host; selecting the same path reuses its grant.
  These are in-process authorities, not cryptographic remote tokens or open-file
  snapshots. A filesystem object can change between selection and reading.
- POSIX tests use real foreground pseudo-terminals. Windows has a native
  console backend but requires native hosted qualification; local macOS tests
  do not establish that result. Native Windows input does not advertise paste
  boundaries. Keyboard operation never requires mouse support.
- Sanitizer qualification remains the agreed later tracer-scope gate.

For low-level signatures and event fields, read
[rxconsole](../plugins/console/README.md). For executable commands, see
[Text Inspector](../../examples/ui/text-inspector/README.md). Retained local
validation is in the [terminal qualification record](contracts/QUALIFICATION-2026-09-06-TERMINAL.md).
