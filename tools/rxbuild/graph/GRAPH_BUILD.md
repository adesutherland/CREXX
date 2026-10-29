# Graph build reference

Graph plans are the canonical input for `rxbuild`. Existing `.build` plans
continue to use the separately named `rxbuild-legacy` compatibility command
until they are migrated.

The cREXX toolchain also supports graph plans directly. When `crexx.exe` is
given a `.graph.build` file, it dispatches the request to the canonical
`rxbuild` executable and forwards the optional target and other runner
arguments. This makes graph plans usable as normal cREXX toolchain inputs.

## Run a graph build

From the repository root:

```powershell
.\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\example.graph.build
```

Use `--verbose` (or `-v`) to inspect resolved paths and action state:

```powershell
.\cmake-build-release\bin\rxbuild.exe --verbose `
  examples\php\setup.graph.build run-php-string
```

Verbose output reports the action, target, input, output, dependencies,
working directory, arguments, and whether the target is being built or is
already up to date. The option may appear before or after the graph path.

An explicit target may be supplied as the second argument:

```powershell
.\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\example.graph.build .crexx-build/graph/grammar.rxbin
```

The same graph plan can be invoked through `crexx.exe`:

```powershell
.\cmake-build-release\bin\crexx.exe tools\rxbuild\graph\example.graph.build
.\cmake-build-release\bin\crexx.exe examples\php\setup.graph.build run-php-string
```

For legacy compatibility, a file ending in `.build` is dispatched by
`crexx.exe` to `rxbuild-legacy` instead:

```powershell
.\cmake-build-release\bin\crexx.exe examples\php\setup.build
```

## Supported syntax

Assignments define string variables. Assignments require a semicolon:

```text
BUILD_DIR = ".crexx-build/graph";
SOURCE = "tools/rxbuild/build_demo.crexx";
```

Variables can be referenced inside quoted values with `${NAME}`.

A rule has a typed action and optional description:

```text
RULE compile_program
  ACTION = "COMPILE PROGRAM"
  DESCRIPTION = "PROGRAM $out"
END RULE;
```

A build declaration names one output target and gives it a rule, input, and
optional target dependencies:

```text
BUILD "${BUILD_DIR}/reader.rxbin"
  RULE = compile_program
  INPUT = "${SOURCE}"
  DEPENDS_ON = "${BUILD_DIR}/grammar.rxbin"
END BUILD;
```

The default target is selected when no target argument is supplied:

```text
DEFAULT "${BUILD_DIR}/reader.rxbin";
```

Builds may also have a logical name. Logical dependencies refer to
`BUILD_NAME` values, while `DEPENDS_ON` continues to refer to physical paths:

```text
BUILD "${BUILD_DIR}/grammar.rxbin"
  BUILD_NAME = grammar
  RULE = compile_program
  INPUT = "${SOURCE}"
END BUILD;

BUILD "${BUILD_DIR}/reader.rxbin"
  BUILD_NAME = reader
  RULE = compile_program
  INPUT = "${SOURCE}"
  DEPENDS_ON_BUILD = grammar
END BUILD;

DEFAULT_BUILD = reader;
```

Logical names are simple identifiers and must be unique. The command line
accepts either a logical name or a physical output path:

```powershell
.\cmake-build-release\bin\rxbuild.exe tools\rxbuild\graph\example.graph.build reader
```

`DEPENDS_ON` and `DEPENDS_ON_BUILD` may both occur in one declaration. They
are resolved in separate namespaces, and duplicate dependencies are rejected
after resolution. A graph must not define both `DEFAULT` and `DEFAULT_BUILD`.

Native packaging and execution are separate typed actions:

```text
RULE create_exe
  ACTION = "CREATE EXE"
END RULE;

RULE run_program
  ACTION = "RUN"
END RULE;

BUILD "${BUILD_DIR}/demo"
  RULE = create_exe
  INPUT = "tools/rxbuild/build_demo.crexx"
END BUILD;

BUILD "run-demo"
  RULE = run_program
  INPUT = "${BUILD_DIR}/demo.rxbin"
  DEPENDS_ON = "${BUILD_DIR}/demo.rxbin"
  WORK_DIR = "."
  ARGUMENT = "-a" "tools/rxbuild/graph/example.graph.build"
  STDIN = ""
END BUILD;
```

`CREATE EXE` stages the source before invoking `crexx --native`, so the
original source directory is not modified. `RUN` is a logical target: its
input must be an existing RXBIN, and it executes whenever selected. `RUN`
also accepts optional `WORK_DIR`, two-value `ARGUMENT`, and `STDIN`
attributes, matching the current PHP `setup.build` run parameters.

## Current behavior

- Dependencies are visited before their dependent target.
- A target is rebuilt when its output is missing, an input is newer, or a
  dependency was rebuilt during the same invocation.
- Existing targets are reported as `up to date`.
- The reader reports duplicate targets, unknown rules/targets, dependency
  cycles, missing defaults, duplicate logical names, unknown logical
  dependencies, ambiguous defaults, and variable errors.
- Supported rule actions are `COMPILE PROGRAM`, `COMPILE GRAMMAR`,
  `COMPILE READER`, `COMPILE APPLICATION`, `CREATE EXE`, and `RUN`.
- Each action requires exactly one `INPUT`. The compile actions produce the
  declared `.rxbin` output through the existing cREXX toolchain. `COMPILE
  READER` requires one grammar dependency. `COMPILE APPLICATION` requires
  grammar then reader dependencies; additional dependencies are built first
  and make their outputs available on the import path.
- `CREATE EXE` takes one CREXX source input and creates the declared native
  executable target. The source is copied to a private staging directory
  before invoking `crexx --native`.
- `RUN` takes one RXBIN input and executes it with `rxtvm`. Its target name is
  a logical graph name and does not need to be a file; `RUN` executes whenever
  the target is selected. Optional `WORK_DIR`, `ARGUMENT = "name" "value"`,
  and `STDIN` attributes are passed to the VM.
- The graph file is parsed by the separate `graph.levell` grammar compiled
  into the packaged graph reader.
- `BUILD_NAME` is an optional logical alias for a physical output target.
  `DEPENDS_ON_BUILD` and `DEFAULT_BUILD` use those aliases; physical output
  paths remain the artifact and timestamp identities.

## Not supported yet

The graph language does not support generic command templates, multiple outputs,
dynamic dependencies, parallel graph execution, or arbitrary compiler
dependency selection. The PHP graph sample builds and runs its four
application RXBINs when the default target is selected. Individual
`run-php-functions`, `run-php-string`, `run-php-numeric`, and
`run-php-migration` targets can also be selected.

## Build and test

Build the separate command with the repository's configured toolchain:

```powershell
cmake --build cmake-build-release --target rxbuild --parallel 10
```

Run the focused graph test and the unchanged legacy `rxbuild` tests:

```powershell
ctest --test-dir cmake-build-release --parallel 1 --output-on-failure -R "^(rxbuild_graph|rxbuild_)"
```

The focused test is implemented by
[`tests/check-graph-build.py`](tests/check-graph-build.py). It is only a CTest
harness: it creates temporary graph files, invokes canonical `rxbuild`, checks
diagnostics and incremental behavior, and removes its temporary outputs. The
graph implementation itself remains `graph.levell` plus
`graph_reader.crexx`.
