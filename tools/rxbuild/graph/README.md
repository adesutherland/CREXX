# Canonical `rxbuild` graph plans

This directory contains the canonical Ninja-inspired build graph reader for
`rxbuild`. Existing `.build` plans remain supported by the separately named
`rxbuild-legacy` compatibility executable while they are migrated.

See [GRAPH_BUILD.md](GRAPH_BUILD.md) for the current syntax and supported
behavior.

The graph language accepts rules, build targets, physical and logical
dependency edges, string variables, and physical or logical default targets.
It supports the typed `COMPILE PROGRAM`,
`COMPILE RXPP`,
`COMPILE GRAMMAR`, `COMPILE READER`, `COMPILE APPLICATION`, `CREATE EXE`, and
`RUN` actions. It resolves dependencies recursively, detects
cycles and missing declarations, and skips a target when its output is newer
than its inputs and dependencies.
Dependency cycles and duplicate dependency references are rejected during
graph validation before any action is executed.

Included graphs can be given namespaces with `AS`, allowing identical local
build names to coexist:

```text
INCLUDE_GRAPH "php.graph.build" AS php
INCLUDE_GRAPH "fortran_iv.graph.build" AS fortran_iv
DEPENDS_ON_BUILD = php.reader
DEPENDS_ON_BUILD = fortran_iv.reader
```

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
default-target selection are visible. Generic shell commands and parallel
execution are outside the current graph language.

Root graphs may set execution defaults for the complete composed graph:

```text
VERBOSE = "true";
DRY_RUN = "true";
EXPLAIN = "true";
TOOLCHAIN_DIR = "cmake-build-release/bin";
```

Included graphs cannot override these root settings. Use `--no-dry-run` or
`--execute` when an invocation must execute despite a root dry-run default, or
`--no-explain` to suppress root explanation output.

Both block comments and trailing `--` line comments are supported:

```text
BUILD_NAME = copy_fixture; -- logical target name
```

Source-discovered build groups are available for self-contained program trees.
`FILES IN` creates a deterministic source manifest, and `BUILD_GROUP` expands
it into one ordinary `COMPILE PROGRAM` target per source plus a virtual
aggregate target:

```text
SOURCES = FILES IN "src"
          RECURSIVE
          EXTENSION ".crexx";

RULE compile_program
  ACTION = "COMPILE PROGRAM"
END RULE;

BUILD_GROUP "programs"
  RULE = compile_program
  SOURCES = SOURCES
  OUTPUT_DIR = "build"
END BUILD_GROUP;

DEFAULT_BUILD = programs;
```

Recursive members retain their source-relative directories under `OUTPUT_DIR`.
The group is a virtual aggregate, so selecting `programs` builds its members
but does not create a file named `programs` or remove outputs for deleted
sources. The sorted source manifest is part of incremental state, so additions
and removals invalidate consumers. The maintained example is
`tools/rxbuild/graph/groups/groups.graph.build`:

```powershell
& .\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\graph\groups\groups.graph.build `
  --toolchain-dir .\cmake-build-release\bin --list-targets
```

The same example also contains an `RXPP_SOURCES` file set and a
`preprocessed` group. It expands two `.rxpp` files into persistent `.crexx`
intermediates and then `.rxbin` outputs using the composite
`COMPILE RXPP PROGRAM` action:

```powershell
& .\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\graph\groups\groups.graph.build `
  --toolchain-dir .\cmake-build-release\bin preprocessed
```

The `programs` group declares `DEPENDS_ON_BUILD = preprocessed`, so selecting
`programs` orders preprocessing first while keeping the generated `.crexx`
files owned by the RXPP group.

For direct external tools, use the `COMMAND` action. This invokes the
executable without a shell; repeated `ARGUMENT` entries form the argument
vector. The maintained example uses CMake's portable `-E copy` helper:

```text
RULE run_tool
  ACTION = "COMMAND"
END RULE;

BUILD ".crexx-build/graph-command/output.dat"
  RULE = run_tool
  INPUT = "source/input.dat"
  WORK_DIR = "tools/rxbuild/graph/command"
  ARGUMENT = "cmake"
  ARGUMENT = "-E"
  ARGUMENT = "copy"
  ARGUMENT = "${input}"
  ARGUMENT = "${output}"
END BUILD;
```

For `COMMAND`, `${input}` resolves to the primary declared input and
`${output}` resolves to the primary `BUILD` output before the argv vector is
launched.

Use `BUILD_NAME` to give a target a stable logical name, then refer to it with
`DEPENDS_ON_BUILD` or `DEFAULT_BUILD`. Existing `DEPENDS_ON` and `DEFAULT`
forms remain supported.

The PHP graph example is [`examples/php/setup.graph.build`](../../examples/php/setup.graph.build).
It builds the PHP grammar, reader, runtime module, and four application RXBINs,
then runs the applications through its default target.

For a smaller composition and execution exercise, use the maintained complex
example:

```powershell
& .\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\graph\complex\complex.graph.build `
  --toolchain-dir .\cmake-build-release\bin `
  --verbose --explain
```

