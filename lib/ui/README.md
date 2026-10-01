# Experimental cREXX UI library

A driver-independent **Level G** programming model, demonstrated by one Text
Inspector feature running under line TUI, ANSI full-screen terminal and GTK.
This is a reference implementation to extend, not a complete widget toolkit.
Desktop, terminal, browser and mobile needs inform the vocabulary; browser and
mobile drivers are not implemented yet.

## The framework in a minute

A **component** is a cohesive feature: its model, `update(event)` and `view()`
live together. A **command** is a named intent shared by buttons, menus and
shortcuts. A **session** owns the components and resolves commands to their
registered, targeted events. An **event** says something happened or was
requested; an **effect** asks for external work. Each update returns an
`uieffects` collection—zero, one or many requests—without doing I/O itself.

A **driver** implements `uidriver.run(session)`: it owns input, presentation,
physical operations and the loop. All three drivers use the same contract:

```text
input -> invoke/post -> session queue -> pump -> component.update
                             ^                        |
                             |                    effects
                             |                        v
                         complete <- driver/executor <- take_effect
```

The driver renders the component's view after processing work. A view is a
disposable description with stable node IDs, not live native widgets. A dialog
is a **deferred effect**: its request remains active until acceptance,
cancellation or failure returns as a correlated event. “Deferred” does not imply
a worker thread. Physical file reads in this tracer are synchronous.

Lifecycle: compose/register; start the surface; post `ui.ready`; service input
and effects; repeat. `ui.close.requested` is vetoable intent. The component
accepts by returning `ui.close`. The driver stops native operations and closes
the surface, then completes `ui.closed` and pumps that final event before
`session.close()` tears down ownership. A failed host cleans up and returns
nonzero; it must not fabricate a successful close.

Framework `ui.*`/`io.*` and widget `widget.*` names are reserved. Applications
register their own names and payload shapes, such as `document.open.requested`.
The [catalogue](contracts/catalogue.json) defines the standard names;
[contract v0](contracts/README.md) explains fields, correlation and queue limits.
An application effect also needs a capability, outcome schema and executor.
Declaring a name alone does not implement it.

## Start here, then follow the code

1. [Text Inspector](../../examples/ui/text-inspector/text_inspector.rxpp):
   one session factory, one feature and declarative commands/view.
2. [ui_contract.crexx](ui_contract.crexx): `uicomponent`, `uidriver`,
   `uimessage`, `uieffects`, commands, capabilities and `uisession`.
3. [ui.crexx](ui.crexx): logical view interface and builder.
4. Choose a direct host: [line](ui_tui.crexx), [ANSI](ui_ansi.crexx) or
   [GTK](ui_gtk.crexx). There is no scalar adapter or second application shell.
5. Shared [local resources](ui_local_resources.crexx); ANSI
   [dialogs](ui_dialogs.crexx) and [cell projection](ui_terminal_view.crexx);
   native [console](../plugins/console/README.md) or
   [GTK mechanism](drivers/gtk/ui_gtk.c).
6. [Extending the model](EXTENDING.md): add a feature, widget, effect or driver.

The UI library owns its GTK mechanism. All application and framework code is
Level G; native providers expose lower-level mechanism, not application policy.

## Three hosts, one example

| Host | Loop and dialogs | Input/presentation limits |
| --- | --- | --- |
| Line TUI | Owner loop; retained chooser/confirmation request; next line supplies response | Blocking line input; shortcuts/command IDs; row-oriented rendering |
| ANSI | Timed console polling; Level G modal dialog state; resize stays live | Keyboard, text, mouse, paste where supported; bounded cell projection |
| GTK | Native GTK loop; owned signal mailbox serviced on idle; response-driven dialogs | One GTK 3 window; fixed label/line/button topology |

