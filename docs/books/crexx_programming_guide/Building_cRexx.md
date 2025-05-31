# Building the Toolchain

Most users will download and install a binary distribution for
their platforms and will not need this information. In some cases
when a binary distribution is unavailable, it will be beneficial to be
able to build the \crexx{} system using a standard C toolchain.

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

<!-- \begin{verbatim} -->
<!-- cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ../CREXX && ninja && ctest -->
<!-- \end{verbatim} -->

<!-- This will do a lot of things. In fact, if all goes well, you will have a -->
<!-- built and tested cRexx system. -->

<!-- ## Explaining the build process -->

<!-- Let's zoom in a little on what we did. The first step is to tell CMake -->
<!-- to validate your system environment and generate a build script (and a -->
<!-- test script) for the chosen build tool. Cmake will read the file -->
<!-- CmakeLists.txt and validate that your system can do what it asks it to -->
<!-- do. This can yield error messages, for example if the C compiler lacks -->
<!-- certain functions or header files. (When that happens, open an -->
<!-- \href{https://github.com/adesutherland/CREXX/issues}{issue} and someone -->
<!-- will have a look at it - or peruse -->
<!-- \href{https://stackoverflow.com}{stack overflow} which is what we -->
<!-- probably also will do). -->

<!-- After CMake has successfully validated the build environment, it will -->
<!-- generate a build script (a Makefile in the case of Make and a -->
<!-- build.ninja file in the case of Ninja). This is specified after the -G -->
<!-- flag. The -DCMAKE\_BUILD\_TYPE=Release flag makes sure we do an -->
<!-- optimized build, which means we specify an -O3 flag to the C compiler, -->
<!-- which then will spend some time optimizing the executable modules, which -->
<!-- makes them run faster (they do!). The alternative is a `debug' build -->
<!-- which will yield slower executables, but with more debugging information -->
<!-- in them. -->

<!-- The two ampersands (\&\&) mean we do the next part only if the previous -->
<!-- step was successful. This is a `ninja' statement, which will build -->
<!-- everything in the build.ninja specification file. These are a lot of -->
<!-- parts, and the good news is, when they are built once, only the changed -->
<!-- source will be built, which will be fast. -->

After this, the generated test suite is run with the `ctest' command.
This knows what to do, and will show you successes and failures. If what
you checked out if git is not a released version, there is a small
change that some test cases fail, but generally these should indicate
success.

## Use of \crexx{} to build \crexx{}

\crexx{} is used to build the library of built-in functions that written in Rexx (and Rexx
Assembler) and need to be compiled (carefully observing the
dependencies on other Rexx built-in functions) before they are added to the
library and the cRexx executables in their binary form.


## What do we have after a successful build process

### Native executables

When all went well, we have a set of native executables for the platform
we built cRexx on. These are

| Name    | Function                                        |
|---------|-------------------------------------------------|
| crexx   | cRexx compiler driver                           |
| rxc     | cRexx compiler                                  |
| rxas    | cRexx assembler                                 |
| rxdas   | cRexx disassembler                              |
| rxvm    | cRexx VM, threaded interpreter                  |
| rxpp    | cRexx macro preprocessor                        |
| rxbvm   | cRexx VM, non-threaded conventional interpreter |
| rxvme   | cRexx VM, with linked-in Rexx library           |
| rxdb    | cRexx debugger                                  |
| rxcpack | cRexx C-generator for native executables        |

Table: Delivered products. {#tbl:id}

\crexx{} can compile the Rexx script into an
executable file, that can be run standalone, for example, on a computer
that has no \crexx{} and/or C compiler installed.

The \crexx{} debugging tool `rxdb`, is written in Rexx and compiled and packaged into an
executable file.

The executables in the above table need to be on the `PATH` environment
variable. These are to be found in the compiler, assembler,
disassembler, debugger and cpacker directories of the crexx-build
directory we created earlier. It is up to you to add all these
separately to the `PATH` environment, or to just collect them all into one
directory that is already on the `PATH`.

### Production and debug builds

Production builds are optimized, while debug builds are slower in exection time but deliver support for the analysis and debugging of problems in the code. The standard distribution is an optimized production build, while a debug build can be produced with the Debug CMake build option.

| Name    | Function                                        |
|---------|-------------------------------------------------|
| -DCMAKE_BUILD_TYPE=Release  | An Optimized build (default)|
| -DCMAKE_BUILD_TYPE=Debug    | A Debug build              |

Table: Debug vs Release options. {#tbl:id}

### Libraries

### Optional libraries - build options

| Name    | Function                                        |
|---------|-------------------------------------------------|
| -DENABLE_ODBC=ON  | Produce the ODBC Plugin               |
| -DENABLE_GTK=ON  | Produce the GTK (GUI) Plugin           |


Table: Optional plugin build  options. {#tbl:id}

Some libraries, with dependencies on installed software products, are only produced when they are opted-in with CMake build options. The defaults for these options are \code{OFF}.


