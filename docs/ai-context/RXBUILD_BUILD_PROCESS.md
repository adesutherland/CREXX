# rxbuild process

`rxbuild` is the direct build-plan command for cREXX language examples and
other small Level-L-hosted tools. Its input is a `.build` file. The `.build`
file is read at runtime and is not preprocessed or wrapped as `.rxpp`.

## Responsibilities

The process has two separate parts:

1. CMake packages the `rxbuild` command with its Level L grammar and Level B
   reader.
2. The command reads a `.build` plan and performs its declared phases:
   grammar compilation, reader/RXPP/program compilation, application compilation,
   native executable packaging, and program execution.

The grammar recognizes structure. The reader interprets the parse tree and
invokes the cREXX tools directly. There is no reusable RXPP build runner in
this path.

## Packaging flow

Building the `rxbuild` CMake target produces the native command under the
configured build tree's `bin` directory:

```text
build.levell
    -> rxlang
    -> rxbuildgrammar.rxas
    -> rxbuildgrammar.rxbin

build_reader.crexx + rxbuildgrammar.rxbin
    -> rxc
    -> rxas
    -> rxbuild.rxbin

rxbuild.rxbin + rxbuildgrammar.rxbin + runtime modules
    -> rxlink
    -> rxcpack
    -> bin/rxbuild[.exe]
```

The packaged command contains the grammar, reader, Level L runtime modules,
and the native filesystem/platform providers needed by the reader. A `.build`
file remains an external runtime input.

Both ordinary Level B compilation rules (reader and demo) depend on the
`library` and `compiler_exit_bin` producers and on their `library.rxbin` and
`rxcexits.rxbin` files. These are required even for the small demo: its normal
compiler invocation loads the base library and certified exits. Target edges
provide clean-build ordering; file edges rebuild consumers when imports change.
The `rxbuild_example` target must therefore build from a clean tree on its own,
without depending on another product target to happen to prepare its imports.

The main `crexx` driver also recognizes a single `.build` input and delegates
it to the sibling `rxbuild` executable. This is an editor/terminal convenience
only; `rxbuild` remains the implementation of the build-plan language and the
normal `.crexx`, `.rxpp`, and `.levell` dispatch paths are unchanged.

Build the command with:

```powershell
cmake --build cmake-build-release --target rxbuild --parallel 1
```

Use the appropriate configured build directory and parallelism for other
platforms.

## Runtime flow

For `rxbuild tools/rxbuild/example.build`, the reader:

1. reads the file as binary input;
2. creates an `lllanguage` request from `rxbuildgrammar`;
3. parses the complete plan before starting any build action;
4. reports `RXBUILD_PARSE` with source location information on failure;
5. walks the named parse tree and collects the plan actions;
6. invokes `crexx --library` for `COMPILE GRAMMAR`;
7. invokes `rxc` and `rxas` for `COMPILE READER`, with the output directory
   on the compiler import path so the declared grammar module is available;
8. invokes `rxpp` for `COMPILE RXPP`, writing the generated `.crexx` source
   under its declared output directory;
9. invokes `rxc` and `rxas` for `COMPILE PROGRAM`, with that output directory
   on the import path;
10. invokes the declared compiler with `rxtvm`, then direct `rxc` and `rxas`,
   for `COMPILE APPLICATION`;
11. invokes `crexx --native` on the generated application source for
    `CREATE EXE`, then publishes the requested platform-native executable
    (`.exe` on Windows, extensionless on Linux/macOS);
12. invokes `rxtvm` for `RUN` with `WORK_DIR` as its working location; and
13. returns the first failing phase status.

Relative paths in a plan are resolved from the repository root discovered by
the reader. The command tools are resolved beside `rxbuild`, so the process
uses the same product installation that supplied the command.

## Plan shape

The initial grammar supports one or more semicolon-terminated statements:

```text
COMPILE GRAMMAR OUTPUT_DIR = "..." SOURCE = "..." OUTPUT = "...";
COMPILE READER OUTPUT_DIR = "..." SOURCE = "..." DEPENDS_ON = "..." OUTPUT = "...";
COMPILE RXPP OUTPUT_DIR = "..." SOURCE = "..." OUTPUT = "...";
COMPILE PROGRAM OUTPUT_DIR = "..." SOURCE = "..." OUTPUT = "...";
COMPILE APPLICATION OUTPUT_DIR = "..." SOURCE = "..." DEPENDS_ON = "..." COMPILER = "..." OUTPUT = "...";
CREATE EXE OUTPUT_DIR = "..." APPLICATION = "..." OUTPUT = "...";
RUN WORK_DIR = "..." PROGRAM = "..." ARGUMENT = "..." "..." STDIN = "...";
```

