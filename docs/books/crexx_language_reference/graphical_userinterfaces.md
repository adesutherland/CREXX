# Graphical User Interfaces and Graphics

## The GTK plugins

GTK support is an optional build-time feature.[^option] The legacy
`lib/plugins/gui` API exposes GTK widgets directly and now includes the
experimental `run_event_loop(handler)` callback entry point. It remains useful
for low-level experiments, but application code written against that surface
owns native widget identifiers and GTK-specific sequencing.

The experimental `lib/ui` library is the newer application architecture. Its
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

The current tracer view is intentionally flat and supports labels and buttons.
Nested layout, collections, input validation, focus, accessibility metadata,
commands, subscriptions, asynchronous effects, and incremental rendering are
future contract work rather than implied capabilities of this proof.

The complete example and command lines are in
`examples/ui/text-inspector/README.md`.
