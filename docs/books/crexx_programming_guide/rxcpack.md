# rxcpack - the cRexx C Packer

The C Packer program converts the `.rxbin` files into a C
language structure which links together all needed modules, and a
large part of the Virtual Machine infrastructure, which file then can
be compiled and link edited by the C compiler. `gcc` and `clang` are the
targeted compiler toolchains for Linux, macOS and Windows.

## Command Line Options 

\fontspec{IBM Plex Mono}
\begin{shaded}
  \small
  \obeylines \splice{rxcpack -h | sed 's/\&/\\\&/g'}
 \end{shaded}
\fontspec{TeX Gyre Pagella}
