# The crexx tool

*note that this is in flux*  

The purpose of the `crexx` tool is to support simple execution of \crexx{} programs, and (maybe in conjuction with other build tools) enable the user to build more complex applications out of many parts. Among those would be native applications which would run without having the \crexx{} toolchain installed on the target system.

## Use cases

- Executing a simple script with \rexx{} statements and built-in functions, without having to run the tools in the chain individually and having to specify files and options on each tool

- Using plugins and class libraries with minimal overhead in programs

- Combining multiple source files containing functions and classes and executing them as a unit

- Building a larger application using separate compilation and linking

- Developing Class libraries[^functions] together with their consumers

- See which code is produced for the \crexx{} virtual machine and tools usign verbosity levels


[^functions]: or function libraries

## Options

Options are used to differentiate between the choices that can be made while building a program. All options have defaults so that they can be left out in standard cases.

## Verbosity

With the default verbosity level the tools behaves in the standard unix way where a lack of messages indicates success. This level can be increased gradually to a full explanation of everything that is done.

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
: Compile all REXX program files on the command line to `.rxbin` files (default).

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
: Option decimal (default) links to the decimal arithmetic vm plugin from the mc library. The alternative --nodecimal links in the alternative, high performance db decimal library.

`-l[library path]`
: Use a packaged binary/runtime library relative to `CREXX_HOME/bin`. This affects both compilation and execution/native linking. For example, using `rx_treemap` requires `-lrx_treemap`.

For native packaging, `crexx` now separates `-l` inputs into two groups:

- packaged REXX libraries (`.rxbin` inputs such as `classlib`) are passed into `rxlink`
- plugin/static-native libraries are linked natively when a matching `_static.a` exists

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
: When using `-native`, keep source/file metadata in the linked intermediate instead of using the default stripped output.

`-s`, `-i`, and `--import-rxas` are compile-time controls only. They do not automatically add runtime modules to `rxvme` or to native links. For runtime/native library loading, continue to use `-l`.

Headerless top-level scripts are still compiled with `--level levelb --import rxfnsb`.

## Examples
