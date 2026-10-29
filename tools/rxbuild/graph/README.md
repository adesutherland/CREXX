# Canonical `rxbuild` graph plans

This directory contains the canonical Ninja-inspired build graph reader for
`rxbuild`. Existing `.build` plans remain supported by the separately named
`rxbuild-legacy` compatibility executable while they are migrated.

See [GRAPH_BUILD.md](GRAPH_BUILD.md) for the current syntax and supported
behavior.

The graph language accepts rules, build targets, physical and logical
dependency edges, string variables, and physical or logical default targets.
It supports the typed `COMPILE PROGRAM`,
`COMPILE GRAMMAR`, `COMPILE READER`, `COMPILE APPLICATION`, `CREATE EXE`, and
`RUN` actions. It resolves dependencies recursively, detects
cycles and missing declarations, and skips a target when its output is newer
than its inputs and dependencies.

Build and run it from the repository root:

```powershell
cmake --build cmake-build-release --target rxbuild --parallel 1
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\example.graph.build
```

Graph plans can also be run through the cREXX toolchain. `crexx.exe`
recognizes the `.graph.build` suffix and forwards the plan and any selected
target to the canonical `rxbuild` executable:

```powershell
& .\cmake-build-release\bin\crexx.exe examples\php\setup.graph.build run-php-string
```

For comparison, ordinary `.build` plans are routed to `rxbuild-legacy`:

```powershell
& .\cmake-build-release\bin\crexx.exe examples\php\setup.build
```

The example intentionally has two program targets so the dependency edge and
default-target selection are visible. Generic shell commands, parallel
execution, and multiple outputs are outside the current graph language.

Use `BUILD_NAME` to give a target a stable logical name, then refer to it with
`DEPENDS_ON_BUILD` or `DEFAULT_BUILD`. Existing `DEPENDS_ON` and `DEFAULT`
forms remain supported.

The PHP graph example is [`examples/php/setup.graph.build`](../../examples/php/setup.graph.build).
It builds the PHP grammar, reader, runtime module, and four application RXBINs,
then runs the applications through its default target.

Add `--verbose` to any invocation to print resolved source, target, dependency,
working-directory, and action-state information.

[`actions.graph.build`](actions.graph.build) demonstrates the native and
execution actions. Run it from the repository root with:

```powershell
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\actions.graph.build
```
