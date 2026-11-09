# Experimental cREXX UI library

A driver-independent **Level G** programming model, demonstrated by Text
Inspector, an editable graph and Tile Map Demo across GTK, ANSI and both
browser routes. The line TUI remains a form compatibility host. This is a
reference implementation to extend, not a complete widget toolkit.

## The framework in a minute

A **component** is a cohesive feature: its model, `update(event)` and `view()`
live together. A **command** is a named intent shared by buttons, menus and
shortcuts. A **session** owns the components and resolves commands to their
registered, targeted events. An **event** says something happened or was
requested; an **effect** asks for external work. Each update returns an
`uieffects` collection—zero, one or many requests—without doing I/O itself.

A **driver** implements `uidriver.run(session)`: it owns input, presentation,
physical operations and the loop. All drivers use the same contract:

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
a worker thread. Physical file reads and bounded writes in this tracer are synchronous.

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
4. Choose a host: [line](ui_tui.crexx), [ANSI](ui_ansi.crexx),
   [GTK](ui_gtk.crexx) or [browser](ui_web.crexx). There is no second
   application state owner.
5. Shared [local resources](ui_local_resources.crexx); ANSI
   [dialogs](ui_dialogs.crexx) and [cell projection](ui_terminal_view.crexx);
   native [console](../plugins/console/README.md) or
   [GTK mechanism](drivers/gtk/ui_gtk.c).
6. [Extending the model](EXTENDING.md): add a feature, widget, effect or driver.

The UI library owns its GTK mechanism. All application and framework code is
Level G; native providers expose lower-level mechanism, not application policy.

## Text Inspector host baseline

| Host | Loop and dialogs | Input/presentation limits |
| --- | --- | --- |
| Line TUI | Owner loop; retained chooser/confirmation request; next line supplies response | Blocking line input; shortcuts/command IDs; row-oriented rendering |
| ANSI | Timed console polling; Level G modal dialog state; resize stays live | Keyboard, text, mouse, paste where supported; bounded cell projection |
| GTK | Native GTK loop; owned signal mailbox serviced on idle; response-driven dialogs | One GTK 3 window; labels, status, lines, buttons, inputs, checkboxes, progress, choices, combos and integers |

Each host starts with core capabilities and explicitly enables its implemented
services through `capabilities()`. All three demonstrate Open, Cancel,
confirmation, read failure and orderly close. Capability families are not a
claim to implement every catalogue operation: unsupported effects and chooser
purposes receive explicit failures. The local chooser implements single-file
Open and Save. Multiple selection is outside this slice.

`ui_local_resources` owns bounded, session-scoped path grants and text reading/writing.
The feature may display a grant's name but cannot use it as authority. The same
executor handles missing files, oversized data and forged/expired grants in all
hosts. Open grants read permission; Save grants write permission. Write
completions contain the executor-owned resource, and records use UTF-8 with LF
separators. Failed writes return correlated outcomes; replacement is not atomic.
Limits and terminal-specific qualifications are in [TERMINAL.md](TERMINAL.md).

## Logical view, RXPP and builders

Nodes have stable IDs; buttons contain **command IDs**, not native signals or
event handlers. The current vocabulary is `label`, `statusbar`, `line`, `button`,
and `input` (the ANSI dialog editor), `choice` and bounded `integer`.
`statusbar` is a single-message status display: GTK uses `GtkStatusbar`, while
terminal hosts render the same value as a normal status line.
Relative `root`/`below`/`right` placement
resolves to logical rows/columns. GTK uses a grid, line TUI groups rows, and ANSI
projects cells. Unknown anchors/relations currently fall back below the previous
node: stricter validation, containers, spans and responsive layout are future work.

`add_choice` takes parallel stable item IDs/labels and the selected ID. Selection
posts `widget.selection.changed` with a singleton `items` array. `add_integer`
takes value text, inclusive bounds and a positive step; edits/commits use
`widget.value.changed`/`widget.value.committed`. Features validate before
conversion. Rendering does not emit user events. GTK supplies radios and a spin
control; ANSI supplies exclusive keyboard/pointer selection and numeric editing.
Line input uses `nodeID itemID` (or item ordinal) and `nodeID value`.
Integer bounds in these hosts must fit signed 32-bit values; unsupported bounds
fail explicitly. The [shared Lotto example](../../examples/franz-lotto/SHARED-UI-GUIDE.md)
uses one feature for both console and GTK.

One directory macro package, loaded by `##LOADMACRO ui`, provides:

- `UI_COMMAND`: checked command registration.
- `UI_NODE`: logical view construction.
- Use the typed `add_choice`/`add_integer` builders for their item arrays and
  bounds; the existing scalar `UI_NODE` macro keeps its original syntax.
- `UI_LAUNCHER`: session factory + driver capabilities + `driver.run(session)`.

All three launchers use that same macro. RXPP produces one output per invocation,
so CMake generates the three tiny authored launcher files separately. Edit
`.rxpp`, not build-tree `.crexx`; source maps lead back to authored declarations.
The launcher uses a local lifetime identity; network/reconnecting hosts must
allocate unique session identities rather than reuse this single-run value.

A builder can generate view/layout, command descriptors, schemas and composition.
Handwritten feature logic stays separate and survives regeneration. It must not
generate terminal escapes, GTK handles or browser DOM calls into a feature.

## Architecture and current host coverage

The [UI expansion plan](PLAN.md) is the sole source for acceptance criteria,
approved decisions and later application-service gates. Features own model,
transitions, commands, effects and persistence policy. The session and scene
modules own routing, scene revisions, normalized input and capability refusal.
Hosts own physical controls, painting and device lifecycle. Independently
authored applications can share those boundaries without adding graph, map or
RAG schemas to the UI core.

