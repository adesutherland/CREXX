# Text Inspector: one feature, three hosts

Text Inspector is a **Level G** worked example. It counts physical text records,
whitespace-delimited words and Unicode scalar values excluding line terminators.

Read [the framework introduction](../../../lib/ui/README.md), then
[text_inspector.rxpp](text_inspector.rxpp). Its session factory registers schemas,
commands and one feature; `text_inspector_component` owns the model, update and
view. Follow into [the contract](../../../lib/ui/ui_contract.crexx), your chosen
driver and [resource executor](../../../lib/ui/ui_local_resources.crexx).
There is one application contract and no compatibility shell.

## What to trace

Open invokes `document.open`, which queues `document.open.requested`.
The feature returns `ui.resource.choose`; the driver later completes
`ui.resource.selected` with an opaque grant. The feature requests `io.text.read`;
the shared executor completes `io.text.read.completed`. The feature counts the
records and returns a fresh logical view. Failure/cancellation are
`ui.effect.failed`/`ui.effect.cancelled`, not magic filename values.

Clear demonstrates `ui.dialog.confirm` and its OK/Cancel outcomes. Quit queues
`ui.close.requested`; this read-only feature accepts by returning `ui.close`.
After native destruction/restoration, the driver completes `ui.closed` and
tears down the session. Tests wrap the same feature to prove close can be vetoed.

Framework/widget events are standard; `document.*` belongs to this application.
Commands and effects are not native callbacks. No feature opens files, stores
GTK handles, emits terminal escape sequences or knows its driver.

`##LOADMACRO ui` loads `UI_COMMAND`, `UI_NODE` and the single `UI_LAUNCHER`
macro. The three launcher files differ only in driver namespace/class.
RXPP emits one source per invocation and retains source maps to the authored
`.rxpp`. Stable view IDs and `root`/`below`/`right` layout are the builder
seam; this is not yet a rich or responsive widget system.

## Build and run

All commands below run from the repository root.

```sh
cmake --build cmake-build-debug --target example_text_inspector_artifacts rxvm --parallel 10
```

### Line TUI

```sh
cmake-build-debug/bin/rxvm \
  cmake-build-debug/examples/ui/text-inspector/text_inspector_tui \
  cmake-build-debug/examples/ui/text-inspector/text_inspector \
  cmake-build-debug/bin/ui_tui \
  cmake-build-debug/bin/ui_local_resources \
  cmake-build-debug/bin/ui_contract \
  cmake-build-debug/bin/ui_catalog \
  cmake-build-debug/bin/ui \
  cmake-build-debug/bin/library
```

Enter `o`, then a file path; an empty path cancels. Enter `c`, then `ok` or
`cancel`; `q` closes. This host blocks on line input and is also useful for
deterministic scripted tests. It does not require GTK or raw-console support.

### ANSI full-screen terminal

```sh
sh examples/ui/text-inspector/run-ansi.sh cmake-build-debug
```

Run in a real terminal. `o` opens the selector: type/paste a path, or browse with
arrows, PageUp/PageDown, Tab and the mouse. Enter opens; Escape cancels.
`c` asks for Clear confirmation; `q` closes. Ctrl-C cancels an active dialog
or requests close. Resize preserves state; below 32x10 a resize prompt replaces
ordinary controls. No ncurses or GTK library is needed.

See [terminal lifecycle and limits](../../../lib/ui/TERMINAL.md), then trace
`ui_ansi` -> `ui_dialogs` / `ui_terminal_view` / `ui_local_resources` and
the Level B C console provider.

### GTK

Use a tree configured with `-DENABLE_GTK=ON` and GTK 3 discoverable through
`pkg-config`. On this checkout the existing GTK tree is `cmake-build-ui-v0-gtk`.

```sh
cmake --build cmake-build-ui-v0-gtk --target example_text_inspector_artifacts rxvm --parallel 10
cmake-build-ui-v0-gtk/bin/rxvm \
  cmake-build-ui-v0-gtk/examples/ui/text-inspector/text_inspector_gtk \
  cmake-build-ui-v0-gtk/bin/rx_ui_gtk_native \
  cmake-build-ui-v0-gtk/examples/ui/text-inspector/text_inspector \
  cmake-build-ui-v0-gtk/bin/ui_gtk \
  cmake-build-ui-v0-gtk/bin/ui_local_resources \
  cmake-build-ui-v0-gtk/bin/ui_contract \
  cmake-build-ui-v0-gtk/bin/ui_catalog \
  cmake-build-ui-v0-gtk/bin/ui \
  cmake-build-ui-v0-gtk/bin/library
```

