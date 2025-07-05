# Overview of the toolchain

In the binary distribution of \crexx{} for your platform, the main
toolchain and a number of utilities are included; all programs are statically linked into
their own executable, and no shared object libraries or dll's are needed.\newline
\begin{wrapfigure}{l}{0.4\textwidth}
\includegraphics[scale=0.3]{charts/buildflow.pdf}
\end{wrapfigure}
\fussy
Source can be edited with any text editor\footnote{Vi, Emacs, Xedit,
  VS Code, CLion, Eclipse, etc.} of your liking. The sourcefile needs to have an
file extension of \code{.rexx} and contains (Unicode, UTF-8) text. The \code{rxc} \crexx{} compiler
produces a text file which will have a file extension of
\code{.rxas}. The next file in this sequence is produced by the
\code{rxas} assembler and is a binary \code{.rxbin} file. This file is
executable by the \crexx{} \code{rxvme} virtual machine.\newline\newline
\fussy
If you choose to compile a series of \code{*.rxbin} files to a native
executable, the \code{rxcpack} program produces a \code{.c} file,
which can be compiled by any C compiler toolchain.\newline\newline
\fussy
In the following chapters the detailed workflow for the supported platforms is documented.\newline
