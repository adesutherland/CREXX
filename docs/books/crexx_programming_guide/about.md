# About This Book

This programming guide explains how to use the cRexx Release 1 beta line
toolchain in practice.

It focuses on the tools and workflows around Level B source:

- compiling `.crexx` with `rxc` or `crexx`
- learning practical Level B syntax, modules, classes, interfaces, and tested
  examples through the [Level B tutorial](levelb_tutorial.md)
- assembling `.rxas` with `rxas`
- running `.rxbin` with the crexx virtual machine executables `rxvm` and `rxvme`
- linking modules with `rxlink`
- packaging native executables with `crexx -native` and `rxcpack`
- using standard libraries, classes, interfaces, plugins, compiler exits, and integration APIs

The language itself is defined in the cRexx *Language Reference*. The lower
level bytecode, assembly, and runtime model are described in the cRexx *VM
Specification*.

## Audience

This guide is for developers who want to build and run cRexx programs,
integrate cRexx into tools, or understand how the command-line pieces fit
together. Plugin library and compiler-exit authors should read this guide together
with the VM specification and the relevant public headers.