Each host starts with core capabilities and explicitly enables its implemented
services through `capabilities()`. All three demonstrate Open, Cancel,
confirmation, read failure and orderly close. Capability families are not a
claim to implement every catalogue operation: unsupported effects and chooser
purposes receive explicit failures. The local chooser implements single-file
Open, not Save or multiple selection.

`ui_local_resources` owns bounded, session-scoped path grants and text reading.
The feature may display a grant's name but cannot use it as authority. The same
executor handles missing files, oversized data and forged/expired grants in all
hosts. Limits and terminal-specific qualifications are in [TERMINAL.md](TERMINAL.md).

## Logical view, RXPP and builders

Nodes have stable IDs; buttons contain **command IDs**, not native signals or
event handlers. The current vocabulary is `label`, `line`, `button`, and
`input` (the ANSI dialog editor). Relative `root`/`below`/`right` placement
resolves to logical rows/columns. GTK uses a grid, line TUI groups rows, and ANSI
projects cells. Unknown anchors/relations currently fall back below the previous
node: stricter validation, containers, spans and responsive layout are future work.

One directory macro package, loaded by `##LOADMACRO ui`, provides:

- `UI_COMMAND`: checked command registration.
- `UI_NODE`: logical view construction.
- `UI_LAUNCHER`: session factory + driver capabilities + `driver.run(session)`.

All three launchers use that same macro. RXPP produces one output per invocation,
so CMake generates the three tiny authored launcher files separately. Edit
`.rxpp`, not build-tree `.crexx`; source maps lead back to authored declarations.
The launcher uses a local lifetime identity; network/reconnecting hosts must
allocate unique session identities rather than reuse this single-run value.

A builder can generate view/layout, command descriptors, schemas and composition.
Handwritten feature logic stays separate and survives regeneration. It must not
generate terminal escapes, GTK handles or browser DOM calls into a feature.

## GTK callback lifetime

GTK signals copy native facts into a bounded mailbox (128 records). One idle
source crosses RXPA into the host; only then does the host admit messages and
pump updates. Native response/destroy signals caused by executing an effect
enqueue facts instead of re-entering the active cREXX call. Dialogs use response
signals, not a nested blocking dialog loop. Window-manager close returns to the
application for a decision; destruction is reported afterward.

RXPA `CALLMETHODX` is synchronous. Its borrowed values remain inside native
`run()`. Because cREXX objects have value semantics, the small callback receiver
holds an explicit weak `reference` to the live driver, dereferenced only while
that outer call is active. It owns no application policy and never escapes the
run frame. Foreign threads must marshal owning data to an owner; neither this
session nor those borrowed handles are a thread-safe mailbox.

Implement imported interfaces using the signature spellings shown in their
metadata (for example `.uisession`, `.uimessage`, `.uieffects`, `.uiview`).
Qualified type-name canonicalisation remains a separate compiler limitation.

## Tests and next steps

[Example commands](../../examples/ui/text-inspector/README.md) show how to run
each host. Prepare focused artifacts before testing:

```sh
cmake --build cmake-build-debug --target rxconsole_test_artifacts ui_functional_tests example_text_inspector_artifacts --parallel 10
ctest --test-dir cmake-build-debug -R '^rxconsole_|^ui_|^text_inspector_' --output-on-failure
```

The line and GTK tests wrap the same real feature with a shared event journal,
including an intentionally vetoed close. GTK tests drive real buttons, chooser
responses, confirmation and destruction through a test-only injector that is
not installed. ANSI tests use a real foreground PTY for input, resize, mouse,
dialogs and restoration. Backend-free session tests cover multiple components,
progress, subscriptions, cancellation, stale results and bounded admission.

Use `qa-comprehensive` for prepared broad correctness QA. This slice makes no
sanitizer or cross-platform release claim; sanitizer testing remains the agreed
later tracer gate. Rich widgets, dynamic GTK topology, general Unicode cell
width, asynchronous file I/O, accessibility and browser/mobile implementations
remain subsequent work.
