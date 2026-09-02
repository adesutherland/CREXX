# Graphical User Interfaces and Graphics

## Proposed framework at a glance

The experimental Level G UI library treats a program as a backend-neutral
state machine driven by a replaceable event loop. An application owns its
model, reduces semantic events in `update`, and derives a logical `view`. A
runtime dispatches events and performs portable effects. The selected driver
owns the actual loop, native input and rendering, plus backend-specific effects
such as a file chooser.

An **event** is a fact that has happened (`document.open.requested` or
`file.loaded`). An **effect** is work requested by the application
(`choose_file`, `read_text`, or `quit`). Effect completion is delivered as
another event. A **view** is a disposable description of logical nodes, not a
collection of live GTK widgets. An **action** connects an interactive node to
a semantic event name; drivers translate clicks or commands to that name.

This vocabulary is layered. The framework defines common lifecycle events,
widget interaction contracts, effects, and completion outcomes. Applications
extend it with feature intent such as `document.open.requested`. Dotted event
names run from owner or subject to fact, and the final word—`requested`,
`selected`, `loaded`, `cancelled`, or `failed`—makes lifecycle direction
clear. Names describe meaning rather than the originating toolkit.

Applications can interpret their own new events directly. A new effect also
needs a runtime, driver, or future registered handler plus defined completion
events; returning an unknown string is not sufficient. The tracer's
underscore-form effect kinds are provisional and should receive an explicit
namespacing decision before the API is stabilised.

The lifecycle is: construct application, runtime, and driver; dispatch
`app.ready`; reduce an event; execute any effect and feed back its completion;
derive and render the next view; translate the next user action; repeat until
`quit`. The application therefore has an event-driven structure without
containing a GTK or terminal loop. Larger programs can keep a small root
`uiapp` which routes events to feature classes rather than accumulating all
behaviour in one god class.

The complete vocabulary, lifecycle, file-reading trail, and current limits
are documented in `lib/ui/README.md`. Text Inspector is the executable example.

The public UI library, cREXX drivers, applications, launchers, and their tests
explicitly select `OPTIONS LEVELG`. They use Level B classes, interfaces,
references, and RXPA as implementation mechanisms, but the user-facing UI
policy and application metaphor belong to the general-purpose Level G layer.

## The GTK plugins

GTK support is an optional build-time feature.[^option] The legacy
`lib/plugins/gui` API exposes GTK widgets directly and now includes the
experimental `run_event_loop(handler)` callback entry point. It remains useful
for low-level experiments, but application code written against that surface
owns native widget identifiers and GTK-specific sequencing.

The experimental Level G `lib/ui` library is the newer application architecture. Its
GTK driver deliberately duplicates only the small native subset required by
the tracer bullet. This keeps the legacy API available for comparison while
allowing the library to evolve into the owner of its native drivers.

[^option]: `-DENABLE_GTK=ON`

## Application model

Applications implement `ui.uiapp` as a small feature component with two
operations:

- `update(event)` reduces a semantic event into application state and returns
  an explicit effect;
- `view()` returns a backend-neutral logical view.

The runtime and selected driver form the event loop:

```text
native/TUI input -> semantic event -> update -> model
                                        |
                                        +-> effect -> completion event

model -> view -> logical nodes -> GTK, TUI, test, or a future driver
```

Drivers translate platform input into names such as
`document.open.requested`; application code does not receive GTK signals or
terminal key codes. Effects such as `choose_file`, `read_text`, and `quit`
make impure work visible. The portable runtime currently executes `read_text`
and reports `file.loaded` or `file.load.failed`; the driver owns native file
selection and loop termination.

This split gives one application program to GTK, the line-oriented TUI, and
deterministic tests. A future Windows, macOS, web, or full-screen terminal
driver can preserve the same semantic contracts even when its event source and
rendering mechanism differ.

## Callback lifetime and value semantics

RXPA `CALLMETHOD` provides same-thread, synchronous native-to-cREXX method
dispatch while the outer native procedure is active. It is not background
multitasking, and native code must not retain RXPA value handles after the
outer call returns.

cREXX class instances have value semantics. The GTK driver consequently stores
a weak `reference` to the `ui.uiruntime` argument while its synchronous event
loop is active. A callback dereferences that value before dispatching an event,
so it updates the live runtime rather than a copied snapshot. The reference is
valid only because the GTK loop returns before the driver's `run` frame ends.
Queued or asynchronous drivers require a longer-lived owner and must not reuse
this borrowed-reference pattern beyond that lifetime.

## RXPP and GUI builders

The Text Inspector example uses RXPP only for repetitive logical-view
construction. Its `UI_NODE(spec,text)` macro expands to calls on the cREXX view
builder; model state, event reduction, and effect policy remain readable cREXX.
That is the intended GUI-builder seam: a builder can generate stable node IDs,
bindings, actions, and source-map records without generating GTK, terminal, or
browser calls.

The current tracer supports logical `label`, `line`, and `button` nodes. A
node is placed at the `root`, `below`, or `right` of a prior stable node ID.
The view resolves those relations into logical rows and columns shared by GTK
and TUI. This is intentionally a small, extensible layout seam rather than a
final constraint or container model.

RXPP also generates the TUI and GTK launchers through `UI_LAUNCHER`. RXPP emits
one file per invocation, so each backend has a tiny `.rxpp` declaration and the
build invokes the common macro once per generated launcher. Application state
and effect policy remain ordinary cREXX.

Nested containers, collections, input validation, focus, accessibility
metadata, commands, subscriptions, asynchronous effects, incremental
rendering, and layout diagnostics are future contract work rather than implied
capabilities of this proof. An ANSI full-screen driver is feasible mostly in
cREXX for this scope, but portable raw keyboard input, sizing, resize handling,
and terminal restoration require a narrow native terminal capability; it has
been assessed but not implemented.

The complete example and command lines are in
`examples/ui/text-inspector/README.md`.
