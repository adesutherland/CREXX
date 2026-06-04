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
|Rexx   |(temporarily)   |   |   |   |
|gcc   |C compiler, install g++   |   |   |   |
|Make   |conventional build tool, or   |   |   |   |
|Ninja   |fast build tool   |   |   |   |

Table: Required tools. {#tbl:id}

## Platform specific info

On Linux and macOS, this instruction is identical. For macOS, Xcode
batch tools need to be installed, which will provide you with git, make
and the compiler. Brew will give easy access to regina[^regina] and Ninja-build.

[^regina]: ooRexx and brexx will also work, one of those needs to be on the path. For Linux, you will need to install git (which will be there on most distributions), cmake and gcc or clang.}

On Windows, you will need a compatibility layer like
\href{https://www.msys2.org}{msys} - installing this has the additional
advantage of easy access to git, gcc, cmake and the rest of the
necessary tools. On more modern Windows, the WSL\footnote{WSL: Windows
  subsystem for Linux.} and Ubuntu is not a bad choice.

## Process

Here it is assumed that all tools are installed and working, and
available on your PATH environment variable.

Choose or make a suitably named directory on your system to contain the
source code. Note that the cRexx source is kept in a different directory
on you system than where it is built in, or will run from. Now run this
command:

\begin{verbatim}
git clone https://github.com/adesutherland/CREXX.git
\end{verbatim}

This will give you a CREXX subdirectory in the current directory,
containing the source of cRexx and its dependencies. This is the
`develop' branch, which is the one you would normally want to use. All
of these are written in the C99 version of the C programming language,
which should be widely supported by C compilers on most platforms.

Make a new subdirectory in the current directory (not in CREXX, but in
the one that contains it), like `crexx-build'.

\begin{verbatim}
mkdir crexx-build
\end{verbatim}

and cd into that directory. Now issue the following command (we assume
that you installed ninja, otherwise substitute `make' for the two
<!-- instances of `ninja'): -->

```bash <!--buildcommand.sh-->
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../CREXX && ninja && ctest
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
build.ninja file in the case of Ninja). This is specified after the -G
flag. The -DCMAKE\_BUILD\_TYPE=Release flag makes sure we do an
optimized build, which means we specify an -O3 flag to the C compiler,
which then will spend some time optimizing the executable modules, which
makes them run faster (they do!). The alternative is a `debug' build
which will yield slower executables, but with more debugging information
in them.

The two ampersands (\&\&) mean we do the next part only if the previous
step was successful. This is a `ninja' statement, which will build
everything in the build.ninja specification file. These are a lot of
parts, and the good news is, when they are built once, only the changed
source will be built, which will be fast.

After this, the generated test suite is run with the `ctest' command.
This knows what to do, as the tests were defined in the Cmake recipes, and will show you successes and failures. If what
you checked out if git is not a released version, there is a 
change that some test cases fail, but generally these should indicate
success.

## Useful System Test Subsets

For routine system validation, run the full build and full test suite:

```sh
cmake --build cmake-build-debug
ctest --test-dir cmake-build-debug --output-on-failure
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
as `1.0.0`, `1.0.0-beta.1`, or `1.0.0-beta.1+build.7`.

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

\begin{verbatim}
cmake -DCREXX_BUILD_TIMESTAMP=20260527T120000Z -S ../CREXX -B .
\end{verbatim}

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
| rxvm    | cRexx VM, threaded interpreter                  |
| rxpp    | cRexx macro preprocessor                        |
| rxbvm   | cRexx VM, non-threaded conventional interpreter |
| rxvme   | cRexx VM, with linked-in Rexx library           |
| rxdb    | cRexx debugger                                  |
| rxcpack | cRexx C-generator for native executables        |

Table: Delivered products. {#tbl:id}

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
