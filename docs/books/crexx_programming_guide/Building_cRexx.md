# Building the Toolchain

Most users will download and install a binary distribution for
their platforms and will not need this information. In some cases
when a binary distribution is unavailable, it will be beneficial to be
able to build the cRexx system using a standard C toolchain.

This chapter aims to show all you need to know about how to build
cRexx from scratch, and then run it. It will show what you need, what to
do and when it is build, how to make working programs with it.

## Requirements

Currently cRexx builds on Windows, macOS and
Linux[^linux].

[^linux]: There is a separate instruction for VM/370 (and later)
  mainframe operating systems.
  
You need one of those and:

|Tool   |Function   |   |   |   |
|---|---|---|---|---|
|git   | Source code versioning  |   |   |   |
|CMake   |Build Tool   |   |   |   |
|gcc, clang or MSVC[^clang]   |C compiler   |   |   |   |
|Make   |conventional build tool, or   |   |   |   |
|Ninja   |fast build tool   |   |   |   |

Table: Required tools. {#tbl:id}

[^clang]: clang is a working equivalent and the default on macOS.

## Platform specific info

On Linux and macOS, this instruction is identical. For macOS, Xcode
batch tools need to be installed, which will provide you with git, make
and the compiler. Brew will give easy access to Cmake and Ninja-build[^regina].

[^regina]: For Linux, you will need to install git (which will be there on most distributions), cmake and gcc or clang. For installing the openssl development headers if needed: sudo apt update; sudo apt install libssl-dev pkg-config}

On Windows, MSYS2 or WSL remain useful when a GNU-compatible environment is
preferred. A native x64 build is also supported with the Visual Studio 2022 C
toolchain. Run CMake from an x64 Visual Studio Developer Command Prompt (or
after calling `VsDevCmd.bat`) so `cl.exe`, the Windows SDK and the linker are
available:

```bash
cmake -S CREXX -B crexx-build-msvc -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release -DENABLE_PARSER_MODE=OFF
cmake --build crexx-build-msvc --parallel 8
```

MSVC builds the portable switch-dispatch VM and copies `rxbvm.exe` to the
stable `rxvm.exe` product path. The optional parser-mode/syntax-highlighting
integration currently depends on POSIX DSLSH sources and is therefore disabled
in the native MSVC command above. This does not disable normal `rxc` compiler
operation. Run `crexx -native` from the same Developer Command Prompt; the
driver uses the compiler family and runtime-library mode recorded by CMake.

## Process

Here it is assumed that all tools are installed and working, and
available on your PATH environment variable.

Choose or make a suitably named directory on your system to contain the
source code. Note that the cRexx source is kept in a different directory
on you system than where it is built in, or will run from. Now run this
command:

```bash
git clone https://github.com/adesutherland/CREXX.git
```

This will give you a CREXX subdirectory in the current directory,
containing the source of cRexx and its dependencies. This is the
`develop' branch, which is the one you would normally want to use. All
The sources use C90 or C99 where sufficient. The concurrency-enabled
interpreter requires C11 atomics, which are supported by the documented GCC,
Clang and Visual Studio 2022 toolchains.

Make a new subdirectory in the current directory (not in CREXX, but in
the one that contains it), like `crexx-build'.

```bash
mkdir crexx-build
```