| Host | Form and scene support |
| --- | --- |
| GTK 3 | Native form controls; Cairo drawing area for graph and map; pointer and keyboard input. |
| ANSI terminal | Form controls, semantic graph list, four by four map cell viewport and commands. |
| Browser local Wasm | Level G owner in a Worker; shared HTML/SVG/Canvas projection. |
| Browser hosted native | Same owner image and projection through an authenticated relay. |
| Line TUI | Existing form compatibility route. |

Browser JavaScript is the platform bridge for DOM, file grants and transport;
application decisions stay in cREXX. The local Worker suspends only for owner
stdin input. The hosted relay uses one authenticated session process and
reconnects from its latest full view. Both use versioned input and view frames.
The graph and map examples use the same handwritten features across graphical
and semantic routes. See [scene consumers](../../examples/ui/scene/README.md)
and [browser routes](../../web/ui/README.md) for build and check commands.

The current graphics profile is bounded to one scene per session, 512 items and
128 changes per delta. GTK and the browser implement rect, edge, tile and unit
drawing; ANSI chooses application-owned semantic alternatives. Object picking
uses bounded fixed-point world coordinates and half-open item rectangles;
edge selection, layering and accessible graph navigation remain future work.
No GPU accelerator has been selected. RAG query, compatible embedding,
generation and application packages are separate later plan steps.

Peter's RXPP DEFINE-GUI is a bounded authoring façade over these contracts.
Its generated view takes current values from an application-owned component;
the wrapper forwards each event once and registers shared commands once.
Invalid declarations report authored source locations. The
[Text Inspector sample](../../examples/ui/text-inspector/README.md) exercises
the generated wrapper through GTK and ANSI. ISPF-style panels can use the
general form infrastructure through an application adapter.

## GTK callback lifetime

GTK signals copy native facts into a bounded mailbox (128 records). One idle
source crosses RXPA into the host; only then does the host admit messages and
pump updates. Native response/destroy signals caused by executing an effect
enqueue facts instead of re-entering the active cREXX call. Dialogs use response
signals, not a nested blocking dialog loop. Window-manager close returns to the
application for a decision; destruction is reported afterward.

Choice/integer projection waits while captured native facts remain in that
mailbox. An earlier owner turn must not overwrite a later user edit before its
fact is admitted. The last mailbox turn renders the final model without input
echo; the GTK regression queues selection/editing and commits between turns.

RXPA `CALLMETHODX` is synchronous. Its borrowed values remain inside native
`run()`. Because cREXX objects have value semantics, the small callback receiver
holds an explicit weak `reference` to the live driver, dereferenced only while
that outer call is active. It owns no application policy and never escapes the
run frame. Foreign threads must marshal owning data to an owner; neither this
session nor those borrowed handles are a thread-safe mailbox.

Implement imported interfaces using the signature spellings shown in their
metadata (for example `.uisession`, `.uimessage`, `.uieffects`, `.uiview`).
Qualified type-name canonicalisation remains a separate compiler limitation.

## Additive scene and host support contract

`ui_scene.crexx` adds version 1 scene replication without changing the v0
`uiview`, `uicomponent`, message catalogue or driver interfaces. A host declares
exact `form.<kind>` and `scene.<kind>` operations in `uioperations`, then calls
`preflight_view()` for a complete form or `uiscenereplica.apply_for()` for a
scene packet. A typed `uiresult` identifies an unsupported operation or an
invalid packet. Unsupported content leaves the last valid scene frame and
input authority intact; the application can choose a semantic fallback.

One full snapshot establishes a scene epoch and revision. Atomic deltas name
the current base and the next revision. A gap or invalid operation pauses
scene input until a full authoritative snapshot replaces the replica. An
identical duplicate is harmless. `uipointer` carries normalized integer
viewport coordinates plus epoch/revision, allowing the owner to refuse stale
input before applying domain transitions. `uiviewport` maps these logical
pixels to world ticks with checked integer arithmetic. World coordinates are integers at
256 ticks per world unit; item coordinates and extents are bounded to 2^40
ticks, and wire revisions stay within JavaScript's exact integer range.
Items are presentation records with stable IDs and semantic labels; graph
edits, map turns, terrain data and undo remain with application features.

`uisceneowner` builds presentation revisions and bounded object picking from
application-owned records. A `scene` view node carries a full packet or delta
through the existing `node_value()` method. The GTK host validates the packet
in Level G, stages its Cairo primitives and commits one native frame. The
same graph feature selects a semantic node/edge list and stable commands on
ANSI. An unsupported GTK operation is delivered to the feature as
`ui.view.refused` before a partial frame; a revision gap produces
`ui.scene.resync` and one authoritative full replacement. See the
[direct scene example](../../examples/ui/scene/README.md) and
[`PLAN.md`](PLAN.md) for milestone and later service gates. The focused
`ui_scene_contract_noopt` test checks
preflight, atomic changes, stale input, overflow and full resync. The session
copies its broad capability set on construction; a caller cannot mutate a
running session by changing that original builder.

## Browser routes

The shared Level G browser owner runs Text Inspector, the graph, the map and
the [process designer](../../examples/ui/process-designer/README.md).
A Wasm Worker and a hosted native process use the same version-1 view/input
envelope and browser projection. The browser projects labels, buttons and
single-line text inputs alongside SVG graphs and Canvas 2D maps, handles
physical input and completes file effects.
Scene frames include the current delta and full authoritative snapshot for
reconnect and full resync. See the [browser build and checks](../../web/ui/README.md).

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
width, asynchronous native file I/O and accessibility remain subsequent work.
