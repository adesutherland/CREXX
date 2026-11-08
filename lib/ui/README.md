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

## Proposed GTK, terminal, browser and 2D architecture

The [UI expansion plan](PLAN.md) records options, open decisions and acceptance
criteria. This section describes the proposed boundary; no browser driver,
scene API or general graphics renderer is implemented in this library. GTK,
ANSI full-screen terminal and browser are parallel implementation tracks for
forms, editable graphs and strategy maps. The line TUI keeps its existing
compatibility role.

```text
Level G feature: model, update(event), view() / proposed scene()
          | semantic commands, messages and effects
uisession: validation, owner routing, queues, correlation and close
          | view or scene snapshot                 ^ normalized input/outcome
host adapter: GTK / ANSI terminal / proposed browser / proposed scene renderer
          | native widgets, SVG/Canvas, I/O, device and event loop
          + application effects -> RAG / database / inference executors
```

This is a platform for applications authored by different teams. Composition
registers each application's namespaced effects, schemas and commands, then
wires its executors. The shared session handles capability checks, correlation,
cancellation and outcomes; application teams own their repository contracts,
data formats, migration, transaction and conflict rules, and storage routes.
Graph and map features are independent example consumers, not a common domain
schema. An application-only service does not require a UI-core or global
catalogue edit; exact cross-route compatibility rules remain a design gate in
the [UI expansion plan](PLAN.md).

The selected delivery direction gives each application a versioned package
declaration for its entry composition, UI contract compatibility, required and
optional operations, dependencies and assets. Native and browser assemblies
may differ; each must check the declaration before starting a session. This
package format and its build/install proof are planned, not implemented.
Initial packages are trusted and approved by their deployer. Runtime loading
and untrusted-package isolation are outside the initial UI scope.

The browser track implements two routes in parallel: a local Emscripten Worker
and a hosted native cREXX session. Both run the same feature and use one
versioned view/input/effect envelope and one main-thread projection. The local
Worker owns its cREXX VM, UI image, session and component; the hosted route
owns its session on the server behind an authenticated transport. Both process
one session turn at a time and return bounded views to the adapter. The adapter
uses HTML controls, SVG or Canvas and sends normalized user facts back; it owns
DOM access, focus, browser dialogs and async Web APIs. Wasm does not directly
access the DOM, so replacing application JavaScript with cREXX does not remove
all JavaScript glue. Current `uidriver.run(session)` hosts own synchronous
loops; the browser design must choose an owner-turn/step interface or a
qualified suspend/resume mechanism. A blocking wait in the Worker would prevent
its next input message from being handled. Browser file choice and storage
should return opaque, scoped grants through correlated effects, following the
same feature contract as Text Inspector. The existing single-threaded
Emscripten profile and THE's separate browser preview are feasibility inputs,
not a working `uidriver` for this library. The hosted route separately needs
authentication, authorization, reconnect and revision ownership. Both routes
must pass the forms, graph and map stages through the shared browser projection.

The browser host also needs a route for application services. cREXX-RAG is a
useful stress case: its SQLite library owns corpus, graph and embedding truth,
while immutable vector sidecars accelerate cited lookup. A browser feature
could issue a bounded query effect and display the returned passages without
owning SQLite handles, model secrets or retrieval policy. The selected target
has two query executors behind that effect: an authenticated native RAG service
adapter and a browser-local read-only Wasm route using a validated SQLite and
vector-sidecar bundle. The local route requires `rxsqlite`, `rxvector` and a
compatible query embedding route. Both hosted and browser-local compatible
query embedding providers are targeted behind one contract; the local CPU/Wasm
or WebGPU backend remains unproved. Prove a transient validated bundle import
first, then target an OPFS cache with export/re-import recovery and multi-tab
behavior as a separate conditional stage. If local embedding or retrieval
proves infeasible within reasonable effort, discuss the concrete work then use
the authorized hosted embedding or hosted query fallback respectively. A full
local RAG port would also have to replace its current process/WAL worker and
recovery arrangements. Native/hosted RAG retains corpus writes while the
browser route proves read-only queries and evaluates persistence. Assess
browser-local ingestion, embedding, review and publication after those stages;
its implementation needs a separate decision. Editable graph and map
applications may need databases too, but each keeps its own schema and
transaction owner behind effects.

Small-LLM answer generation targets both native/hosted and browser-local
providers behind one application-service contract, separately from query
embeddings. If local generation proves infeasible within reasonable effort,
discuss the concrete implementation work before using the approved
native/hosted-only fallback. WebGPU can serve compute as well as graphics;
browser-local embedding and generation engines may use it, but their backends
remain unselected. The cREXX feature retains request, citation and result
policy. The existing native `llama` CPU/Metal/CUDA/Vulkan provider is not
itself a browser WebGPU backend. Keep model preparation and inference off the
UI owner turn, with explicit device/memory admission, cancellation, loss and
provider fallback. A WebGPU map renderer, vector scorer and inference provider
are separate optional capabilities. Canvas 2D map drawing and exact CPU
`rxvector` scoring are the initial paths; each GPU path needs its own
representative correctness, latency, resource and device/fallback proof. Local
model backends are chosen during implementation; their browser-local target
does not itself require WebGPU. The [UI expansion plan](PLAN.md) records these
gates. The read-only OPFS cache needs a proved SQLite VFS and bundle-validation
path: OPFS is origin-scoped storage subject to browser quota and clearing.