`DEPENDS_ON` identifies the generated grammar module for the reader.
`ARGUMENT` has two quoted values: the command-line option and its value.
`STDIN` supplies one input line when the executed program needs input.

The current reader requires all three operations and executes them in source
order after successful parsing. Semantic checks such as duplicate attributes,
missing files, and dependency validity are handled by the reader or the
invoked tools, not by the Level L grammar alone.

## Variables

The current variable phase supports top-level string assignments and
case-insensitive `${NAME}` substitution in quoted attribute values:

```text
BUILD_DIR = "cmake-build-release/build";
GRAMMAR = "tools/rxbuild/grammar/build.levell";

COMPILE GRAMMAR OUTPUT_DIR = "${BUILD_DIR}"
  SOURCE = "${GRAMMAR}" OUTPUT = "rxbuildgrammar";
```

Assignments are evaluated in order and should precede the statements that use
them. Undefined variables and malformed `${...}` references are reported by
`rxbuild` before any compiler or runtime tool is started. File selection and
iteration are supported as a separate expansion layer:

```text
GRAMMAR_DIR = "tools/rxbuild/grammar";
OUT_DIR = "cmake-build-release/build/selected";
GRAMMARS = FILES IN "${GRAMMAR_DIR}" RECURSIVE EXTENSION ".levell" BASENAMES;

DO OVER GRAMMARS AS item
  COMPILE GRAMMAR
    OUTPUT_DIR = "${OUT_DIR}"
    SOURCE = "${GRAMMAR_DIR}/${item}.levell"
    OUTPUT = "${item}";
END;
```

Selection includes regular files only and sorts relative names deterministically.
Without `RECURSIVE`, only direct children are considered. With `RECURSIVE`,
subdirectory names are preserved relative to the selected directory.
`BASENAMES` removes the selected extension. The loop body is expanded once
per selected item, after variable substitution, and nested output directories
are created as needed. Missing directories and empty selections stop the plan
before tools are invoked. The complete example is
[`tools/rxbuild/selection.build`](../../tools/rxbuild/selection.build).

For each expanded action, `rxbuild` also prints an `RXBUILD RESOLVED` line
showing the absolute output, source, dependency, or program paths that will be
used. The original statement remains visible so variable-based input can be
compared with the resolved action.

## Diagnostics and artifacts

Parser failures use the `RXBUILD_PARSE` prefix and include the input path,
line, column, byte offset, and parser diagnostic. Tool failures are reported
with the phase label (`grammar`, `reader`, or `execute`) and the captured tool
output.

Generated grammar, reader, linked bytecode, and native packaging intermediates
belong in the CMake build tree. They should not be committed beside the
`.levell`, `.crexx`, or `.build` sources.

## Example

The complete working example is in
[`tools/rxbuild`](../../tools/rxbuild):

```powershell
& .\cmake-build-release\bin\rxbuild.exe `
  tools\rxbuild\example.build
```

The same pattern is used by the MiniBASIC plan at
[`examples/minibasic/BUILD/build-minibasic.build`](../../examples/minibasic/BUILD/build-minibasic.build).

The build example packages `build_demo.crexx` as a separate bytecode program;
its `RUN` phase proves that the plan does not recursively invoke the `rxbuild`
reader itself.

`COMPILE APPLICATION` is the source-language boundary. `COMPILER` names a
previously compiled `.rxbin` compiler, which receives the source path and a
generated cREXX output path after `-a`. rxbuild then compiles that generated
source with `rxc` and assembles it with `rxas`. The published application is a
single `<OUTPUT>.rxbin`; grammar and compiler modules are build-time inputs and
are not linked into that application image. `CREATE EXE` must name a preceding
application and uses its generated source with the existing cREXX native
packaging path. It publishes `<OUTPUT>.exe` on Windows or extensionless
`<OUTPUT>` on Linux/macOS without replacing the `.rxbin`.
The MiniBASIC example is
[`build-minibasic-application.build`](../../examples/minibasic/BUILD/build-minibasic-application.build).
