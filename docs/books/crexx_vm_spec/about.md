# About This Book

The \crexx{} language is a further development, and variant of the
\rexx{} language\footnote{Cowlishaw, 1979}. This book aims to
document the workings of this implementation and serves as reference
for users and implementers alike. It focuses on the instruction set of
the ```rxvm``` *virtual machine* which is the base of the code generated
by the ```rxc``` compiler for the ```rxas``` assembler. The working of the toolchain for producing executable \crexx{} code is described in the *\crexx{} Programming Guide*.

This book documents the RXAS instruction set and \crexx{} level B, which is a
typed subset of Classic \rexx{}.

\crexx{} level B features access to assembler
level from Rexx source, so in addition to documenting the inner workings
of the \crexx{} toolchain, this reference is useful for application
programmers who need this level of access, or anyone who wants to have a deeper understanding of the underlying infrastructure.

<!-- %% \section*{History} -->

<!-- %% \begin{description} -->
<!-- %% \item[mvp] This version documents the Minimally Viable Product -->
<!-- %%   release, Q1 2022 and is intended for developers only. It documents -->
<!-- %%   the RXAS instruction set and \crexx{} level B, which is a typed -->
<!-- %%   subset of Classic Rexx. -->
<!-- %% \item[f0047] First version, Git feature [F0047] -->
<!-- %% \end{description} -->

