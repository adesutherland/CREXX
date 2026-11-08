# Direct scene consumers

`scene_examples.crexx` contains two small Level G features. The graph feature
owns node positions and selection and builds scene revisions through
`uisceneowner`. When a host advertises `form.scene`, `scene.rect` and
`scene.edge`, its view has a scene node. Otherwise it builds a labeled
node/edge list and the same stable commands. The map fixture owns its turn and
terrain/unit identity; full map interaction and graphical maps are STEP05.

The GTK driver renders accepted scene items through a drawing area and Cairo.
The ANSI launcher uses the same graph session factory, selecting the semantic
view from ANSI's operation profile. Use these focused checks after building
`example_ui_scene_tests` and `example_ui_scene_ansi`:

```sh
ctest --test-dir cmake-build-ui -R '^ui_scene_(examples_noopt|gtk_noopt|ansi_pty)$' --output-on-failure
```

The GTK test exercises actual native setup, three driver lifetimes, accepted
deltas, one application-visible unsupported-operation fallback, full resync
after a gap, and teardown. The ANSI PTY test exercises keyboard commands,
semantic labels and terminal restoration. Browser projections, graph gesture
editing/undo and map rendering are subsequent steps in
[`lib/ui/PLAN.md`](../../../lib/ui/PLAN.md).
