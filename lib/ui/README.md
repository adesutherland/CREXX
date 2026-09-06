# Experimental cREXX UI library

This is a driver-independent **Level G** UI framework and a worked Text Inspector
example. The shared feature runs under GTK, a line-oriented TUI and deterministic
tests. A full-screen text driver is the next priority; a modern browser surface
comes afterward. The event contract considers desktop, terminal, web and mobile
without presenting their native APIs as the cREXX programming model.

Start with [UI contract v0](contracts/README.md) for the complete vocabulary,
payload rules, ownership and lifecycle. [catalogue.json](contracts/catalogue.json)
is the machine-readable standard list; the build generates cREXX and TypeScript
bindings, JSON Schema and a readable catalogue from it.

## The framework in a minute

A feature owns its model, `update(event)` and `view()`. A command is a named intent
shared by buttons, menus and shortcuts. The session resolves enabled commands
to registered events targeted at the owning feature. Events describe facts;
effects request external work. An update returns zero or more effects, and
completion returns through the queue as another event.

The driver owns the real input/rendering loop. The session owns feature values,
bounded queues and request correlation. Executors perform I/O. Keeping those
responsibilities separate gives us cohesive feature classes without either
native callbacks scattered through the application or one all-application
controller.

The lifecycle is: construct/register -> surface ready -> enqueue input -> pump
feature updates -> take effects -> complete effects -> render -> repeat. A user
close request can trigger save/confirmation; accepted close is a host effect.
After native operations and resources are stopped, explicitly tear down the
session. No frame-scoped callback reference may survive its owning call.

Framework `ui.*` and `io.*` names and widget `widget.*` names are reserved.
Applications register extensions such as `document.open.requested`, including
their payload schemas. Application effects also declare a capability and result
event; merely returning a new name does not install an executor. Optional
capability families describe compatible semantics, not universal availability.

## Code-reading map

1. [Text Inspector](../../examples/ui/text-inspector/text_inspector.rxpp):
   ordinary cREXX feature logic plus RXPP command/view declarations.
2. [UI contract v0](contracts/README.md) and [ui_contract.crexx](ui_contract.crexx):
   typed payloads, messages, commands, capabilities, registry and queued session.
3. [ui_compat.crexx](ui_compat.crexx): temporary scalar-driver translation and
   local resource grants; native path handling stays outside the feature.
4. [ui.crexx](ui.crexx): retained logical view and legacy driver/runtime contract.
5. [ui_tui.crexx](ui_tui.crexx) or [ui_gtk.crexx](ui_gtk.crexx), then
   [GTK mechanism](drivers/gtk/ui_gtk.c).
6. [RXPA bridge](../../interpreter/rxpacallmethod.c): synchronous native-to-cREXX
   method dispatch, owned by the complete VM.

The existing line TUI and GTK are intentionally kept as compatibility drivers.
They remain synchronous. The new session's delayed completions, subscriptions,
multiple feature owners and cancellation are proven with backend-free executors,
not misrepresented as new asynchronous GTK support. The bridge's close-handshake
limitation is documented in the contract.

`drivers/gtk` is owned by this UI library. It duplicates only the required
GTK 3 mechanism; the older `lib/plugins/gui` remains a separate comparison
surface. Native mechanism is C; public UI policy, libraries and application code
use `OPTIONS LEVELG`.

## Logical view and layout

The current logical node vocabulary is `label`, `line`, and `button`. Nodes
have stable IDs. Buttons add a semantic action and a TUI shortcut. This is
small, but it proves that drivers consume a cREXX view rather than application
code calling native widget procedures.

Layout uses durable relative relationships rather than GTK coordinates:

```text
kind | id | action | shortcut | relation | anchor
```

The first node is `root`. Another node can be `below` or `right` of a prior
stable ID. `uiview_impl` resolves those relationships into a logical row and
column once; GTK attaches nodes to a grid and the TUI groups nodes by row. The
line node becomes a `GtkSeparator` or a textual rule. A later layout system can
add containers, spans, alignment, and constraints while preserving stable IDs,
semantic actions, and the driver-facing row/column seam. Unknown relations or
anchors currently fall back below the preceding node; production validation is
still required.

## RXPP and a future GUI builder

The current example uses `##LOADMACRO ui` and the directory package
[rxpp/ui](rxpp/ui), with lowercase filenames:

