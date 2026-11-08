# Graph and map UI consumers

scene_editors.crexx contains two independent handwritten Level G features.
The graph owns its nodes, edges, selection and undo history. The map owns its
terrain, unit, turn and selection state. Both publish scene revisions through
uisceneowner, accept normalized pointer and keyboard events, and choose a
semantic fallback when a host has no scene operations. The shared UI has no
graph or game rules.

| Route | Graph | Map |
| --- | --- | --- |
| GTK | Cairo drawing area with commands, pointer and keyboard | Cairo terrain and unit scene with turn and viewport controls |
| ANSI | Labeled node/edge list and commands | Four by four cell viewport, detail and commands |
| Browser local Wasm | SVG scene and HTML commands | Canvas 2D and HTML commands |
| Browser hosted native | The same SVG projection | The same Canvas projection |

The direct transition test compares graph and map models across graphical and
semantic feature instances. GTK tests inject native drawing-area signals;
ANSI tests use a real foreground PTY. The browser check exercises both routes
in Chrome. Build the native artifacts and run the focused checks with:

~~~sh
cmake --build cmake-build-ui --target example_ui_scene_tests example_ui_scene_editor_artifacts --parallel 6
ctest --test-dir cmake-build-ui -R '^ui_scene_(editors|editors_gtk|editors_ansi_pty)' --output-on-failure
~~~

The earlier scene_examples.crexx fixture remains a small contract example,
including a GTK application-visible refusal and full resync probe. See the
[UI plan](../../../lib/ui/PLAN.md) for acceptance criteria and later gates.

`cmake --build cmake-build-ui --target stage-product` prepares the selected
native install. `cmake --install cmake-build-ui --prefix <destination>` places
`scene_editors.rxbin`, both web launchers, the graph ANSI fixture and the
graph/map ANSI and GTK launchers in `examples/ui/scene/` alongside the shared
UI modules and host providers under `bin/`. Browser packaging is described in
the [browser route guide](../../../web/ui/README.md).
