# Experimental cREXX UI library

This directory contains the tracer-bullet implementation of a proposed cREXX
Level G UI architecture. It is backend-neutral application code with replaceable
drivers, rather than a cREXX wrapper around one native widget toolkit. The same
application currently runs under GTK, a portable line-oriented TUI, and
deterministic tests.

The public UI contracts, cREXX drivers, applications, launchers, and tests use
`OPTIONS LEVELG`. They build on Level B mechanisms such as typed classes,
interfaces, references, and RXPA, but UI policy belongs at the general-purpose
Level G layer. The native GTK plugin is C mechanism underneath that boundary;
it is not itself assigned a cREXX language level.

## The framework in one page

A UI program is a small state machine surrounded by an event loop:

```text
                        semantic event
                              |
                              v
native input ---> driver ---> runtime ---> app.update(event)
     ^                                      |          |
     |                                      |          +--> effect request
     |                                      v                    |
     +--- rendered nodes <--- app.view() <--- model <--- completion event
                                                              ^
                                                              |
                                                    runtime or driver
```

The terms have deliberately narrow meanings:

| Term | Meaning and owner |
| --- | --- |
| **application** | A feature component implementing `uiapp`. It owns model state, `update`, and `view`, but no GTK handles or terminal codes. |
| **model** | The application's current data: for example the selected path, status, and counts. |
| **event** | A semantic fact that has already happened, such as `document.open.requested`, `file.selected`, or `file.loaded`. It is not a GTK signal or key code. |
| **update** | The deterministic method that applies one event to the model and returns the next effect request. |
| **effect** | A request for work outside `update`, such as `choose_file`, `read_text`, or `quit`. Completion returns as another event. |
| **view** | A disposable, backend-neutral description derived from the model. It contains logical nodes, stable IDs, actions, text, and layout—not live native widgets. |
| **action** | The semantic event name attached to an interactive node. A driver converts a click or terminal command into that event. This is the present event-to-function or “slot” connection. |
| **runtime** | The dispatcher joining a `uiapp` to a driver. It calls `update`, executes portable effects, feeds completion events back, and exposes the next view. |
| **driver** | The owner of input, rendering, the actual event-loop repetition, and backend-specific effects. GTK and TUI are drivers of the same application. |

The lifecycle is:

1. An RXPP-generated launcher constructs the application, wraps it in a
   `uiruntime`, constructs one driver, and calls `driver.run(runtime)`.
2. The driver starts its backend and dispatches `app.ready` once. This is a
   lifecycle event, not an instruction to draw a particular window.
3. The runtime calls `app.update(event)`. The application may change its model
   and return an effect.
4. The runtime handles portable effects such as `read_text`; the driver handles
   backend effects such as `choose_file`. Success, failure, and cancellation
   return to the application as semantic events.
5. When no more immediate effects remain, the driver asks for `app.view()` and
   renders its logical nodes.
6. A click, command, or future key/mouse message is translated by the driver
   into the action's semantic event, and the cycle repeats.
7. A `quit` effect tells the driver to leave its loop, restore backend
   resources, and return to the launcher.

### Who defines events and effects?

The vocabulary is layered rather than wholly invented by each application:

- the framework defines lifecycle events and broadly reusable effects and
  outcomes, such as `app.ready`, `choose_file`, `file.selected`,
  `file.selection.cancelled`, `read_text`, `file.loaded`,
  `file.load.failed`, and `quit`;
- logical widgets define which interactions they can report and the shape of
  their payloads. A button, for example, activates the semantic action stored
  in its node; it does not expose a GTK signal or terminal key to the
  application; and
- an application extends the vocabulary with feature or domain intent, such
  as `document.open.requested` and `document.clear.requested`.

Event names are dotted namespaces. They read from the owning area or subject
towards a completed fact: `app.ready`, `document.open.requested`,
`file.selection.cancelled`. Final words such as `requested`, `selected`,
`loaded`, `cancelled`, and `failed` make direction and lifecycle visible.
The name describes meaning, not origin: `document.open.requested` remains the
same event whether it came from GTK, a TUI command, a web message, or a test.

Applications may introduce new events freely because their own `update`
method interprets them. Effects are extensible only together with an executor:
a new effect kind must be implemented by the runtime, a driver, or a future
registered effect handler, and its success/failure events and payload contract
must also be defined. An arbitrary effect string does not acquire behaviour by
being returned.

