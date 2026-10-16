# rxbuild user guide

rxbuild reads a .build file and executes its declared build phases directly.
It is the build tool for Level L grammars and their compatible cREXX readers.
The .build file is a runtime input; it is not compiled into an .rxpp file.

## Quick start

Run these commands from the CREXX repository root:

~~~powershell
cmake --build cmake-build-release --target rxbuild --parallel 1

.\cmake-build-release\bin\rxbuild.exe tools\build\example.build
~~~

The recursive selection sample is:

~~~powershell
.\cmake-build-release\bin\rxbuild.exe tools\build\selection.build
~~~

It selects build.levell, nested/deep.levell, and tiny.levell, compiles them,
compiles the reader, and runs the demo program.

For the smallest plan, use simple.build. It has literal paths only, with no
variables and no DO OVER loop:

~~~powershell
.\cmake-build-release\bin\rxbuild.exe tools\build\simple.build
~~~

The MiniBASIC example uses the same reader process:

~~~powershell
.\cmake-build-release\bin\rxbuild.exe examples\minibasic\BUILD\build-minibasic.build
~~~

## Build phases

A plan normally contains these operations:

~~~text
COMPILE GRAMMAR
COMPILE READER
COMPILE PROGRAM or COMPILE APPLICATION
CREATE EXE
RUN
~~~

COMPILE GRAMMAR invokes the cREXX compiler for a .levell grammar and publishes
the generated grammar module under OUTPUT_DIR.

COMPILE READER compiles a .crexx reader and loads the grammar named by
DEPENDS_ON.

COMPILE PROGRAM compiles an ordinary cREXX program into a standalone .rxbin.

COMPILE APPLICATION invokes a declared source-language compiler, compiles its
generated cREXX source, and emits one standalone application .rxbin.

CREATE EXE packages a preceding COMPILE APPLICATION as a native executable
using the existing cREXX native driver. APPLICATION names the application
output from the preceding phase; OUTPUT is the executable stem. The .rxbin
remains available as an independent bytecode deliverable.

RUN invokes the generated .rxbin program. ARGUMENT supplies command-line
arguments and STDIN supplies one input line when needed.

### Source-language applications

`COMPILE APPLICATION` expects a compiler `.rxbin` built by a preceding
`COMPILE READER` phase. The compiler receives two application arguments after
the VM's `-a` marker: the source path and the generated cREXX output path.

```text
COMPILE APPLICATION
  OUTPUT_DIR = "cmake-build-release/minibasic/application"
  SOURCE = "examples/minibasic/basic1.bas"
  DEPENDS_ON = "minibasicgrammar"
  COMPILER = "minibasic_compiler"
  OUTPUT = "basic1";
```

The generated cREXX source is an intermediate artifact under the build tree's
`.rxbuild-generated` directory. The published application is the final
`basic1.rxbin`; grammar and compiler modules are not linked into that runtime
image. See `examples/minibasic/BUILD/build-minibasic-application.build`.

To create a native executable for the prime application:

```text
CREATE EXE
  OUTPUT_DIR = "cmake-build-release/minibasic/primes"
  APPLICATION = "prime"
  OUTPUT = "prime";
```

On Windows this produces
`cmake-build-release/minibasic/primes/prime.exe`; Linux and macOS produce the
extensionless native executable `prime` alongside `prime.rxbin`. The native
compiler and linker configuration come from the CREXX build; native packaging
is platform-specific even though the `.rxbin` is portable.

## A complete plan

~~~text
BUILD_DIR = "cmake-build-release/build";
GRAMMAR_SOURCE = "tools/build/grammar/build.levell";
READER_SOURCE = "tools/build/build_reader.crexx";

COMPILE GRAMMAR
  OUTPUT_DIR = "${BUILD_DIR}"
  SOURCE = "${GRAMMAR_SOURCE}"
  OUTPUT = "rxbuildgrammar";

COMPILE READER
  OUTPUT_DIR = "${BUILD_DIR}"
  SOURCE = "${READER_SOURCE}"
  DEPENDS_ON = "rxbuildgrammar"
  OUTPUT = "build_reader";