- `UI_NODE` emits logical view construction;
- `UI_COMMAND` emits checked command registration;
- `UI_LAUNCHER` emits the small legacy application/runtime/driver composition root.

CMake stages that package under the selected macro-library directory. The older
`ui_macros.rxpm` INCLUDE package is retained for existing callers, but the v0
example does not depend on it. RXPP keeps source maps back to authored declarations.
Use the .rxpp files as source, not the generated .crexx files in the build tree.

RXPP still emits one source output per invocation. The two small launcher .rxpp
files therefore generate the TUI and GTK launchers separately. The recent
`##BUILDDIR` directive is consumed by the command-line `crexx` driver; CMake
already owns its explicit output directories and does not need that directive.

This is the builder seam: view IDs/layout, command descriptors, message schemas,
capabilities and feature ownership. A builder can emit RXPP or equivalent cREXX
wiring while preserving handwritten feature methods. It should not emit GTK,
terminal-control or browser-DOM calls into application logic.

## Callback and object lifetime

The legacy GTK callback path is synchronous. RXPA `CALLMETHOD` enters a named cREXX
method from native code while the outer plugin call is active. RXPA values are
borrowed for that call; a plugin must not retain them after it returns.

cREXX class instances have value semantics. `gtkdriver.run` therefore retains
an explicit weak `reference` to its runtime argument and dereferences it inside
each native callback. The reference is safe because GTK's event loop is
synchronous and returns before the `run` frame expires. An asynchronous driver
must instead own a longer-lived queue or runtime object; it must not retain this
borrowed RXPA handle or the frame-scoped reference.

There is also a current compiler/linker limitation: a class implementing an
imported interface must spell member argument and return types the same way as
the interface metadata. The drivers therefore use imported `.uievent`,
`.uieffect`, `.uiview`, and `.uiruntime` names in signatures. The equivalent
qualified form such as `.ui..uievent` currently fails interface-conformance
linking and needs canonicalisation work.

## Feasibility of an ANSI full-screen driver

A third, ANSI-only driver is highly feasible for the tracer's present scope
and should be mostly cREXX. It should use the new `ui_contract` session boundary, render the existing
logical rows and columns with ANSI cursor movement, draw `line`, map the current
shortcuts to semantic actions, and retain the same feature and session conformance tests.
No application or event vocabulary needs to change.

It is important not to call that a pure-ANSI replacement for curses in the
general case. ANSI escape sequences are chiefly an output protocol. Robust
single-key input, raw/canonical mode, terminal size and resize, signal-safe
restoration, escape-sequence decoding, and Windows console differences require
terminal services that cREXX does not yet expose portably. The scalable shape
would be:

- `ui_ansi.crexx` for layout, rendering, key mapping, redraw, and the event
  loop;
- a very small cross-platform terminal capability plugin for raw mode,
  restoration, size, and key reads; and
- no ncurses dependency unless later requirements justify its richer terminal
  database and input handling.

For the present application, an initial ANSI driver could remain line-input
based and still demonstrate cursor-positioned redraw entirely in cREXX. Before
claiming full-screen portability, it should add pseudo-terminal tests for input
and teardown plus snapshot tests for emitted escape sequences. This is an
assessment only; that driver is deliberately not part of this tracer increment.

## What is implemented, and what is next?

Implemented: the versioned standard catalogue and schemas; Level G typed payload
builders and validation; application message registration; command descriptors
and enablement; capability checks; targeted component ownership; bounded,
owner-driven event/effect queues; correlation, progress, subscriptions, logical
cancellation, stale-result rejection and explicit teardown. Text Inspector uses
these contracts through the compatibility shell.

Still deliberately narrow: one GTK window, a flat three-widget view
(`label`, `line`, `button`), relative placement, scalar legacy callbacks and
synchronous physical effects. Names for richer widget/input/service families do
not implement those widgets. Rich view properties and containers, console C
primitives, full-screen terminal rendering, a browser transport/frontend,
remote-session security, responsive layout and accessibility implementation are
next stages.

Build the focused artifacts before running their tests:

```sh
cmake --build cmake-build-debug --target ui_functional_tests example_text_inspector_artifacts --parallel 10
ctest --test-dir cmake-build-debug -R '^ui_|^text_inspector_' --output-on-failure
```

Use `qa-comprehensive` for prepared broad correctness QA. This contract step does
not claim sanitizer qualification; that remains the agreed later tracer gate.