The tracer's effect kinds currently use compact underscore names
(`choose_file`, `read_text`) while events use dotted names. That is a
provisional PoC detail, not yet a naming standard. Before the effect surface is
made public, it would be reasonable to choose consistent namespaced kinds such
as `dialog.file.choose` and `io.text.read`; that change is deliberately left
as an explicit API decision.

This is a real event loop, but it is intentionally owned by the driver rather
than written into the application. That lets GTK call `gtk_main()` while a
terminal driver reads commands, without splitting the application into native
versions. `update` is the centre of one feature, not a proposed all-application
god class: a larger program should use a small composition-root `uiapp` to
route feature events to cohesive feature classes and combine their views.
Effects keep filesystem, network, dialog, timer, and similar policy out of
those feature reducers.

For the quickest code-reading path, follow:

1. `examples/ui/text-inspector/text_inspector.rxpp` — Level G model, events,
   effects, update, view, and declarative nodes;
2. `ui.crexx` — the public contracts and runtime;
3. `ui_tui.crexx` — the simplest complete event loop;
4. `ui_gtk.crexx` — cREXX GTK driver and callback receiver;
5. `drivers/gtk/ui_gtk.c` — the deliberately small native mechanism; and
6. `interpreter/rxpafuncs.c` — RXPA `CALLMETHOD`, the native-to-cREXX bridge.

## Components and ownership

`ui.crexx` owns the Level G events, effects, views, application/driver
contracts, and runtime. It has no dependency on GTK or a terminal protocol. The
runtime currently performs portable `read_text` effects and returns
driver-owned effects such as `choose_file` and `quit`.

`ui_tui.crexx` is an always-built, line-oriented terminal driver. It uses the
same event loop in interactive and scripted test modes; scripted input is not a
separate application path. Like GTK, it dispatches `app.ready` and processes
any resulting effect before the first render.

`drivers/gtk` owns a deliberately small GTK 3 driver and its RXPA plugin. It
duplicates only the GTK mechanisms needed by the tracer. The legacy
`lib/plugins/gui` surface remains available for comparison and can be retired
after the new library has earned a broader view contract. Keeping native code
behind an explicit driver makes three operating systems primarily a driver
and packaging concern, rather than something spread throughout applications.

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

`rxpp/ui_macros.rxpm` supplies two declarative operations:

- `UI_NODE(spec,text)` emits one logical view-builder call;
- `UI_LAUNCHER(driver_namespace,driver_class,app_namespace,app_class)` emits
  the small application/runtime/driver composition root.

RXPP produces one output stream per invocation, so one macro call cannot
currently create several sibling files as side effects. The example therefore
keeps one tiny `.rxpp` declaration per launcher and CMake invokes RXPP once for
each output. Both `_tui.crexx` and `_gtk.crexx` are generated in the build
tree; there are no hand-maintained backend launchers to drift. This uses RXPP's
existing model cleanly and does not require a multi-output preprocessor feature.
Each authored launcher declaration carries the warning to edit the `.rxpp`
source; generated files also receive RXPP's standard precompiled header.

This is also the intended GUI-builder seam. A builder can author stable IDs,
relative layout, semantic actions, and eventually validation/accessibility
metadata, then emit RXPP or equivalent cREXX declarations. It should not emit
GTK calls. RXPP source maps retain a route from generated cREXX diagnostics
back to the authored declaration.

## Callback and object lifetime

The first contract is synchronous. RXPA `CALLMETHOD` enters a named cREXX
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
and should be mostly cREXX. It can implement `uidriver`, render the existing
logical rows and columns with ANSI cursor movement, draw `line`, map the current
shortcuts to semantic actions, and retain the same application/runtime tests.
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

## Current limits

The tracer currently has one GTK window, three logical node kinds, relative
`root`/`below`/`right` layout, one active GTK loop, scalar callback payloads,
and synchronous effects. It does not yet define input fields, collections,
focus, validation, accessibility metadata, commands, subscriptions, async
effects, an event/effect registry and payload schema, incremental rendering,
layout diagnostics, or a long-lived application supervisor. Those are explicit
future contract decisions, not capabilities implied by this proof.

The tracer also qualifies RXPP behaviour needed by this structure: relative
`INCLUDE`/`USE` paths resolve against the selected macro-library directory, and
scripted macro dispatch matches complete directive names so `ui.rxpm` cannot
capture `UI_NODE`.