RUN
  WORK_DIR = "${BUILD_DIR}"
  PROGRAM = "build_demo"
  ARGUMENT = "-a" "tools/build/example.build"
  STDIN = "";
~~~

Statements end with a semicolon. Attribute order is not significant, but the
required attributes must be present for the relevant operation.

## Variables

A plan can assign string variables:

~~~text
OUT_DIR = "cmake-build-release/build";
GRAMMAR = "tools/build/grammar/build.levell";
~~~

Use ${NAME} inside quoted attribute values:

~~~text
SOURCE = "${GRAMMAR}"
OUTPUT_DIR = "${OUT_DIR}"
~~~

Assignments are evaluated in order. Undefined variables and malformed
substitutions stop the plan before any compiler or runtime tool is started.

## File selection and iteration

FILES selects regular files from a directory. BASENAMES removes the selected
extension from each result:

~~~text
GRAMMAR_DIR = "tools/build/grammar";
OUT_DIR = "cmake-build-release/build/selected";
GRAMMARS = FILES IN "${GRAMMAR_DIR}" EXTENSION ".levell" BASENAMES;

DO OVER GRAMMARS AS item
  COMPILE GRAMMAR
    OUTPUT_DIR = "${OUT_DIR}"
    SOURCE = "${GRAMMAR_DIR}/${item}.levell"
    OUTPUT = "${item}";
END;
~~~

The result for build.levell is build. Results are sorted deterministically.

Add RECURSIVE to include subdirectories:

~~~text
GRAMMARS = FILES IN "${GRAMMAR_DIR}" RECURSIVE
           EXTENSION ".levell" BASENAMES;
~~~

Recursive results preserve their path relative to GRAMMAR_DIR. For example:

~~~text
build.levell        -> build
nested/deep.levell  -> nested/deep
~~~

When item contains a subdirectory, generated output directories mirror that
relative path. The reader creates those directories as needed.

Selections fail clearly when the directory is missing, cannot be listed, or
contains no matching files.

## Path resolution

Absolute paths are used unchanged. Relative paths are resolved against the
CREXX project root discovered by rxbuild. In a development checkout, the
project root is found by walking upward from the current directory until a
directory containing both bin and examples is found.

For reliable invocation, start rxbuild from the repository root:

~~~powershell
cd C:\Users\PeterJ\CLionProjects\CREXX\Beta2.1
.\cmake-build-release\bin\rxbuild.exe tools\build\selection.build
~~~

The current implementation does not use CREXX_HOME. Installed deployments
should use absolute paths or start from a project directory whose relative
paths are meaningful. The rxbuild executable locates its compiler and runtime
tools from the installed tool path.

## Output and diagnostics

rxbuild prints the original parsed operation, followed by an RXBUILD RESOLVED
line containing the absolute paths and artifact names used for execution.
This makes variable substitution and recursive path expansion visible.

Generated grammar, reader, and intermediate files belong under the CMake build
tree. Source directories should contain the .levell, .crexx, and .build files,
not generated .rxbin output.

Common diagnostics include:

- RXBUILD_PARSE for malformed .build syntax.
- RXBUILD_IO for an unreadable plan.
- RXBUILD selection error for invalid or empty file selections.
- RXBUILD substitution error for undefined or malformed variables.
- BUILD GRAMMAR, BUILD READER, CREATE EXE, and EXECUTE lines for phase
  execution.

## Current boundaries

- The build file is read directly; no .rxpp wrapper is involved.
- File selection is non-recursive unless RECURSIVE is specified.
- Selection returns regular files only and ignores other filesystem entries.
- Reader/program pairing is explicit; rxbuild does not discover readers
  automatically.
- Application compilers are explicit; rxbuild does not infer a language
  compiler from a grammar or source extension.
- Relative source paths should be written for the selected project root.

See [tools/build/README.md](../tools/build/README.md) and the runnable
plans in [tools/build](../tools/build/).
