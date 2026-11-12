# cREXX Graph Builds and Ninja

## Purpose

The current `rxbuild` graph implementation is a second build model developed alongside the existing legacy build process.

The legacy `.build` format remains available through `rxbuild-legacy`. Graph plans use the separate `.graph.build` suffix and are processed by the canonical `rxbuild` command.

The new implementation is inspired by Ninja, but it is not intended to be a complete Ninja clone. Its purpose is to provide a small, cREXX-oriented dependency-based build system that can be defined with a Level L grammar and executed by a cREXX reader.

## The central idea

The original build process describes a sequence of operations:

```text
compile grammar
compile reader
compile application
run program
```

A graph build describes relationships between targets:

```text
grammar  -> reader  -> application
    \__________________________ runtime support
```

Each target has a rule, inputs, outputs and dependencies. The build engine visits the dependency graph and executes only the operations that are necessary.

This is the essential idea behind Ninja. Ninja describes the dependency graph and the commands used to produce its outputs. Its design deliberately keeps the build language small and leaves more complicated decisions to a generator or higher-level tool.

Reference: [The Ninja build system manual](https://ninja-build.org/manual.html)

## Example graph plan

```text
BUILD_DIR = ".crexx-build/php";

GRAMMAR_SOURCE = "examples/php/grammar/php.levell";
READER_SOURCE = "examples/php/php_reader.crexx";
APPLICATION_SOURCE = "examples/php/migration.php";

RULE compile_grammar
  ACTION = "COMPILE GRAMMAR"
  DESCRIPTION = "GRAMMAR $out"
END RULE;

RULE compile_reader
  ACTION = "COMPILE READER"
  DESCRIPTION = "READER $out"
END RULE;

RULE compile_application
  ACTION = "COMPILE APPLICATION"
  DESCRIPTION = "APPLICATION $out"
END RULE;

BUILD "${BUILD_DIR}/phpgrammar.rxbin"
  BUILD_NAME = grammar
  RULE = compile_grammar
  INPUT = "${GRAMMAR_SOURCE}"
END BUILD;

BUILD "${BUILD_DIR}/php_reader.rxbin"
  BUILD_NAME = reader
  RULE = compile_reader
  INPUT = "${READER_SOURCE}"
  DEPENDS_ON_BUILD = grammar
END BUILD;

BUILD "${BUILD_DIR}/migration.rxbin"
  BUILD_NAME = migration
  RULE = compile_application
  INPUT = "${APPLICATION_SOURCE}"
  DEPENDS_ON_BUILD = grammar
  DEPENDS_ON_BUILD = reader
END BUILD;

DEFAULT_BUILD = migration;
```

The logical names `grammar`, `reader` and `migration` make the dependency graph readable. The physical output files remain important internally for timestamp checks and for passing generated artefacts to the existing cREXX toolchain.

## What the current implementation already provides

The current graph reader and runner support the essential dependency-based build model:

- string variable assignments;
- variable substitution inside quoted values;
- typed build rules;
- build targets with inputs and outputs;
- physical dependencies through `DEPENDS_ON`;
- logical dependencies through `DEPENDS_ON_BUILD`;
- optional logical target names through `BUILD_NAME`;
- default targets;
- explicit target selection from the command line;
- recursive dependency traversal;
- incremental timestamp-based rebuilding;
- reporting of targets that are already up to date;
- duplicate-target detection;
- duplicate logical-name detection;
- unknown-rule and unknown-target diagnostics;
- unknown logical-dependency diagnostics;
- dependency-cycle detection;
- recursive include-cycle detection;
- graph composition through `INCLUDE_GRAPH`;
- explicit include namespaces through `INCLUDE_GRAPH ... AS ...`;
- local variable and logical-name scopes for included graphs;
- source-aware resolved graph inspection through `--dump-graph`;
- freshness explanations through `--explain`;
- machine-readable resolved graph output through `--dump-graph=json`;
- read-only target and dependency queries through `--list-targets`,
  `--show-dependencies`, `--reverse-dependencies`, and `--show-graph`;
- secondary outputs through repeatable `OUTPUT` attributes;
- content-based incremental state through SHA-256 target fingerprints;
- interrupted-action recovery through in-progress markers;
- explicit cross-platform toolchain selection through `--toolchain-dir`;
- redacted toolchain/environment diagnostics through graph dumps;
- graph-owned semantic diagnostics and include-stack reporting;
- graph-level validation and dependency traversal through `--dry-run` (some
  action-specific execution checks remain deferred);
- duplicate physical and logical dependency diagnostics before execution;
- variable and default-target validation;
- source discovery through `FILES IN ... RECURSIVE EXTENSION ...`;
- build groups that expand manifests into ordinary compile targets;
- bounded parallel scheduling of independent dependency waves through
  `--jobs N` or root `JOBS = N` settings;
- forced rebuild policy through root `FORCE = "true"`;
- typed actions for the existing cREXX toolchain.

The supported actions are currently:

```text
COMPILE PROGRAM
COMPILE RXPP
COMPILE GRAMMAR
COMPILE READER
COMPILE APPLICATION
CREATE EXE
RUN
```

The PHP graph example is already a substantial end-to-end test. It exercises grammar generation, reader compilation, runtime-module compilation, application compilation and execution.

## Physical and logical dependencies

The two dependency forms have deliberately different meanings.

### Physical dependency

```text
DEPENDS_ON = ".crexx-build/php/phpgrammar.rxbin"
```

This refers to a file or physical output path.

### Logical build dependency

```text
DEPENDS_ON_BUILD = grammar
```

This refers to a `BUILD_NAME` declared in the current graph or in one of its
included graphs. An included graph alias may qualify the name, such as
`php.reader`, to select a target whose local name is `reader`.

Both forms may occur in one build declaration. This is useful when a target depends on both a generated build target and an external file:

```text
DEPENDS_ON_BUILD = grammar
DEPENDS_ON = "config/version.txt"
```

Duplicate dependencies should be rejected after resolution, even when the same dependency is written once physically and once logically.

## Relationship with Ninja

The current implementation already captures Ninja's central model:

| Concept | Current `rxbuild` implementation |
|---|---|
| Rule | Typed `RULE` with an `ACTION` |
| Build edge | `BUILD` declaration |
| Input | `INPUT` |
| Output | Physical target name and output path |
| Dependency | `DEPENDS_ON` or `DEPENDS_ON_BUILD` |
| Default target | `DEFAULT` or `DEFAULT_BUILD` |
| Incremental build | Timestamp, dependency and SHA-256 state checks |
| Target selection | Command-line target argument |
| Execution | Existing cREXX build actions with bounded parallel waves |

The main difference is that Ninja uses generic command templates, whereas the current implementation uses typed cREXX actions such as `COMPILE GRAMMAR` and `COMPILE READER`.

This is a deliberate design choice. Typed actions provide:

- better control over platform-specific execution;
- simpler argument handling;
- fewer shell-quoting problems;
- clearer diagnostics;
- direct reuse of the existing cREXX toolchain;
- a smaller and safer first implementation.

The current system is therefore more specialised than Ninja, but also more closely integrated with cREXX.

## Features not yet supported

The current implementation does not yet support all features found in Ninja. In particular, it does not currently provide:

- generic command templates;
- dynamic dependencies;
- compiler-generated dependency files;
- resource pools beyond the worker limit;
- command-line change tracking;
- a persistent build log file;
- clean and clean-dead operations;
- graphical dependency visualisation beyond the textual resolved graph dump;
- arbitrary compiler dependency selection.

These omissions are intentional. They do not prevent the current implementation from demonstrating the main dependency-graph model.

## Are the missing features essential?

Not all of them are equally important.

### Important future additions

The most useful future additions would probably be:

1. clean and rebuild operations;
2. graphical dependency visualisation;
3. generic command templates, if users need custom actions.

### Features that can wait

Resource pools, compiler dependency files and persistent command logs are useful
for larger projects, but they are not required to validate the current design.

The graph should remain correct and reliable when executed sequentially.
Parallel execution is already available for independent dependency waves; more
advanced resource locking and cancellation remain future work.

## Graph composition

Separate graph files can cooperate through `INCLUDE_GRAPH`. The include path is
resolved relative to the containing graph and nested includes are supported:

```text
INCLUDE_GRAPH "grammar.graph.build"

BUILD "${BUILD_DIR}/reader.rxbin"
  BUILD_NAME = reader
  RULE = compile_reader
  INPUT = "examples/php/php_reader.crexx"
  DEPENDS_ON_BUILD = grammar
END BUILD;
```

Included graphs contribute their rules and targets to the merged graph. Their
variables and logical names remain local to the graph that defines them.
Unqualified logical references resolve locally first, then globally only when
the name is unique; ambiguous names are rejected. Physical output paths remain
global and must not collide. Include cycles and duplicate loads are detected.
Only the root graph supplies the effective `DEFAULT` or `DEFAULT_BUILD`.

Composition combines graph descriptions into one dependency graph. It does
not coordinate independent `rxbuild` processes running at the same time; that
would require separate locking and process-coordination rules.

## Current conclusion

The current graph implementation is sufficient as a first Ninja-inspired build engine. It already provides the most important concepts:

```text
rules
targets
inputs
outputs
dependencies
incremental rebuilding
```

It is not a complete Ninja replacement, and it does not need to become one. Its main purpose is to provide a cREXX-native build description that is more declarative and dependency-aware than the original fixed sequence of build commands.

The existing legacy build process should remain unchanged. The graph implementation should continue to evolve as a separate build model until its syntax and behaviour have been evaluated through real examples such as PHP, MiniBASIC and the cREXX build itself.

Future work includes resource locks, cancellation, persistent command logs and
broader repeatability diagnostics. Bounded parallel scheduling is implemented
only for independent waves whose transitive dependency closures do not overlap.