and cd into that directory. Now issue the following command (we assume
that you installed ninja, otherwise substitute `make' for the two
<!-- instances of `ninja'): -->

```bash <!--buildcommand.sh-->
cmake -S ../CREXX -B crexx-build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build crexx-build --parallel 5
cmake --build crexx-build --target qa-comprehensive
```

This will do a lot of things. In fact, if all goes well, you will have a
built and tested cRexx system.

## Explaining the build process

Let's zoom in a little on what we did. The first step is to tell CMake
to validate your system environment and generate a build script (and a
test script) for the chosen build tool. Cmake will read the file
CmakeLists.txt and validate that your system can do what it asks it to
do. This can yield error messages, for example if the C compiler lacks
certain functions or header files. (When that happens, open an
\href{https://github.com/adesutherland/CREXX/issues}{issue} and someone
will have a look at it - or peruse
\href{https://stackoverflow.com}{stack overflow} which is what we
probably also will do).

After CMake has successfully validated the build environment, it will
generate a build script (a Makefile in the case of Make and a
`build.ninja` file in the case of Ninja). This is specified after the -G
flag. The -DCMAKE\_BUILD\_TYPE=Release flag makes sure we do an
optimized build, which means we specify an -O3 flag to the C compiler,
which then will spend some time optimizing the executable modules, which
makes them run faster (they do!). The alternative is a `debug' build
which will yield slower executables, but with more debugging information
in them.

The two ampersands (\&\&) mean we do the next part only if the previous
step was successful. This is a `ninja` statement, which will build
everything in the `build.ninja` specification file. These are a lot of
parts, and the good news is, when they are built once, only the changed
source will be built, which will be fast.

Ninja builds also use named resource pools for work whose memory pressure is
not represented by the global `--parallel` value. Configure with
`-DCREXX_BUILD_RESOURCE_PROFILE=developer-fast` on a capable development
machine, `portable` on slower or unknown hosts, or `memory-constrained` when
RAM is tight. `auto` is the default: it selects `developer-fast` on Apple ARM64
and `portable` elsewhere. These profiles currently limit concurrent VM-core C
compiles and native links while leaving independent graph work free to use the
global job count. The limits can be overridden explicitly with
`CREXX_VM_COMPILE_POOL_DEPTH` and `CREXX_NATIVE_LINK_POOL_DEPTH`; both must be
positive integers.

For example, Adrian's macOS development profile still permits 30 runnable
graph actions while limiting the high-memory VM compile family separately:

```sh
cmake -S . -B cmake-build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCREXX_BUILD_RESOURCE_PROFILE=developer-fast
cmake --build cmake-build-debug --parallel 30
```

On an unknown or smaller host, select `portable` and use five global jobs.
Non-Ninja generators retain their own scheduling because CMake job pools are a
Ninja facility.

Production self-builds do not use the compiler's normal broad discovery
defaults. CMake copies each action's declared RXBIN and RXPA metadata providers
to a private import root and invokes `rxc --no-exe-import`. Where a source
directory contains sibling modules or test fixtures, the primary source is
also copied to an action-private source root. This is necessary because `rxc`
normally considers sibling source before RXAS/RXBIN metadata and also appends
its executable directory to the binary roots. Those defaults remain useful for
interactive compilation, but would allow an older linked library or an
unrelated test plugin to affect a clean or incremental self-build.

The build declarations, rather than filesystem timestamps, therefore select
the metadata route. When adding a production `rxc` action, name its exact
provider files in `DEPENDS`, stage only those files with
`crexx_add_import_root`, isolate the source when it has siblings, and keep the
resolution report as an opt-in diagnostic rather than a routine side effect.

After the product build, select a named QA target such as `qa-smoke` or
`qa-comprehensive`. Each target first builds the generated artifacts for its
own test tier and then starts CTest. CTest executes tests only; it does not
start another build.
This knows what to do, as the tests were defined in the Cmake recipes, and will show you successes and failures. If what
you checked out if git is not a released version, there is a 
change that some test cases fail, but generally these should indicate
success.

The default product build is deliberately offline. Parser mode uses a local
sibling `DSL-Syntax-Highlighter` checkout when it exists; otherwise parser
mode defaults off. To permit CMake to fetch the pinned parser dependency,
configure with `-DCREXX_ALLOW_NETWORK_DOWNLOADS=ON`. An explicit
`-DENABLE_PARSER_MODE=ON` without either a local checkout or that network
permission is a configuration error. The native SQLite ADDRESS demo is also
off by default and requires both `-DCREXX_BUILD_SQLITE_ADDRESS_DEMO=ON` and
`-DCREXX_ALLOW_NETWORK_DOWNLOADS=ON`.

Standalone examples and demonstrations are not members of the default product
build. Request the documented auxiliary groups explicitly:

```sh
cmake --build crexx-build --target crexx-examples --parallel 5
cmake --build crexx-build --target crexx-demos --parallel 5
```

Comprehensive QA still prepares the example and demonstration artifacts that
its tests consume through `qa-prep-comprehensive`; some source examples are
therefore also visible as QA fixture inputs. Measurement preparation is
separate and does not execute a workload. Contributions and experiments are not currently
configured as product targets. New ones should remain explicit opt-ins rather
than joining the default product implicitly.

## QA tiers and useful system test subsets

Every configured test has exactly one execution tier while retaining its
topical labels. The named targets make the intended barriers visible:

| Target | Purpose |
| --- | --- |
| `qa-essential` | Smallest correctness blockers |
| `qa-smoke` | Essential plus quick representative coverage |
| `qa-comprehensive` | Normal correctness sweep, excluding stress and measurement |
| `qa-qualification` | Install, packaging, reproducibility and external-consumer proof |
| `qa-stress` | Explicit high-load and race-oriented workloads |
| `qa-measurement` | Performance measurement only, serially on a quiescent host |

Each target depends on its matching `qa-prep-*` closure. Comprehensive
preparation includes smoke and essential; qualification and stress remain
independent. The compatibility `qa-prep` target combines all non-measurement
closures for older scripts. `qa-measurement` uses `qa-prep-measurement` and
always selects one CTest worker. Tests carrying the topical `performance`
label default to this serial measurement tier unless they explicitly declare
qualification or stress. Do not combine performance measurement with a busy
correctness or stress worker pool; timings from an active host are only
indicative.

The named correctness targets use 30 CTest workers by default on Apple ARM64
and five elsewhere. Override that independently of build parallelism with
`-DCREXX_QA_CTEST_JOBS=<positive-number>`.

For routine system validation, run the product build and normal correctness
suite:

```sh
cmake --build cmake-build-debug --parallel 5
cmake --build cmake-build-debug --target qa-comprehensive --parallel 5
```

For a direct CTest invocation, prepare the same tier first and use its exact
labels:

```sh
cmake --build cmake-build-debug --target qa-prep-comprehensive --parallel 5
ctest --test-dir cmake-build-debug \
  --label-regex '^(essential|smoke|comprehensive)$' \
  --output-on-failure --parallel 5
```

For standard-library and BIF work, useful focused checks are:

```sh
cmake --build cmake-build-debug --target testbifs
ctest --test-dir cmake-build-debug -R '^ts_.*_(noopt|opt)$' --output-on-failure
```

If a BIF source change under `lib/rxfnsb/rexx/` causes compiler RXAS
golden tests to fail but the corresponding runtime tests still pass, rebuild
the linked standard-library image before judging the golden diff:

```sh
cmake --build cmake-build-debug --target library
cmake --build cmake-build-debug --target testbifs
```

Then rerun the focused compiler/runtime tests and review the generated RXAS:

```sh
ctest --test-dir cmake-build-debug/compiler/tests -R '13_stems' --output-on-failure
ctest --test-dir cmake-build-debug -R '^ts_stem_(noopt|opt)$' --output-on-failure
git diff -- compiler/tests/golden
```

Consumer RXAS import blocks are a snapshot of the callables needed for
linking/runtime lookup, not a copy of the full provider API. If the diff only
adds or removes unused imported declarations, update goldens only after
confirming that the consumer still imports the callables it actually calls.
The maintainer testing details are in `compiler/docs/testing.md`.

The `lib/rxfnsb/rexx` BIF build is a bootstrap build. It compiles most BIF
source files with compiler exits disabled (`rxc -x`), so explicit certified
exits such as `TRACE`, `PARSE`, and `ADDRESS` are not available inside those
library source files during the BIF build. To debug a BIF, call it from a
normal test fixture or scratch program compiled with exits enabled. Add
`TRACE UNSUPPRESS NAMESPACE rxfnsb` when you need library frames in the trace.

The native `system` plugin has its own smoke test:

```sh
ctest --test-dir cmake-build-debug -R '^test_system$' --output-on-failure
```

For TRACE/debug metadata work, combine focused TRACE and linker checks before
the full suite:

```sh
ctest --test-dir cmake-build-debug \
  -R '^(trace_event_metadata|test_trace_|ts_trace_|rxlink_format_check|rxlink_rxdas_strip_smoke)' \
  --output-on-failure
```

## Build version and timestamp

The project version is read from the top-level `VERSION` file. To change
the cREXX version number, edit that file and use a semantic version such
as `1.0.0`, `1.0.0-beta.3`, or `1.0.0-beta.3+build.7`.

Local, non-release builds also include build metadata in the displayed
version: the build channel, the short Git commit id when Git is
available, and a `dirty` marker when the working tree has local changes.
The commit id is the part of the local version string that identifies
the source revision. After a `git pull`, the commit id is refreshed the
next time CMake configures that build directory. If CMake does not
reconfigure automatically, run the same `cmake -S ../CREXX -B .`
configure command again before rebuilding.

`CREXX_BUILD_TIMESTAMP` is recorded in `BUILDINFO` for package
provenance, but it is not part of the displayed tool version. When it is
not supplied, CMake creates a UTC timestamp during the first configure
of a build directory and stores it in `CMakeCache.txt`. Release and CI
builds should pass an explicit timestamp:

```sh
cmake -DCREXX_BUILD_TIMESTAMP=20260527T120000Z -S ../CREXX -B .
```

## Use of cRexx to build cRexx

cRexx is used to build the library of built-in functions that are written in Rexx (and,
for a very small part in Rexx Assembler) and need to be compiled (carefully observing the
dependencies on other Rexx built-in functions) before they are added to the
library and the cRexx executables in their binary form.


## What do we have after a successful build

### Native executables

When all went well, we have a set of native executables for the platform
we built cRexx on. These are

| Name    | Function                                        |
|---------|-------------------------------------------------|
| cRexx   | cRexx compiler driver                           |
| rxc     | cRexx compiler                                  |
| rxas    | cRexx assembler                                 |
| rxlink  | cRexx linker                                    |
| rxdas   | cRexx disassembler                              |
| rxvm    | cRexx VM, compiler-selected product entry point |
| rxpp    | cRexx macro preprocessor                        |
| rxbvm   | cRexx VM, portable switch-dispatch interpreter  |
| rxtvm   | cRexx VM, direct-threaded interpreter           |
| rxvme   | compiler-selected VM with linked-in Rexx library |
| rxdb    | cRexx debugger                                  |
| rxcpack | cRexx C-generator for native executables        |

Table: Delivered products. {#tbl:id}

Clang and AppleClang make `rxvm` select `rxbvm`; GCC makes it select `rxtvm`.
MSVC builds only the switch engine and supplies `rxvm.exe` as a copy of
`rxbvm.exe`. The `rxtvm` executable is therefore not present in an MSVC build.

cRexx can compile the Rexx script into an
executable file, that can be run standalone, for example, on a computer
that has no cRexx and/or C compiler installed.

The cRexx debugging tool `rxdb`, is written in Rexx and compiled and packaged into an
executable file.

The executables in the above table need to be on the `PATH` environment
variable. These are to be found in the compiler, assembler,
disassembler, debugger and cpacker directories of the cRexx-build
directory we created earlier. It is up to you to add all these
separately to the `PATH` environment, or to just collect them all into one
directory that is already on the `PATH`.

### Production and debug builds

Production builds are optimized, while debug builds are slower in execution time but deliver support for the analysis and debugging of problems in the code. The standard distribution is an optimized production build, while a debug build can be produced with the Debug CMake build option.

| Name    | Function                                        |
|---------|-------------------------------------------------|
| -DCMAKE_BUILD_TYPE=Release  | An Optimized build (default)|
| -DCMAKE_BUILD_TYPE=Debug    | A Debug build              |

Table: Debug vs Release options. {#tbl:id}

### Libraries

We also have a set of libraries. Some are written in cRexx and others can be written in C and other programming languages.
All executables and libraries are delivered in the `bin` directory of the distribution package.

### Optional libraries - build options

| Name    | Function                                        |
|---------|-------------------------------------------------|
| -DENABLE_ODBC=ON  | Build the ODBC Plugin               |
| -DENABLE_GTK=ON  | Build the GTK (GUI) Plugin           |


Table: Optional plugin build  options. {#tbl:id}

Some libraries, with dependencies on installed software products, are only produced when they are opted-in with CMake build options. The defaults for these options are \code{OFF}.
