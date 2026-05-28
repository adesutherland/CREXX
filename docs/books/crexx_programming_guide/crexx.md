# The crexx Tool

The `crexx` tool is the convenience driver for common \crexx{} workflows. It
can compile, assemble, execute, link, and package programs without requiring
the user to call each toolchain binary by hand.

It is also useful in larger builds because it keeps the release defaults in one
place: headerless scripts compile as Level B with `rxfnsb` imported, native
packaging runs through `rxlink` before `rxcpack`, and source/binary import
paths are passed to the compiler phase consistently.

## Use cases

- Executing a simple script with \rexx{} statements and built-in functions, without having to run the tools in the chain individually and having to specify files and options on each tool

- Using plugins and class libraries with minimal overhead in programs

- Combining multiple source files containing functions and classes and executing them as a unit

- Building a larger application using separate compilation and linking

- Developing Class libraries[^functions] together with their consumers

- See which code is produced for the \crexx{} virtual machine and tools using verbosity levels


[^functions]: or function libraries

## Options

Options are used to differentiate between the choices that can be made while building a program. All options have defaults so that they can be left out in standard cases.


## Options description

The following options are available (single and double dashes work for all options):

`-help`
: Display help on the usage of this tool. 

`-version`
: Display the version of this tool. This is the same as the compiler and interpretr version.

`-exec`
: Execute the compiled `.rxbin` under `rxvme` (default).

`-noexec`
: Compile only; do not execute the resulting `.rxbin`.

`-compile`
: Compile all \crexx{} program files on the command line to `.rxbin` files (default).

`-nocompile`
: Skip the `rxc` and `rxas` phases and reuse an existing `<stem>.rxbin`.

`-native`
: Compile to a native executable; default `--nonative`. This produces an executable file for the current operating system and instruction set architecture. The native route now links the compiled program with `rxlink` before `rxcpack` generates C source.

`-nonative`
: Disable native packaging.

`-verbose[0-4]`
: Report on progress; default verbose0, which only issues error messages when the compile fails. Verbose 2 shows the command lines to the toolchain utilities rxc, rxas and rxvme. Verbose 3 includes options and source listings, while verbose 4 includes the contents of the generated assembly code.

`-[no]colo[u]r`
: Enable or disable colourized progress output.

`-[no]optimize`
: Enable or disable optimization.

`-keep`
: Keep compile/link intermediates (default).

`-nokeep`
: Delete compile/link intermediates after the run.

`-decimal`
: Use decimal arithmetic where the driver has to choose arithmetic mode.

`-l[library path]`
: Use a packaged binary/runtime library relative to `CREXX_HOME/bin`. Runtime/native library loading is separate from the compiler's `-s` and `-i` import-discovery paths.

For native packaging, `crexx` now separates `-l` inputs into two groups:

- packaged \crexx{} libraries (`.rxbin` inputs such as `classlib`) are passed into `rxlink`
- plugin/static-native libraries are linked natively when a matching platform static library is available

This keeps the direct interpreter path fast while still producing compact native executables.

`-s[path]` or `--source path`
: Add an extra source import root for the `rxc` phase. This is for off-directory `.rexx` modules that should be visible to source import discovery.

`-i[path]`
: Add an extra raw binary import root for the `rxc` phase. This is for `.rxbin` imports discovered during compilation.

`--import-rxas`
: Allow the `rxc` phase to auto-import `.rxas` files from binary roots. This is off by default.

`--linkmap path`
: When using `-native`, ask `rxlink` to write a link map.

`--link-keep-source`
: When using `-native`, keep source/TRACE debug metadata in the linked intermediate instead of using the default stripped output.

`--link-keep-inline`
: When using `-native`, keep inline-body metadata in the linked intermediate. The native link strips this metadata by default because it is only needed by later compiler imports and debugging/tooling checks.

`-s`, `-i`, and `--import-rxas` are compile-time controls only. They do not automatically add runtime modules to `rxvme` or to native links. For runtime/native library loading, continue to use `-l`.

Headerless top-level scripts are still compiled with `--level levelb --import rxfnsb`.

## Examples

### Just run it

The simplest way to run a \crexx{} program is to just specify its source file as input to the `crexx` program. It will excute the compiler, the assembler and start it with the standard threaded runtime interpreter. All included libraries and plugins are linked automatically.

```rexx <!--crexx-1.crexx-->
say 'hello crexx!'
say 'today''s date is:' date()
```

<!--splice--crexx-1.crexx-->

## Verbosity

With the default verbosity level the tools behaves in the standard unix way where a lack of messages indicates success. This level can be increased gradually to a full explanation of everything that is done. The default is `--verbose0`, which gives no inidcation of what happened unless something went wrong. This is the way to run known-good programs without any overhead.

All verbosity level examples run the following short script:

```rexx <!--hello.crexx-->
options levelb
import rxfnsb
say 'hello rexx!'
say 'today it''s' date('w')
'echo 42'
```

## `--verbose1`

With `--verbose1`, the driver tells in a very condensed way what it did and how it went. When the return codes from the `rxc` and `rxas` are 0, these are displayed with an 'OK' between square brackets.

<!--splice--hello --verbose1 --nocolor -->

It issues some reassuring messages about the compiler and the assembler running successfully and skips the starting of the runtime engine, because the output of the program follows these messages. 

## `--verbose2`

With the `--verbose2` setting, there is more tourist information.

<!--splice--hello --verbose2 --nocolor -->

It starts by identifying the exact release of the crexx version. After this, a number of paths are shown:

| Name | Meaning  |
|------|----------|
| relpath | The path where the \crexx{} system is installed|
| lpath   | The path from which executables and libraries are found |
| s roots | additional sourcefile lookup locations |
| i roots | additional binary (.rxbin) lookup locations |

After this, the *simple script defaults* are mentioned: these are the lines that are inserted for scripts that have no explicit `main` procedure.

With this verbosity level, the exact invocations of the tools in the toolchain are documented, including the complete paths and arguments. Also, the invocation of the runtime engine, with all libraries explicity named - and this includes the libraries that are not used. With native compiles, the `rxlink` tool will extract the used code from these libraries into the executable. It also shows the `-a` flag as a last option, even if this is unused.

## `--verbose3`

In verbosity level 3, the output level is on par with traditional IBM compiler output, with a complete summary of used and unused compiler options.

<!--splice--hello --verbose3 --nocolor -->

## `--verbose4`

Verbosity level 4 is for the moment the most verbose level of tool output. In this level, the complete \rexx{} input source is expanded (when it is produced by the `rxpp` preprocessor), and all generated assembler output is visible and available for inspection and debugging, as well as the set of generated import statements for runtime linking.  It is advisable to page this output through a `less`-like processor.

<!--splice--hello --verbose4 --nocolor -->

## `--verbose[2-4]` with native compilation

When the program is compiled and linked to a native executable, no execution of this program follows. Instead the arguments to the `rxpack` object packager and the linkage editor `rxlink` and their results are included.

Also, the complete command line to the c-compiler and its linkage editor step is documented here, and can be copied into private building tools or scripts. Here the use of static import libraries for the native executables can be seen.


In the following chapters, these tools are documented in detail.

