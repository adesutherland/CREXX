# Process designer example

This independently authored Level G example shows a linear process with a
graph and an editable form for the selected step. It starts with Request,
Review and Approve. Select a box, edit Name, Owner or Notes, add a step after
the selection, remove or reorder a step, and use Undo/Redo. The example keeps
up to four steps in memory; it does not execute workflows or save them.

The same `process_feature` owns the records, selection, edge order and undo
history on every route. GTK and browser hosts draw rectangles and links from
the shared scene protocol. ANSI shows numbered step buttons and the same
editable form. On browser routes, a field commits when it loses focus. In ANSI,
Tab focuses fields, typing edits the value and Enter commits it; 1–4 select a
step and Ctrl-C requests close. GTK uses native entry widgets. The browser
currently uses a single-line text input for all three fields, including Notes.

Build the example in a configured tree with GTK enabled:

```sh
cmake --build cmake-build-ui --target example_ui_process_artifacts rxvm --parallel 6
```

For the browser, [build and serve the site](../../../web/ui/README.md) and open
`http://127.0.0.1:8787/?app=process` for local Wasm, or add `&hosted=1` for the
authenticated native route. `web/ui/test-process.mjs` checks both routes.

Run ANSI in a real terminal from the repository root:

```sh
cmake-build-ui/bin/rxvm \
  cmake-build-ui/examples/ui/process-designer/process_ansi \
  cmake-build-ui/examples/ui/process-designer/process_designer \
  cmake-build-ui/bin/ui_ansi cmake-build-ui/bin/ui_dialogs \
  cmake-build-ui/bin/ui_terminal_view cmake-build-ui/bin/ui_local_resources \
  cmake-build-ui/bin/ui_scene cmake-build-ui/bin/ui_contract \
  cmake-build-ui/bin/ui_catalog cmake-build-ui/bin/ui cmake-build-ui/bin/library
```

Run GTK with the same application image and its GTK launcher:

```sh
cmake-build-ui/bin/rxvm \
  cmake-build-ui/examples/ui/process-designer/process_gtk \
  cmake-build-ui/bin/rx_ui_gtk_native \
  cmake-build-ui/examples/ui/process-designer/process_designer \
  cmake-build-ui/bin/ui_gtk cmake-build-ui/bin/ui_scene \
  cmake-build-ui/bin/ui_local_resources cmake-build-ui/bin/ui_contract \
  cmake-build-ui/bin/ui_catalog cmake-build-ui/bin/ui cmake-build-ui/bin/library
```

The [UI plan](../../../lib/ui/PLAN.md) records this showcase as STEP07A/AC16.
The four-step cap and linear edges belong to this application, not the scene
or host APIs. General branching, persistence and package distribution remain
outside this example.