For 2D work, add a semantic scene beside the current flat widget view only
after defining its public contract. The candidate scene contains stable IDs,
bounded geometry, layers and z-order, world coordinates, a viewport transform,
style and accessible labels. Application features retain graph/map objects,
selection, edit transactions, undo/redo and strategy rules. The approved design
places world geometry, picking and scene revisions in one reusable Level G owner;
drivers translate physical pointer, keyboard and touch input into logical
viewport units and paint the returned scene. The ANSI renderer keeps its own
cell/control hit areas and uses stable IDs for semantic graph/map actions. A
stale event must not mutate a newer scene. Existing `ui.pointer`, viewport and
gesture names are vocabulary only; they do not yet establish scene hit testing
or rendering behavior. The scene contract must define fixed-point scale and
CSS/device/world conversion before pointer events can drive graph edits or map
selection.

World geometry will use bounded fixed-point integer coordinates. The shared
scene contract must state the scale, range, rounding and overflow checks; map
tile identities remain separate. Browser numeric transport must preserve every
accepted coordinate exactly, while hosts convert world positions to physical
pixels for painting. The [plan](PLAN.md) keeps these API details open.

The selected scene update direction starts with a full versioned snapshot and
then sends bounded deltas against the last applied revision. The host
validates and applies each delta atomically. On a revision gap, conflicting
duplicate, invalid change or route restart, it keeps the last valid frame,
pauses scene interaction and requests a full snapshot from the Level G owner.
The replacement is atomic; repeated failure ends in a typed error. Message
shapes, bounds and recovery proof remain open in the [plan](PLAN.md).

The same Level G graph or map feature should drive all three delivery hosts.
GTK and browser may offer 2D projections; ANSI can expose a labeled node/edge
list or bounded cell map with keyboard selection and actions. Each operation
needs precise declared host support. The application chooses its named semantic
fallback and command availability. The approved direction checks a complete
view or scene before projection and returns an unexpected unsupported request
to the application as a typed refusal, leaving the last valid frame intact.
The user then receives a specific reason or a usable fallback. Current v0
effect capability failures already reach the feature, but an unsupported view
node currently fails the host render path; application-visible view refusal
remains design work. The first extension also keeps capabilities fixed during
each session. A host can recover internally while preserving its advertised
operations; otherwise it reports host loss and a new session must be composed
from application-owned saved state. The [plan](PLAN.md) gates the exact
recovery API and requires equivalent domain transitions across hosts, not
identical pixels.

The selected native 2D projection is a GTK 3 drawing area with Cairo for both
graph and map. The browser uses SVG with ordinary HTML controls for editable
UML-like graphs, retaining stable element identities and text/focus overlays.
It uses Canvas 2D for a bounded strategy map; culling and layering come before
an accelerated renderer. ANSI supplies semantic node/edge and bounded cell-map
views, with explicit refusal for unavailable gestures. WebGL2 remains an option
for measured large-map redraw pressure, while WebGPU rendering and compute need
separate gates. No general scene renderer is implemented yet.

HTML controls and a semantic graph/list alternative remain necessary for
keyboard and assistive access to graphics. Text/terminal hosts may present a
functional list or viewport rather than a pixel-equivalent drawing.

Peter's `DEFINE-GUI` remains an authoring layer over these logical contracts.
Its current generated controller and `node|value` store are experimental: a
generated setter can change display text separately from the feature model,
and the macro currently contains Text Inspector-specific dispatch. First prove
new core capabilities with handwritten Level G consumers. A later RXPP increment
must specify how dynamic labels are computed from the feature's current
`view()` on every render, and how static routing forwards exactly one semantic
event through the session. Generated scene declarations may describe layers,
styles and action IDs, but cannot own a graph editor's or game's model. The
macro must diagnose unsupported declarations instead of silently ignoring
them. The example graph currently references two absent source files, so its
macro run/probe targets need repair and focused replay before use as evidence.

Browser API and scheduling constraints are described in the
[WebAssembly concepts](https://developer.mozilla.org/en-US/docs/WebAssembly/Guides/Concepts)
and [Emscripten runtime guide](https://emscripten.org/docs/porting/emscripten-runtime-environment.html).
The rendering options use the browser's
[SVG DOM](https://developer.mozilla.org/en-US/docs/Web/SVG/Guides/Scripting),
[Canvas 2D](https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API/Tutorial/Basic_usage),
[WebGL](https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API) and
[WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) capabilities;
their support and cost must be checked against the selected targets.
Browser data and compute options draw on
[SQLite's OPFS guidance](https://www.sqlite.org/wasm/doc/trunk/persistence.md),
[browser storage limits](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria),
and [WebGPU Worker/compute support](https://developer.mozilla.org/en-US/docs/Web/API/WorkerNavigator/gpu).

For native packaging, GTK supplies a stable RXPA provider ID and the ordinary
dynamic/static pair. Its V2 capability query remains conservative (zero); it
does not promise GTK reentrancy or per-VM window state. The provider package
declares concrete GTK link files and hashes its adjacent DLL closure. On Windows
with an MSYS2 SDK, MSVC must use its own runtime headers and GTK DLL import
archives; a Visual Studio developer environment supplies the SDK include order.

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
[`PLAN.md`](PLAN.md) for the later graph/map and browser work. The focused
`ui_scene_contract_noopt` test checks
preflight, atomic changes, stale input, overflow and full resync. The session
copies its broad capability set on construction; a caller cannot mutate a
running session by changing that original builder.

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
