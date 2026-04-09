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
: Execute the compiled binaries. --noexec only compiles but does not run the `.rxbin`

`-compile`
: compile all rexx program files on the command line to `.rxbin` files

`-native`
: Compile to a native executable; implies `--noexec`; default `--nonative`. This produces an executable file for the current operating system and instruction set architecture. Static versions of -l included plugins are linked into this load module.

`-verbose[0-4]`
: Report on progress; default verbose0, which only issues error messages when the compile fails. Verbose 2 shows the command lines to the toolchain utilities rxc, rxas and rxvme. Verbose 3 includes options and source listings, while verbose 4 includes the contents of the generated assembly code.

`-colo[u]r`
: Can be color or colour; differentiates messages types in color.

`-keep`
: Keep .rxas source (default nokeep) (This does not work yet).

`-decimal`
: Option decimal (default) links to the decimal arithmetic vm plugin from the mc library. The alternative --nodecimal links in the alternative, high performance db decimal library.

`-l[library path]`
: Use plugin import library relative to the bin library, for example, using the rx_treemap requires a `-lrx_treemap` option

## Examples

