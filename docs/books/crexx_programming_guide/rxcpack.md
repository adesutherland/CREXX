# rxcpack - the cRexx C Packer

The C Packer program converts the `.rxbin` files into a C
language structure which links together all needed modules, and a
large part of the Virtual Machine infrastructure, which file then can
be compiled and link edited by the C compiler. `gcc` and `clang` are the
targeted compiler toolchains for Linux, macOS and Windows.

## Command Line Options 

\fontspec{IBM Plex Mono}
\begin{terminaloutput}
\small
\obeylines \splice{rxcpack -h | sed 's/\&/\\\&/g'}
\end{terminaloutput}
\fontspec{TeX Gyre Pagella}

## Source Code

Note that the files that are produced by `rxcpack` are valid C source code but only offer the minimal structures to link dumps of binary objects together. These are already platform dependent and can be combined with the statically linked plugin images for the target platform. The flags used for the compiler and linkage editor can be seen in the verbose output of the `crexx` compiler driver; this syntax is slightly different for `gcc` and `clang`.

## The rxlink step

Though not strictly required, it is advisable to use the the `rxlink` tool in combination with the `rxpack`ed modules. It is automatically called by the `crexx` compiler driver to de-duplicate variable occurrences and select only referenced modules from libraries, shrinking the resulting native executable considerably.