Click Open, select a file, then Open/Cancel. Clear uses an explicit OK/Cancel
dialog. Quit and the window close control go through the same vetoable intent.
GTK uses its native loop and non-blocking dialog responses. Its idle mailbox
enters cREXX through synchronous RXPA; callbacks never outlive the native run.

The same GTK launch can be performed through the graph builder after the GTK
CMake artifacts exist:

```powershell
.\cmake-build-release\bin\rxbuild.exe `
  examples\ui\text-inspector\text_inspector_gtk.graph.build `
  --verbose
```

The graph plan compiles the generated GTK launcher into its own
`.crexx-build/ui-graph/text_inspector_gtk.rxbin` output and uses
`RUNTIME_ARGUMENT` for the GTK provider and UI modules. CMake remains
responsible for RXPP generation, the shared UI modules, and the native GTK
plugin.

### Scripted GUI handler

`text_inspector_gui_macro.rxpp` demonstrates the generic scripted-handler form:

```rexx
##DALIAS DEFINE-GUI HANDLER UI_GUI
DEFINE-GUI TextInspector
  LABEL path "File: (none)"
END DEFINE-GUI
```

`HANDLER UI_GUI` registers `DEFINE-GUI` as one alias and dispatches the
completed block to the separately loaded `UI_GUI` scripted macro. The
preprocessor does not contain GUI-specific tag logic; other DSLs can register
their own handler macro in the same way. The handler currently emits the
`TextInspector..render(view, bindings)` and
`TextInspector..register(session)` facade procedures; widget interpretation is
owned by the UI layer and remains an experimental extension point.

The `.rxpp` source is also part of `text_inspector_gtk.graph.build`. CMake
first prepares the RXPP macro library through
`example_text_inspector_artifacts`; the graph builder then preprocesses the
`.rxpp` source and compiles its generated `.crexx` output into the separate
`text_inspector_gui_macro.rxbin` target.

The graph declares an experimental `run-text-inspector-gtk-macro` target. Its
intended path loads the generated `DEFINE-GUI` view through a separate launcher
while delegating event updates to the existing Text Inspector component. The
normal `run-text-inspector-gtk` target remains the reference application. The
graph currently refers to absent authored
`text_inspector_gui_macro_gtk.rxpp` and
`text_inspector_gui_macro_test.crexx` inputs, so its macro run/probe targets
cannot be treated as runnable checkout evidence. After restoring those inputs,
checking dependencies and qualifying the path, the intended GTK invocation is:

```powershell
.\cmake-build-release\bin\rxbuild.exe `
  examples\ui\text-inspector\text_inspector_gtk.graph.build `
  run-text-inspector-gtk-macro --verbose
```

This Phase 1 target demonstrates generated layout and generic command routing;
Phase 2 additionally applies \`widget.value.changed\` payloads to generated
bindings. The payload contract is the existing \`uimessage.body()\` API:
\`body.text("node")\` identifies the control and \`body.text("value")\` carries
the new text. No backend-specific event or payload API was added.

The binding records use the experimental internal form \`node|value\`. The
generated renderer substitutes a matching value when rebuilding the view.
This remains an experimental contract; it does not yet generate application
update logic for every control type or derive calculated labels automatically.

The intended focused Phase 2 regression runs without opening a GTK window once
the missing graph input and its dependencies are repaired:

```powershell
.\\cmake-build-release\\bin\\rxbuild.exe \`
  examples\\ui\\text-inspector\\text_inspector_gtk.graph.build \`
  run-text-inspector-gtk-macro-test \`
  --verbose
```

It checks initial binding rendering, `widget.value.changed` replacement,
metadata generation, and preservation across unrelated events.

Phase 3 adds experimental generic event declarations:

```text
ON ACTIVATE control
ON CHECKED control
ON SELECTION control
```

The generated namespace exposes `event_node`, `event_value`,
`event_checked`, and `event_selection`. These read the existing
`uimessage` payload fields; they do not add driver-specific callbacks or
change the UI contract.

Phase 4 adds `event_interested(event)`. It matches a generic `widget.*`
event against the control ID and event declaration from the RXPP block. The
helper is an application-side filter; it does not register or suppress the
reserved `widget.*` catalogue events. `session.register_event()` remains for
application-owned events such as `document.open.requested`.