It includes separate `frontend` and `backend` graphs, resolves the qualified
dependencies `frontend.reader` and `backend.runtime`, builds the `package`
target, and executes the run-only `run_complex` target. Useful inspection
commands are:

The three compile targets intentionally reuse the small `build_demo.crexx`
source. Their names demonstrate graph roles and namespace resolution; they do
not claim to be separate frontend, backend, and package implementations.

`run_complex` is a phony `RUN` target. `INPUT` identifies the RXBIN passed to
the VM, while `DEPENDS_ON` ensures that the same package target is checked and
built before execution. `ARGUMENT` supplies the VM's one name/value option;
`RUNTIME_ARGUMENT` adds further arguments after the program path.

```powershell
& .\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\graph\complex\complex.graph.build `
  --list-targets
& .\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\graph\complex\complex.graph.build `
  --show-graph package
```

A target may declare additional outputs with repeatable `OUTPUT = "path"`
attributes. The `BUILD` path remains the primary target identity; all declared
outputs participate in freshness checks and must exist after the action:

```text
BUILD "out/program.rxbin"
  RULE = compile_program
  INPUT = "src/program.crexx"
  OUTPUT = "out/program.map"
END BUILD;
```

Add `--verbose` to any invocation to print resolved source, target, dependency,
working-directory, runtime-argument, and action-state information. `RUN`
supports repeatable `RUNTIME_ARGUMENT = "path"` attributes for applications
that need multiple runtime modules after the program RXBIN.

Use `--dry-run` (or `-n`) to perform graph-level checks and traverse a selected
dependency closure without executing actions or creating outputs. With
`--verbose`, stale targets are shown with `state=dry-run` and announced as
`would build`; run targets are announced as `would run`. Some action-specific
execution checks are deferred until the action would actually run.

Use `--explain` (or `-why`) to print why each visited target is rebuilt or
skipped, such as `output missing`, `input newer`, `dependency rebuilt`, or
`up to date`.

Successful non-`RUN` targets retain a sidecar `<primary-output>.rxbuild-state`
containing a SHA-256 fingerprint of their inputs, dependencies, outputs, and
action attributes. A missing or changed fingerprint causes a rebuild even when
file timestamps are unchanged.

While a target action is running, `rxbuild` creates an adjacent
`.rxbuild-state.in-progress` marker. If the process is interrupted, the next
invocation reports `interrupted previous action` and rebuilds the target before
publishing fresh state.

Graph-local semantic errors identify the graph file that owns the invalid rule,
target, include, or attribute. Parser errors additionally retain the existing
line/column diagnostic, and include-cycle errors print the include path stack.

Use `--dump-graph` to print the fully resolved graph after nested includes and
variable substitution. Each target includes its source graph and local logical
name; logical dependencies show the resolved physical output. The command then
prints the rules and defaults and exits without executing actions.

Use `--dump-graph=json` for machine-readable output with `rules`, `targets`,
`defaults`, and `default_builds` fields. This is suitable for IDE and CI
consumers without parsing the human-readable dump.

Use the read-only query options to inspect a validated graph without executing
actions:

```powershell
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\composed\main.graph.build --list-targets
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\composed\main.graph.build --show-dependencies parent
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\composed\main.graph.build --reverse-dependencies child
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\composed\main.graph.build --show-graph main_program
```

`--list-targets` prints each resolved output, logical name, and source graph.
The dependency queries print transitive physical and logical relationships and
accept the same target aliases as a normal build. `--show-graph` renders the
selected target and its prerequisites as an indented semi-graphic tree,
including source inputs.

Use `--toolchain-dir` to select the directory containing the graph tools
explicitly. The path is passed through the platform-neutral argument-array
runner, and is reported during verbose or dry-run builds:

```powershell
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\example.graph.build `
  --toolchain-dir .\cmake-build-release\bin --verbose
```

Use repeatable `--env NAME=VALUE` and `--unset-env NAME` options to control
the environment visible to graph-launched tools and `RUN` targets. Overrides
are applied by the cREXX process environment before each direct argv launch;
the prior worker environment is restored after the launch. They do not modify
the parent shell and do not require shell quoting.

Toolchain and environment settings are included in the target state fingerprint,
so changing them causes affected targets to rebuild. Environment values are
hashed in state files rather than written in clear text.

Graph dumps report the resolved toolchain directory and environment variable
names. They label overridden variables as `<set>` and never print their values,
so `--dump-graph` and `--dump-graph=json` are safe diagnostic views for CI logs.

[`actions.graph.build`](actions.graph.build) demonstrates the native and
execution actions. Run it from the repository root with:

```powershell
& .\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\actions.graph.build
```

Future planned work covers an allow-listed environment policy,
graph-declared toolchains and environment settings, tool/version validation,
and Windows/Linux/macOS qualification. Phase 10 will build on that contract
with bounded parallel scheduling, resource controls, cancellation, cleanup,
and repeatability tests.
