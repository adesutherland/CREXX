# Experimental cREXX UI library

This directory owns the tracer-bullet UI architecture. It is intentionally an
experimental library rather than an extension of the legacy widget-oriented
`lib/plugins/gui` surface.

The public application boundary is backend-neutral:

```text
native input -> semantic event -> app.update -> model
                                     |
                                     +-> effect -> completion event

model -> app.view -> logical nodes -> GTK, TUI or test driver
```

`ui.crexx` owns events, effects, views, the application/driver contracts and
the runtime. The runtime currently performs portable `read_text` effects and
returns driver-owned effects such as `choose_file` and `quit`.

`ui_tui.crexx` is an always-built line-oriented terminal driver. It uses the
same event loop in interactive and scripted test modes; scripted input is not a
separate application path. Like the GTK driver, it begins by delivering the
backend-neutral `app.ready` lifecycle event and processes any resulting effect
before its first render.

`drivers/gtk` owns a deliberately small GTK 3 driver and its RXPA plugin. It
duplicates only the GTK mechanisms needed by the tracer. The legacy
`lib/plugins/gui` implementation remains available for comparison and can be
retired after the new driver has earned a broader widget/view contract.

`rxpp/ui_macros.rxpm` provides one uniform `UI_NODE(spec,text)` operation for
repetitive logical-view construction. Application state transitions and effect
decisions remain ordinary cREXX so generated glue does not hide the program
architecture.

The first contract is synchronous. RXPA callbacks borrow their cREXX receiver
only during the active plugin call. Effects nevertheless complete through
semantic events, preserving the application shape required by a later queued
or asynchronous executor.

cREXX class instances have value semantics. `gtkdriver.run` therefore retains
an explicit weak `reference` to its runtime argument and dereferences it inside
each native callback. The reference is safe because GTK's event loop is
synchronous and returns before the `run` frame expires. An asynchronous driver
must instead own a longer-lived queue or runtime object; it must not retain this
borrowed RXPA handle or the frame-scoped reference.

Current compiler/linker limitation: a class implementing an imported interface
must spell member argument and return types the same way as the interface
metadata. The drivers therefore use imported `.uievent`, `.uieffect`,
`.uiview`, and `.uiruntime` names in their signatures. Writing the equivalent
qualified form such as `.ui..uievent` currently fails interface-conformance
linking and needs compiler/linker canonicalisation work.

The tracer also qualifies relative `##INCLUDE` resolution. RXPP now resolves
relative INCLUDE/USE paths against the selected macro-library directory and
retains a focused regression for that contract. Scripted macro dispatch now
matches a complete directive name, so a package named `ui.rxpm` cannot capture
an invocation of `##UI_NODE`.

Current deliberate limits are one GTK window, a flat label/button view, one
active GTK loop, scalar callback payloads, and synchronous effects. These keep
the experiment small while proving the architectural seams; they are not the
intended final widget, layout, validation, accessibility, or concurrency API.