Phase 5 accepts callback labels on event declarations:

```text
ON COMMIT filter CALL document.filter
ON ACTIVATE refresh CALL document.refresh
```

The generated `event_callback(event)` returns the declared symbolic label for
a matching event, or an empty string. The application remains responsible for
dispatching that label to an ordinary cREXX procedure; the RXPP layer does not
perform dynamic procedure calls.

The generated namespace also exposes `make_controller(delegate)`. It returns
an ordinary `.uicomponent` that owns the generated view and binding store while
forwarding application events and effects to the supplied delegate. The
generated `binding_read(bindings, id)` and `binding_write(bindings, id, value)`
helpers provide the experimental `node|value` store operations used by the
controller. They do not synchronize an application model or add a new UI
contract. The sample host is now emitted into the same precompiled
`text_inspector_gui_macro.crexx` output through named RXPP sections. The GTK
launcher uses its generated `make_text_inspector_session()` entry point; the
former separate `text_inspector_gui_macro_app.crexx` adapter is no longer part
of the graph build.

The generated `control_metadata()` table describes each control as the
experimental record `id|kind|label|binding|action|events`. The final field
contains the declared generic event policies separated by semicolons. This is
controller metadata, not a public serialization format.

Work Order 3 adds the first generated-controller metadata and binding-store
surface. The controller is deliberately returned through the existing
backend-neutral `.uicomponent` interface, so applications continue to use the
normal `view()` and `update()` methods. The descriptor table and binding
helpers are experimental implementation aids and do not invoke callback labels
through dynamic procedure lookup.

Work Order 4 adds the generated flat dispatch table. `dispatch_metadata()`
returns experimental records in the form `node|event|handler`, combining
button `ACTION` declarations with `ON ... CALL` declarations. The generated
`dispatch_event(event)` performs the corresponding lookup, and
`event_callback(event)` delegates to it. It returns the symbolic handler name;
the application still decides how to invoke that ordinary cREXX procedure.
No backend-specific event registration or dynamic procedure execution is
introduced.

Work Order 5 connects the generated controller to the existing generic event
path. Its `update(event)` method consumes the current `uimessage` payload
through the existing `body().text(...)` accessors, updates the generated
binding store for `widget.value.changed`, and forwards the original event to
the delegate. The driver remains responsible for producing generic events;
GTK and TUI drivers require no changes for this generated-controller feature.

Work Order 6 adds generated ordinary cREXX handler plumbing. The flat
`handler_message(event)` procedure converts a matched callback such as
`document.refresh` into the existing application event
`document.refresh.requested`, preserving the current node/value payload when
present. `dispatch_handler(delegate, event)` sends that message to the
delegate, while unmatched generic events are forwarded unchanged. This is a
static generated mapping; it does not perform dynamic procedure lookup.

The generated host uses direct static calls. For each callback label, it has an
explicit branch such as:

```rexx
if callback = "document.filter" then
  return TextInspector..application_filter(callback_event, _delegate)
```

The called procedure receives `(event, delegate)`, can inspect the existing
payload APIs, print diagnostics, and either forward the event or return its own
effects. No `GUI-HANDLERS` block or dynamic procedure lookup is required.

Work Order 7 declares the `run-text-inspector-gui-probe` graph target. It is
intended as a backend-neutral regression for the generated controller: it would
load the ordinary UI contract modules and exercise descriptors, bindings,
generic payloads, dispatch, and handler-message generation. The declared
`run-text-inspector-gtk-macro` target is the intended GTK end-to-end path;
both need the missing sources and focused replay before their results can be
claimed. See the [UI expansion plan](../../../lib/ui/PLAN.md).

## QA and extension

```sh
cmake --build cmake-build-debug --target rxconsole_test_artifacts ui_functional_tests example_text_inspector_artifacts --parallel 10
ctest --test-dir cmake-build-debug -R '^rxconsole_|^ui_|^text_inspector_' --output-on-failure
```

Repeat with the GTK tree to include real GTK signal/dialog tests (they briefly
open windows and require an available display). Optimized tests link bytecode
before running it. The line and GTK tests compare a shared event journal;
ANSI tests exercise a real PTY, resize, input, dialogs and terminal restoration.
Session tests separately cover routing, correlation, stale outcomes and limits.
See [how to extend](../../../lib/ui/EXTENDING.md) for the feature/widget/effect/
driver checklist. Sanitizers remain the agreed later tracer-scope gate.
