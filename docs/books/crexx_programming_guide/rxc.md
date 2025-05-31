# rxc - the \crexx{} Compiler

## Command Line Options

\fontspec{IBM Plex Mono}
\begin{shaded}
  \small
  \obeylines \splice{rxc -h | sed "s/&/\and/g"}
 \end{shaded}
 \fontspec{TeX Gyre Pagella}
 \section{Inline Assembler}
 On page \pageref{inlineAssembly} the inline assembler function of
 the \crexx{} compiler is discussed. This enables the incorporation
 of \code{rxas} assembler instructions into a \textsc{Rexx} source
 file.

## Optimizer

 The compiler can do a number of optimizations that can make the
 execution of a program much faster; the next example shows how an
 operation can be done at compile time, to avoid execution at
 runtime:
 
\lstinputlisting[language=rexx,label=fpow_example]{examples/fpowtest.rexx}
\fontspec{IBM Plex Mono}
\splice{rxc examples/fpowtest}
\splice{rxas examples/fpowtest}
\begin{shaded}
  \small
\obeylines \splice{rxvm examples/fpowtest}
\end{shaded}
\lstinputlisting[language=rxas,label=fpow_example_rxas,caption=optimization]{examples/fpowtest.rxas}
\fontspec{TeX Gyre Pagella}
